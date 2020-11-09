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

#include "stdafx.h"
#include "6502View.h"
#include "Deasm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPTSTR s_vpszInstructionList[] =
{
	_T("LDA"),
	_T("LDX"),
	_T("LDY"),
	_T("STA"),
	_T("STX"),
	_T("STY"),
	_T("STZ"),
	_T("TAX"),
	_T("TXA"),
	_T("TAY"),
	_T("TYA"),
	_T("TXS"),
	_T("TSX"),
	_T("ADC"),
	_T("SBC"),
	_T("CMP"),
	_T("CPX"),
	_T("CPY"),
	_T("INC"),
	_T("DEC"),
	_T("INA"),
	_T("DEA"),
	_T("INX"),
	_T("DEX"),
	_T("INY"),
	_T("DEY"),
	_T("ASL"),
	_T("LSR"),
	_T("ROL"),
	_T("ROR"),
	_T("AND"),
	_T("ORA"),
	_T("EOR"),
	_T("BIT"),
	_T("TSB"),
	_T("TRB"),
	_T("JMP"),
	_T("JSR"),
	_T("BRK"),
	_T("BRA"),
	_T("BPL"),
	_T("BMI"),
	_T("BVC"),
	_T("BVS"),
	_T("BCC"),
	_T("BCS"),
	_T("BNE"),
	_T("BEQ"),
	_T("RTS"),
	_T("RTI"),
	_T("PHA"),
	_T("PLA"),
	_T("PHX"),
	_T("PLX"),
	_T("PHY"),
	_T("PLY"),
	_T("PHP"),
	_T("PLP"),
	_T("CLC"),
	_T("SEC"),
	_T("CLV"),
	_T("CLD"),
	_T("SED"),
	_T("CLI"),
	_T("SEI"),
	_T("NOP"),
	_T("BBR"),
	_T("BBS"),
	NULL
};

static LPTSTR s_vpszDirectiveList[]=
{
	".ASCII",
	".ASCIS",
	".BYTE",
	".DB",
	".DBYTE",
	".DCB",
	".DD",
	".DS",
	".DW",
	".ELSE",
	".END",
	".ENDIF",
	".ENDM",
	".ENDR",
	".ERROR",
	".EXITM",
	".IF",
	".INCLUDE",
	".IO_WND",
	".MACRO",
	".OPT",
	".ORG",
	".REPEAT",
	".REPT",
	".ROM_AREA",
	".RS",
	".SET",
	".START",
	".STR",
	".STRING",
	".WORD",
	NULL
};

static LPTSTR s_vpszConstantList[]=
{
	"ORG",
	"IO_AREA",
	0
};


extern BOOL IsMnemonic(LPCTSTR pszChars, int nLength)
{
	if (nLength != 3)
		return false;

	for (int L = 0; s_vpszInstructionList[L] != NULL; L ++)
	{
		if (_tcsnicmp(s_vpszInstructionList[L], pszChars, nLength) == 0)
//				&& s_vpszInstructionList[L][nLength] == 0)
			return true;
	}
	return false;
}


extern bool IsDirective(LPCTSTR pszChars, int nLength)
{
	if (nLength < 2)
		return false;

	if (pszChars[0] != '.')
	{
		for (int L = 0; s_vpszConstantList[L] != NULL; L ++)
		{
			if (_tcsnicmp(s_vpszConstantList[L], pszChars, nLength) == 0
				&& s_vpszConstantList[L][nLength] == 0)
				return true;
		}

		return false;
	}

	for (int L = 0; s_vpszDirectiveList[L] != NULL; L ++)
	{
		if (_tcsnicmp(s_vpszDirectiveList[L], pszChars, nLength) == 0
				&& s_vpszDirectiveList[L][nLength] == 0)
			return true;
	}

	return false;
}


static BOOL Is6502Number(LPCTSTR pszChars, int nLength)
{
	if (nLength > 1) // && (pszChars[0] == '$' || pszChars[0] == '@'))
	{
		if (pszChars[0] == '$')
		{
			for (int i= 1; i < nLength; ++i)
			{
				if (isdigit(pszChars[i]) || (pszChars[i] >= 'A' && pszChars[i] <= 'F') ||
					(pszChars[i] >= 'a' && pszChars[i] <= 'f'))
					continue;
				return false;
			}
		}
		else if (pszChars[0] == '@')	// binary?
		{
			for (int i= 1; i < nLength; ++i)
			{
				if (pszChars[i] == '0' || pszChars[i] == '1')
					continue;
				return false;
			}
		}
		else
		{
			for (int i= 0; i < nLength; ++i)
			{
				if (isdigit(pszChars[i]))
					continue;
				return false;
			}
		}
		return true;
	}

	if (isdigit(pszChars[0]))
		return true;

	return false;
}


#define DEFINE_BLOCK(pos, colorindex)	\
	ASSERT((pos) >= 0 && (pos) <= nLength);\
	if (pBuf != NULL)\
	{\
		if (nActualItems == 0 || pBuf[nActualItems - 1].m_nCharPos <= (pos)){\
		pBuf[nActualItems].m_nCharPos = (pos);\
		pBuf[nActualItems].m_nColorIndex = (colorindex);\
		nActualItems ++;}\
	}

#define COOKIE_COMMENT			0x0001
#define COOKIE_PREPROCESSOR		0x0002
#define COOKIE_EXT_COMMENT		0x0004
#define COOKIE_STRING			0x0008
#define COOKIE_CHAR				0x0010


extern DWORD ParseLine(DWORD dwCookie, LPCTSTR pszChars, int nLength, TEXTBLOCK* pBuf, int& nActualItems)
{
	BOOL bFirstChar= (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
	BOOL bRedefineBlock= TRUE;
	BOOL bDecIndex= FALSE;
	int nIdentBegin= -1;
	bool bSkipChar= false;

	int I= 0;
	for (I = 0; ; I++)
	{
		if (bRedefineBlock)
		{
			int nPos = I;
			if (bDecIndex)
				nPos--;
			if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
			}
			else
			if (dwCookie & (COOKIE_CHAR | COOKIE_STRING))
			{
				DEFINE_BLOCK(nPos, COLORINDEX_STRING);
			}
			else
			if (dwCookie & COOKIE_PREPROCESSOR)
			{
				DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
			}
			else
			{
				DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
			}
			bRedefineBlock = FALSE;
			bDecIndex      = FALSE;
		}

		if (I == nLength)
			break;

		if (bSkipChar)
		{
			bSkipChar = false;
			continue;
		}

		if (dwCookie & COOKIE_COMMENT)
		{
			DEFINE_BLOCK(I, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			break;
		}

		//	String constant "...."
		if (dwCookie & COOKIE_STRING)
		{
			if (pszChars[I] == '"' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_STRING;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Char constant '..'
		if (dwCookie & COOKIE_CHAR)
		{
			if (pszChars[I] == '\'' && (I == 0 || pszChars[I - 1] != '\\'))
			{
				dwCookie &= ~COOKIE_CHAR;
				bRedefineBlock = TRUE;
			}
			continue;
		}

		//	Extended comment /*....*/
/*		if (dwCookie & COOKIE_EXT_COMMENT)
		{
			if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '*')
			{
				dwCookie &= ~COOKIE_EXT_COMMENT;
				bRedefineBlock = TRUE;
			}
			continue;
		}
*/
		if (pszChars[I] == ';')
		{
			DEFINE_BLOCK(I, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			continue;
		}
/*
		if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_COMMENT;
			continue;
//			break;
		}
*/
/*		//	Preprocessor directive #....
		if (dwCookie & COOKIE_PREPROCESSOR)
		{
			if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
			{
				DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
				dwCookie |= COOKIE_EXT_COMMENT;

				// MiK
				bSkipChar = true;
			}
			continue;
		}
*/
		//	Normal text
		if (pszChars[I] == '"')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_STRING;
			continue;
		}
		if (pszChars[I] == '\'')
		{
			DEFINE_BLOCK(I, COLORINDEX_STRING);
			dwCookie |= COOKIE_CHAR;
			continue;
		}
/*		if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
		{
			DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
			dwCookie |= COOKIE_EXT_COMMENT;

			// MiK
			bSkipChar = true;

			continue;
		}
*/
/*		if (bFirstChar)
		{
			if (pszChars[I] == '#')
			{
				DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
				dwCookie |= COOKIE_PREPROCESSOR;
				continue;
			}
			if (! isspace(pszChars[I]))
				bFirstChar = FALSE;
		} */

/*		if (pszChars[I] == '+' || pszChars[I] == '-' || pszChars[I] == '*' || pszChars[I] == '/' ||
			pszChars[I] == '#' || pszChars[I] == '>' || pszChars[I] == '<' || pszChars[I] == '(' || pszChars[I] == ')')
		{
			DEFINE_BLOCK(I, COLORINDEX_OPERATOR);
			continue;
		} */

		if (pBuf == NULL)
			continue;	//	We don't need to extract keywords,
						//	for faster parsing skip the rest of loop

		if (isalnum(pszChars[I]) || pszChars[I] == '_' || pszChars[I] == '.' || pszChars[I] == '$' || pszChars[I] == '@')
		{
			if (nIdentBegin == -1)
				nIdentBegin = I;
		}
		else
		{
			if (nIdentBegin >= 0)
			{
				if (nIdentBegin > 0 && IsMnemonic(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
				}
				else if (IsDirective(pszChars + nIdentBegin, I - nIdentBegin))
				{
					if (nIdentBegin > 0 || pszChars[0] != '.')
					{
						DEFINE_BLOCK(nIdentBegin, COLORINDEX_PREPROCESSOR);
					}
				}
				else if (Is6502Number(pszChars + nIdentBegin, I - nIdentBegin))
				{
					DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
				}
				bRedefineBlock = TRUE;
				bDecIndex = TRUE;
				nIdentBegin = -1;
			}
		}
	}

	if (nIdentBegin >= 0)
	{
		if (nIdentBegin > 0 && IsMnemonic(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
		}
		else if (nIdentBegin > 0 && IsDirective(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_PREPROCESSOR);
		}
		else if (Is6502Number(pszChars + nIdentBegin, I - nIdentBegin))
		{
			DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
		}
	}

//	if (pszChars[nLength - 1] != '\\')
//		dwCookie &= COOKIE_EXT_COMMENT;
	return dwCookie;
}


DWORD CSrc6502View::ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK* pBuf, int& nActualItems)
{
	dwCookie = 0; // no history

	int nLength = GetLineLength(nLineIndex);
	if (nLength <= 0)
	{
		ClearCollapsibleBlockMark(nLineIndex);
		return 0; //dwCookie & COOKIE_EXT_COMMENT;
	}

/*
	if (pszChars[0] > _T(' ') && pszChars[0] != '.' && pszChars[0] != ';')
	{
		MarkCollapsibleBlockLine(nLineIndex, true);
//		if (nLineIndex > 0)
//			MarkCollapsibleBlockLine(nLineIndex - 1, false);
	}
	else
		ClearCollapsibleBlockMark(nLineIndex);
*/

	LPCTSTR pszChars= GetLineChars(nLineIndex);

	return ::ParseLine(dwCookie, pszChars, nLength, pBuf, nActualItems);
}

///////////////////////////////////////////////////////////////////////////////


extern int MatchingDirectives(const CString& strWord, CString& strOut)
{
	strOut.Empty();

	if (strWord.IsEmpty())
		return 0;

	int nMatch= 0;

	if (strWord[0] == _T('.'))
	{
		for (int i= 0; s_vpszDirectiveList[i] != 0; ++i)
		{
			if (_tcsnicmp(s_vpszDirectiveList[i], strWord, strWord.GetLength()) == 0)
			{
				// whole name matches?
				if (strWord.GetLength() == _tcslen(s_vpszDirectiveList[i]))
				{
					strOut = s_vpszDirectiveList[i];
					return 1;
				}

				strOut += s_vpszDirectiveList[i];
				strOut += _T("\n");
				++nMatch;
			}
		}
	}
	else
	{
		for (int i= 0; s_vpszConstantList[i] != 0; ++i)
		{
			if (_tcsnicmp(s_vpszConstantList[i], strWord, strWord.GetLength()) == 0)
			{
				// whole name matches?
				if (strWord.GetLength() == _tcslen(s_vpszConstantList[i]))
				{
					strOut = s_vpszConstantList[i];
					return 1;
				}

				strOut += s_vpszConstantList[i];
				strOut += _T("\n");
				++nMatch;
			}
		}
	}

	if (nMatch == 1)
		strOut.Delete(strOut.GetLength() - 1);

	return nMatch;
}


extern CString GetDirectiveDesc(const CString& strDirective)
{
	if (strDirective.CompareNoCase(_T(".ASCIS")) == 0)
		return _T("#title#.ASCIS#text#\nDirective defining values of single bytes using string argument."
		" Last byte has toggled (XOR-ed) most significant bit to mark end of string."
		"#syntax#[<label>[:]] .ASCIS <expr> | txtexpr { , <expr> | txtexpr }.\n"
		"#exmpl#alpha: .ASCIS \"ABC\" ; generates bytes $41, $42, $C3.\n"
		"beta:  .ASCIS \"Stop\",$D ; generates bytes ‘S’, ‘t’, ‘o’, ‘p’, $8D\n"
		"#desc#.ASCIS directive is helpful to generate string with it's end marked by toggling most significant bit."
		" Printing subroutine for example can use this information to detect the end of string.\n");

	else if (strDirective.CompareNoCase(_T(".ASCII")) == 0 ||
		strDirective.CompareNoCase(_T(".BYTE")) == 0 ||
		strDirective.CompareNoCase(_T(".DB")) == 0)
		return _T("#title#.BYTE .DB .ASCII#text#\nDirectives defining values of single bytes using passed arguments."
		"#syntax#"
			"[<label>[:]] .DB <expr> | txtexpr { , <expr> | txtexpr }\n"
			"[<label>[:]] .BYTE <expr> | txtexpr { , <expr> | txtexpr }\n"
			"[<label>[:]] .ASCII <expr> | txtexpr { , <expr> | txtexpr }\n"
		"#exmpl#"
			"alpha: .DB \"ABC\", 0 ; generates bytes ‘A’, ‘B’, ‘C’, 0\n"
			"beta:  .DB %1, %1$  ; macro params; string length and string itself\n"
			" .BYTE <[alpha-1], >[alpha-1]\n"
			" .ASCII \"Text\"\n"
		"#desc#"
			".BYTE (.DB, .ASCII) directives generates and defines single byte values. Input data might be entered in numerical or string form. Numerical expressions are also accepted.\n");

	else if (strDirective.CompareNoCase(_T(".WORD")) == 0 ||
		strDirective.CompareNoCase(_T(".DW")) == 0)
		return _T("#title#.WORD .DW#text#\nDirectives defining 16-bit word values using passed arguments."
		" Words are written according to low-endian 6502 convention: low byte first, high byte follows"
		"#syntax#"
			"[<label>[:]] .DW expr { , expr }\n"
			"[<label>[:]] .WORD expr { , expr }\n"
		"#exmpl#"
			"alpha: .DW $1234, 0 ; generates sequence $34, $12, 0, 0\n"
			"beta:  .WORD alpha\n"
			"       .WORD alpha-1, beta\n");

	else if (strDirective.CompareNoCase(_T(".DBYTE")) == 0 ||
		strDirective.CompareNoCase(_T(".DD")) == 0)
		return _T("#title#.DBYTE .DD#text#\nDirective defining double byte values."
		" Double bytes numbers are written according to big-endian convention: high byte first, low byte follows."
		"#syntax#"
			"[<label>[:]] .DD expr { , expr }\n"
			"[<label>[:]] .DBYTE expr { , expr }\n"
		"#exmpl#"
			"alpha: .DD $1234, 0 ; generates sequence $12, $34, 0, 0\n"
			"beta:  .DBYTE alpha\n"
			"       .DBYTE alpha-1, beta\n");

	else if (strDirective.CompareNoCase(_T(".STRING")) == 0 ||
		strDirective.CompareNoCase(_T(".STR")) == 0)
		return _T("#title#.STR .STRING#text#\nDirectives defining byte values using passed string argument."
		" First generated byte contains string length. Remaining bytes are verbatim copies of string's characters. String length is limited to 255 characters."
		"#syntax#"
			"[<label>[:]] .STR expr { , expr }\n"
			"[<label>[:]] .STRING expr { , expr }\n"
		"#exmpl#"
			"alpha: .STRING \"ABC\", $D ; generates sequence 4, ‘A’, ‘B’, ‘C’, $0D\n"
			"beta:  .STR \"Test string\", $D, $A\n"
			"       .STR \"AB\", \"CD\", 13 ; generates: 5, ‘A’, ‘B’, ‘C’, ‘D’, $0D\n");

	else if (strDirective.CompareNoCase(_T(".DCB")) == 0)
		return _T("#title#.DCB#text#\nDirective reserving memory area with initialization of reserved memory by given value."
		"#syntax#"
			"[<label>[:]] .DCB count_bytes [ , init_val ]\n"
		"#exmpl#"
			"buf: .DCB $20,$FF  ; next $20 bytes is reserved and set to $FF\n"
		"#desc#"
			".DCB directive reserves ‘count_bytes’ and initializes them with ‘init_val’ (if it's given).\n");

	else if (strDirective.CompareNoCase(_T(".RS")) == 0 ||
		strDirective.CompareNoCase(_T(".DS")) == 0)
		return _T("#title#.RS .DS#text#\nDirectives reserving memory area by adding given value to the pointer of current location."
		"#syntax#"
			"[<label>[:]] .RS expr\n"
		"#exmpl#"
			"buf: .RS $100 ; reserve $100 bytes\n"
			"     .RS size ; same as *= * + size\n"
		"#desc#"
			".RS and .DS move current origin (‘*’, ORG) by specified amount of bytes.\n");

	else if (strDirective.CompareNoCase(_T(".OPT")) == 0)
		return _T("#title#.OPT#text#\nDirective setting assembly options. Available options are:\n\n"
		"\\sa100"
		"Proc6502\t select basic command set of 6502 microprocessor\n"
		"Proc65c02\t select extended command set of 65c02, 6501 and other microprocessors\n"
		"Proc6501\t ditto\n"
		"CaseSensitive\t treat lowercase and uppercase letters in label names as different\n"
		"CaseInsensitive\t treat lowercase and uppercase letters in label names as same"
		"#syntax#"
			"  .OPT option_name { , option_name }\n"
		"#exmpl#"
			"  .OPT Proc65c02, CaseInsensitive\n");

	else if (strDirective.CompareNoCase(_T(".ORG")) == 0 ||
		strDirective.CompareNoCase(_T("*=")) == 0)
		return _T("#title#.ORG  *=#text#\nDirectives setting code generation location."
		"#syntax#"
			"[<label>[:]] .ORG expr\n"
		"#exmpl#"
			"    *= $1000 ; code location: $1000\n"
			"buf: .ORG * + $10 ; offset code location by $10 bytes,\n"
			" ; label ‘buf’ will be set to previous code location address\n"
		"#desc#"
			"Use .ORG directive to designate beginning of an area where assambled code will be generated.\n");

	else if (strDirective.CompareNoCase(_T(".START")) == 0)
		return _T("#title#.START#text#\nDirective setting simulator entry (start) point address."
		"#syntax#"
			"  .START expr\n"
		"#exmpl#"
			"  .START Start ; start program execution from ‘Start’ address\n"
			"  .START $A100 ; start program execution from $A100 address\n"
		"#desc#"
			".START directive selects simulator entry point. Simulator will attempt to launch program using given address."
			" If there is no .START directive used, simulator tries to start execution using address specified by first .ORG directive."
			" .START directive allows using forward referencing (unlike .ORG).\n");

	else if (strDirective.CompareNoCase(_T(".END")) == 0)
		return _T("#title#.END#text#\nDirective finishing source code assembly."
		"#syntax#"
			"[<label>[:]] .END\n"
		"#exmpl#"
			"fin: .END  ; rest of source code in current file won't be assembled\n"
		"#desc#"
			".END directive finishes assembly process of the file it was placed in. Used in main source file finishes assembly at the line it is used in.");

	else if (strDirective.CompareNoCase(_T(".INCLUDE")) == 0)
		return _T("#title#.INCLUDE#text#\nDirective including source file to assembly."
		"#syntax#"
			"  .INCLUDE file_name\n"
		"#exmpl#"
			"  .INCLUDE \"c:\\\\asm6502\\\\const_vals.65s\"\n"
			"  .INCLUDE \".\\\\macros\\\\macros\"\n"
		"#desc#"
			".INCLUDE directive includes given source file. It's useful to include predefined labels or macros.");

	else if (strDirective.CompareNoCase(_T(".MACRO")) == 0)
		return _T("#title#.MACRO#text#\nDirective opening macro definition."
		"#syntax#"
			"<label>[:] .MACRO [param {, param} [, ...]] | [...]"
		"#exmpl#"
			"PushX  .MACRO      ; parameterless macro\n"
			"Print: .MACRO ...  ; macro accepting any number of params\n"
			"Put:   .MACRO chr  ; macro accepting exactly one parameter\n"
			""
		"#desc#"
			"\\sa80"
			".MACRO directive defines block of code (macro definitions). Label placed before .MACRO becomes macro definition name and is placed in macro name dictionary (which is separate from label names dictionary).\n"
			"After .MACRO directive one can place macro parameters and/or ellipsis (...). Parameter name can then be used in macro definition block. Defined parameters will be required when macro is used later in source code. To pass any number of parameters (also none) one can use ellipsis (...). If there are no params defined macro can be invoked without params only.\n"
			"To use params inside macro definition one can use their names or consecutive numbers (starting from 1) preceded by '%' character. Param number zero (%0) has special meaning--it contains number of actual parameters macro was invoked with. Instead of numbers numeric expression can be used if they are enclosed in square brackets (for example %[.cnt+1]).\n"
			"In macro invocation actual parameters are placed after macro name. Param expressions are separated by commas. All those expression are assembly time expressions. They get interpreted and calculated and result values are passed to macro definition.\n"
			"All the labels starting with dot (.) are local to macro definition block and are not accessible nor visible from the outside code using macrodefinition. All the other labels are global. Macrodefinition code can use local labels (from the place it was invoked), global labels, as well as it's own local labels.\n"
			"Macro definition parameters could be referenced with ‘$’ suffix. If given param was passed as string it is still accessible as string using dollar sign. Accessing it without '$' suffix returns string length. Param 0$ has special meaning: macro name.\n"
		"#exmpl#"
			"Put:	.MACRO chr		; print single character\n"
			"	LDA #chr		; load value of parameter ‘chr’\n"
			"	JSR sys_put_char\n"
			"	.ENDM\n"
			"; invocation:\n"
			"	Put ’A’\n"
			"\n"
			"Print:	.MACRO ...	; printing\n"
			".cnt	.= 0			; param counter\n"
			"	.REPEAT %0			; for each parameter\n"
			".cnt	.= .cnt + 1\n"
			"	.IF .PARAMTYPE(%.cnt) == 2	; text param?\n"
			"	  JSR sys_print_text	; string is placed *after* procedure call\n"
			"	  .BYTE .STRLEN(%.cnt), %.cnt\n"
			"	.ELSE					; numerical param -> address of string\n"
			"	  LDA #>%.cnt		; high address byte\n"
			"	  LDX #<%.cnt		; low address byte\n"
			"	  JSR sys_print\n"
			"	.ENDIF\n"
			"	.ENDR\n"
			"	.ENDM\n");

	else if (strDirective.CompareNoCase(_T(".ENDM")) == 0)
		return _T("#title#.ENDM#text#\nDirective closing macro definition block. Check also .MACRO."
		"#syntax#"
			"  .ENDM"
		"#exmpl#"
			"  .ENDM  ; end of macro definition\n");

	else if (strDirective.CompareNoCase(_T(".EXITM")) == 0)
		return _T("#title#.EXITM#text#\nDirective stopping macro definition."
		"#syntax#"
			"  .EXITM"
		"#exmpl#"
			"  .EXITM  ; all the rest of macro code won't be inserted in place it was invoked\n");
//		"#desc#"
//			".EXITM directive is useful when used in conjunction with .IF directive to conditionally stop macro definition");

	else if (strDirective.CompareNoCase(_T(".IF")) == 0)
		return _T("#title#.IF#text#\nDirective opening conditional assembly block."
		"#syntax#"
			"  .IF expr"
		"#exmpl#"
			"  .IF .REF(alpha) ; if 'alpha' label was referenced\n"
			"  .IF a==5  ; if label 'a' equals 5\n"
			"  .IF b     ; if label 'b' has non-zero value\n"
			"  .IF %0>2  ; if macro invoked with more then two params\n");

	else if (strDirective.CompareNoCase(_T(".ELSE")) == 0)
		return _T("#title#.ELSE#text#\nDirective of conditional assembly."
		"#syntax#"
			"  .ELSE"
		"#exmpl#"
			"  .IF b\n"
			"    RTS\n"
			"  .ELSE ; if b wasn't <> 0\n"
			"   ; then following lines will be assembled\n"
			"   ; ...\n"
			"  .ENDIF\n");

	else if (strDirective.CompareNoCase(_T(".ENDIF")) == 0)
		return _T("#title#.ENDIF#text#\nDirective closing conditional assembly block."
		"#syntax#"
			"  .ENDIF"
		"#exmpl#"
			"  .ENDIF ; end of conditional assembly block\n");

	else if (strDirective.CompareNoCase(_T(".ERROR")) == 0)
		return _T("#title#.ERROR#text#\nDirective generating user assembly error."
		"#syntax#"
			"  .ERROR [textexpr]"
		"#exmpl#"
			"  .ERROR \"Required text parameter missing in macro \" + %0$\n");

	else if (strDirective.CompareNoCase(_T(".REPEAT")) == 0 ||
			 strDirective.CompareNoCase(_T(".REPT")) == 0)
		return _T("#title#.REPEAT .REPT#text#\nDirective opening block of source text to be repeated given number of times."
		"#syntax#"
			"[<label>[:]] .REPEAT expr"
		"#exmpl#"
			"  .REPEAT 10  ; repeat 10 times\n\n"
			"  .REPEAT %0  ; repeat as many times as number of macro params\n\n"
			"  .REPEAT 4\n"
			"    LSR\n"
			"  .ENDR\n");

	else if (strDirective.CompareNoCase(_T(".ENDR")) == 0)
		return _T("#title#.ENDR#text#\nDirective closing block of source text to repeat."
		"#syntax#"
			"[<label>[:]] .ENDR"
		"#exmpl#"
			"  .ENDR\n");

	else if (strDirective.CompareNoCase(_T(".SET")) == 0 ||
			 strDirective.CompareNoCase(_T(".=")) == 0)
		return _T("#title#.SET .=#text#\nDirective .SET assigns value to the label. This value can be changed (reassigned)."
		"#syntax#"
			"<label>[:] .SET expr\n"
		"#exmpl#"
			".cnt .SET .cnt+1 ; increment .cnt\n");

	else if (strDirective.CompareNoCase(_T(".ROM_AREA")) == 0)
		return _T("#title#.ROM_AREA#text#\nDirective establishing memory protection area."
		"#syntax#"
			"[<label>[:]] .ROM_AREA addr_from_expr, addr_to_expr\n"
		"#exmpl#"
			" .ROM_AREA $a000, $afff\n"
			" .ROM_AREA Start, * ; from 'Start' to here\n"
		"#desc#"
			".ROM_AREA turns on memory protection for a given range of addresses. Any attempt to write to this area"
			" will stop program execution. Write attempts to the EPROM usually indicate a bug and memory protection"
			" can facilitate locating such bugs. Specifying same start and end address turns protection off.\n");

	else if (strDirective.CompareNoCase(_T(".IO_WND")) == 0)
		return _T("#title#.IO_WND#text#\nDirective setting terminal window size."
		"#syntax#"
			"[<label>[:]] .IO_WND cols_expr, rows_expr\n"
		"#exmpl#"
			" .IO_WND 40, 20; 40 columns, 20 rows\n"
		"#desc#"
			".IO_WND directive sets size of terminal window. It requires two parameters: number of columns and rows."
			" Both columns and rows are limited to 1..255 range.\n");

	else if (strDirective.CompareNoCase(_T("IO_AREA")) == 0)
		return _T("#title#IO_AREA#text#\nLabel representing beginning of simulator I/O area."
		"#syntax#"
			"IO_AREA = addr_expr ; set I/O area\n"
			"  <instruction> IO_AREA ; use I/O area\n"
		"#exmpl#"
			"IO_Cls = IO_AREA + 0 ; clear window port\n"
			"  STA IO_AREA+1 ; put char\n"
		"#desc#"
			"IO_AREA label represents beginning of simulator I/O area. Simulator can detect read and write attempts"
			" from/to its I/O area. Starting from IO_AREA address consecutive bytes are treated as virtual ports."
			" Following ports are defined:\n"
			"IO_AREA+0: TERMINAL_CLS (w)\n"
			"IO_AREA+1: TERMINAL_OUT (w)\n"
			"IO_AREA+2: TERMINAL_OUT_CHR (w)\n"
			"IO_AREA+3: TERMINAL_OUT_HEX (w)\n"
			"IO_AREA+4: TERMINAL_IN (r)\n"
			"IO_AREA+5: TERMINAL_X_POS (r/w)\n"
			"IO_AREA+6: TERMINAL_Y_POS (r/w)\n"
			"(w) means write only port, (r) read only, (r/w) read/write.\n\n"
			"TERMINAL_CLS - clear terminal window, set cursor at (0,0) position.\n"
			"TERMINAL_OUT - output single character interpreting control characters.\n"
			" Terminal can only recognize those characters:\n"
			" $d char (caret) moving cursor to the beginning of line,\n"
			" $a char (line feed) moving cursor to the next line and scrolling window if necessary,\n"
			" 8 char (backspace) moving one position to the left and erasing char below cursor.\n"
			"TERMINAL_OUT_CHR - outputs single character; control chars are being output just like regular characters.\n"
			"TERMINAL_OUT_HEX - outputs single byte as a two-digit hexadecimal number.\n"
			"TERMINAL_IN - input single byte, returns 0 if there's no characters available in terminal's buffer;"
			" when I/O terminal window is active it can accept keyboard input; press <Ins> key to paste clipboard's contents into terminal.\n"
			"TERMINAL_X_POS - cursor X position (column).\n"
			"TERMINAL_Y_POS - cursor Y position (row).\n"
			);

	return "";
}


extern int MatchingInstructions(const CString& strWord, CString& strResult)
{
	strResult.Empty();

	if (strWord.IsEmpty())
		return 0;

	int nMatch= 0;

	for (int i= 0; s_vpszInstructionList[i] != 0; ++i)
	{
		if (_tcsnicmp(s_vpszInstructionList[i], strWord, strWord.GetLength()) == 0)
		{
			strResult += s_vpszInstructionList[i];
			strResult += _T("\n");
			++nMatch;
		}
	}

	if (nMatch == 1)
		strResult.Delete(strResult.GetLength() - 1);

	return nMatch;
}


class Instructions : CDeasm
{
	CString AddMode(UINT8 cmd, OpCode inst, CodeAdr mode, bool b65c02);

public:

	CString GetModes(OpCode inst);
	CString GetBranchInfo(bool bConditional= true);
};


CString Instructions::AddMode(UINT8 cmd, OpCode inst, CodeAdr mode, bool b65c02)
{
	CmdInfo ci(0, 0, 0, 0, 0, cmd, 0x34, 0x12, 0x8000);
	return DeasmInstr(ci, DF_HELP | DF_USE_BRK | DF_65C02) + _T('\n');
}


CString Instructions::GetBranchInfo(bool bConditional/*= true*/)
{
	CString str= "All branch instructions are relative to the PC (current location--program counter)."
		" They can jump forward or backward but are limited to local range (+/- 128 bytes).";

	if (bConditional)
		str += " Jump is effective if corresponding flag in status register is set/clear.";
	else
		str += " Jump is always effective regardless of flags set in status register.";

	return str;
}


CString Instructions::GetModes(OpCode inst)
{
	const UINT8 NA= 0xBB;
	CString strModes;
	bool bExt= false;

	for (int i= 0; i < A_NO_OF_MODES; ++i)
	{
		UINT8 code= trans_table[inst][i];
		if (code != NA)
			strModes += _T("<pre> ") + AddMode(code, inst, CodeAdr(i), false);
		else
		{
			code = trans_table_c[inst][i];
			if (code != NA)
			{
				strModes += _T("<pre>¹") + AddMode(code, inst, CodeAdr(i), true);
				bExt = true;
			}
		}
	}

	if (bExt)
		strModes += _T("\n¹<small> (65c02 only)\n");

	return strModes;
}


	//	strDesc += GetInstructionModes(C_ADC);

extern CString GetInstructionDesc(const CString& strInstruction)
{
	Instructions inst;
	CString strDesc;

	if (strInstruction.CompareNoCase(_T("ADC")) == 0)
	{
		strDesc = "#title#ADC#text#\nAdd Memory to Accumulator with Carry."
		"#flags#NVZC#modes#";
		strDesc += inst.GetModes(CAsm::C_ADC);
		strDesc += "#desc#ADC adds memory to the accumulator. If D (decimal) flag bit is set ADC operates"
			" in BCD (packed Binary Coded Decimal) mode, where only decimal digits are allowed. If D flag is clear"
			" ADC operates in binary two's complement mode.\n"
		"#exmpl#"
			" ; add 5 to 'data' word\n"
			" CLD        ; binary mode\n"
			" CLC        ; clear carry\n"
			" LDA data   ; load data\n"
			" ADC #5     ; add 5\n"
			" STA data   ; store low byte\n"
			" BCC .skip  ; no carry over?\n"
			" INC data+1 ; inc hi byte\n"
			".skip:\n"
			"\n"
			" ; add $0395 to 'data' word\n"
			" CLC        ; clear carry\n"
			" LDA data   ; load data\n"
			" ADC #$95\n"
			" STA data   ; store low byte\n"
			" LDA data+1\n"
			" ADC #$03   ; add with carry\n"
			" STA data+1 ; store hi byte\n"
		"\n<small> (in BCD mode flags N & Z are only set by 65c02 and undefined in case of 6502)\n";
	}
	else if (strInstruction.CompareNoCase(_T("AND")) == 0)
	{
		strDesc = "#title#AND#text#\n\"AND\" Memory with Accumulator."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_AND);
//		strDesc += "#desc#opis blah blah";
		strDesc +=
		"#exmpl#"
			" ; extract bits 0-3\n"
			" LDA data,X\n"
			" AND #$0F ; mask four bits\n";
	}
	else if (strInstruction.CompareNoCase(_T("ASL")) == 0)
	{
		strDesc = "#title#ASL#text#\nShift One Bit Left."
		"#flags#NZC#modes#";
		strDesc += inst.GetModes(CAsm::C_ASL);
		strDesc += "#desc#ASL shifts all bits left one position. Bit 0 is cleared and original bit 7 is moved into the Carry.\n"
		"#exmpl#"
			" ; extract bits 4-7\n"
			" LDA data,X\n"
			" ASL\n"
			" ASL\n"
			" ASL\n"
			" ASL\n"
			" ; bits 4-7 are in 0-3 position\n";
	}
	else if (strInstruction.CompareNoCase(_T("BBR")) == 0)
	{
		strDesc = "#title#BBR#text#\nBranch on Bit Reset."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BBR);
//		strDesc += "#desc#opis blah blah";
	}
	else if (strInstruction.CompareNoCase(_T("BBS")) == 0)
	{
		strDesc = "#title#BBS#text#\nBranch on Bit Set."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BBS);
	}
	else if (strInstruction.CompareNoCase(_T("BCC")) == 0)
	{
		strDesc = "#title#BCC#text#\nBranch on Carry Clear."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BCC);
		strDesc += "#exmpl#"
			" LDA data  ; load data\n"
			" CMP #10\n"
			" BCC .less ; jump if data < 10\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BCS")) == 0)
	{
		strDesc = "#title#BCS#text#\nBranch on Carry Set."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BCS);
		strDesc += "#exmpl#"
			" LDA data  ; load data\n"
			" CMP #10\n"
			" BCS .gt_eq ; jump if data >= 10\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BEQ")) == 0)
	{
		strDesc = "#title#BEQ#text#\nBranch on Result Zero."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BEQ);
		strDesc += "#exmpl#"
			" LDA flag  ; load data\n"
			" BEQ .zero ; jump if flag == 0\n"
			" CMP #5\n"
			" BEQ .five ; jump if flag == 5\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BIT")) == 0)
	{
		strDesc = "#title#BIT#text#\nTest Memory Bits with Accumulator."
		"#flags#NVZ#modes#";
		strDesc += inst.GetModes(CAsm::C_BIT);
		strDesc += "#desc#BIT performs \"AND\" operation on its argument and accumulator."
			" Result is not stored but Z(ero) flag is set accordingly. Flags N and V become"
			" copies of 7-th (oldest) and 6-th bits of BIT argument.\n"
		"#exmpl#"
			" LDA #@1010 ; bits to test\n"
			" BIT port   ; test port's bits\n"
			" BEQ .zero  ; both bits clear\n";
	}
	else if (strInstruction.CompareNoCase(_T("BMI")) == 0)
	{
		strDesc = "#title#BMI#text#\nBranch on Result Minus."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BMI);
		strDesc += "#exmpl#"
			" BIT flag  ; flag to test\n"
			" BMI .neg  ; jump if flag negative\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BNE")) == 0)
	{
		strDesc = "#title#BNE#text#\nBranch on Result Not Zero."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BNE);
		strDesc += "#exmpl#"
			" LDA flag      ; load data\n"
			" BNE .not_zero ; jump if flag != 0\n"
			" CMP #2\n"
			" BNE .not_two  ; jump if flag != 2\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BPL")) == 0)
	{
		strDesc = "#title#BPL#text#\nBranch on Result Plus."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BPL);
		strDesc += "#exmpl#"
			" LDX #10    ; load counter\n"
			".delay:\n"
			" DEX\n"
			" BPL .delay ; jump if X >= 0\n";
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BRA")) == 0)
	{
		strDesc = "#title#BRA#text#\nBranch Always."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BRA);
		strDesc += "#desc#" + inst.GetBranchInfo(false);
	}
	else if (strInstruction.CompareNoCase(_T("BRK")) == 0)
	{
		strDesc = "#title#BRK#text#\nForce Break."
		"#flags#B¹DI#modes#";
		strDesc += inst.GetModes(CAsm::C_BRK);
		strDesc += "#desc#BRK forces interrupt. CPU fetches interrupt vector ($FFFE/F)"
			" and jumps to the interrupt handler routine. Bits I and B are set.\n"
			"Simulator can use this instruction to stop execution of your program.\n"
			"\n¹<small> (D flag cleared only by 65c02 CPU)\n";
	}
	else if (strInstruction.CompareNoCase(_T("BVC")) == 0)
	{
		strDesc = "#title#BVC#text#\nBranch on Overflow Clear."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BVC);
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("BVS")) == 0)
	{
		strDesc = "#title#BVS#text#\nBranch on Overflow Set."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_BVS);
		strDesc += "#desc#" + inst.GetBranchInfo();
	}
	else if (strInstruction.CompareNoCase(_T("CLC")) == 0)
	{
		strDesc = "#title#CLC#text#\nClear Carry Flag."
		"#flags#C#modes#";
		strDesc += inst.GetModes(CAsm::C_CLC);
	}
	else if (strInstruction.CompareNoCase(_T("CLD")) == 0)
	{
		strDesc = "#title#CLD#text#\nClear Decimal Mode."
		"#flags#D#modes#";
		strDesc += inst.GetModes(CAsm::C_CLD);
	}
	else if (strInstruction.CompareNoCase(_T("CLI")) == 0)
	{
		strDesc = "#title#CLI#text#\nClear Interrupt Disable Bit."
		"#flags#I#modes#";
		strDesc += inst.GetModes(CAsm::C_CLI);
	}
	else if (strInstruction.CompareNoCase(_T("CLV")) == 0)
	{
		strDesc = "#title#CLV#text#\nClear Overflow Flag."
		"#flags#V#modes#";
		strDesc += inst.GetModes(CAsm::C_CLV);
	}
	else if (strInstruction.CompareNoCase(_T("CMP")) == 0)
	{
		strDesc = "#title#CMP#text#\nCompare Memory and Accumulator."
		"#flags#NZC#modes#";
		strDesc += inst.GetModes(CAsm::C_CMP);
	}
	else if (strInstruction.CompareNoCase(_T("CPX")) == 0)
	{
		strDesc = "#title#CPX#text#\nCompare Memory and Index X."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_CPX);
	}
	else if (strInstruction.CompareNoCase(_T("CPY")) == 0)
	{
		strDesc = "#title#CPY#text#\nCompare Memory and Index Y."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_CPY);
	}
	else if (strInstruction.CompareNoCase(_T("DEA")) == 0)
	{
		strDesc = "#title#DEA#text#\n.Decrement Accumulator by One"
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_DEA);
	}
	else if (strInstruction.CompareNoCase(_T("DEC")) == 0)
	{
		strDesc = "#title#DEC#text#\nDecrement by One."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_DEC);
		strDesc += "#exmpl#"
			" ; subtract 1 from 6 conecutive bytes\n"
			" LDX #5      ; counter\n"
			".dec:\n"
			" DEC data,X  ; decrement\n"
			" DEX\n"
			" BPL .dec    ; loop\n";
	}
	else if (strInstruction.CompareNoCase(_T("DEX")) == 0)
	{
		strDesc = "#title#DEX#text#\nDecrement Index X by One."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_DEX);
		strDesc += "#exmpl#"
			" ; clear buf[0..31]\n"
			" LDX #31   ; counter\n"
			" LDA #0\n"
			".clr:\n"
			" STA buf,X ; clear buffer\n"
			" DEX\n"
			" BPL .clr  ; loop\n";
	}
	else if (strInstruction.CompareNoCase(_T("DEY")) == 0)
	{
		strDesc = "#title#DEY#text#\nDecrement Index Y by One."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_DEY);
		strDesc += "#exmpl#"
			" ; copy 200 bytes\n"
			" LDY #200\n"
			".copy\n"
			" LDA (src),Y\n"
			" STA (dst),Y\n"
			" DEY\n"
			" BNE .copy\n";
	}
	else if (strInstruction.CompareNoCase(_T("EOR")) == 0)
	{
		strDesc = "#title#EOR#text#\n\"Exclusive-or\" Memory with Accumulator."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_EOR);
	}
	else if (strInstruction.CompareNoCase(_T("INA")) == 0)
	{
		strDesc = "#title#INA#text#\nIncrement Accumulator by One."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_INA);
	}
	else if (strInstruction.CompareNoCase(_T("INC")) == 0)
	{
		strDesc = "#title#INC#text#\nIncrement by One."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_INC);
		strDesc += "#exmpl#"
			" ; add 1 to 'data' word\n"
			" INC data   ; inc low byte\n"
			" BNE .skip\n"
			" INC data+1 ; inc hi byte\n"
			".skip:\n";
	}
	else if (strInstruction.CompareNoCase(_T("INX")) == 0)
	{
		strDesc = "#title#INX#text#\nIncrement Index X by One."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_INX);
	}
	else if (strInstruction.CompareNoCase(_T("INY")) == 0)
	{
		strDesc = "#title#INY#text#\n.Increment Index Y by One"
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_INY);
	}
	else if (strInstruction.CompareNoCase(_T("JMP")) == 0)
	{
		strDesc = "#title#JMP#text#\nJump to New Location."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_JMP);
	}
	else if (strInstruction.CompareNoCase(_T("JSR")) == 0)
	{
		strDesc = "#title#JSR#text#\nJump to Subroutine."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_JSR);
		strDesc += "#desc#JSR calls subroutine: it jumps to the new location saving return address on the stack,"
		" so program execution can be resumed when subroutine ends with RTS.\n"
		"Due to the peculiarity of 6502 return address pushed on the stack is one less then an address of the"
		" instruction following JSR (i.e. addr - 1 is stored instead of addr).";
	}
	else if (strInstruction.CompareNoCase(_T("LDA")) == 0)
	{
		strDesc = "#title#LDA#text#\nLoad Accumulator with Memory."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_LDA);
	}
	else if (strInstruction.CompareNoCase(_T("LDX")) == 0)
	{
		strDesc = "#title#LDX#text#\nLoad Index X with Memory."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_LDX);
	}
	else if (strInstruction.CompareNoCase(_T("LDY")) == 0)
	{
		strDesc = "#title#LDY#text#\nLoad Index Y with Memory."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_LDY);
	}
	else if (strInstruction.CompareNoCase(_T("LSR")) == 0)
	{
		strDesc = "#title#LSR#text#\nShift One Bit Right."
		"#flags#NZC#modes#";
		strDesc += inst.GetModes(CAsm::C_LSR);
		strDesc += "#desc#LSR shifts all bits right one position. Bit 7 is cleared and original bit 0 is moved into the Carry.\n"
		"#exmpl#"
			" ; fast multiply by 4\n"
			" LDA data ; load data\n"
			" LSR      ; times 2\n"
			" LSR      ; times 2\n";
	}
	else if (strInstruction.CompareNoCase(_T("NOP")) == 0)
	{
		strDesc = "#title#NOP#text#\nNo Operation."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_NOP);
	}
	else if (strInstruction.CompareNoCase(_T("ORA")) == 0)
	{
		strDesc = "#title#ORA#text#\n\"OR\" Memory with Accumulator."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_ORA);
	}
	else if (strInstruction.CompareNoCase(_T("PHA")) == 0)
	{
		strDesc = "#title#PHA#text#\nPush Accumulator on Stack."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_PHA);
		strDesc += "#desc#PHA stores accumulator on the stack. PLA could be used to restore it.\n"
		"#exmpl#"
			" PHA      ; push A\n"
			" JSR putC\n"
			" PLA      ; pull A\n";
	}
	else if (strInstruction.CompareNoCase(_T("PHP")) == 0)
	{
		strDesc = "#title#PHP#text#\nPush Processor Status on Stack."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_PHP);
	}
	else if (strInstruction.CompareNoCase(_T("PHX")) == 0)
	{
		strDesc = "#title#PHX#text#\nPush Index X on Stack."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_PHX);
	}
	else if (strInstruction.CompareNoCase(_T("PHY")) == 0)
	{
		strDesc = "#title#PHY#text#\nPush Index Y on Stack."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_PHY);
	}
	else if (strInstruction.CompareNoCase(_T("PLA")) == 0)
	{
		strDesc = "#title#PLA#text#\nPull Accumulator from Stack."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_PLA);
	}
	else if (strInstruction.CompareNoCase(_T("PLP")) == 0)
	{
		strDesc = "#title#PLP#text#\nPull Process Status from Stack."
		"#flags#all#modes#";
		strDesc += inst.GetModes(CAsm::C_PLP);
	}
	else if (strInstruction.CompareNoCase(_T("PLX")) == 0)
	{
		strDesc = "#title#PLX#text#\nPull Index X from Stack."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_PLX);
	}
	else if (strInstruction.CompareNoCase(_T("PLY")) == 0)
	{
		strDesc = "#title#PLY#text#\nPull Index Y from Stack."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_PLY);
	}
	else if (strInstruction.CompareNoCase(_T("RMB")) == 0)
	{
		strDesc = "#title#RMB#text#\nReset Memory Bit."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_RMB);
	}
	else if (strInstruction.CompareNoCase(_T("ROL")) == 0)
	{
		strDesc = "#title#ROL#text#\nRotate One Bit Left."
		"#flags#NZC#modes#";
		strDesc += inst.GetModes(CAsm::C_ROL);
		strDesc += "#desc#ROL shifts all bits left one position. Carry is copied to bit 0 and original bit 7 is moved into the Carry.\n"
		"#exmpl#"
			" ; shift left word data\n"
			" ASL data   ; shift low byte\n"
			" ; using Carry as temp bit\n"
			" ROL data+1 ; shift hi byte\n";
	}
	else if (strInstruction.CompareNoCase(_T("ROR")) == 0)
	{
		strDesc = "#title#ROR#text#\nRotate One Bit Right."
		"#flags#NZC#modes#";
		strDesc += inst.GetModes(CAsm::C_ROR);
		strDesc += "#desc#ROR shifts all bits right one position. Carry is copied to bit 7 and original bit 0 is moved into the Carry.\n"
		"#exmpl#"
			" ; shift right word data\n"
			" LSR data+1 ; shift hi byte\n"
			" ; using Carry as temp bit\n"
			" ROR data   ; shift low byte\n";
	}
	else if (strInstruction.CompareNoCase(_T("RTI")) == 0)
	{
		strDesc = "#title#RTI#text#\nReturn from Interrupt."
		"#flags#NVDIZC#modes#";
		strDesc += inst.GetModes(CAsm::C_RTI);
		strDesc += "#desc#RTI retrieves flags register from the stack, then it retrieves return address, so"
			" program execution can be resumed after an interrupt.";
	}
	else if (strInstruction.CompareNoCase(_T("RTS")) == 0)
	{
		strDesc = "#title#RTS#text#\nReturn from Subroutine."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_RTS);
		strDesc += "#desc#RTS retrieves return address from the stack. RTS is used to return from subroutine invoked by JSR.\n"
			"Note: because JSR places address-1 value on the stack, RTS modifies it by adding 1 before it's used.";
	}
	else if (strInstruction.CompareNoCase(_T("SBC")) == 0)
	{
		strDesc = "#title#SBC#text#\nSubtract Memory from Accumulator with Borrow."
		"#flags#NVZC#modes#";
		strDesc += inst.GetModes(CAsm::C_SBC);
		strDesc += "#desc#SBC subtracts memory from the accumulator. If D (decimal) flag bit is set SBC operates"
			" in BCD (packed Binary Coded Decimal) mode, where only decimal digits are allowed. If D flag is clear"
			" SBC operates in binary two's complement mode.\n"
		"#exmpl#"
			" CLD       ; binary mode\n"
			" SEC       ; clear borrow\n"
			" LDA #$90  ; load $90\n"
			" SBC #1    ; minus 1\n"
			" STA data  ; data=$8F\n"
			"\n"
			" SED       ; decimal mode\n"
			" SEC       ; clear borrow\n"
			" LDA #$90  ; this is 90 in BCD\n"
			" SBC #1    ; minus 1\n"
			" STA data  ; data=89 in BCD\n"
		"\n<small> (in BCD mode flags N & Z are only set by 65c02 and undefined in case of 6502)\n";
	}
	else if (strInstruction.CompareNoCase(_T("SEC")) == 0)
	{
		strDesc = "#title#SEC#text#\nSet Carry Flag."
		"#flags#C#modes#";
		strDesc += inst.GetModes(CAsm::C_SEC);
	}
	else if (strInstruction.CompareNoCase(_T("SED")) == 0)
	{
		strDesc = "#title#SED#text#\nSet Decimal Mode."
		"#flags#D#modes#";
		strDesc += inst.GetModes(CAsm::C_SED);
		strDesc += "#desc#SED sets decimal mode for ADC and SBC instructions. In BCD (packed Binary Coded Decimal)"
			" mode addition and subtraction operates on packed BCD numbers.";
	}
	else if (strInstruction.CompareNoCase(_T("SEI")) == 0)
	{
		strDesc = "#title#SEI#text#\nSet Interrupt Disable Bit."
		"#flags#I#modes#";
		strDesc += inst.GetModes(CAsm::C_SEI);
	}
	else if (strInstruction.CompareNoCase(_T("SMB")) == 0)
	{
		strDesc = "#title#SMB#text#\nSet Memory Bit."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_SMB);
	}
	else if (strInstruction.CompareNoCase(_T("STA")) == 0)
	{
		strDesc = "#title#STA#text#\nStore Accumulator in Memory."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_STA);
		strDesc += "#exmpl#"
			" LDA #$FF\n"
			" STA flag ; flag = $FF\n";
	}
	else if (strInstruction.CompareNoCase(_T("STX")) == 0)
	{
		strDesc = "#title#STX#text#\nStore Index X in Memory."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_STX);
	}
	else if (strInstruction.CompareNoCase(_T("STY")) == 0)
	{
		strDesc = "#title#STY#text#\nStore Index Y in Memory."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_STY);
	}
	else if (strInstruction.CompareNoCase(_T("STZ")) == 0)
	{
		strDesc = "#title#STZ#text#\nStore Zero in Memory."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_STZ);
		strDesc += "#exmpl#"
			" STZ data  ; clear data byte\n";
	}
	else if (strInstruction.CompareNoCase(_T("TAX")) == 0)
	{
		strDesc = "#title#TAX#text#\nTransfer Accumulator in Index X."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_TAX);
		strDesc += "#desc#TAX copies accumulator into the X register.\n";
	}
	else if (strInstruction.CompareNoCase(_T("TAY")) == 0)
	{
		strDesc = "#title#TAY#text#\nTransfer Accumulator in Index Y."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_TAY);
		strDesc += "#desc#TAY copies accumulator into the Y register.\n";
	}
	else if (strInstruction.CompareNoCase(_T("TRB")) == 0)
	{
		strDesc = "#title#TRB#text#\nTest and Reset Memory Bits with Accumulator."
		"#flags#Z#modes#";
		strDesc += inst.GetModes(CAsm::C_TRB);
	}
	else if (strInstruction.CompareNoCase(_T("TSB")) == 0)
	{
		strDesc = "#title#TSB#text#\nTest and Set Memory Bits with Accumulator."
		"#flags#Z#modes#";
		strDesc += inst.GetModes(CAsm::C_TSB);
	}
	else if (strInstruction.CompareNoCase(_T("TSX")) == 0)
	{
		strDesc = "#title#TSX#text#\nTransfer Stack Pointer to Index X."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_TSX);
		strDesc += "#desc#TSX copies stack pointer register S into the X register.\n";
	}
	else if (strInstruction.CompareNoCase(_T("TXA")) == 0)
	{
		strDesc = "#title#TXA#text#\nTransfer Index X to Accumulator."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_TXA);
		strDesc += "#desc#TXA copies X register into the accumulator.\n";
	}
	else if (strInstruction.CompareNoCase(_T("TXS")) == 0)
	{
		strDesc = "#title#TXS#text#\nTransfer Index X to Stack Pointer."
		"#flags#-#modes#";
		strDesc += inst.GetModes(CAsm::C_TXS);
		strDesc += "#desc#TXS copies X register into the stack pointer register S.\n"
		"#exmpl#"
			" LDX #$FF\n"
			" TXS ; empty the stack\n";
	}
	else if (strInstruction.CompareNoCase(_T("TYA")) == 0)
	{
		strDesc = "#title#TYA#text#\nTransfer Index Y to Accumulator."
		"#flags#NZ#modes#";
		strDesc += inst.GetModes(CAsm::C_TYA);
		strDesc += "#desc#TYA copies Y register into the accumulator.\n"
		"#exmpl#"
			" PHA ; store accumulator\n"
			" TYA\n"
			" PHA ; and store Y\n";
	}

/*
STP Stop the Clock
WAI Wait for Interrupt
*/

	return strDesc;
}
