#ifndef MYSQLDATATYPES_H
#define MYSQLDATATYPES_H

#include "MySQLFwd.h"

//- Union that holds element data
union SQLElementUnion
{
    PreparedStatement *stmtData;
    char *query;
};

//- Type specifier of our element data
enum SQLElementDataType
{
    SQL_ELEMENT_RAW,
    SQL_ELEMENT_PREPARED
};

//- The element
struct SQLElementData
{
    SQLElementUnion element;
    SQLElementDataType type;
};

//- For ambigious resultsets
union SQLResultSetUnion
{
    PreparedResultSet *presult;
    ResultSet *qresult;
};

#endif // MYSQLDATATYPES_H
