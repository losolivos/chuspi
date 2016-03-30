#ifndef RESULTSET_H
#define RESULTSET_H

#include "MySQLFwd.h"

#include <cstddef>

class ResultSet
{
public:
    ResultSet(MYSQL_RES *result);
    ~ResultSet();

    bool NextRow();

    std::size_t GetRowCount() const { return m_rowCount; }
    std::size_t GetFieldCount() const { return m_fieldCount; }

    Field * Fetch() const { return m_currentRow; }
    Field const & operator [] (std::size_t index) const;

private:
    MYSQL_RES *m_result;
    MYSQL_FIELD *m_fields;

    std::size_t m_rowCount;
    std::size_t m_fieldCount;

    Field *m_currentRow;
};

#endif // RESULTSET_H
