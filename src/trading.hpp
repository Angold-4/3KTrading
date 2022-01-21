#pragma once

#include <curl/curl.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <map>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#define QUERY "https://query1.finance.yahoo.com/v7/finance/download/" // searching query

using namespace::std; // will just using std lib


// Url is the class that concadinate url
class Url {
public:
    Url (string s, string e) : start(s), end(e) {};

    /**
     * Url::getUrl
     *
     * concadinate url
     */
    string getUrl(string code);
private:
    string start;
    string end;

    /**
     * Url::getEpoch
     *
     * translate DD/MM/YYYY into epoch (for yahoo finance)
     */
    long getEpoch(string date) {
	// parse the DD/MM/YYYY into epoch
	long ret;
	struct tm mytm;
	date += " 00:00:00";
	strptime(date.c_str(), "%d/%m/%Y %H:%M:%S", &mytm);

	ret = mktime(&mytm);

	return ret + 28800;
    }
};


// CurlObj is the class to handle url
class CurlObj {
public:
    CurlObj (std::string url) {
        curl = curl_easy_init();
	if (!curl) throw std::string("Curl did not initialize.");
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriter);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlBuffer);
	curl_easy_perform(curl);
    }

    static int curlWriter(char* data, int size, int nmemb, std::string* buffer) {
	// write to the buffer
	int result = 0;
	if (buffer != NULL) {
	    buffer->append(data, size * nmemb);
	    result = size * nmemb;
	}
	return result;
    }

    std::string getData() {
	return curlBuffer;
    }
private:
    CURL* curl;
    std::string curlBuffer; // write to curlBuffer
};


// Parser is the class that parse the csv data
// parse csv data from Oct (prev year) to May (next year)
class Parser {
public:
    Parser(string data) : csvdata(data) {}

    /*
     * Parser::parse
     *
     * for each day, fill the string and avg price (sum/4) 
     * and then return them
     */
    vector<pair<string, double>> parse();
    

private:
    string csvdata;
    int begm;
    int endm;
    int mbegin; // each month (divide them)
    inline int get_month(string line) { return stoi(line.substr(5, 2)); }
    inline string get_date(string line) { return line.substr(0, 10); }

    /*
     * Parser::ohlc
     *
     * get ohlc avg price from dayprice line
     */
    double ohlcavg(string dayprice) {
	int firstq = dayprice.find(',') + 1;
	string tmp = "";
	int count = 0;
	double sum = 0;
	for (int i = firstq; i < dayprice.size(); i++) {
	    if (count == 4) {
		// Termination Condition
		break;
	    }

	    if (dayprice[i] == ',') {
		sum += stof(tmp);
		tmp = "";
		count++;
		continue;
	    }
	    tmp += dayprice[i];
	}
	return sum / 4;
    }
};

// Trading is the class to simulate all kinds of trades
class Trading {
public:
    Trading (vector<pair<string, double>> datevalue) : yearval(datevalue) {
	this->daymap = {};
	this->maxprice = 0;
	this->maxdate = "";
	for (auto p : this->yearval) this->daymap.insert(p);
	this->totaldiff = 0;
    }
    
    // #1 SMA trade simulation
    double sma(int index, int sinterval, int linterval);



private:
    vector<pair<string, double>>  yearval;
    unordered_map<string, double> daymap;
    double maxprice;
    string maxdate;

    double totaldiff; // total diff value of this year
    /*
     * Trading::calcsma
     *
     * calculate sma with this interval
     */

public:
    vector<pair<string, double>> caldoublesma(int interval) {
	vector<pair<string, double>> ret = {};
	bool start = false;
	double suma = 0; // sum of prev interval days
	double sma = 0;

	// interval
	for (int i = 0; i < interval; i++) {
	    suma += this->yearval[i].second;
	}
	sma = suma / interval; // currenct interval
	
	for (vector<pair<string, double>>::iterator it = this->yearval.begin()+interval+1; it < this->yearval.end(); it++) {
	    pair<string, double> dayval = *it;
	    if (it->first == "BS") {
		start = true;
		ret.push_back(*it);
		continue;
	    }
	    if ((it - interval)->first == "BS")  {
		suma -= (it - interval - 1)->second;
	    } else { suma -= (it - interval)->second; }
	    suma += it->second;
	    if (start) {
		if (it->second > this->maxprice) {
		    // get maximum value
		    this->maxprice = it->second;
		    this->maxdate = it->first;
		}
		sma = suma / interval;
		ret.push_back(make_pair(it->first, sma));
	    }
	}
	return ret;
    }

};
