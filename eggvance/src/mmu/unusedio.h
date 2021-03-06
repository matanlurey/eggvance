#pragma once

#include "base/config.h"
#include "base/register.h"

struct UnusedIO
{
    UnusedIO()
    {
        if (config.bios_skip)
        {
            memcontrol.write<0>(0x20);
            memcontrol.write<1>(0x00);
            memcontrol.write<2>(0x00);
            memcontrol.write<3>(0xD0);

            postflag.write<0>(0x01);

            rcnt.write<0>(0x00);
            rcnt.write<1>(0x80);
        }
    }

    RegisterRW<1> postflag;
    RegisterRW<2> greenswap;
    RegisterRW<4> memcontrol;

    RegisterRW<2> rcnt;
    RegisterRW<2> joycnt;
    RegisterRW<4> joyrecv;
    RegisterRW<4> joytrans;
    RegisterRW<2> joystat;
    RegisterRW<2> siocnt;
    RegisterRW<2> siomulti[4];
    RegisterRW<2> siosend;

    RegisterRW<2> soundcnt1_l;
    RegisterRW<2> soundcnt1_h;
    RegisterRW<2> soundcnt1_x;
    RegisterRW<2> soundcnt2_l;
    RegisterRW<2> soundcnt2_h;
    RegisterRW<2> soundcnt3_l;
    RegisterRW<2> soundcnt3_h;
    RegisterRW<2> soundcnt3_x;
    RegisterRW<2> soundcnt4_l;
    RegisterRW<2> soundcnt4_h;
    RegisterRW<2> soundcnt_l;
    RegisterRW<2> soundcnt_h;
    RegisterRW<2> soundcnt_x;
    RegisterRW<2> soundbias;
    RegisterRW<2> waveram[8];
    RegisterRW<4> fifo[2];
};
