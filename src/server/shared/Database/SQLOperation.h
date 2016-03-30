#ifndef SQLOPERATION_H
#define SQLOPERATION_H

#include "MySQLFwd.h"

class SQLOperation
{
public:
    void execute(MySQLConnection *con)
    {
        executeImpl(con);
    }

    virtual ~SQLOperation() { }

private:
    virtual void executeImpl(MySQLConnection *) = 0;
};

#endif // SQLOPERATION_H
