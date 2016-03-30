#ifndef MYSQLPTRTYPESFWD_H
#define MYSQLPTRTYPESFWD_H

#include "MySQLFwd.h"

#include <memory>

typedef std::shared_ptr<ResultSet> QueryResult;
typedef std::shared_ptr<PreparedResultSet> PreparedQueryResult;
typedef std::shared_ptr<Transaction> SQLTransaction;

#endif // MYSQLPTRTYPESFWD_H
