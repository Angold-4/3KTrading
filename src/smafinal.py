from AlgoAPI import AlgoAPIUtil, AlgoAPI_Backtest
from datetime import datetime, timedelta
import talib, numpy

class AlgoEvent:
    def __init__(self):
        self.lasttradetime = datetime(2000,1,1)

        self.arr_close = numpy.array([])

        self.arr_fastbuyMA = numpy.array([])    # buy in the 12 and 1
        self.arr_slowbuyMA = numpy.array([])  
        self.arr_fastsellMA = numpy.array([])   # sell between 3 to 6
        self.arr_slowsellMA = numpy.array([])

        self.fastbuyperiod = 8
        self.slowbuyperiod = 22

        self.fastsellperiod = 7;
        self.slowsellperiod = 14;

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
            self.evt.consoleLog("month = ", self.currentmonth == 12)

            # self.evt.consoleLog("fastbuyMA=", self.arr_fastMA)
            lastprice = bd[self.myinstrument]['lastPrice']

            # retrieve recent observations
            res = self.evt.getHistoricalBar({"instrument": self.myinstrument}, self.fastbuyperiod+self.slowbuyperiod, 'D')

            self.arr_close = numpy.array([res[t]['c'] for t in res])

            self.arr_fastbuyMA = talib.SMA(self.arr_close, timeperiod=int(self.fastbuyperiod))
            self.arr_slowbuyMA = talib.SMA(self.arr_close, timeperiod=int(self.slowbuyperiod))

            self.arr_fastsellMA = talib.SMA(self.arr_close, timeperiod=int(self.fastsellperiod))
            self.arr_slowsellMA = talib.SMA(self.arr_close, timeperiod=int(self.slowsellperiod))

            # debug print result
            # self.evt.consoleLog("fastbuyMA=", self.arr_fastMA)
            # self.evt.consoleLog("slowsellMA=", self.arr_slowMA)
            

            # only valid
            if not numpy.isnan(self.arr_fastbuyMA[-1]) and not numpy.isnan(self.arr_fastbuyMA[-2]) and not numpy.isnan(self.arr_slowbuyMA[-1]) and not numpy.isnan(self.arr_slowbuyMA[-2]) :
                self.evt.consoleLog("A Valid!", self.lasttradetime)
                # send a buy order for Golden Cross  (Only Jan or Nov)
                if self.arr_fastbuyMA[-1] > self.arr_slowbuyMA[-1] and self.arr_fastbuyMA[-2] < self.arr_slowbuyMA[-2] and (self.currentmonth == 1 or self.currentmonth == 12):
                    self.evt.consoleLog("Buy: at ", self.lasttradetime)
                    self.test_sendOrder(lastprice, 1, 'open')

                # send a sell order for Death Cross (Can choose date if it is valid)
                if self.arr_fastsellMA[-1] < self.arr_slowsellMA[-1] and self.arr_fastsellMA[-2] > self.arr_slowsellMA[-2]:
                    self.evt.consoleLog("Sell: at ", self.lasttradetime)
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
        # Buy or sell
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
