#ifndef TRINITY_SHARED_UTILITIES_FLAG128_HPP
#define TRINITY_SHARED_UTILITIES_FLAG128_HPP

#include <cstdint>
#include <cstddef>

namespace Trinity {

// handler for operations on large flags
class Flag128 final
{
public:
    Flag128()
        : part()
    { }

    explicit Flag128(std::uint32_t p1, std::uint32_t p2 = 0, std::uint32_t p3 = 0, std::uint32_t p4 = 0)
    {
        part[0] = p1;
        part[1] = p2;
        part[2] = p3;
        part[3] = p4;
    }

    bool IsEqual(std::uint32_t p1, std::uint32_t p2 = 0, std::uint32_t p3 = 0, std::uint32_t p4 = 0) const
    {
        return part[0] == p1 && part[1] == p2 && part[2] == p3 && part[3] == p4;
    }

    bool HasFlag(std::uint32_t p1, std::uint32_t p2 = 0, std::uint32_t p3 = 0, std::uint32_t p4 = 0) const
    {
        return part[0] & p1 || part[1] & p2 || part[2] & p3 || part[3] & p4;
    };

    void Set(std::uint32_t p1, std::uint32_t p2 = 0, std::uint32_t p3 = 0, std::uint32_t p4 = 0)
    {
        part[0] = p1;
        part[1] = p2;
        part[2] = p3;
        part[3] = p4;
    };

    bool operator<(Flag128 const &right) const
    {
        for (uint8 i = 4; i > 0; --i)
        {
            if (part[i - 1] < right.part[i - 1])
                return true;
            else if (part[i - 1] > right.part[i - 1])
                return false;
        }
        return false;
    }

    Flag128 & operator=(Flag128 const &right)
    {
        part[0] = right.part[0];
        part[1] = right.part[1];
        part[2] = right.part[2];
        part[3] = right.part[3];

        return *this;
    };

    Flag128 operator&(Flag128 const &right) const
    {
        return Flag128(
            part[0] & right.part[0], part[1] & right.part[1],
            part[2] & right.part[2], part[3] & right.part[3]);
    };

    Flag128 & operator&=(Flag128 const &right)
    {
        part[0] &= right.part[0];
        part[1] &= right.part[1];
        part[2] &= right.part[2];
        part[3] &= right.part[3];

        return *this;
    };

    Flag128 operator|(Flag128 const &right) const
    {
        return Flag128(
            part[0] | right.part[0], part[1] | right.part[1],
            part[2] | right.part[2], part[3] | right.part[3]);
    };

    Flag128 & operator|=(Flag128 const &right)
    {
        part[0] |= right.part[0];
        part[1] |= right.part[1];
        part[2] |= right.part[2];
        part[3] |= right.part[3];

        return *this;
    };

    Flag128 operator~() const
    {
        return Flag128(~part[0], ~part[1], ~part[2], ~part[3]);
    };

    Flag128 operator^(Flag128 const &right) const
    {
        return Flag128(
            part[0] ^ right.part[0], part[1] ^ right.part[1],
            part[2] ^ right.part[2], part[3] ^ right.part[3]);
    };

    Flag128 & operator^=(Flag128 const &right)
    {
        part[0] ^= right.part[0];
        part[1] ^= right.part[1];
        part[2] ^= right.part[2];
        part[3] ^= right.part[3];

        return *this;
    };

#ifndef _MSC_VER
    explicit operator bool() const
#else
    operator bool() const
#endif
    {
        return part[0] != 0 || part[1] != 0 || part[2] != 0 || part[3] != 0;
    };

    bool operator!() const
    {
        return !operator bool();
    };

    std::uint32_t & operator[](std::size_t el)
    {
        return part[el];
    };

    std::uint32_t const & operator[](std::size_t el) const
    {
        return part[el];
    }

private:
    std::uint32_t part[4];
};

inline bool operator==(Flag128 const &left, Flag128 const &right)
{
    return left.IsEqual(right[0], right[1], right[2], right[3]);
}

inline bool operator!=(Flag128 const &left, Flag128 const &right)
{
    return !(left == right);
};

} // namespace Trinity

#endif // TRINITY_SHARED_UTILITIES_FLAG128_HPP
