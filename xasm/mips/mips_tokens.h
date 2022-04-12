/*  Copyright 2008-2022 Carsten Elton Sorensen and contributors

    This file is part of ASMotor.

    ASMotor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ASMotor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ASMotor.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef XASM_MIPS_TOKENS_H_INCLUDED_
#define XASM_MIPS_TOKENS_H_INCLUDED_

typedef enum {
    T_MIPS_ADD = 6000,
    T_MIPS_ADDU,
    T_MIPS_AND,
    T_MIPS_MOVN,
    T_MIPS_MOVZ,
    T_MIPS_MUL,
    T_MIPS_NOR,
    T_MIPS_OR,
    T_MIPS_ROTRV,
    T_MIPS_SLLV,
    T_MIPS_SLT,
    T_MIPS_SLTU,
    T_MIPS_SRAV,
    T_MIPS_SRLV,
    T_MIPS_SUB,
    T_MIPS_SUBU,
    T_MIPS_XOR,

    T_MIPS_INTEGER_RRR_FIRST = T_MIPS_ADD,
    T_MIPS_INTEGER_RRR_LAST = T_MIPS_XOR,

    T_MIPS_ADDI,
    T_MIPS_ADDIU,
    T_MIPS_ANDI,
    T_MIPS_ORI,
    T_MIPS_SLTI,
    T_MIPS_SLTIU,
    T_MIPS_XORI,

    T_MIPS_INTEGER_RRI_FIRST = T_MIPS_ADDI,
    T_MIPS_INTEGER_RRI_LAST = T_MIPS_XORI,

    // Branch R,R,addr

    T_MIPS_BEQ,
    T_MIPS_BEQL,
    T_MIPS_BNE,
    T_MIPS_BNEL,

    // Branch R,addr

    T_MIPS_BGEZ,
    T_MIPS_BGEZAL,
    T_MIPS_BGEZALL,
    T_MIPS_BGEZL,
    T_MIPS_BGTZ,
    T_MIPS_BGTZL,
    T_MIPS_BLEZ,
    T_MIPS_BLEZL,
    T_MIPS_BLTZ,
    T_MIPS_BLTZAL,
    T_MIPS_BLTZALL,
    T_MIPS_BLTZL,

    // Branch addr

    T_MIPS_B,
    T_MIPS_BAL,

    T_MIPS_BRANCH_FIRST = T_MIPS_BEQ,
    T_MIPS_BRANCH_LAST = T_MIPS_BAL,

    T_MIPS_ROTR,
    T_MIPS_SLL,
    T_MIPS_SRA,
    T_MIPS_SRL,

    T_MIPS_SHIFT_FIRST = T_MIPS_ROTR,
    T_MIPS_SHIFT_LAST = T_MIPS_SRL,

    /* Load/store: */

            T_MIPS_LB,
    T_MIPS_LBU,
    T_MIPS_LH,
    T_MIPS_LHU,
    T_MIPS_LL,
    T_MIPS_LW,
    T_MIPS_LWC1,
    T_MIPS_LWC2,
    T_MIPS_LWL,
    T_MIPS_LWR,
    T_MIPS_SB,
    T_MIPS_SC,
    T_MIPS_SH,
    T_MIPS_SW,
    T_MIPS_SWC1,
    T_MIPS_SWC2,
    T_MIPS_SWL,
    T_MIPS_SWR,

    T_MIPS_LOADSTORE_FIRST = T_MIPS_LB,
    T_MIPS_LOADSTORE_LAST = T_MIPS_SWR,

    // R-format, but only rs and rt

    T_MIPS_DIV,
    T_MIPS_DIVU,
    T_MIPS_MADD,
    T_MIPS_MADDU,
    T_MIPS_MSUB,
    T_MIPS_MSUBU,
    T_MIPS_MULT,
    T_MIPS_MULU,

    T_MIPS_RSRT_FIRST = T_MIPS_DIV,
    T_MIPS_RSRT_LAST = T_MIPS_MULU,

    // R-format, but only rd and rt

    T_MIPS_RDPGPR,
    T_MIPS_SEB,
    T_MIPS_SEH,
    T_MIPS_WRPGPR,
    T_MIPS_WSBH,

    T_MIPS_RDRT_FIRST = T_MIPS_RDPGPR,
    T_MIPS_RDRT_LAST = T_MIPS_WSBH,

    // rs, rt [, code]

    T_MIPS_TEQ,
    T_MIPS_TGE,
    T_MIPS_TGEU,
    T_MIPS_TLT,
    T_MIPS_TLTU,
    T_MIPS_TNE,

    T_MIPS_RSRTCODE_FIRST = T_MIPS_TEQ,
    T_MIPS_RSRTCODE_LAST = T_MIPS_TNE,

    T_MIPS_TEQI,
    T_MIPS_TGEI,
    T_MIPS_TGEIU,
    T_MIPS_TLTI,
    T_MIPS_TLTIU,
    T_MIPS_TNEI,

    T_MIPS_INTEGER_RI_FIRST = T_MIPS_TEQI,
    T_MIPS_INTEGER_RI_LAST = T_MIPS_TNEI,

    // R-format: rd, rs (rt=rd):

    T_MIPS_CLO,
    T_MIPS_CLZ,

    T_MIPS_INTEGER_RDRS_RTCOPY_FIRST = T_MIPS_CLO,
    T_MIPS_INTEGER_RDRS_RTCOPY_LAST = T_MIPS_CLZ,

    // No parameter:

    T_MIPS_DERET,
    T_MIPS_EHB,
    T_MIPS_ERET,
    T_MIPS_NOP,
    T_MIPS_SSNOP,
    T_MIPS_TLBP,
    T_MIPS_TLBR,
    T_MIPS_TLBWI,
    T_MIPS_TLBWR,

    T_MIPS_INTEGER_NO_PARAMETER_FIRST = T_MIPS_DERET,
    T_MIPS_INTEGER_NO_PARAMETER_LAST = T_MIPS_TLBWR,

    // R-format: rt

    T_MIPS_DI,
    T_MIPS_EI,

    T_MIPS_INTEGER_RT_FIRST = T_MIPS_DI,
    T_MIPS_INTEGER_RT_LAST = T_MIPS_EI,

    // R-format: rd

    T_MIPS_MFHI,
    T_MIPS_MFLO,

    T_MIPS_INTEGER_RD_FIRST = T_MIPS_MFHI,
    T_MIPS_INTEGER_RD_LAST = T_MIPS_MFLO,

    // R-format: rs

    T_MIPS_JR,
    T_MIPS_JR_HB,
    T_MIPS_MTHI,
    T_MIPS_MTLO,

    T_MIPS_INTEGER_RS_FIRST = T_MIPS_JR,
    T_MIPS_INTEGER_RS_LAST = T_MIPS_MTLO,

    // Jumps:

    T_MIPS_J,
    T_MIPS_JAL,

    T_MIPS_INTEGER_J_ABS_FIRST = T_MIPS_J,
    T_MIPS_INTEGER_J_ABS_LAST = T_MIPS_JAL,

    T_MIPS_LUI,

    // Others:

    T_MIPS_JALR,
    T_MIPS_JALR_HB,

    T_MIPS_INTEGER_J_REG_FIRST = T_MIPS_JALR,
    T_MIPS_INTEGER_J_REG_LAST = T_MIPS_JALR_HB,

    T_MIPS_EXT,
    T_MIPS_INS,

    T_MIPS_INTEGER_BITFIELD_FIRST = T_MIPS_EXT,
    T_MIPS_INTEGER_BITFIELD_LAST = T_MIPS_INS,

    T_MIPS_BREAK,
    T_MIPS_CACHE,
    T_MIPS_CFC1,
    T_MIPS_CFC2,
    T_MIPS_COP2,
    T_MIPS_CTC1,
    T_MIPS_CTC2,
    T_MIPS_MFC0,
    T_MIPS_MFC1,
    T_MIPS_MFC2,
    T_MIPS_MTC0,
    T_MIPS_MTC1,
    T_MIPS_MTC2,
    T_MIPS_PREF,
    T_MIPS_PREFX,
    T_MIPS_RDHWR,
    T_MIPS_SDBBP,
    T_MIPS_SYNC,
    T_MIPS_SYNCI,
    T_MIPS_SYSCALL,
    T_MIPS_WAIT,

    T_MIPS_REG_R0,
    T_MIPS_REG_R1,
    T_MIPS_REG_R2,
    T_MIPS_REG_R3,
    T_MIPS_REG_R4,
    T_MIPS_REG_R5,
    T_MIPS_REG_R6,
    T_MIPS_REG_R7,
    T_MIPS_REG_R8,
    T_MIPS_REG_R9,
    T_MIPS_REG_R10,
    T_MIPS_REG_R11,
    T_MIPS_REG_R12,
    T_MIPS_REG_R13,
    T_MIPS_REG_R14,
    T_MIPS_REG_R15,
    T_MIPS_REG_R16,
    T_MIPS_REG_R17,
    T_MIPS_REG_R18,
    T_MIPS_REG_R19,
    T_MIPS_REG_R20,
    T_MIPS_REG_R21,
    T_MIPS_REG_R22,
    T_MIPS_REG_R23,
    T_MIPS_REG_R24,
    T_MIPS_REG_R25,
    T_MIPS_REG_R26,
    T_MIPS_REG_R27,
    T_MIPS_REG_R28,
    T_MIPS_REG_R29,
    T_MIPS_REG_R30,
    T_MIPS_REG_R31,

    T_MIPS_MIPS32R1,
    T_MIPS_MIPS32R2,
} ETargetToken;

extern void
mips_DefineTokens(void);

#endif
