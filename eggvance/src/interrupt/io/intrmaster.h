#pragma once

#include "base/integer.h"

class IntrMaster
{
public:
    inline operator bool() const
    {
        return value & 0x1;
    }

    template<uint index>
    inline u8 read() const
    {
        static_assert(index < 2);

        return reinterpret_cast<const u8*>(&value)[index];
    }

    template<uint index>
    inline void write(u8 byte)
    {
        static_assert(index < 2);

        reinterpret_cast<u8*>(&value)[index] = byte;
    }

    u16 value = 0;
};
