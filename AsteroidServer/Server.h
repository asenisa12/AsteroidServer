#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <string>
#include <vector>
#include <mysql.h>
#include "reader.h"
#include "document.h"
#include "writer.h"
#include "stringbuffer.h"
#include <stdlib.h>
#include <time.h> 

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;
using namespace rapidjson;

Document parse_jsonFile(std::string path);
string to_utf8string(utility::string_t str);



class AsteroidServer
{
	enum {PLAYER = 0 , ADD_SCORE = 1, GET_SCORES = 2};
	int count;

	static const int LISTENERS_COUNT = 3;
	static const vector<string> clans;
	static const vector<string> addr_path;
	vector<http_listener*> listeners_;

	string serverAddress_, databaseAddress_;
	MYSQL *connect;
	Document configuration_;
	
	bool name_exist(string name);
	void exec_query(const char* query);
	bool init_database();
	void create_tables();
	void create_listeners();
	void create_PlayerListener();
	void create_ScoreListener();
	void create_dataListener();
	string choose_clan();
public:
	AsteroidServer();
	void execute();
	~AsteroidServer();
};