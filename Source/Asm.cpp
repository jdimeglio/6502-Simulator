/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

#include "StdAfx.h"

//-----------------------------------------------------------------------------

const TCHAR CAsm::LOCAL_LABEL_CHAR = _T('.');	// znak rozpoczynaj¹cy etykietê lokaln¹
const TCHAR CAsm::MULTIPARAM[]= _T("...");	// wielokropek - dowolna iloœæ parametrów

//-----------------------------------------------------------------------------
static const UINT8 NA= 0xBB;

		// zamiana rozkazu w danym trybie adresowania na kod (65XX)
const UINT8 CAsm::trans_table[C_ILL][A_NO_OF_MODES]=
{
// IMP   ACC   IMM   ZPG   ABS  ABS_X ABS_Y ZPG_X ZPG_Y  REL  ZPGI ZPGI_X ZPGI_Y  ABSI ABSI_X ZREL ZPG2
  NA,   NA  , 0xA9, 0xA5, 0xAD, 0xBD, 0xB9, 0xB5, NA  , NA  , NA  , 0xA1,  0xB1,  NA  , NA  , NA, NA, // C_LDA
  NA  , NA  , 0xA2, 0xA6, 0xAE, NA  , 0xBE, NA  , 0xB6, NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LDX
  NA  , NA  , 0xA0, 0xA4, 0xAC, 0xBC, NA  , 0xB4, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LDY
  NA  , NA  , NA  , 0x85, 0x8D, 0x9D, 0x99, 0x95, NA  , NA  , NA  , 0x81,  0x91,  NA  , NA  , NA, NA, // C_STA
  NA  , NA  , NA  , 0x86, 0x8E, NA  , NA  , NA  , 0x96, NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STX
  NA  , NA  , NA  , 0x84, 0x8C, NA  , NA  , 0x94, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STY
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STZ
  0xAA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TAX
  0x8A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TXA
  0xA8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TAY
  0x98, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TYA
  0x9A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TXS
  0xBA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TSX
  NA  , NA  , 0x69, 0x65, 0x6D, 0x7D, 0x79, 0x75, NA  , NA  , NA  , 0x61,  0x71,  NA  , NA  , NA, NA, // C_ADC
  NA  , NA  , 0xE9, 0xE5, 0xED, 0xFD, 0xF9, 0xF5, NA  , NA  , NA  , 0xE1,  0xF1,  NA  , NA  , NA, NA, // C_SBC
  NA  , NA  , 0xC9, 0xC5, 0xCD, 0xDD, 0xD9, 0xD5, NA  , NA  , NA  , 0xC1,  0xD1,  NA  , NA  , NA, NA, // C_CMP
  NA  , NA  , 0xE0, 0xE4, 0xEC, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CPX
  NA  , NA  , 0xC0, 0xC4, 0xCC, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CPY
  NA  , NA  , NA  , 0xE6, 0xEE, 0xFE, NA  , 0xF6, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INC
  NA  , NA  , NA  , 0xC6, 0xCE, 0xDE, NA  , 0xD6, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEC
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INA
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEA
  0xE8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INX
  0xCA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEX
  0xC8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INY
  0x88, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEY
  NA  , 0x0A, NA  , 0x06, 0x0E, 0x1E, NA  , 0x16, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ASL
  NA  , 0x4A, NA  , 0x46, 0x4E, 0x5E, NA  , 0x56, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LSR
  NA  , 0x2A, NA  , 0x26, 0x2E, 0x3E, NA  , 0x36, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ROL
  NA  , 0x6A, NA  , 0x66, 0x6E, 0x7E, NA  , 0x76, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ROR
  NA  , NA  , 0x29, 0x25, 0x2D, 0x3D, 0x39, 0x35, NA  , NA  , NA  , 0x21,  0x31,  NA  , NA  , NA, NA, // C_AND
  NA  , NA  , 0x09, 0x05, 0x0D, 0x1D, 0x19, 0x15, NA  , NA  , NA  , 0x01,  0x11,  NA  , NA  , NA, NA, // C_ORA
  NA  , NA  , 0x49, 0x45, 0x4D, 0x5D, 0x59, 0x55, NA  , NA  , NA  , 0x41,  0x51,  NA  , NA  , NA, NA, // C_EOR
  NA  , NA  , NA  , 0x24, 0x2C, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BIT
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TSB
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TRB
  NA  , NA  , NA  , NA  , 0x4C, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  0x6C, NA  , NA, NA, // C_JMP
  NA  , NA  , NA  , NA  , 0x20, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_JSR
  0x00, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BRK
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BRA
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x10, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BPL
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x30, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BMI
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x50, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BVC
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x70, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BVS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x90, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BCC
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xB0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BCS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xD0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BNE
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xF0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BEQ
  0x60, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_RTS
  0x40, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_RTI
  0x48, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHA
  0x68, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLA
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHX
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLX
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHY
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLY
  0x08, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHP
  0x28, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLP
  0x18, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLC
  0x38, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SEC
  0xB8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLV
  0xD8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLD
  0xF8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SED
  0x58, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLI
  0x78, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SEI
  0xEA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_NOP
  NA  ,	NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BBR
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BBS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_RMB
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA  // C_SMB
};

//-----------------------------------------------------------------------------

		// zamiana rozkazu w danym trybie adresowania na kod (65C02)
const UINT8 CAsm::trans_table_c[C_ILL][A_NO_OF_MODES]=
{
// IMP   ACC   IMM   ZPG   ABS  ABS_X ABS_Y ZPG_X ZPG_Y  REL  ZPGI ZPGI_X ZPGI_Y  ABSI ABSI_X ZREL ZPG2
  NA  , NA  , 0xA9, 0xA5, 0xAD, 0xBD, 0xB9, 0xB5, NA  , NA  , 0xB2, 0xA1,  0xB1,  NA  , NA  , NA, NA, // C_LDA
  NA  , NA  , 0xA2, 0xA6, 0xAE, NA  , 0xBE, NA  , 0xB6, NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LDX
  NA  , NA  , 0xA0, 0xA4, 0xAC, 0xBC, NA  , 0xB4, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LDY
  NA  , NA  , NA  , 0x85, 0x8D, 0x9D, 0x99, 0x95, NA  , NA  , 0x92, 0x81,  0x91,  NA  , NA  , NA, NA, // C_STA
  NA  , NA  , NA  , 0x86, 0x8E, NA  , NA  , NA  , 0x96, NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STX
  NA  , NA  , NA  , 0x84, 0x8C, NA  , NA  , 0x94, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STY
  NA  , NA  , NA  , 0x64, 0x9C, 0x9E, NA  , 0x74, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_STZ
  0xAA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TAX
  0x8A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TXA
  0xA8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TAY
  0x98, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TYA
  0x9A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TXS
  0xBA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TSX
  NA  , NA  , 0x69, 0x65, 0x6D, 0x7D, 0x79, 0x75, NA  , NA  , 0x72, 0x61,  0x71,  NA  , NA  , NA, NA, // C_ADC
  NA  , NA  , 0xE9, 0xE5, 0xED, 0xFD, 0xF9, 0xF5, NA  , NA  , 0xF2, 0xE1,  0xF1,  NA  , NA  , NA, NA, // C_SBC
  NA  , NA  , 0xC9, 0xC5, 0xCD, 0xDD, 0xD9, 0xD5, NA  , NA  , 0xD2, 0xC1,  0xD1,  NA  , NA  , NA, NA, // C_CMP
  NA  , NA  , 0xE0, 0xE4, 0xEC, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CPX
  NA  , NA  , 0xC0, 0xC4, 0xCC, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CPY
  NA  , NA  , NA  , 0xE6, 0xEE, 0xFE, NA  , 0xF6, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INC
  NA  , NA  , NA  , 0xC6, 0xCE, 0xDE, NA  , 0xD6, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEC
  NA  , 0x1A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INA
  NA  , 0x3A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEA
  0xE8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INX
  0xCA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEX
  0xC8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_INY
  0x88, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_DEY
  NA  , 0x0A, NA  , 0x06, 0x0E, 0x1E, NA  , 0x16, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ASL
  NA  , 0x4A, NA  , 0x46, 0x4E, 0x5E, NA  , 0x56, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_LSR
  NA  , 0x2A, NA  , 0x26, 0x2E, 0x3E, NA  , 0x36, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ROL
  NA  , 0x6A, NA  , 0x66, 0x6E, 0x7E, NA  , 0x76, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_ROR
  NA  , NA  , 0x29, 0x25, 0x2D, 0x3D, 0x39, 0x35, NA  , NA  , 0x32, 0x21,  0x31,  NA  , NA  , NA, NA, // C_AND
  NA  , NA  , 0x09, 0x05, 0x0D, 0x1D, 0x19, 0x15, NA  , NA  , 0x12, 0x01,  0x11,  NA  , NA  , NA, NA, // C_ORA
  NA  , NA  , 0x49, 0x45, 0x4D, 0x5D, 0x59, 0x55, NA  , NA  , 0x52, 0x41,  0x51,  NA  , NA  , NA, NA, // C_EOR
  NA  , NA  , 0x89, 0x24, 0x2C, 0x3C, NA  , 0x34, NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BIT
  NA  , NA  , NA  , 0x04, 0x0C, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TSB
  NA  , NA  , NA  , 0x14, 0x1C, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_TRB
  NA  , NA  , NA  , NA  , 0x4C, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  0x6C, 0x7C, NA, NA, // C_JMP
  NA  , NA  , NA  , NA  , 0x20, NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_JSR
  0x00, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BRK
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x80, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BRA
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x10, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BPL
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x30, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BMI
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x50, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BVC
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x70, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BVS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0x90, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BCC
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xB0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BCS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xD0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BNE
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , 0xF0, NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_BEQ
  0x60, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_RTS
  0x40, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_RTI
  0x48, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHA
  0x68, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLA
  0xDA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHX
  0xFA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLX
  0x5A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHY
  0x7A, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLY
  0x08, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PHP
  0x28, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_PLP
  0x18, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLC
  0x38, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SEC
  0xB8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLV
  0xD8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLD
  0xF8, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SED
  0x58, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_CLI
  0x78, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_SEI
  0xEA, NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, NA, // C_NOP
  NA  ,	NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , 0x0F,NA,// C_BBR
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , 0x8F,NA,// C_BBS
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, 0x07,// C_RMB
  NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  , NA  ,  NA  ,  NA  , NA  , NA, 0x87 // C_SMB
};

//-----------------------------------------------------------------------------

  // tablica transformacji kodu rozkazu na odpowiadaj¹cy mu tryb adresowania 6502
const UINT8 CAsm::code_to_mode[]=
{// x0     x1	     x2     x3     x4      x5       x6       x7     x8     x9       xA     xB     xC      xD 	   xE	    xF
  A_IMP2,A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ILL,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
  A_ABS, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
  A_IMP, A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
  A_IMP, A_ZPGI_X, A_ILL, A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABSI, A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
  A_ILL, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_ILL,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ILL, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ILL,  A_ABS_X, A_ILL,   A_ILL,
  A_IMM, A_ZPGI_X, A_IMM, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ILL, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ABS_X,A_ABS_X, A_ABS_Y, A_ILL,
  A_IMM, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL,
  A_IMM, A_ZPGI_X, A_ILL, A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ILL, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,  A_ABS,   A_ABS,   A_ILL,
  A_REL, A_ZPGI_Y, A_ILL, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ILL, A_IMP, A_ABS_Y, A_ILL, A_ILL, A_ILL,  A_ABS_X, A_ABS_X, A_ILL
};

  // j.w. dla 65c02
const UINT8 CAsm::code_to_mode_c[]=
{// x0     x1	     x2     x3     x4      x5       x6       x7      x8     x9       xA     xB     xC       xD 	     xE       xF
  A_IMP2,A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ZPG,  A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_ACC, A_ILL, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,
  A_ABS, A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_ACC, A_ILL, A_ABS_X, A_ABS_X, A_ABS_X, A_ZREL,
  A_IMP, A_ZPGI_X, A_ILL,  A_ILL, A_ILL,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ILL,   A_ABS_X, A_ABS_X, A_ZREL,
  A_IMP, A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_ACC, A_ILL, A_ABSI,  A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ABSI_X,A_ABS_X, A_ABS_X, A_ZREL,
  A_REL, A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ABS,   A_ABS_X, A_ABS_X, A_ZREL,
  A_IMM, A_ZPGI_X, A_IMM,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ZPG_X,A_ZPG_X, A_ZPG_Y, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ABS_X, A_ABS_X, A_ABS_Y, A_ZREL,
  A_IMM, A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ILL,   A_ABS_X, A_ABS_X, A_ZREL,
  A_IMM, A_ZPGI_X, A_ILL,  A_ILL, A_ZPG,  A_ZPG,   A_ZPG,   A_ZPG2, A_IMP, A_IMM,   A_IMP, A_ILL, A_ABS,   A_ABS,   A_ABS,   A_ZREL,
  A_REL, A_ZPGI_Y, A_ZPGI, A_ILL, A_ILL,  A_ZPG_X, A_ZPG_X, A_ZPG2, A_IMP, A_ABS_Y, A_IMP, A_ILL, A_ILL,   A_ABS_X, A_ABS_X, A_ZREL
};


  // tablica transformacji kodu rozkazu na odpowiadaj¹cy mu rozkaz (dzia³anie) procesora 6502
const UINT8 CAsm::code_to_command[]=
{// x0     x1	  x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF
  C_BRK, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, C_PHP, C_ORA, C_ASL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, // 0
  C_BPL, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, C_CLC, C_ORA, C_ILL, C_ILL, C_ILL, C_ORA, C_ASL, C_ILL, // 1
  C_JSR, C_AND, C_ILL, C_ILL, C_BIT, C_AND, C_ROL, C_ILL, C_PLP, C_AND, C_ROL, C_ILL, C_BIT, C_AND, C_ROL, C_ILL, // 2
  C_BMI, C_AND, C_ILL, C_ILL, C_ILL, C_AND, C_ROL, C_ILL, C_SEC, C_AND, C_ILL, C_ILL, C_ILL, C_AND, C_ROL, C_ILL, // 3
  C_RTI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, C_PHA, C_EOR, C_LSR, C_ILL, C_JMP, C_EOR, C_LSR, C_ILL, // 4
  C_BVC, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, C_CLI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_ILL, // 5
  C_RTS, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, C_PLA, C_ADC, C_ROR, C_ILL, C_JMP, C_ADC, C_ROR, C_ILL, // 6
  C_BVS, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, C_SEI, C_ADC, C_ILL, C_ILL, C_ILL, C_ADC, C_ROR, C_ILL, // 7

  C_ILL, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_ILL, C_DEY, C_ILL, C_TXA, C_ILL, C_STY, C_STA, C_STX, C_ILL, // 8
  C_BCC, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_ILL, C_TYA, C_STA, C_TXS, C_ILL, C_ILL, C_STA, C_ILL, C_ILL, // 9
  C_LDY, C_LDA, C_LDX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, C_TAY, C_LDA, C_TAX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, // A
  C_BCS, C_LDA, C_ILL, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, C_CLV, C_LDA, C_TSX, C_ILL, C_LDY, C_LDA, C_LDX, C_ILL, // B
  C_CPY, C_CMP, C_ILL, C_ILL, C_CPY, C_CMP, C_DEC, C_ILL, C_INY, C_CMP, C_DEX, C_ILL, C_CPY, C_CMP, C_DEC, C_ILL, // C
  C_BNE, C_CMP, C_ILL, C_ILL, C_ILL, C_CMP, C_DEC, C_ILL, C_CLD, C_CMP, C_ILL, C_ILL, C_ILL, C_CMP, C_DEC, C_ILL, // D
  C_CPX, C_SBC, C_ILL, C_ILL, C_CPX, C_SBC, C_INC, C_ILL, C_INX, C_SBC, C_NOP, C_ILL, C_CPX, C_SBC, C_INC, C_ILL, // E
  C_BEQ, C_SBC, C_ILL, C_ILL, C_ILL, C_SBC, C_INC, C_ILL, C_SED, C_SBC, C_ILL, C_ILL, C_ILL, C_SBC, C_INC, C_ILL  // F
};

  // j.w. dla 65c02
const UINT8 CAsm::code_to_command_c[]=
{// x0     x1	  x2     x3     x4     x5     x6     x7     x8     x9     xA     xB     xC     xD     xE     xF
  C_BRK, C_ORA, C_ILL, C_ILL, C_TSB, C_ORA, C_ASL, C_RMB, C_PHP, C_ORA, C_ASL, C_ILL, C_TSB, C_ORA, C_ASL, C_BBR,
  C_BPL, C_ORA, C_ORA, C_ILL, C_TRB, C_ORA, C_ASL, C_RMB, C_CLC, C_ORA, C_INA, C_ILL, C_TRB, C_ORA, C_ASL, C_BBR,
  C_JSR, C_AND, C_ILL, C_ILL, C_BIT, C_AND, C_ROL, C_RMB, C_PLP, C_AND, C_ROL, C_ILL, C_BIT, C_AND, C_ROL, C_BBR,
  C_BMI, C_AND, C_AND, C_ILL, C_BIT, C_AND, C_ROL, C_RMB, C_SEC, C_AND, C_DEA, C_ILL, C_BIT, C_AND, C_ROL, C_BBR,
  C_RTI, C_EOR, C_ILL, C_ILL, C_ILL, C_EOR, C_LSR, C_RMB, C_PHA, C_EOR, C_LSR, C_ILL, C_JMP, C_EOR, C_LSR, C_BBR,
  C_BVC, C_EOR, C_EOR, C_ILL, C_ILL, C_EOR, C_LSR, C_RMB, C_CLI, C_EOR, C_PHY, C_ILL, C_ILL, C_EOR, C_LSR, C_BBR,
  C_RTS, C_ADC, C_ILL, C_ILL, C_STZ, C_ADC, C_ROR, C_RMB, C_PLA, C_ADC, C_ROR, C_ILL, C_JMP, C_ADC, C_ROR, C_BBR,
  C_BVS, C_ADC, C_ADC, C_ILL, C_STZ, C_ADC, C_ROR, C_RMB, C_SEI, C_ADC, C_PLY, C_ILL, C_JMP, C_ADC, C_ROR, C_BBR,

  C_BRA, C_STA, C_ILL, C_ILL, C_STY, C_STA, C_STX, C_SMB, C_DEY, C_BIT, C_TXA, C_ILL, C_STY, C_STA, C_STX, C_BBS,
  C_BCC, C_STA, C_STA, C_ILL, C_STY, C_STA, C_STX, C_SMB, C_TYA, C_STA, C_TXS, C_ILL, C_STZ, C_STA, C_STZ, C_BBS,
  C_LDY, C_LDA, C_LDX, C_ILL, C_LDY, C_LDA, C_LDX, C_SMB, C_TAY, C_LDA, C_TAX, C_ILL, C_LDY, C_LDA, C_LDX, C_BBS,
  C_BCS, C_LDA, C_LDA, C_ILL, C_LDY, C_LDA, C_LDX, C_SMB, C_CLV, C_LDA, C_TSX, C_ILL, C_LDY, C_LDA, C_LDX, C_BBS,
  C_CPY, C_CMP, C_ILL, C_ILL, C_CPY, C_CMP, C_DEC, C_SMB, C_INY, C_CMP, C_DEX, C_ILL, C_CPY, C_CMP, C_DEC, C_BBS,
  C_BNE, C_CMP, C_CMP, C_ILL, C_ILL, C_CMP, C_DEC, C_SMB, C_CLD, C_CMP, C_PHX, C_ILL, C_ILL, C_CMP, C_DEC, C_BBS,
  C_CPX, C_SBC, C_ILL, C_ILL, C_CPX, C_SBC, C_INC, C_SMB, C_INX, C_SBC, C_NOP, C_ILL, C_CPX, C_SBC, C_INC, C_BBS,
  C_BEQ, C_SBC, C_SBC, C_ILL, C_ILL, C_SBC, C_INC, C_SMB, C_SED, C_SBC, C_PLX, C_ILL, C_ILL, C_SBC, C_INC, C_BBS
};



const UINT8 CAsm::mode_to_len[]=	// zamiana trybu adresowania na d³ugoœæ rozkazu i argumentów
{
1,	// A_IMP,	 implied
1,	// A_ACC,	 accumulator
2,	// A_IMM,	 immediate
2,	// A_ZPG,	 zero page
3,	// A_ABS,	 absolute
3,	// A_ABS_X,	 absolute indexed X
3,	// A_ABS_Y,	 absolute indexed Y
2,	// A_ZPG_X,	 zero page indexed X
2,	// A_ZPG_Y,	 zero page indexed Y
2,	// A_REL,	 relative
2,	// A_ZPGI,	 zero page indirect
2,	// A_ZPGI_X,	 zero page indirect, indexed X
2,	// A_ZPGI_Y,	 zero page indirect, indexed Y
3,	// A_ABSI,	 absolute indirect
3,	// A_ABSI_X,	 absolute indirect, indexed X
3,	// A_ZREL,	 zero page / relative -> BBS i BBR z 6501
2,	// A_ZPG2,	 zero page dla rozkazów RMB SMB z 6501
0,	// A_NO_OF_MODES,	 iloœæ trybów adresowania
	// A_ABS_OR_ZPG= A_NO_OF_MODES,	 niezdeterminowany tryb adresowania
0,	// A_ABSX_OR_ZPGX,
0,	// A_ABSY_OR_ZPGY,
0,	// A_ABSI_OR_ZPGI,
0,	// A_IMP_OR_ACC,
0,	// A_ABSIX_OR_ZPGIX,
2,	// A_IMP2,	 implied dla rozkazu BRK
1	// A_ILL	 wartoœæ do oznaczania nielegalnych rozkazów w symulatorze (ILLEGAL)
};


const UINT8 CAsm::code_cycles[256]=
{
//0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
  7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,   // 0
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0,   // 1
  6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,   // 2
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0,   // 3
  6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,   // 4
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0,   // 5
  6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,   // 6
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0,   // 7
  0, 6, 0, 0, 0, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,   // 8
  2, 6, 0, 0, 0, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,   // 9
  2, 6, 2, 0, 0, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,   // a
  2, 5, 0, 0, 0, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,   // b
  2, 6, 0, 0, 0, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,   // c
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0,   // d
  2, 6, 0, 0, 0, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,   // e
  2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 5, 7, 0    // f
};
const UINT8 CAsm::code_cycles_c[256]=
{
//0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
  7, 6, 0, 0, 3, 3, 5, 0, 3, 2, 2, 0, 6, 4, 6, 0,
  2, 5, 2, 0, 4, 4, 6, 0, 2, 4, 2, 0, 6, 5, 7, 0,
  6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
  2, 5, 2, 0, 4, 4, 6, 0, 2, 4, 2, 0, 5, 5, 7, 0,
  6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
  2, 5, 2, 0, 0, 4, 6, 0, 2, 4, 2, 0, 0, 5, 7, 0,
  6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
  2, 5, 2, 0, 4, 4, 6, 0, 2, 4, 2, 0, 6, 5, 7, 0,
  2, 6, 0, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
  2, 6, 2, 0, 4, 4, 4, 0, 2, 5, 2, 0, 5, 5, 5, 0,
  2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
  2, 5, 2, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
  2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
  2, 5, 2, 0, 0, 4, 6, 0, 2, 4, 2, 0, 0, 5, 7, 0,
  2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
  2, 5, 2, 0, 0, 4, 6, 0, 2, 4, 2, 0, 0, 5, 7, 0
};


extern C6502App theApp;

const UINT8 (&CAsm::TransformTable(const bool bProc6502))[C_ILL][A_NO_OF_MODES]
{
//  return theApp.m_global.m_bProc6502 ? trans_table : trans_table_c;
  if (bProc6502)
    return trans_table;
  else
    return trans_table_c;
}


const UINT8 (&CAsm::CodeToCommand())[0x100]
{
  return CodeToCommand(theApp.m_global.m_bProc6502);
}
const UINT8 (&CAsm::CodeToCommand(const bool bProc6502))[0x100]
{
  if (bProc6502)
    return code_to_command;
  else
    return code_to_command_c;
//  return theApp.m_global.m_bProc6502 != FALSE ? code_to_command : code_to_command_c;
}


const UINT8 (&CAsm::CodeToMode())[0x100]
{
  return CodeToMode(theApp.m_global.m_bProc6502);
}
const UINT8 (&CAsm::CodeToMode(const bool bProc6502))[0x100]
{
  if (bProc6502)
    return code_to_mode;
  else
    return code_to_mode_c;
}


inline bool CAsm::ProcType()
{ 
  return theApp.m_global.m_bProc6502;
}


const UINT8 (&CAsm::CodeToCycles())[0x100]
{
  return CodeToCycles(theApp.m_global.m_bProc6502);
}
const UINT8 (&CAsm::CodeToCycles(const bool bProc6502))[0x100]
{
  if (bProc6502)
    return code_cycles;
  else
    return code_cycles_c;
}
