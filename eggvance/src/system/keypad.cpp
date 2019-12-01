#include "keypad.h"

#include "arm/arm.h"
#include "devices/inputdevice.h"

Keypad keypad;

void Keypad::reset()
{
    io.keycnt.reset();
    io.keyinput.reset();
}

void Keypad::update()
{
    u16 previous = io.keyinput;

    input_device->poll(io.keyinput.value);

    if (previous != io.keyinput && io.keycnt.irq)
    {
        bool interrupt = io.keycnt.logic
            ? (~io.keyinput == io.keycnt.mask)
            : (~io.keyinput &  io.keycnt.mask);

        if (interrupt)
        {
            arm.request(Interrupt::Keypad);
        }
    }
}
