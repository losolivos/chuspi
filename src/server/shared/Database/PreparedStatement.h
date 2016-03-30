#ifndef PREPARED_STATEMENT_H
#define PREPARED_STATEMENT_H

#include "Define.h"

#include <string>
#include <vector>

//- Upper-level class that is used in code
class PreparedStatement
{
public:
    enum FieldType
    {
        TYPE_BOOL,
        TYPE_UI8,
        TYPE_UI16,
        TYPE_UI32,
        TYPE_UI64,
        TYPE_I8,
        TYPE_I16,
        TYPE_I32,
        TYPE_I64,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_BINARY,
        TYPE_NULL
    };

private:
    //- Union for data buffer (upper-level bind -> queue -> lower-level bind)
    union Numeric
    {
        uint8 as_uint8;
        int8 as_int8;
        uint16 as_uint16;
        int16 as_int16;
        uint32 as_uint32;
        int32 as_int32;
        uint64 as_uint64;
        int64 as_int64;
        float as_float;
        double as_double;
    };

    struct InternalDataRep
    {
        Numeric num;
        std::string str;
        std::vector<uint8> bin;
        FieldType type;
    };

public:
    typedef InternalDataRep ValueType;
    typedef std::vector<ValueType> StorageType;
    typedef StorageType::const_iterator const_iterator;

public:
    PreparedStatement(uint32 index);

    uint32 index() const { return m_index; }
    std::size_t paramCount() const { return m_statementData.size(); }

    const_iterator begin() const { return m_statementData.begin(); }
    const_iterator end() const { return m_statementData.end(); }

    void setNull(uint8 index);

    void setBool(uint8 index, bool value);

    void setUInt8(uint8 index, uint8 value);
    void setInt8(uint8 index, int8 value);

    void setUInt16(uint8 index, uint16 value);
    void setInt16(uint8 index, int16 value);

    void setUInt32(uint8 index, uint32 value);
    void setInt32(uint8 index, int32 value);

    void setUInt64(uint8 index, uint64 value);
    void setInt64(uint8 index, int64 value);

    void setFloat(uint8 index, float value);
    void setDouble(uint8 index, double value);

    void setString(uint8 index, std::string const &value);

    void setBinary(uint8 index, const uint8 *value, size_t size);

private:
    uint32 m_index;
    StorageType m_statementData;
};

#endif // PREPARED_STATEMENT_H
