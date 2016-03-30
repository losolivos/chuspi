#ifndef MYSQLFWD_H
#define MYSQLFWD_H

class PreparedStatement;
class MySQLPreparedStatement;

struct MySQLConnectionInfo;

class MySQLConnection;
typedef void (* MySQLConnectionInitHook)(MySQLConnection &);

class ResultSet;
class PreparedResultSet;

class SQLOperation;

class Transaction;

class SQLQueryHolder;

class Field;

class DatabaseWorker;

struct st_mysql_res;
typedef st_mysql_res MYSQL_RES;

struct st_mysql;
typedef st_mysql MYSQL;

struct st_mysql_field;
typedef st_mysql_field MYSQL_FIELD;

struct st_mysql_stmt;
typedef st_mysql_stmt MYSQL_STMT;

struct st_mysql_bind;
typedef st_mysql_bind MYSQL_BIND;

typedef char **MYSQL_ROW;

#endif // MYSQLFWD_H
