#include "ResultSet.h"
#include "Field.h"
#include <ace/Assert.h>

#ifdef _WIN32
  #include <winsock2.h>
#endif
#include <mysql.h>

ResultSet::ResultSet(MYSQL_RES *result)
    : m_result(result)
    , m_fields(mysql_fetch_fields(m_result))
    , m_rowCount(mysql_num_rows(m_result))
    , m_fieldCount(mysql_num_fields(m_result))
    , m_currentRow(new Field[m_fieldCount])
{
    ACE_ASSERT(m_currentRow);
}

ResultSet::~ResultSet()
{
    delete [] m_currentRow;
    mysql_free_result(m_result);
}

bool ResultSet::NextRow()
{
    if (!m_result)
        return false;

    MYSQL_ROW row = mysql_fetch_row(m_result);
    if (!row)
        return false;

    unsigned long const * const lengths = mysql_fetch_lengths(m_result);

    for (std::size_t i = 0; i < m_fieldCount; ++i)
        m_currentRow[i].SetStructuredValue(row[i], lengths[i], m_fields[i].type);

    return true;
}

Field const & ResultSet::operator [] (std::size_t index) const
{
    return m_currentRow[index];
}
