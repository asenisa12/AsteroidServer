
#include <cpprest/http_listener.h>
////#include <mysql_driver.h>
#include <mysql.h>
//#include <cppconn>
//#include <cppconn/exception.h>
//#include <cppconn/resultset.h>
//#include <cppconn/statement.h>
//#include <cppconn/prepared_statement.h>
#include "Server.h"


int main()
{

	AsteroidServer server;
	server.execute();
	
	system("pause");
	return 0;
}