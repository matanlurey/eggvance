#pragma once

#include "common/bits.h"
#include "common/macros.h"

struct PSR
{
    enum class Mode
    {
        Usr = 0b10000,
        Fiq = 0b10001,
        Irq = 0b10010,
        Svc = 0b10011,
        Abt = 0b10111,
        Sys = 0b11111,
        Und = 0b11011
    };

    enum class Condition
    {
        EQ = 0x0,
        NE = 0x1,
        CS = 0x2,
        CC = 0x3,
        MI = 0x4,
        PL = 0x5,
        VS = 0x6,
        VC = 0x7,
        HI = 0x8,
        LS = 0x9,
        GE = 0xA,
        LT = 0xB,
        GT = 0xC,
        LE = 0xD,
        AL = 0xE,
        NV = 0xF
    };

    inline PSR& operator=(u32 value)
    {
        m = static_cast<Mode>(bits<0, 5>(value));
        t = bits< 5, 1>(value);
        f = bits< 6, 1>(value);
        i = bits< 7, 1>(value);
        v = bits<28, 1>(value);
        c = bits<29, 1>(value);
        z = bits<30, 1>(value);
        n = bits<31, 1>(value);

        return *this;
    }

    inline operator u32() const
    {
        return static_cast<u32>(m)
            | (static_cast<u32>(t) <<  5)
            | (static_cast<u32>(f) <<  6)
            | (static_cast<u32>(i) <<  7)
            | (static_cast<u32>(v) << 28)
            | (static_cast<u32>(c) << 29)
            | (static_cast<u32>(z) << 30)
            | (static_cast<u32>(n) << 31);
    }

    inline uint size() const
    {
        return 2 << (static_cast<uint>(t) ^ 0x1);
    }

    inline bool check(Condition condition) const
    {
        switch (condition)
        {
        case Condition::EQ: return z;
        case Condition::NE: return !z;
        case Condition::CS: return c;
        case Condition::CC: return !c;
        case Condition::MI: return n;
        case Condition::PL: return !n;
        case Condition::VS: return v;
        case Condition::VC: return !v;
        case Condition::HI: return c && !z;
        case Condition::LS: return !c || z;
        case Condition::GE: return n == v;
        case Condition::LT: return n != v;
        case Condition::GT: return !z && (n == v);
        case Condition::LE: return z || (n != v);
        case Condition::AL: return true;
        case Condition::NV: return false;

        default:
            UNREACHABLE;
            return false;
        }
    }

    Mode m = Mode::Sys;
    bool t = false;
    bool f = false;
    bool i = false;
    bool v = false;
    bool c = false;
    bool z = false;
    bool n = false;
};
