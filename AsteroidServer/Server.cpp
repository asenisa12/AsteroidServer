#include "Server.h"

Document parse_jsonFile(std::string path)
{
	std::fstream f(path);
	Document data;
	if (f.is_open())
	{
		std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		data.Parse(content.c_str());
		f.close();
	}
	else
	{
		printf("Error: Can't open file %s\n", path.c_str());
	}

	return data;
}

string to_utf8string(utility::string_t str){
	return utility::conversions::to_utf8string(str);
}

AsteroidServer::AsteroidServer()
	: configuration_(parse_jsonFile("Config"))
{
	if (init_database())
	{
		serverAddress_ = configuration_["serverAddress"].GetString();
		srand(time(NULL));
		create_listeners();
		create_PlayerListener();
		create_ScoreListener();
		create_dataListener();
	}
}

const vector<string> AsteroidServer::clans = {"clan1", "clan2", "clan3", "clan4"};
const vector<string> AsteroidServer::addr_path = { "/add_player", "/add_score", "/get_scores"};

string AsteroidServer::choose_clan()
{
	return clans[rand() % clans.size()];
}

void AsteroidServer::exec_query(const char* query)
{
	if (mysql_query(connect, query))
		cout << mysql_error(connect) << endl << endl;
}

void AsteroidServer::create_listeners()
{
	for (int i = 0; i < LISTENERS_COUNT; i++)
	{
		string addr = serverAddress_ + addr_path[i];

		web::uri listener_addr(
			utility::conversions::to_string_t(addr));

		listeners_.push_back(new http_listener(listener_addr));
	}
}


void AsteroidServer::create_PlayerListener()
{
	listeners_[PLAYER]->support(methods::POST, [this](http_request request)
		{
			json::value data = request.extract_json().get();
			std::string playerName = to_utf8string((data[U("name")].as_string()));

			utility::string_t body_data;
			utility::string_t content_type;
			content_type = U("text/plain");

			if (!name_exist(playerName))
			{
				string query = "INSERT INTO Player ( NAME, CLAN)"
					"VALUES ('" + playerName + "', '" + choose_clan() + "')";

				exec_query(query.c_str());
				body_data = U("Player is created");
			}
			else
			{
				body_data = U("This name is taken!");
			}

			request.reply(status_codes::OK, body_data, content_type);
		});
}

void AsteroidServer::create_ScoreListener()
{
	listeners_[ADD_SCORE]->support(methods::POST, [this](http_request request)
	{
		json::value data = request.extract_json().get();
		std::string playerName = to_utf8string((data[U("name")].as_string()));

		string query = "SELECT ID FROM Player "
			"WHERE NAME = '" + playerName + "'" ;


		MYSQL_RES *result = mysql_store_result(connect);
		MYSQL_ROW row;

		if (mysql_query(connect, query.c_str()))
		{
			cout << mysql_error(connect) << endl << endl;
		}
		else
		{
			
			result = mysql_store_result(connect);
			int num_fields = mysql_num_fields(result);

			while ((row = mysql_fetch_row(result)))
			{
				string score = to_utf8string((data[U("score")].as_string()));
				string id = row[0];
				string query = "INSERT INTO Scores ( PLAYER_ID, SCORE, DATE)"
					"VALUES ('" + id + "', '" + score + "', CURDATE())";

				exec_query(query.c_str());
			}
		}

		mysql_free_result(result);
	});
}

void AsteroidServer::create_dataListener()
{

}

void AsteroidServer::create_tables()
{
	char *query[] = {
		//PLayer table
		"CREATE TABLE IF NOT EXISTS Player ("
		"id INT NOT NULL AUTO_INCREMENT,"
		"PRIMARY KEY(id),"
		"NAME VARCHAR(30),"
		"CLAN VARCHAR(30))",
		//Score table
		"CREATE TABLE IF NOT EXISTS Scores ("
		"id INT NOT NULL AUTO_INCREMENT,"
		"PRIMARY KEY(id),"
		"PLAYER_ID INT,"
		"SCORE INT,"
		"DATE    DATE NOT NULL)"
	};

	for (int i = 0; i < 2; i++)
		exec_query(query[i]);
}

bool AsteroidServer::init_database()
{
	connect = mysql_init(NULL);
	if (!connect) 
	{
		fprintf(stderr, "MySQL Initialization Failed");
		return false;
	}

	if (!configuration_.IsObject())
	{
		printf("NOT valid config file!\n");
		return false;
	}

	Value& dbData = configuration_["database"];

	const char* address = dbData["address"].GetString();
	const char* user = dbData["user"].GetString();
	const char* password = dbData["password"].GetString();
	const char* dbName = dbData["dbName"].GetString();
	int port = dbData["port"].GetInt();

	connect = mysql_real_connect(connect, address, user, password, dbName, port, NULL, 0);

	if (connect){
		printf("Connection to database Succeeded\n");
	}
	else{
		printf("Connection database Failed!\n");
		return false;
	}

	create_tables();

	return true;
}

bool AsteroidServer::name_exist(string name)
{
	string query = "SELECT ID FROM Player "
		"WHERE NAME = '" + name + "'";

	MYSQL_RES *result = mysql_store_result(connect);
	bool exist = false;

	if (mysql_query(connect, query.c_str()))
	{
		cout << mysql_error(connect) << endl << endl;
	}
	else
	{
		result = mysql_store_result(connect);
		if (mysql_fetch_row(result) != NULL) exist = true;
	}
	mysql_free_result(result);

	return exist;
}



void AsteroidServer::execute()
{
	try
	{
		count = 0;
		for (auto listener : listeners_)
		{
			listener->open()
				.then([&listener, this](){cout << "\n>>starting to listen on: " <<
				serverAddress_ << addr_path[count] << endl; })
				.wait();
			count++;
		}
			while (true);
	}
	catch (exception const & e)
	{
		wcout << e.what() << endl;
	}
}

AsteroidServer::~AsteroidServer()
{
	for (auto listener : listeners_)listener->close();
	mysql_close(connect);
}