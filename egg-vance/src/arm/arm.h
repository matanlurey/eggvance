#pragma once

#include "common/integer.h"
#include "mmu/mmu.h"
#include "registers.h"

class ARM
{
public:
    void reset();

    void step();

    MMU* mmu;

private:
    Registers regs;

    void fetch();
    void decode();
    void execute();
    void advance();

    struct PipeItem
    {
        u32 instr;
        Instruction decoded;
    } pipe[3];

    bool needs_flush;

    void flushPipe();

    void updateZ(u32 result);
    void updateN(u32 result);
    void updateC(u32 op1, u32 op2, bool addition);
    void updateV(u32 op1, u32 op2, bool addition);

    void updateArithmetic(u32 op1, u32 op2, bool addition);
    void updateLogical(u32 result);
    void updateShift(u32 result, bool carry);

    u32 logicalShiftLeft(u32 value, u8 offset, bool& carry);
    u32 logicalShiftRight(u32 value, u8 offset, bool& carry);
    u32 arithmeticShiftRight(u32 value, u8 offset, bool& carry);
    u32 rotateRight(u32 value, u8 offset, bool& carry);

    void moveShiftedRegister(u16 instr);
    void addSubImmediate(u16 instr);
    void moveCmpAddSubImmediate(u16 instr);
    void aluOperations(u16 instr);
    void highRegisterBranchExchange(u16 instr);
    void loadPcRelative(u16 instr);
    void loadStoreRegisterOffset(u16 instr);
    void loadStoreHalfSignExtended(u16 instr);
    void loadStoreImmediateOffset(u16 instr);
    void loadStoreHalf(u16 instr);
    void loadStoreSpRelative(u16 instr);
    void loadAddress(u16 instr);
    void addOffsetSp(u16 instr);
    void pushPopRegisters(u16 instr);
    void loadStoreMultiple(u16 instr);
    void conditionalBranch(u16 instr);
    void softwareInterruptThumb(u16 instr);
    void unconditionalBranch(u16 instr);
    void longBranchLink(u16 instr);

    void dataProcessing(u32 instr);
    void branchExchange(u32 instr);
    void branchLink(u32 instr);
};
