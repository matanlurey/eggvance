#include "arm.h"

#include <fmt/printf.h>

#include "common/utility.h"
#include "mmu/interrupt.h"
#include "disassembler.h"
#include "decode.h"

ARM::ARM(MMU& mmu)
    : mmu(mmu)
    , mmio(mmu.mmio)
{
    reset();
}

void ARM::reset()
{
    Registers::reset();

    cycles = 0;
    cycles_total = 0;
}

int ARM::emulate()
{
    cycles = 0;

    if (!cpsr.irqd && Interrupt::requested())
    {
        interrupt();
    }
     
    //debug();

    execute();
    advance();

    cycles_total += cycles;

    return cycles;
}

void ARM::interrupt()
{
    u32 cpsr = this->cpsr;
    u32 next = this->pc - 2 * instrWidth();

    switchMode(PSR::IRQ);
    spsr = cpsr;

    // Interrupts return with subs pc, lr, 4
    lr = next + 4;

    this->cpsr.thumb = false;
    this->cpsr.irqd  = true;

    pc = EXV_IRQ;
    advance();
    advance();
}

void ARM::softwareInterrupt()
{
    cycle(pc, NSEQ);

    u32 cpsr = this->cpsr;
    u32 next = pc - instrWidth();

    switchMode(PSR::SVC);
    spsr = cpsr;
    lr = next;

    this->cpsr.thumb = false;
    this->cpsr.irqd  = true;

    pc = EXV_SWI;
    advance();

    cycle(pc, SEQ);
    cycle(pc + 4, SEQ);
}

int ARM::instrWidth() const
{
    static constexpr int widths[2] = { 4, 2 };

    return widths[cpsr.thumb];
}

void ARM::execute()
{
    if (cpsr.thumb)
    {
        u16 instr = mmu.readHalf(pc - 4);

        switch (decodeThumb(instr))
        {
        case InstructionThumb::MoveShiftedRegister: moveShiftedRegister(instr); break;
        case InstructionThumb::AddSubtractImmediate: addSubtractImmediate(instr); break;
        case InstructionThumb::AddSubtractMoveCompareImmediate: addSubtractMoveCompareImmediate(instr); break;
        case InstructionThumb::ALUOperations: aluOperations(instr); break;
        case InstructionThumb::HighRegisterBranchExchange: highRegisterBranchExchange(instr); break;
        case InstructionThumb::LoadPCRelative: loadPCRelative(instr); break;
        case InstructionThumb::LoadStoreRegisterOffset: loadStoreRegisterOffset(instr); break;
        case InstructionThumb::LoadStoreHalfwordSigned: loadStoreHalfwordSigned(instr); break;
        case InstructionThumb::LoadStoreImmediateOffset: loadStoreImmediateOffset(instr); break;
        case InstructionThumb::LoadStoreHalfword: loadStoreHalfword(instr); break;
        case InstructionThumb::LoadStoreSPRelative: loadStoreSPRelative(instr); break;
        case InstructionThumb::LoadAddress: loadAddress(instr); break;
        case InstructionThumb::AddOffsetSP: addOffsetSP(instr); break;
        case InstructionThumb::PushPopRegisters: pushPopRegisters(instr); break;
        case InstructionThumb::LoadStoreMultiple: loadStoreMultiple(instr); break;
        case InstructionThumb::ConditionalBranch: conditionalBranch(instr); break;
        case InstructionThumb::SoftwareInterrupt: softwareInterrupt(); break;
        case InstructionThumb::UnconditionalBranch: unconditionalBranch(instr); break;
        case InstructionThumb::LongBranchLink: longBranchLink(instr); break;
        }
    }
    else
    {
        u32 instr = mmu.readWord(pc - 8);

        if (cpsr.matches(static_cast<PSR::Condition>(instr >> 28)))
        {
            switch (decodeArm(instr))
            {
            case InstructionArm::BranchExchange: branchExchange(instr); break;
            case InstructionArm::BranchLink: branchLink(instr); break;
            case InstructionArm::DataProcessing: dataProcessing(instr); break;
            case InstructionArm::PSRTransfer: psrTransfer(instr); break;
            case InstructionArm::Multiply: multiply(instr); break;
            case InstructionArm::MultiplyLong: multiplyLong(instr); break;
            case InstructionArm::SingleDataTransfer: singleDataTransfer(instr); break;
            case InstructionArm::HalfwordSignedDataTransfer: halfwordSignedDataTransfer(instr); break;
            case InstructionArm::BlockDataTransfer: blockDataTransfer(instr); break;
            case InstructionArm::SingleDataSwap: singleDataSwap(instr); break;
            case InstructionArm::SoftwareInterrupt: softwareInterrupt(); break;
            }
        }
        else
        {
            cycle(pc + 4, SEQ);
        }
    }
}

void ARM::advance()
{
    pc += instrWidth();
}

void ARM::debug()
{
    u32 data = cpsr.thumb
        ? mmu.readHalf(pc - 4)
        : mmu.readWord(pc - 8);

    fmt::printf("%08X  %08X  %08X  %s\n", 
        cycles_total, 
        pc - 2 * instrWidth(), 
        data, 
        Disassembler::disassemble(data, *this)
    );
}

void ARM::logical(u32 result)
{
    cpsr.z = result == 0;
    cpsr.n = result >> 31;
}

void ARM::logical(u32 result, bool carry)
{
    cpsr.z = result == 0;
    cpsr.n = result >> 31;
    cpsr.c = carry;
}

void ARM::arithmetic(u32 op1, u32 op2, bool addition)
{
    u32 result = addition
        ? op1 + op2
        : op1 - op2;

    cpsr.z = result == 0;
    cpsr.n = result >> 31;

    cpsr.c = addition
        ? op2 > (0xFFFFFFFF - op1)
        : op2 <= op1;

    int msb_op1 = op1 >> 31;
    int msb_op2 = op2 >> 31;

    cpsr.v = addition
        ? msb_op1 == msb_op2 && (result >> 31) != msb_op1
        : msb_op1 != msb_op2 && (result >> 31) == msb_op2;
}

u32 ARM::lsl(u32 value, int amount, bool& carry)
{
    if (amount != 0)
    {
        if (amount < 32)
        {
            carry = (value >> (32 - amount)) & 0x1;
            value <<= amount;
        }
        else
        {
            if (amount == 32)
                carry = value & 0x1;
            else
                carry = 0;

            value = 0;
        }
    }
    else  // Special case LSL #0
    {
        carry = cpsr.c;
    }
    return value;
}

u32 ARM::lsr(u32 value, int amount, bool& carry, bool immediate)
{
    if (amount != 0)
    {
        if (amount < 32)
        {
            carry = (value >> (amount - 1)) & 0x1;
            value >>= amount;
        }
        else
        {
            carry = 0;
            value = 0;
        }
    }
    else  // Special case LSR #0 / #32
    {
        if (immediate)
        {
            carry = value >> 31;
            value = 0;
        }
        else
        {
            carry = cpsr.c;
        }
    }
    return value;
}

u32 ARM::asr(u32 value, int amount, bool& carry, bool immediate)
{
    if (amount != 0)
    {
        if (amount < 32)
        {
            carry = value >> (amount - 1) & 0x1;
            value = static_cast<s32>(value) >> amount;
        }
        else
        {
            carry = value >> 31;
            value = carry ? 0xFFFFFFFF : 0;
        }
    }
    else  // Special case ASR #0 / #32
    {
        if (immediate)
        {
            carry = value >> 31;
            value = carry ? 0xFFFFFFFF : 0;
        }
        else
        {
            carry = cpsr.c;
        }
    }
    return value;
}

u32 ARM::ror(u32 value, int amount, bool& carry, bool immediate)
{
    if (amount != 0)
    {
        amount %= 32;

        if (amount != 0)
            value = (value << (32 - amount)) | (value >> amount);

        carry = value >> 31;
    }
    else  // Special case RRX
    {
        if (immediate)
        {
            carry = value & 0x1;
            value >>= 1;
            value |= cpsr.c << 31;
        }
        else
        {
            carry = cpsr.c;
        }
    }
    return value;
}

u32 ARM::ldr(u32 addr)
{
    u32 value = mmu.readWord(addr);
    if (misalignedWord(addr))
    {
        bool carry;
        int rotation = (addr & 0x3) << 3;
        value = ror(value, rotation, carry);
    }
    return value;
}

u32 ARM::ldrh(u32 addr)
{
    u32 value = mmu.readHalf(addr);
    if (misalignedHalf(addr))
    {
        bool carry;
        value = ror(value, 8, carry);
    }
    return value;
}

u32 ARM::ldrsh(u32 addr)
{
    u32 value;
    if (misalignedHalf(addr))
    {
        value = mmu.readByte(addr);
        value = signExtend<8>(value);
    }
    else
    {
        value = mmu.readHalf(addr);
        value = signExtend<16>(value);
    }
    return value;
}

void ARM::cycle()
{
    cycles++;
}

void ARM::cycle(u32 addr, AccessType access)
{
    cycles++;

    static constexpr int seq[3][2] = { { 1, 2 }, { 1, 4 }, { 1, 8} };
    static constexpr int nonseq[4] = { 4, 3, 2, 8 };

    switch (addr >> 24)
    {
    case PAGE_PALETTE:
    case PAGE_VRAM:
    case PAGE_OAM:
        if (!mmio.dispstat.hblank && !mmio.dispstat.vblank)
            cycles++;
        break;

    case PAGE_GAMEPAK_0:
    case PAGE_GAMEPAK_0+1:
        if (access == SEQ)
            cycles += seq[0][mmio.waitcnt.ws0.s];
        else
            cycles += nonseq[mmio.waitcnt.ws0.n];
        break;

    case PAGE_GAMEPAK_1:
    case PAGE_GAMEPAK_1+1:
        if (access == SEQ)
            cycles += seq[1][mmio.waitcnt.ws1.s];
        else
            cycles += nonseq[mmio.waitcnt.ws1.n];
        break;

    case PAGE_GAMEPAK_2:
    case PAGE_GAMEPAK_2+1:
        if (access == SEQ)
            cycles += seq[2][mmio.waitcnt.ws2.s];
        else
            cycles += nonseq[mmio.waitcnt.ws2.n];
        break;

    default:
        if (addr >= MAP_GAMEPAK_SRAM)
            cycles += nonseq[mmio.waitcnt.sram];
        break;
    }
}

void ARM::cycleBooth(u32 multiplier, bool allow_ones)
{
    static constexpr int masks[3] = 
    {
        0xFF000000,
        0xFFFF0000,
        0xFFFFFF00
    };

    int internal = 4;
    for (u32 mask : masks)
    {
        u32 bits = multiplier & mask;
        if (bits == 0 || (allow_ones && bits == mask))
            internal--;
        else
            break;
    }
    cycles += internal;
}
