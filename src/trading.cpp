#include "./trading.hpp"


string Url::getUrl(string code) {
    string interval = "1d"; 
    string url = QUERY + code + '?' + "period1=" + to_string(this->getEpoch(this->start))
	+ '&' + "period2=" + to_string(this->getEpoch(this->end)) + '&'
	+ "interval=" + interval + "&events=history&includeAdjustedClose=true";
    return url;
}


unordered_map<string, int> Parser::parse() {
    unordered_map<string, int> ret = {};

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
	    ret.insert(make_pair("12", 0));
	    bflag = false;
	    continue;
	}

	else if (this->get_month(line) == 2 && !bflag) {
	    ret.insert(make_pair("2", 0));
	    continue;
	}

	else if (this->get_month(line) == 3 && sflag) {
	    ret.insert(make_pair("3", 0));
	    continue;
	}

	else if (this->get_month(line) == 6 && !sflag) {
	    ret.insert(make_pair("6", 0));
	    continue;
	}

	// 2. insert them into ret umap
	string date = this->get_date(line);
	double avgvalue = this->ohlcavg(line);
	// cout << "date: " << date << ", " " avg value: " << avgvalue << endl; // debug only

	ret[date] = avgvalue;
    }
    return ret;
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
    unordered_map<string, int> datevalue = ParserObj->parse();



}
