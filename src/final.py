from AlgoAPI import AlgoAPIUtil, AlgoAPI_Backtest
from datetime import datetime, timedelta
import numpy as np
from talib import EMA

MAGICMA = 20131111
sinterval = 8;
linterval = 22;
Lots = 0.1
MaximumRisk = 0.02
DecreaseFactor = 3
MovingPeriod = 12
MovingShift = 0


class AlgoEvent:
    def __init__(self):
        self.lasttradetime = datetime(2000,1,1)
        self.pos, self.osOrder, self.pendOrder = {}, {}, {}

    def start(self, mEvt):
        self.evt = AlgoAPI_Backtest.AlgoEvtHandler(self, mEvt)
        self.evt.start()
        
    def on_marketdatafeed(self, md, ab):
        if md.timestamp>self.lasttradetime+timedelta(hours=1):
            self.lasttradetime = md.timestamp
        
            # get latest closing price
            fullbar = self.evt.getHistoricalBar(contract={"instrument":md.instrument}, numOfBar=MovingPeriod+MovingShift, interval="D")
            self.obs_open = [fullbar[t]["o"] for t in fullbar]
            self.obs_close = [fullbar[t]["c"] for t in fullbar]

            if self.CalculateCurrentOrders(md.instrument)==0:
                self.CheckForOpen(md, ab)
            else:
                self.CheckForClose(md, ab)

    def CheckForOpen(self,md,ab):
        mas = EMA(np.array(self.obs_close), sinterval)[-1-MovingShift]
        mal = EMA(np.array(self.obs_close), linterval)[-1-MovingShift]

        # sell conditions
        if self.obs_open[-1]>ma and self.obs_close[-1]<ma:
            order = AlgoAPIUtil.OrderObject(
                instrument = md.instrument,
                orderRef = MAGICMA,
                openclose = 'open', 
                buysell = -1,        #1=buy, -1=sell
                ordertype = 0,      #0=market, 1=limit
                volume = self.LotsOptimized(md,ab)
            )
            self.evt.sendOrder(order)
        # buy condition
        if self.obs_open[-1]<ma and self.obs_close[-1]>ma:
            order = AlgoAPIUtil.OrderObject(
                instrument = md.instrument,
                orderRef = MAGICMA,
                openclose = 'open', 
                buysell = 1,        #1=buy, -1=sell
                ordertype = 0,      #0=market, 1=limit
                volume = self.LotsOptimized(md,ab)
            )
            self.evt.sendOrder(order)

    def CheckForClose(self, md, ab):
        ma = EMA(np.array(self.obs_close), MovingPeriod)[-1-MovingShift]
        for _id in list(self.osOrder):
            if self.osOrder[_id]["buysell"]==1:
                if self.obs_open[-1]>ma and self.obs_close[-1]<ma:
                    order = AlgoAPIUtil.OrderObject(
                        tradeID = _id,
                        openclose = 'close'
                    )
                    self.evt.sendOrder(order)
            elif self.osOrder[_id]["buysell"]==-1:
                if self.obs_open[-1]<ma and self.obs_close[-1]>ma:
                    order = AlgoAPIUtil.OrderObject(
                        tradeID = _id,
                        openclose = 'close'
                    )
                    self.evt.sendOrder(order)

    def CalculateCurrentOrders(self, symbol):
        self.pos, self.osOrder, self.pendOrder = self.evt.getSystemOrders()
        if symbol in self.pos:
            return self.pos[symbol]["netVolume"]
        return 0

    def LotsOptimized(self,md,ab):
        losses = 0
        # select lot size
        lot = round(ab["availableBalance"]*MaximumRisk/1000.0,1)
        # calcuulate number of losses orders without a break
        if DecreaseFactor>0:
            for _id in self.osOrder:
                if self.osOrder[_id]["buysell"]==1: #buy
                    if md.bidPrice < self.osOrder[_id]["openprice"]:
                        losses+=1
                elif self.osOrder[_id]["buysell"]==-1: #sell
                    if md.askPrice > self.osOrder[_id]["openprice"]:
                        losses+=1
            if losses>1:
                lot = round(lot-lot*losses/DecreaseFactor,1)
        # return lot size
        if lot<Lots: lot = Lots
        return lot

    def on_bulkdatafeed(self, isSync, bd, ab):
        pass

    def on_newsdatafeed(self, nd):
        pass

    def on_weatherdatafeed(self, wd):
        pass
    
    def on_econsdatafeed(self, ed):
        pass
    
    def on_orderfeed(self, of):
        pass

    def on_dailyPLfeed(self, pl):
        pass

    def on_openPositionfeed(self, op, oo, uo):
        pass
