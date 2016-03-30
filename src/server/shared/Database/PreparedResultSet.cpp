#include "PreparedResultSet.h"
#include "Field.h"
#include "Log.h"

#include <ace/Assert.h>
#ifdef _WIN32
#  include <winsock2.h>
#endif
#include <mysql.h>

namespace {

typedef std::vector<MYSQL_BIND> BindStorageType;

void cleanupBindBuffersHelper(BindStorageType::iterator begin,
                              BindStorageType::iterator end)
{
    for (; begin != end; ++begin)
    {
        delete[] static_cast<char *>((*begin).buffer);
        delete (*begin).is_null;
        delete (*begin).length;
    }
}

bool nextRowFetchHelper(MYSQL_STMT *stmt)
{
    switch (mysql_stmt_fetch(stmt))
    {
    case 0:
    case MYSQL_DATA_TRUNCATED:
        return true;
    default:
        return false;
    }
}

std::size_t SizeForType(enum_field_types type)
{
    switch (type)
    {
    case MYSQL_TYPE_TINY:
        return sizeof(int8);
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_SHORT:
        return sizeof(int16);
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT:
        return sizeof(int32);
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
        return sizeof(int64);
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
        return sizeof(MYSQL_TIME);
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_BIT:
        return 0;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
        return 64;
    case MYSQL_TYPE_GEOMETRY:
        /*
            Following types are not sent over the wire:
            MYSQL_TYPE_ENUM:
            MYSQL_TYPE_SET:
            */
    default:
        TC_LOG_ERROR("sql.sql", "SQL::SizeForType(): invalid field type %u", uint32(type));
        return 0;
    }
}


} // namespace

PreparedResultSet::PreparedResultSet(MYSQL_STMT *stmt)
    : m_rowPosition(0)
    , m_fieldCount(mysql_stmt_field_count(stmt))
{
    MYSQL_RES * const metadata = mysql_stmt_result_metadata(stmt);
    if (!metadata)
    {
        TC_LOG_ERROR("sql.sql", "PreparedResultSet: mysql_stmt_result_metadata failed. Error: %s", mysql_stmt_error(stmt));
        return;
    }

    BindStorageType bindData(m_fieldCount);

    {
        MYSQL_BIND *bindDataPtr = &bindData[0];

        //- This is where we prepare the buffer based on metadata
        while (MYSQL_FIELD * const field = mysql_fetch_field(metadata))
        {
            bindDataPtr->buffer_type = field->type;
            bindDataPtr->is_null = new my_bool;

            if ((bindDataPtr->buffer_length = SizeForType(field->type)))
                bindDataPtr->buffer = new char[bindDataPtr->buffer_length];
            else
                bindDataPtr->length = new unsigned long;

            ++bindDataPtr;
        }
    }

    mysql_free_result(metadata);

    //- This is where we bind the buffer to the statement
    if (mysql_stmt_bind_result(stmt, &bindData[0]) == 0
            && mysql_stmt_store_result(stmt) == 0)
    {
        m_rows.reserve(mysql_stmt_num_rows(stmt));

        while (nextRowFetchHelper(stmt))
        {
            Field *rowPtr = new Field[m_fieldCount];

            for (std::size_t i = 0; i < m_fieldCount; ++i)
            {
                MYSQL_BIND &bind = bindData[i];

                if (*bind.is_null == 0 && bind.length)
                {
                    delete[] static_cast<char *>(bind.buffer);
                    bind.buffer_length = *bind.length;

                    if (*bind.length != 0)
                    {
                        bind.buffer = new char[*bind.length];
                        mysql_stmt_fetch_column(stmt, &bind, i, 0);
                    }
                    else
                    {
                        bind.buffer = NULL;
                    }
                }

                char const *exactBuffer = (*bind.is_null == 0)
                        ? static_cast<char const *>(bind.buffer)
                        : NULL;

                rowPtr[i].SetByteValue(exactBuffer, bind.buffer_length, bind.buffer_type);
            }

            m_rows.push_back(rowPtr);
        }
    }

    cleanupBindBuffersHelper(bindData.begin(), bindData.end());
    mysql_stmt_free_result(stmt);
}

PreparedResultSet::~PreparedResultSet()
{
    for (StorageType::const_iterator i = m_rows.begin(); i != m_rows.end(); ++i)
        delete[] (*i);
}

bool PreparedResultSet::NextRow()
{
    /// Only updates the m_rowPosition so upper level code knows in which element
    /// of the rows vector to look
    return (++m_rowPosition < GetRowCount());
}

Field const & PreparedResultSet::operator [] (std::size_t index) const
{
    return m_rows[m_rowPosition][index];
}
