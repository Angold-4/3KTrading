from AlgoAPI import AlgoAPIUtil, AlgoAPI_Backtest
from datetime import datetime, timedelta
import talib, numpy

class AlgoEvent:
    def __init__(self):
        self.lasttradetime = datetime(2000,1,1)
        self.arr_close = numpy.array([])
        self.arr_fastMA = numpy.array([])
        self.arr_slowMA = numpy.array([])
        self.fastbuyperiod = 8
        self.slowbuyperiod = 22
        self.currentmonth = 0;

    def start(self, mEvt):
        self.myinstrument = mEvt['subscribeList'][0]
        self.evt = AlgoAPI_Backtest.AlgoEvtHandler(self, mEvt)
        self.evt.start()

    def on_bulkdatafeed(self, isSync, bd, ab):
        if bd[self.myinstrument]['timestamp'] >= self.lasttradetime + timedelta(hours=24):
            self.lasttradetime = bd[self.myinstrument]['timestamp']

            # debug only


            self.currentmonth = int(self.lasttradetime.strftime("%m"));
            self.evt.consoleLog("month = " currentmonth)

            # self.evt.consoleLog("fastbuyMA=", self.arr_fastMA)
            lastprice = bd[self.myinstrument]['lastPrice']
            # retrieve recent observations
            res = self.evt.getHistoricalBar({"instrument": self.myinstrument}, self.fastbuyperiod+self.slowbuyperiod, 'D')
            self.arr_close = numpy.array([res[t]['c'] for t in res])
            # fit SMA line
            self.arr_fastMA = talib.SMA(self.arr_close, timeperiod=int(self.fastbuyperiod))
            self.arr_slowMA = talib.SMA(self.arr_close, timeperiod=int(self.slowbuyperiod))
            # debug print result
            # self.evt.consoleLog("fastbuyMA=", self.arr_fastMA)
            # self.evt.consoleLog("slowsellMA=", self.arr_slowMA)
            

            # only valid

            if not numpy.isnan(self.arr_fastMA[-1]) and not numpy.isnan(self.arr_fastMA[-2]) 
            and not numpy.isnan(self.arr_slowMA[-1]) and not numpy.isnan(self.arr_slowMA[-2]):

                # send a buy order for Golden Cross  (Only Jan or Nov)
                if self.arr_fastMA[-1] > self.arr_slowMA[-1] and self.arr_fastMA[-2] < self.arr_slowMA[-2]
                and (self.currentmonth == 1 or self.currentmonth == 12):
                    self.test_sendOrder(lastprice, 1, 'open')

                # send a sell order for Death Cross
                if self.arr_fastMA[-1] < self.arr_slowMA[-1] and self.arr_fastMA[-2] > self.arr_slowMA[-2]:
                    self.test_sendOrder(lastprice, -1, 'open')

    def on_marketdatafeed(self, md, ab):
        pass

    def on_orderfeed(self, of):
        pass

    def on_dailyPLfeed(self, pl):
        pass

    def on_openPositionfeed(self, op, oo, uo):
        pass

    def test_sendOrder(self, lastprice, buysell, openclose):
        order = AlgoAPIUtil.OrderObject()
        order.instrument = self.myinstrument
        order.orderRef = 1
        if buysell==1:
            order.takeProfitLevel = lastprice*1.1
            order.stopLossLevel = lastprice*0.9
        elif buysell==-1:
            order.takeProfitLevel = lastprice*0.9
            order.stopLossLevel = lastprice*1.1
        order.volume = 0.01
        order.openclose = openclose
        order.buysell = buysell
        order.ordertype = 0  # 0=market_order, 1=limit_order
        self.evt.sendOrder(order)
