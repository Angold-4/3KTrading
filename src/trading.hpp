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
    unordered_map<string, int> parse();
    

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
    Trading (unordered_map<string, double> datevalue) : yearval(datevalue) {}
    
    // #1 SMA trade simulation
    void sma (int index);


private:
    unordered_map<string, double> yearval;

    /*
     * Trading::calcsma
     *
     * calculate sma with this interval
     */
    unordered_map<string, double> calcsma(int interval);

};
