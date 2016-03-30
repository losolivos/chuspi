#ifndef MYSQL_CONNECTION_INFO_H
#define MYSQL_CONNECTION_INFO_H

#include <string>

struct MySQLConnectionInfo
{
    MySQLConnectionInfo() { }
    MySQLConnectionInfo(std::string const &infoString);

    std::string user;
    std::string password;
    std::string database;
    std::string host;
    std::string port_or_socket;
};

#endif // MYSQL_CONNECTION_INFO_H
