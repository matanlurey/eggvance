#include "keycontrol.h"

#include "common/macros.h"
#include "common/utility.h"

void KeyControl::reset()
{
    *this = {};
}

u8 KeyControl::readByte(int index)
{
    EGG_ASSERT(index <= 1, "Invalid index");

    return data[index];
}

void KeyControl::writeByte(int index, u8 byte)
{
    EGG_ASSERT(index <= 1, "Invalid index");

    if (index == 0)
    {
        bcast(keys)[0] = byte;
    }
    else
    {
        bcast(keys)[1] = bits<0, 2>(byte);
        irq            = bits<6, 1>(byte);
        logic          = bits<7, 1>(byte);
    }
    data[index] = byte;
}