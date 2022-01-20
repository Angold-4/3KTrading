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
    long getEpoch(string date);

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
class Parser {
public:
    Parser(string data, vector<int> buymonth, vector<int> sellmonth) : csvdata(data) {

    }


private:
    string csvdata;
    unordered_map<string, vector<int>> dateprices; // each date with its prices

    /*
     * Parser::ohlc
     *
     * get ohlc price from dayprice line
     */
    vector<int> ohlc(string dayprice);
};

// Trading is the class to simulate all kinds of trades
class Trading {

};
