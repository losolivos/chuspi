#include "PreparedStatement.h"

#include <cstring>

PreparedStatement::PreparedStatement(uint32 index)
    : m_index(index)
{ }

void PreparedStatement::setNull(uint8 index)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].type = TYPE_NULL;
}

void PreparedStatement::setBool(uint8 index, bool value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_uint8 = value;
    m_statementData[index].type = TYPE_BOOL;
}

void PreparedStatement::setUInt8(uint8 index, uint8 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_uint8 = value;
    m_statementData[index].type = TYPE_UI8;
}

void PreparedStatement::setInt8(uint8 index, int8 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_int8 = value;
    m_statementData[index].type = TYPE_I8;
}

void PreparedStatement::setUInt16(uint8 index, uint16 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_uint16 = value;
    m_statementData[index].type = TYPE_UI16;
}

void PreparedStatement::setInt16(uint8 index, int16 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_int16 = value;
    m_statementData[index].type = TYPE_I16;
}

void PreparedStatement::setUInt32(uint8 index, uint32 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_uint32 = value;
    m_statementData[index].type = TYPE_UI32;
}

void PreparedStatement::setInt32(uint8 index, int32 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_int32 = value;
    m_statementData[index].type = TYPE_I32;
}

void PreparedStatement::setUInt64(uint8 index, uint64 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_uint64 = value;
    m_statementData[index].type = TYPE_UI64;
}

void PreparedStatement::setInt64(uint8 index, int64 value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_int64 = value;
    m_statementData[index].type = TYPE_I64;
}

void PreparedStatement::setFloat(uint8 index, float value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_float = value;
    m_statementData[index].type = TYPE_FLOAT;
}

void PreparedStatement::setDouble(uint8 index, double value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].num.as_double = value;
    m_statementData[index].type = TYPE_DOUBLE;
}

void PreparedStatement::setString(uint8 index, std::string const &value)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].str.assign(value);
    m_statementData[index].type = TYPE_STRING;
}

void PreparedStatement::setBinary(uint8 index, uint8 const *value, size_t size)
{
    if (index >= m_statementData.size())
        m_statementData.resize(index + 1);
    m_statementData[index].bin.resize(size);
    std::memcpy(&m_statementData[index].bin[0], value, size);
    m_statementData[index].type = TYPE_BINARY;
}
