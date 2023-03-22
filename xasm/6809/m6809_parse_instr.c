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

#include <assert.h>
#include <stdbool.h>

#include "expression.h"
#include "lexer.h"
#include "parse.h"
#include "errors.h"

#include "m6809_errors.h"
#include "m6809_parse.h"
#include "m6809_tokens.h"

#define PAGE1 0x10
#define PAGE2 0x11

#define MODE_P	(MODE_IMMEDIATE | MODE_ADDRESS | MODE_DIRECT | MODE_EXTENDED | MODE_ALL_INDEXED)
#define MODE_Q	(MODE_ADDRESS | MODE_DIRECT | MODE_EXTENDED | MODE_ALL_INDEXED)

uint16_t g_dp_base = 0x0000;

typedef struct Parser {
    uint8_t baseOpcode;
    uint32_t allowedModes;
    bool (*handler)(uint8_t baseOpcode, SAddressingMode* addrMode);
} SParser;


static bool
emitOpcode(uint8_t baseOpcode, SAddressingMode* addrMode, uint8_t directCode, uint8_t extendedCode, uint8_t indexedCode) {
	if (addrMode->mode == MODE_DIRECT) {
		sect_OutputConst8(baseOpcode | directCode);
		sect_OutputExpr8(addrMode->expr);
		return true;
	} else if (addrMode->mode == MODE_EXTENDED) {
		sect_OutputConst8(baseOpcode | extendedCode);
		sect_OutputExpr16(addrMode->expr);
		return true;
	} else if (addrMode->mode & MODE_ALL_INDEXED) {
		sect_OutputConst8(baseOpcode | indexedCode);
		sect_OutputConst8(addrMode->indexed_post_byte);
		if (addrMode->mode & MODE_INDEXED_R_8BIT) {
			sect_OutputExpr8(addrMode->expr);
			return true;
		} else if (addrMode->mode & (MODE_INDEXED_R_16BIT | MODE_EXTENDED_INDIRECT)) {
			sect_OutputExpr16(addrMode->expr);
			return true;
		} else if (addrMode->mode & MODE_INDEXED_PC_8BIT) {
			sect_OutputExpr8(expr_PcRelative(addrMode->expr, -1));
			return true;
		} else if (addrMode->mode & MODE_INDEXED_PC_16BIT) {
			sect_OutputExpr16(expr_PcRelative(addrMode->expr, -2));
			return true;
		}
	}
	return false;
}


static bool
handleOpcodeQ(uint8_t baseOpcode, SAddressingMode* addrMode) {
	return emitOpcode(baseOpcode, addrMode, 0x00, 0x70, 0x60);
}


static bool
handleOpcodeP8(uint8_t baseOpcode, SAddressingMode* addrMode) {
	if (addrMode->mode == MODE_IMMEDIATE) {
		sect_OutputConst8(baseOpcode);
		sect_OutputExpr8(addrMode->expr);
		return true;
	}
	
	return emitOpcode(baseOpcode, addrMode, 0x10, 0x30, 0x20);
}

static bool
handleOpcodeP16(uint8_t baseOpcode, SAddressingMode* addrMode) {
	if (addrMode->mode == MODE_IMMEDIATE) {
		sect_OutputConst8(baseOpcode);
		sect_OutputExpr16(addrMode->expr);
		return true;
	}
	
	return emitOpcode(baseOpcode, addrMode, 0x10, 0x30, 0x20);
}

static bool
handleOpcodeP16Page1(uint8_t baseOpcode, SAddressingMode* addrMode) {
	sect_OutputConst8(PAGE1);
	return handleOpcodeP16(baseOpcode, addrMode);
}

static bool
handleOpcodeP16Page2(uint8_t baseOpcode, SAddressingMode* addrMode) {
	sect_OutputConst8(PAGE2);
	return handleOpcodeP16(baseOpcode, addrMode);
}

static bool
handleImplied(uint8_t baseOpcode, SAddressingMode* addrMode) {
    sect_OutputConst8(baseOpcode);
    return true;
}

static bool
handleANDCC(uint8_t baseOpcode, SAddressingMode* addrMode) {
    sect_OutputConst8(baseOpcode);
	sect_OutputExpr8(addrMode->expr);
    return true;
}

static bool
handleBcc(uint8_t baseOpcode, SAddressingMode* addrMode) {
    sect_OutputConst8(baseOpcode);
	sect_OutputExpr8(expr_PcRelative(addrMode->expr, -1));
    return true;
}

static bool
handleLBcc(uint8_t baseOpcode, SAddressingMode* addrMode) {
    sect_OutputConst8(PAGE1);
    sect_OutputConst8(baseOpcode);
	sect_OutputExpr16(expr_PcRelative(addrMode->expr, -2));
    return true;
}

static bool
handleLBRA(uint8_t baseOpcode, SAddressingMode* addrMode) {
    sect_OutputConst8(baseOpcode);
	sect_OutputExpr16(expr_PcRelative(addrMode->expr, -2));
    return true;
}

static SParser g_instructionHandlers[T_6809_NOP - T_6809_ABX + 1] = {
    { 0x3A, MODE_NONE, handleImplied },	/* ABX */
    { 0x89, MODE_P, handleOpcodeP8 },	/* ADCA */
    { 0xC9, MODE_P, handleOpcodeP8 },	/* ADCB */
    { 0x8B, MODE_P, handleOpcodeP8 },	/* ADDA */
    { 0xCB, MODE_P, handleOpcodeP8 },	/* ADDB */
    { 0xC3, MODE_P, handleOpcodeP16 },	/* ADDD */
    { 0x84, MODE_P, handleOpcodeP8 },	/* ANDA */
    { 0xC4, MODE_P, handleOpcodeP8 },	/* ANDB */
    { 0x1C, MODE_IMMEDIATE, handleANDCC },	/* ANDCC */
    { 0x08, MODE_Q, handleOpcodeQ },	/* ASL */
    { 0x48, MODE_NONE, handleImplied },	/* ASLA */
    { 0x58, MODE_NONE, handleImplied },	/* ASLB */
    { 0x07, MODE_Q, handleOpcodeQ },	/* ASR */
    { 0x47, MODE_NONE, handleImplied },	/* ASRA */
    { 0x57, MODE_NONE, handleImplied },	/* ASRB */

	{ 0x20, MODE_ADDRESS, handleBcc }, 	/* BRA */
	{ 0x21, MODE_ADDRESS, handleBcc }, 	/* BRN */
	{ 0x22, MODE_ADDRESS, handleBcc }, 	/* BHI */
	{ 0x23, MODE_ADDRESS, handleBcc }, 	/* BLS */
	{ 0x24, MODE_ADDRESS, handleBcc }, 	/* BHS */
	{ 0x25, MODE_ADDRESS, handleBcc }, 	/* BLO */
	{ 0x26, MODE_ADDRESS, handleBcc }, 	/* BNE */
	{ 0x27, MODE_ADDRESS, handleBcc }, 	/* BEQ */
	{ 0x28, MODE_ADDRESS, handleBcc }, 	/* BVC */
	{ 0x29, MODE_ADDRESS, handleBcc }, 	/* BVS */
	{ 0x2A, MODE_ADDRESS, handleBcc }, 	/* BPL */
	{ 0x2B, MODE_ADDRESS, handleBcc }, 	/* BMI */
	{ 0x2C, MODE_ADDRESS, handleBcc }, 	/* BGE */
	{ 0x2D, MODE_ADDRESS, handleBcc }, 	/* BLT */
	{ 0x2E, MODE_ADDRESS, handleBcc }, 	/* BGT */
	{ 0x2F, MODE_ADDRESS, handleBcc }, 	/* BLE */
	{ 0x8D, MODE_ADDRESS, handleBcc }, 	/* BSR */

	{ 0x16, MODE_ADDRESS, handleLBRA }, 	/* LBRA */
	{ 0x21, MODE_ADDRESS, handleLBcc }, 	/* LBRN */
	{ 0x22, MODE_ADDRESS, handleLBcc }, 	/* LBHI */
	{ 0x23, MODE_ADDRESS, handleLBcc }, 	/* LBLS */
	{ 0x24, MODE_ADDRESS, handleLBcc }, 	/* LBHS */
	{ 0x25, MODE_ADDRESS, handleLBcc }, 	/* LBLO */
	{ 0x26, MODE_ADDRESS, handleLBcc }, 	/* LBNE */
	{ 0x27, MODE_ADDRESS, handleLBcc }, 	/* LBEQ */
	{ 0x28, MODE_ADDRESS, handleLBcc }, 	/* LBVC */
	{ 0x29, MODE_ADDRESS, handleLBcc }, 	/* LBVS */
	{ 0x2A, MODE_ADDRESS, handleLBcc }, 	/* LBPL */
	{ 0x2B, MODE_ADDRESS, handleLBcc }, 	/* LBMI */
	{ 0x2C, MODE_ADDRESS, handleLBcc }, 	/* LBGE */
	{ 0x2D, MODE_ADDRESS, handleLBcc }, 	/* LBLT */
	{ 0x2E, MODE_ADDRESS, handleLBcc }, 	/* LBGT */
	{ 0x2F, MODE_ADDRESS, handleLBcc }, 	/* LBLE */
	{ 0x17, MODE_ADDRESS, handleLBRA }, 	/* LBSR */

    { 0x85, MODE_P, handleOpcodeP8 },	/* BITA */
    { 0xC5, MODE_P, handleOpcodeP8 },	/* BITB */

    { 0x0F, MODE_Q, handleOpcodeQ },	/* CLR */
    { 0x4F, MODE_NONE, handleImplied },	/* CLRA */
    { 0x5F, MODE_NONE, handleImplied },	/* CLRB */


	{ 0x81, MODE_P, handleOpcodeP8 },		/* CMPA */
	{ 0xC1, MODE_P, handleOpcodeP8 },		/* CMPB */
	{ 0x83, MODE_P, handleOpcodeP16Page1 },	/* CMPD */
	{ 0x8C, MODE_P, handleOpcodeP16 },		/* CMPX */
	{ 0x8C, MODE_P, handleOpcodeP16Page1 },	/* CMPY */
	{ 0x83, MODE_P, handleOpcodeP16Page2 },	/* CMPU */
	{ 0x8C, MODE_P, handleOpcodeP16Page2 },	/* CMPS */


    { 0x12, MODE_NONE, handleImplied }, /* NOP */
};

bool
m6809_ParseIntegerInstruction(void) {
    if (T_6809_ABX <= lex_Context->token.id && lex_Context->token.id <= T_6809_NOP) {
        SAddressingMode addrMode;
        ETargetToken token = (ETargetToken) lex_Context->token.id;
        SParser* handler = &g_instructionHandlers[token - T_6809_ABX];

        parse_GetToken();
        if (m6809_ParseAddressingMode(&addrMode, handler->allowedModes)) {
			if (addrMode.mode == MODE_ADDRESS) {
				if (expr_IsConstant(addrMode.expr) && (addrMode.expr->value.integer & 0xFF00) == g_dp_base) {
					addrMode.mode = MODE_DIRECT;
				} else {
					addrMode.mode = MODE_EXTENDED;
				}
			}
            return handler->handler(handler->baseOpcode, &addrMode);
		} else {
            err_Error(MERROR_ILLEGAL_ADDRMODE);
		}
    }

    return false;
}
