#ifndef PREPAREDRESULTSET_H
#define PREPAREDRESULTSET_H

#include "Define.h"
#include "MySQLFwd.h"

#include <vector>

class PreparedResultSet
{
    typedef std::vector<Field *> StorageType;

public:
    PreparedResultSet(MYSQL_STMT* stmt);
    ~PreparedResultSet();

    bool NextRow();

    std::size_t GetRowCount() const { return m_rows.size(); }
    std::size_t GetFieldCount() const { return m_fieldCount; }

    Field * Fetch() const
    {
        return m_rows[m_rowPosition];
    }

    Field const & operator [] (std::size_t index) const;

private:
    std::size_t m_rowPosition;
    StorageType m_rows;
    std::size_t m_fieldCount;
};

#endif // PREPAREDRESULTSET_H
