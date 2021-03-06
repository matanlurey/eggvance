#pragma once

#include "base/integer.h"

class KeyInput
{
public:
    inline KeyInput& operator=(u16 value)
    {
        this->value = value & ~0xFC00;

        return *this;
    }

    inline operator u16() const
    {
        return value;
    }

    template<uint index>
    inline u8 read() const
    {
        static_assert(index < 2);

        return reinterpret_cast<const u8*>(&value)[index];
    }

    u16 value = 0x03FF;
};
