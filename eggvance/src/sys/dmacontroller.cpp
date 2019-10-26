#include "dmacontroller.h"

#include "common/macros.h"
#include "mmu/memmap.h"

#define CASE2(label) case label + 0: case label + 1:
#define CASE4(label) case label + 0: case label + 1: case label + 2: case label + 3:

#define READ2(label, reg) CASE2(label) return reg.readByte(addr - label)
#define READ4(label, reg) CASE4(label) return reg.readByte(addr - label)

#define WRITE2(label, reg) CASE2(label) reg.writeByte(addr - label, byte); break
#define WRITE4(label, reg) CASE4(label) reg.writeByte(addr - label, byte); break

DMAController::DMAController()
{
    dmas[0].id = 0;
    dmas[1].id = 1;
    dmas[2].id = 2;
    dmas[3].id = 3;

    dmas[0].count.max = 0x04000;
    dmas[1].count.max = 0x04000;
    dmas[2].count.max = 0x04000;
    dmas[3].count.max = 0x10000;
}

void DMAController::reset()
{
    active = nullptr;

    for (auto& dma : dmas)
    {
        dma.reset();
    }
}

void DMAController::run(int& cycles)
{
    active->run(cycles);

    if (active->state == DMA::State::Finished)
    {
        active = nullptr;

        for (auto& dma : dmas)
        {
            if (dma.state == DMA::State::Running)
            {
                active = &dma;
                break;
            }
        }
    }

}

void DMAController::signal(DMA::Timing timing)
{
    for (auto& dma : dmas)
    {
        if (DMA::Timing(dma.control.timing) == timing)
        {
            if (dma.control.enabled && dma.state != DMA::State::Running)
            {
                dma.start();
                if (!active || dma.id < active->id)
                    active = &dma;
            }
        }
    }
}

u8 DMAController::readByte(u32 addr)
{    
    switch (addr)
    {
    READ2(REG_DMA0CNT_H, dmas[0].control);
    READ2(REG_DMA1CNT_H, dmas[1].control);
    READ2(REG_DMA2CNT_H, dmas[2].control);
    READ2(REG_DMA3CNT_H, dmas[3].control);

    default:
        EGG_UNREACHABLE;
        break;
    }
    return 0;
}

void DMAController::writeByte(u32 addr, u8 byte)
{
    switch (addr)
    {
    WRITE4(REG_DMA0SAD, dmas[0].sad);
    WRITE4(REG_DMA1SAD, dmas[1].sad);
    WRITE4(REG_DMA2SAD, dmas[2].sad);
    WRITE4(REG_DMA3SAD, dmas[3].sad);
    WRITE4(REG_DMA0DAD, dmas[0].dad);
    WRITE4(REG_DMA1DAD, dmas[1].dad);
    WRITE4(REG_DMA2DAD, dmas[2].dad);
    WRITE4(REG_DMA3DAD, dmas[3].dad);
    WRITE2(REG_DMA0CNT_L, dmas[0].count);
    WRITE2(REG_DMA1CNT_L, dmas[1].count);
    WRITE2(REG_DMA2CNT_L, dmas[2].count);
    WRITE2(REG_DMA3CNT_L, dmas[3].count);
    WRITE2(REG_DMA0CNT_H, dmas[0].control);
    WRITE2(REG_DMA1CNT_H, dmas[1].control);
    WRITE2(REG_DMA2CNT_H, dmas[2].control);
    WRITE2(REG_DMA3CNT_H, dmas[3].control);

    default:
        EGG_UNREACHABLE;
        break;
    }

    switch (addr)
    {
    case REG_DMA0CNT_L + 3:
    case REG_DMA1CNT_L + 3:
    case REG_DMA2CNT_L + 3:
    case REG_DMA3CNT_L + 3:
        signal(DMA::Timing::Immediate);
        break;
    }
}