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
void Trading::sma(int index) {
    int sinterval = 5;
    int linterval = 40;
    
    vector<pair<string, double>> smas = this->calcsma(sinterval); // small sma
    vector<pair<string, double>> smal = this->calcsma(linterval); // large sma

    if (smas.size() != smal.size()) cout << "Invalid, Comparison" << endl;
}


// Entry
int main() {
    string start = "01/10/1997";
    string end   = "01/06/1998";
    Url* UrlObj = new Url(start, end);
    string url = UrlObj->getUrl("JPY=X");

    CurlObj* CObj = new CurlObj(url);
    string csvdata = CObj->getData();

    Parser* ParserObj = new Parser(csvdata);
    vector<pair<string, double>> datevalue = ParserObj->parse();

    Trading* TradeObj = new Trading(datevalue);
    TradeObj->sma(1);
}
