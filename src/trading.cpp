#include "./trading.hpp"


string Url::getUrl(string code) {
    string interval = "1d"; 
    string url = QUERY + code + '?' + "period1=" + to_string(this->getEpoch(this->start))
	+ '&' + "period2=" + to_string(this->getEpoch(this->end)) + '&'
	+ "interval=" + interval + "&events=history&includeAdjustedClose=true";
    return url;
}


vector<pair<string, double>> Parser::parse() {
    vector<pair<string, double>> ret = {};
    // unordered_map<string, int> ret = {};

    string line;
    bool bflag = true; // flag for buy in 12 1
    bool sflag = true; // flag for sell in 3 4 5
    istringstream iss(this->csvdata);
    getline(iss, line); // do not need first line

    // then iterate each line of this data
    while (getline(iss, line)) {

	// 1. deal with buy/sell indicator
	if (line.substr(11, 4) == "null") continue; // null date

	if (this->get_month(line) == 12 && bflag) {
	    ret.push_back(make_pair("BS", 12));
	    bflag = false;
	    continue;
	}

	else if (this->get_month(line) == 2 && !bflag) {
	    ret.push_back(make_pair("BS", 2));
	    bflag = true;
	    continue;
	}

	else if (this->get_month(line) == 3 && sflag) {
	    ret.push_back(make_pair("BS", 3));
	    sflag = false;
	    continue;
	}

	else if (this->get_month(line) == 6 && !sflag) {
	    ret.push_back(make_pair("BS", 6));
	    sflag = true;
	    continue;
	}

	// 2. insert them into ret umap
	string date = this->get_date(line);
	double avgvalue = this->ohlcavg(line);
	// cout << "date: " << date << ", " " avg value: " << avgvalue << endl; // debug only
	ret.push_back(make_pair(date, avgvalue));
    }
    return ret;
}

// Main sma algorithm
double Trading::sma(int index, int sinterval, int linterval) {
    vector<pair<string, double>> smas = this->caldoublesma(sinterval); // small sma
    vector<pair<string, double>> smal = this->caldoublesma(linterval); // large sma

    if (smas.size() != smal.size()) cout << "Invalid sma calculation, please check your interval" << endl; // detect error

    if (smas[0].first != "BS" || smal[0].first != "BS") cout << "Invalid B/S" << endl;
    int validbuydays = 1;
    for (int i = 1; i < smas.size(); i++) {
	// get number of days buy and sell
	if (smas[i].first == "BS") break;
	validbuydays++;
    }

    double diff = INT_MAX; // sma diff
    string diffday = "";
    for (int i = 1; i < validbuydays; i++) {
	string sday = smas[i].first;
	string lday = smal[i].first;
	double sval = smas[i].second;
	double lval = smal[i].second;
	// cout << sday << "  " << lday << " value: " << this->daymap[sday] << endl;
	// cout << sval << "  " << lval << endl;

	// absolute value (smallest)
	
	double smadiff = (sval - lval) > 0 ? sval - lval : lval - sval;
	if (smadiff < diff) {
	    diffday = sday;
	    diff = smadiff;
	}
    }

    this->totaldiff += this->maxprice - diff;
    return this->totaldiff;

    cout << index << ": diff " << diff << " date: " << diffday << " value: " << this->daymap[diffday] << " maxvalue: " << this->maxprice 
	<< " maxdate: " << this->maxdate << endl;
}


// Entry
int main() {
    string pattenstart = "01/10/20";
    string pattenend = "01/06/20";

    double mindiff = INT_MAX;
    string mins = "invalid";
    string minl = "invalid";
    for (int s = 5; s < 11; s++) {
	// for (int l = 30; l <= 40; l++) {
	    // for each s, l
	    double totaldiff = 0; // fixed s, l. totaldiff
	    for (int i = 0; i < 22; i++) {
		// for each year
		string ftail = "";
		string etail = "";
		if (i < 10) {
		    ftail += '0';
		    ftail += to_string(i);
		} else {
		    ftail = to_string(i);
		}

		if (i < 9) {
		    etail += '0';
		    etail += to_string(i+1);
		} else {
		    etail += to_string(i+1);
		}
		string start = pattenstart + ftail;
		string end   = pattenend + etail;
		cout << start << " " << end << endl;
		Url* UrlObj = new Url(start, end);
		string url = UrlObj->getUrl("JPY=X");

		CurlObj* CObj = new CurlObj(url);
		string csvdata = CObj->getData();

		Parser* ParserObj = new Parser(csvdata);
		vector<pair<string, double>> datevalue = ParserObj->parse();

		Trading* TradeObj = new Trading(datevalue);
		totaldiff += TradeObj->sma(i, s, 20);
	    }
	    cout << totaldiff << " " << s << " " << 20 << endl;
	    // finish this 22 years
	    if (totaldiff < mindiff) {
		mindiff = totaldiff;
		mins = s;
		minl = 20;
	    }
	// }
    }
    cout << "When sinterval = " << mins << ", linterval = " << minl << ", we can find the most accurate value: " << mindiff << endl;
}
