#pragma once

#include "base/bits.h"
#include "base/config.h"
#include "base/macros.h"
#include "base/register.h"

class DisplayControl : public RegisterRW<2>
{
public:
    template<uint index>
    inline void write(u8 byte)
    {
        static_assert(index < 2);

        data[index] = byte;

        if (index == 0)
        {
            mode     = bits::seq<0, 3>(byte);
            frame    = bits::seq<4, 1>(byte) * 0xA000;
            oam_free = bits::seq<5, 1>(byte);
            mapping  = bits::seq<6, 1>(byte);
            blank    = bits::seq<7, 1>(byte);
        }
        else
        {
            layers = bits::seq<0, 5>(byte);
            win0   = bits::seq<5, 1>(byte);
            win1   = bits::seq<6, 1>(byte);
            winobj = bits::seq<7, 1>(byte);
        }
    }

    inline bool isActive() const
    {
        switch (mode)
        {
        case 0: return 0b11111 & layers;
        case 1: return 0b10111 & layers;
        case 2: return 0b11100 & layers;
        case 3: return 0b10100 & layers;
        case 4: return 0b10100 & layers;
        case 5: return 0b10100 & layers;

        default:
            return false;
        }
    }
    
    inline bool isBitmap() const
    {
        return mode > 2;
    }

    uint mode     = 0;
    uint frame    = 0;
    uint oam_free = 0;
    uint mapping  = 0;
    uint blank    = config.bios_skip;
    uint layers   = 0;
    uint win0     = 0;
    uint win1     = 0;
    uint winobj   = 0;
};
