def bits(position, amount, value):
    return (value >> position) & ((1 << amount) - 1)


def matches(pattern, value):
    for idx, char in enumerate(reversed(pattern)):
        if char == "0" and  value & (1 << idx): return False
        if char == "1" and ~value & (1 << idx): return False
    return True


def decode(x):
    if matches("1111xxxxxxxx", x):
        return "SoftwareInterrupt"

    if matches("110xxxxxxxxx", x):
        return "CoprocessorDataTransfers"

    if matches("1110xxxxxxx0", x):
        return "CoprocessorDataOperations"

    if matches("1110xxxxxxx1", x):
        return "CoprocessorRegisterTransfers"

    if matches("101xxxxxxxxx", x):
        link = bits(8, 1, x)

        return "BranchLink<{link}>".format(
            link=link)

    if matches("100xxxxxxxxx", x):
        load      = bits(4, 1, x)
        writeback = bits(5, 1, x)
        user_mode = bits(6, 1, x)
        increment = bits(7, 1, x)
        pre_index = bits(8, 1, x)

        return "BlockDataTransfer<{load}, {writeback}, {user_mode}, {increment}, {pre_index}>".format(
            load=load,
            writeback=writeback,
            user_mode=user_mode,
            increment=increment,
            pre_index=pre_index
        )

    if matches("011xxxxxxxx1", x):
        return "Undefined"

    if matches("01xxxxxxxxxx", x):
        load      = bits(4, 1, x)
        writeback = bits(5, 1, x)
        byte      = bits(6, 1, x)
        increment = bits(7, 1, x)
        pre_index = bits(8, 1, x)
        reg_op    = bits(9, 1, x)

        return "SingleDataTransfer<{load}, {writeback}, {byte}, {increment}, {pre_index}, {imm_op}>".format(
            load=load,
            writeback=writeback | (pre_index ^ 0x1),
            byte=byte,
            increment=increment,
            pre_index=pre_index,
            imm_op=reg_op ^ 0x1
        )

    if matches("000100100001", x):
        return "BranchExchange"

    if matches("000000xx1001", x):
        flags      = bits(4, 1, x)
        accumulate = bits(5, 1, x)

        return "Multiply<{flags}, {accumulate}>".format(
            flags=flags,
            accumulate=accumulate
        )

    if matches("00001xxx1001", x):
        flags      = bits(4, 1, x)
        accumulate = bits(5, 1, x)
        sign       = bits(6, 1, x)

        return "MultiplyLong<{flags}, {accumulate}, {sign}>".format(
            flags=flags,
            accumulate=accumulate,
            sign=sign
        )

    if matches("00010x001001", x):
        byte = bits(6, 1, x)

        return "SingleDataSwap<{byte}>".format(
            byte=byte)

    if matches("000xxxxx1xx1", x):
        opcode    = bits(1, 2, x)
        load      = bits(4, 1, x)
        writeback = bits(5, 1, x)
        imm_op    = bits(6, 1, x)
        increment = bits(7, 1, x)
        pre_index = bits(8, 1, x)

        if opcode == 0b00:
            return "Undefined"

        return "HalfSignedDataTransfer<{opcode}, {load}, {writeback}, {imm_op}, {increment}, {pre_index}>".format(
            opcode=opcode,
            load=load,
            writeback=writeback | (pre_index ^ 0x1),
            imm_op=imm_op,
            increment=increment,
            pre_index=pre_index
        )

    if matches("00x10xx0xxxx", x):
        write    = bits(5, 1, x)
        use_spsr = bits(6, 1, x)
        imm_op   = bits(9, 1, x)

        return "StatusTransfer<{write}, {use_spsr}, {imm_op}>".format(
            write=write,
            use_spsr=use_spsr,
            imm_op=imm_op
        )

    if matches("00xxxxxxxxxx", x):
        flags  = bits(4, 1, x)
        opcode = bits(5, 4, x)
        imm_op = bits(9, 1, x)

        if (opcode >> 2) == 0b10 and not flags:
            return "Undefined"

        return "DataProcessing<{flags}, {opcode}, {imm_op}>".format(
            flags=flags,
            opcode=opcode,
            imm_op=imm_op
        )

    return "Undefined"


prefix = "    &ARM::Arm_"
template = """std::array<void(ARM::*)(u32), 4096> ARM::instr_arm =
{{
{}
}};
"""

lut = [prefix + decode(x) for x in range(4096)]
with open("lut_arm.cpp", "w") as output:
    output.write(template.format(",\n".join(lut)))
