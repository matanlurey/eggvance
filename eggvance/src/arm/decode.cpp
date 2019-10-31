#include "decode.h"

#include <array>

#include "common/utility.h"

InstructionArm decodeHashArm(int hash)
{
    if ((hash & 0b1111'0000'0000) == 0b1111'0000'0000) return InstructionArm::SoftwareInterrupt;
    if ((hash & 0b1110'0000'0000) == 0b1100'0000'0000) return InstructionArm::CoprocessorDataTransfers;
    if ((hash & 0b1111'0000'0001) == 0b1110'0000'0000) return InstructionArm::CoprocessorDataOperations;
    if ((hash & 0b1111'0000'0001) == 0b1110'0000'0001) return InstructionArm::CoprocessorRegisterTransfers;
    if ((hash & 0b1110'0000'0000) == 0b1010'0000'0000) return InstructionArm::BranchLink;
    if ((hash & 0b1110'0000'0000) == 0b1000'0000'0000) return InstructionArm::BlockDataTransfer;
    if ((hash & 0b1110'0000'0001) == 0b0110'0000'0001) return InstructionArm::Undefined;
    if ((hash & 0b1100'0000'0000) == 0b0100'0000'0000) return InstructionArm::SingleDataTransfer;
    if ((hash & 0b1111'1111'1111) == 0b0001'0010'0001) return InstructionArm::BranchExchange;
    if ((hash & 0b1111'1100'1111) == 0b0000'0000'1001) return InstructionArm::Multiply;
    if ((hash & 0b1111'1000'1111) == 0b0000'1000'1001) return InstructionArm::MultiplyLong;
    if ((hash & 0b1111'1011'1111) == 0b0001'0000'1001) return InstructionArm::SingleDataSwap;
    if ((hash & 0b1110'0000'1001) == 0b0000'0000'1001) return InstructionArm::HalfSignedDataTransfer;
    if ((hash & 0b1101'1001'0000) == 0b0001'0000'0000) return InstructionArm::StatusTransfer;
    if ((hash & 0b1100'0000'0000) == 0b0000'0000'0000)
    {
        int flags  = bits<4, 1>(hash);
        int opcode = bits<5, 4>(hash);

        if ((opcode >> 2) == 0b10 && !flags)
            return InstructionArm::Undefined;

        return InstructionArm::DataProcessing;
    }

    return InstructionArm::Undefined;
}

static const auto lut_arm = [](){
    std::array<InstructionArm, 4096> lut;
    for (int hash = 0; hash < lut.size(); ++hash)
        lut[hash] = decodeHashArm(hash);
    return lut;
}();

inline int hashArm(u32 instr)
{
    return ((instr >> 16) & 0xFF0) | ((instr >> 4) & 0xF);
}

InstructionArm decodeArm(u32 instr)
{
    return lut_arm[hashArm(instr)];
}

InstructionThumb decodeHashThumb(int hash)
{
    if ((hash & 0b11'1110'0000) == 0b00'0110'0000) return InstructionThumb::AddSubtract;
    if ((hash & 0b11'1000'0000) == 0b00'0000'0000) return InstructionThumb::MoveShiftedRegister;
    if ((hash & 0b11'1000'0000) == 0b00'1000'0000) return InstructionThumb::ImmediateOperations;
    if ((hash & 0b11'1111'0000) == 0b01'0000'0000) return InstructionThumb::ALUOperations;
    if ((hash & 0b11'1111'0000) == 0b01'0001'0000)
    {
        int hs     = bits<0, 1>(hash);
        int hd     = bits<1, 1>(hash);
        int opcode = bits<2, 2>(hash);

        if (opcode != 0b11 && hs == 0 && hd == 0)
            return InstructionThumb::Undefined;
        if (opcode == 0b11 && hd == 1)
            return InstructionThumb::Undefined;

        return InstructionThumb::HighRegisterOperations;
    }
    if ((hash & 0b11'1110'0000) == 0b01'0010'0000) return InstructionThumb::LoadPCRelative;
    if ((hash & 0b11'1100'1000) == 0b01'0100'0000) return InstructionThumb::LoadStoreRegisterOffset;
    if ((hash & 0b11'1100'1000) == 0b01'0100'1000) return InstructionThumb::LoadStoreByteHalf;
    if ((hash & 0b11'1000'0000) == 0b01'1000'0000) return InstructionThumb::LoadStoreImmediateOffset;
    if ((hash & 0b11'1100'0000) == 0b10'0000'0000) return InstructionThumb::LoadStoreHalf;
    if ((hash & 0b11'1100'0000) == 0b10'0100'0000) return InstructionThumb::LoadStoreSPRelative;
    if ((hash & 0b11'1100'0000) == 0b10'1000'0000) return InstructionThumb::LoadRelativeAddress;
    if ((hash & 0b11'1111'1100) == 0b10'1100'0000) return InstructionThumb::AddOffsetSP;
    if ((hash & 0b11'1101'1000) == 0b10'1101'0000) return InstructionThumb::PushPopRegisters;
    if ((hash & 0b11'1100'0000) == 0b11'0000'0000) return InstructionThumb::LoadStoreMultiple;
    if ((hash & 0b11'1111'1100) == 0b11'0111'1100) return InstructionThumb::SoftwareInterrupt;
    if ((hash & 0b11'1100'0000) == 0b11'0100'0000)
    {
        int condition = bits<2, 4>(hash);

        if (condition == 0b1110)
            return InstructionThumb::Undefined;
        if (condition == 0b1111)
            return InstructionThumb::Undefined;

        return InstructionThumb::ConditionalBranch;
    }
    if ((hash & 0b11'1110'0000) == 0b11'1000'0000) return InstructionThumb::UnconditionalBranch;
    if ((hash & 0b11'1100'0000) == 0b11'1100'0000) return InstructionThumb::LongBranchLink;

    return InstructionThumb::Undefined;
}

static const auto lut_thumb = [](){
    std::array<InstructionThumb, 1024> lut;
    for (int hash = 0; hash < lut.size(); ++hash)
        lut[hash] = decodeHashThumb(hash);
    return lut;
}();

inline int hashThumb(u16 instr)
{
    return instr >> 6;
}

InstructionThumb decodeThumb(u16 instr)
{
    return lut_thumb[hashThumb(instr)];
}
