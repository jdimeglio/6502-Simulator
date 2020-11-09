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

// Asembler dla mikroprocesorów M65XX i M65C02

#include "stdafx.h"
#include "resource.h"
#include "typeinfo.h"
#include "MarkArea.h"
#include "IOWindow.h"	// this is sloppy, but right now there's no mechanism to let framework know about requested new terminal wnd size

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CAsm6502::case_insensitive= false;	// true -> small/capital letters in label names are treated as same
bool CAsm6502::generateBRKExtraByte= false;	// generate extra byte after BRK command?
UINT8 CAsm6502::BRKExtraByte= 0x0;			// value of extra byte generated after BRK command
//-----------------------------------------------------------------------------
char* CLeksem::CLString::s_ptr_1= 0;
char* CLeksem::CLString::s_ptr_2= 0;
char CLeksem::CLString::s_buf_1[258];
char CLeksem::CLString::s_buf_2[258];
const size_t CLeksem::CLString::s_cnMAX= 256;

CLeksem::CLeksem(const CLeksem &leks) : type(leks.type)
{
	memcpy(this, &leks, sizeof *this);	// copy whole union and type field
	if (leks.type==L_STR || leks.type==L_IDENT || leks.type==L_IDENT_N)	// contains string?
		str = new CLString(*leks.str);	// duplicate string
}


CLeksem & CLeksem::operator = (const CLeksem &leks)
{
//  ASSERT(type == leks.type);
	if (type==L_STR || type==L_IDENT || type==L_IDENT_N)	// leksem docelowy zawiera ³añcuch znaków?
	{
		delete str;
		str = NULL;
	}
//  type = leks.type;
	memcpy(this,&leks,sizeof *this);	// skopiowanie ca³ej unii i pola typu
	if (leks.type==L_STR || leks.type==L_IDENT || leks.type==L_IDENT_N)	// leksem Ÿród³owy zawiera ³añcuch znaków?
		str = new CLString(*leks.str);	// zdublowanie ³añcucha znaków
	return *this;
}


CLeksem::~CLeksem()
{
	switch (type)
	{
	case L_STR:
	case L_IDENT:
	case L_IDENT_N:
		if (str)
		{
#ifdef _DEBUG
			if (str->GetLength())
				str->SetAt(0,'X');		// zmiana tekstu dla ew. wisz¹cych odwo³añ
#endif
			delete str;
		}
		break;
	}
}

//-----------------------------------------------------------------------------

bool CIdentTable::insert(const CString &str, CIdent &ident)
{
	CIdent &val= this->operator[](str);
	if (val.info==CIdent::I_INIT)	// true -> nowy element, false -> ju¿ by³
	{
		val = ident;	// wpisanie nowego elementu
		return true;
	}
	else
	{
		ident = val;	// zwrócenie starego elementu
		return false;
	}
}


bool CIdentTable::replace(const CString &str, const CIdent &ident)
{
	CIdent &val= this->operator[](str);
	if (val.info==CIdent::I_INIT)	// true -> nowy element, false -> ju¿ by³
	{
		val = ident;	// wpisanie nowego elementu
	}
	else
	{
		if ((val.variable || val.info==CIdent::I_UNDEF) && ident.variable)
		{
			val = ident;	// zast¹pienie nowym elementem starej wartoœci zmiennej
			return true;
		}
		else if (val.variable || ident.variable)	// tutaj spr. albo, albo
		{
			return false;	// niedozwolone przedefiniowanie (zmiana typu ze sta³ej na zmienn¹ lub odwrotnie)
		}
		else if (val.info!=CIdent::I_UNDEF)	// stary element ju¿ zdefiniowany?
		{
			val = ident;	// zast¹pienie nowym elementem starego
			return false;	// zg³oszenie redefinicji
		}
		val = ident;	// zast¹pienie nowym elementem starego, niezdefiniowanego
	}
	return true;		// OK
}

//=============================================================================

LPTSTR CInputFile::read_line(LPTSTR str, UINT max_len)
{
	LPTSTR ptr= ReadString(str,max_len);
	if (ptr)
		m_nLine++;
	return ptr;
}

//-----------------------------------------------------------------------------

LPTSTR CInputWin::read_line(LPTSTR str, UINT max_len)
{
	int ret= SendMessage(m_pWnd->GetSafeHwnd(), WM_USER_GET_NEXT_LINE,
		WPARAM(max_len), LPARAM(str));
	ASSERT(ret);
	if (ret>0)
		m_nLine++;
	return str;
}


const CString &CInputWin::get_file_name()
{
	CString fname;
	int ret= SendMessage(m_pWnd->GetSafeHwnd(), WM_USER_GET_TITLE, WPARAM(_MAX_PATH),
		LPARAM(m_strFileName.GetBuffer(_MAX_PATH+1)));
	m_strFileName.ReleaseBuffer();
	ASSERT(ret);
	return m_strFileName;
}


void CInputWin::seek_to_begin()
{
	int ret= SendMessage(m_pWnd->GetSafeHwnd(), WM_USER_NEXT_PASS, WPARAM(0), LPARAM(0));
	ASSERT(ret);
	m_nLine = 0;
}

//-----------------------------------------------------------------------------

void CInput::open_file(const CString &fname)
{
	tail = ::new CInputFile(fname);
	tail->open();
	fuid = (FileUID)calc_index( AddTail(tail) );
}


void CInput::open_file(CWnd *pWnd)
{
	tail = ::new CInputWin(pWnd);
	tail->open();
	fuid = (FileUID)calc_index( AddTail(tail) );
//  AddTail(tail);
}


int CInput::calc_index(POSITION pos)
{
	int idx= 0;
	while (pos)
	{
		idx++;
		GetPrev(pos);
	}
	return idx;
}


void CInput::close_file()
{
	tail->close();
	::delete RemoveTail();
	tail = IsEmpty() ? NULL : GetTail();
	if (tail)
		fuid = (FileUID)calc_index(GetTailPosition());
	else
		fuid = 0;
}


CInput::~CInput()
{
	POSITION pos= GetHeadPosition();

	while (pos)
	{
		CInputBase *inp= GetNext(pos);
		inp->close();
		::delete inp;
	}
}

//=============================================================================

void CAsm6502::init_members()
{
	mem_mask = 0xFFFF;		// memory limit mask
	abort_asm = false;
	program_start = ~0u;
	check_line = false;
	in_macro = NULL;
	expanding_macro = NULL;
	repeating = NULL;
	reptNested = 0;
	originWrapped = false;
	pRept = NULL;
}

void CAsm6502::init()
{
	if (out == NULL)
	{
		out = ::new COutputMem;
		temporary_out = true;
	}
	else
		temporary_out = false;
	init_members();
}

//=============================================================================


CLeksem CAsm6502::next_leks(bool nospace)		// pobranie kolejnego symbolu
{
	if (!ptr)
		return CLeksem(CLeksem::L_FIN);

	TCHAR c= *ptr++;

	switch (c)
	{
	case '\n':
	case '\r':
		return CLeksem(CLeksem::L_CR);

	case '\0':
		ptr--;
		return CLeksem(CLeksem::L_FIN);

	case '$':
		if (!_istxdigit(*ptr))	// znak '$' na koñcu parametru makra?
			return CLeksem(CLeksem::L_STR_ARG);
		break;
	case ';':
		return CLeksem(CLeksem::L_COMMENT);
	case ':':
		return CLeksem(CLeksem::L_LABEL);
	case '=':
		if (*ptr=='=')	// operator '==' równe?
		{
			ptr++;
			return CLeksem(O_EQ);
		}
		return CLeksem(CLeksem::L_EQUAL);
	case '\'':
		return get_char_num();
	case '"':
		return get_string('"');
	case ',':
		return CLeksem(CLeksem::L_COMMA);

	case '(':
		return CLeksem(CLeksem::L_BRACKET_L);
	case ')':
		return CLeksem(CLeksem::L_BRACKET_R);
	case '[':
		return CLeksem(CLeksem::L_EXPR_BRACKET_L);
	case ']':
		return CLeksem(CLeksem::L_EXPR_BRACKET_R);

	case '>':
		if (*ptr=='>')		// operator '>>' przesuniêcia?
		{
			ptr++;
			return CLeksem(O_SHR);
		}
		else if (*ptr=='=')	// operator '>=' wiêksze równe?
		{
			ptr++;
			return CLeksem(O_GTE);
		}
		return CLeksem(O_GT);
	case '<':
		if (*ptr=='<')		// operator '<<' przesuniêcia?
		{
			ptr++;
			return CLeksem(O_SHL);
		}
		else if (*ptr=='=')	// operator '<=' mniejsze równe?
		{
			ptr++;
			return CLeksem(O_LTE);
		}
		return CLeksem(O_LT);
	case '&':
		if (*ptr=='&')	// operator '&&' ?
		{
			ptr++;
			return CLeksem(O_AND);
		}
		return CLeksem(O_B_AND);
	case '|':
		if (*ptr=='|')	// operator '||' ?
		{
			ptr++;
			return CLeksem(O_OR);
		}
		return CLeksem(O_B_OR);
	case '^':
		return CLeksem(O_B_XOR);
	case '+':
		return CLeksem(O_PLUS);
	case '-':
		return CLeksem(O_MINUS);
	case '*':
		if (*ptr=='=')	// operator '*=' .ORG?
		{
			ptr++;
			return CLeksem(I_ORG);
		}
		return CLeksem(O_MUL);
	case '/':
		return CLeksem(O_DIV);
	case '%':
		return CLeksem(O_MOD);
	case '~':
		return CLeksem(O_B_NOT);
	case '!':
		if (*ptr=='=')	// operator '!=' ro¿ne?
		{
			ptr++;
			return CLeksem(O_NE);
		}
		return CLeksem(O_NOT);

	case '#':
		return CLeksem(CLeksem::L_HASH);
	case '.':
		if (*ptr=='=')	// operator '.=' przypisania?
		{
			ptr++;
			return CLeksem(I_SET);
		}
		else if (ptr[0]=='.' && ptr[1]=='.')	// wielokropek '...' ?
		{
			ptr += 2;
			return CLeksem(CLeksem::L_MULTI);
		}
		break;
	};

	if (_istspace(c))
	{
		if (!nospace)		// zwróciæ leksem L_SPACE?
			return eat_space();
		eat_space();
		return next_leks();
	}
	else if (_istdigit(c))	// cyfra dziesiêtna?
	{
		ptr--;
		return get_dec_num();
	}
	else if (c=='$')		// liczba hex?
		return get_hex_num();
	else if (c=='@')		// liczba bin?
		return get_bin_num();
	else if (_istalpha(c) || c=='_' || c=='.' || c=='?')
	{
		ptr--;
		//const CLeksem &leks=
		CLeksem::CLString* pStr= get_ident();
		if (pStr == NULL)
			return CLeksem(CLeksem::ERR_BAD_CHR);
		if (c=='.')				// to mo¿e byæ dyrektywa
		{
			InstrType it;
			if (asm_instr(*pStr,it))
			{
				delete pStr;
				return CLeksem(it);
			}
		}
		else if (pStr->GetLength()==3)	// to mo¿e byæ instrukcja
		{
			OpCode code;
			if (proc_instr(*pStr,code))
			{
				delete pStr;
				return CLeksem(code);
			}
		}
		if (*ptr == '#')			// znak '#' na koñcu etykiety?
		{
			ptr++;
			return CLeksem(pStr,1L);	// identyfikator numerowany (po '#' oczekiwana liczba)
		}
		return CLeksem(pStr,1);	// L_IDENT
	}

	return CLeksem(CLeksem::L_UNKNOWN);	// niesklasyfikowany znak - b³¹d
}



CLeksem CAsm6502::get_hex_num()		// interpretacja liczby szesnastkowej
{
	UINT32 val= 0;
	const TCHAR *tmp= ptr;

	if (!_istxdigit(*ptr))
	{
		err_start = tmp;
		return CLeksem(CLeksem::ERR_NUM_HEX);	// oczekiwana cyfra liczby szesnastkowej
	}

	do
	{
		if (val & 0xF0000000)
		{
			err_start = tmp;
			return CLeksem(CLeksem::ERR_NUM_BIG);	// przekroczenie zakresu liczb 32-bitowych
		}

		TCHAR c= *ptr++;
		val <<= 4;
		if (c>='a')
			val += c-'a'+10;
		else if (c>='A')
			val += c-'A'+10;
		else
			val += c-'0';
	} while (_istxdigit(*ptr));

	return CLeksem(CLeksem::N_HEX,SINT32(val));
}



CLeksem CAsm6502::get_dec_num()		// interpretacja liczby dziesiêtnej
{
	UINT32 val= 0;
	const TCHAR *tmp= ptr;

	if (!_istdigit(*ptr))
	{
		err_start = tmp;
		return CLeksem(CLeksem::ERR_NUM_DEC);	// oczekiwana cyfra
	}

	do
	{
		if (val > ~0u / 10)
		{
			err_start = tmp;
			return CLeksem(CLeksem::ERR_NUM_BIG); // przekroczenie zakresu liczb 32-bitowych
		}

		val *= 10;
		val += *ptr++ - '0';

	} while (_istdigit(*ptr));

	return CLeksem(CLeksem::N_DEC,SINT32(val));
}



CLeksem CAsm6502::get_bin_num()		// interpretacja liczby dwójkowej
{
	UINT32 val= 0;
	const TCHAR *tmp= ptr;

	if (*ptr!='0' && *ptr!='1')
	{
		err_start = tmp;
		return CLeksem(CLeksem::ERR_NUM_HEX); // oczekiwana cyfra liczby szesnastkowej
	}

	do
	{
		if (val & 0x80000000u)
		{
			err_start = tmp;
			return CLeksem(CLeksem::ERR_NUM_BIG); // przekroczenie zakresu liczb 32-bitowych
		}

		val <<= 1;
		if (*ptr++ == '1')
			val++;

	} while (*ptr=='0' || *ptr=='1');

	return CLeksem(CLeksem::N_BIN,SINT32(val));
}



CLeksem CAsm6502::get_char_num()		// interpretacja sta³ej znakowej
{
	TCHAR c1= *ptr++;	// pierwszy znak w apostrofie

	if (*ptr != '\'')
	{
		TCHAR c2 = *ptr++;
		if (*ptr != '\'')
		{
			err_start = ptr-2;  
			return CLeksem(CLeksem::ERR_NUM_CHR);
		}
		ptr++;		// ominiêcie zamykaj¹cego apostrofu
		return CLeksem(CLeksem::N_CHR2,((c2 & 0xFF)<<8)+(c1 & 0xFF));
	}
	else
	{
		ptr++;		// ominiêcie zamykaj¹cego apostrofu
		return CLeksem(CLeksem::N_CHR,c1 & 0xFF);
	}
}



//CLeksem
CLeksem::CLString* CAsm6502::get_ident()	// wyodrêbnienie napisu
{
	const TCHAR *start= ptr;
	TCHAR c= *ptr++;			// pierwszy znak

	if (!(_istalpha(c) || c=='_' || c=='.' || c=='?'))
	{
		err_start = start;
		return NULL;  //CLeksem(CLeksem::ERR_BAD_CHR);
	}

	while (__iscsym(*ptr))		// litera, cyfra lub '_'
		ptr++;

	CLeksem::CLString *pstr= new CLeksem::CLString(start,ptr-start);
	ident_start = start;		// zapamiêtanie po³o¿enia identyfikatora w wierszu
	ident_fin = ptr;

//  return CLeksem(pstr,0);
	return pstr;
}



CLeksem CAsm6502::get_string(TCHAR lim)		// wyodrêbnienie ³añcucha znaków
{
	const TCHAR *fin= _tcschr(ptr,lim);
  
	if (fin==NULL)
	{
		err_start = ptr;
		return CLeksem(CLeksem::ERR_STR_UNLIM);
	}

	CLeksem::CLString *pstr= new CLeksem::CLString(ptr,fin-ptr);

	ptr = fin+1;

	return CLeksem(pstr);
}


CLeksem CAsm6502::eat_space()			// ominiêcie odstêpu
{
	ptr--;
	while ( _istspace(*++ptr) && *ptr!=_T('\n') && *ptr!=_T('\r') )
		;		// "bia³e" znaki (ale nie CR)
	return CLeksem(CLeksem::L_SPACE);
}


bool CAsm6502::proc_instr(const CString &str, OpCode &code)
{
	ASSERT(str.GetLength() == 3);

	switch (_totupper(str[0]))		// spr. czy 'str' jest kodem instrukcji
	{
	case 'A':
		switch (_totupper(str[1]))
		{
		case 'D':
			if (_totupper(str[2])=='C')
				return code=C_ADC, true;
			break;
		case 'N':
			if (_totupper(str[2])=='D')
				return code=C_AND, true;
			break;
		case 'S':
			if (_totupper(str[2])=='L')
				return code=C_ASL, true;
			break;
		}
		break;

	case 'B':
		switch (_totupper(str[1]))
		{
		case 'B':
			if (bProc6502)
				break;
			if (_totupper(str[2])=='S')
				return code=C_BBS, true;
			else if (_totupper(str[2])=='R')
				return code=C_BBR, true;
			break;
		case 'C':
			if (_totupper(str[2])=='C')
				return code=C_BCC, true;
			else if (_totupper(str[2])=='S')
				return code=C_BCS, true;
			break;
		case 'E':
			if (_totupper(str[2])=='Q')
				return code=C_BEQ, true;
			break;
		case 'I':
			if (_totupper(str[2])=='T')
				return code=C_BIT, true;
			break;
		case 'M':
			if (_totupper(str[2])=='I')
				return code=C_BMI, true;
			break;
		case 'N':
			if (_totupper(str[2])=='E')
				return code=C_BNE, true;
			break;
		case 'P':
			if (_totupper(str[2])=='L')
				return code=C_BPL, true;
			break;
		case 'R':
			if (!bProc6502 && _totupper(str[2])=='A')
				return code=C_BRA, true;
			else if (_totupper(str[2])=='K')
				return code=C_BRK, true;
			break;
		case 'V':
			if (_totupper(str[2])=='C')
				return code=C_BVC, true;
			else if (_totupper(str[2])=='S')
				return code=C_BVS, true;
			break;
		}
		break;

	case 'C':
		switch (_totupper(str[1]))
		{
		case 'L':
			if (_totupper(str[2])=='C')
				return code=C_CLC, true;
			else if (_totupper(str[2])=='D')
				return code=C_CLD, true;
			else if (_totupper(str[2])=='I')
				return code=C_CLI, true;
			else if (_totupper(str[2])=='V')
				return code=C_CLV, true;
			break;
		case 'M':
			if (_totupper(str[2])=='P')
				return code=C_CMP, true;
			break;
		case 'P':
			if (_totupper(str[2])=='X')
				return code=C_CPX, true;
			else if (_totupper(str[2])=='Y')
				return code=C_CPY, true;
			break;
		}
		break;

	case 'D':
		if (_totupper(str[1])=='E')
			switch (_totupper(str[2]))
			{
			case 'A':
				return code=C_DEA, true;
			case 'C':
				return code=C_DEC, true;
			case 'X':
				return code=C_DEX, true;
			case 'Y':
				return code=C_DEY, true;
			}
		break;

	case 'E':
		if (_totupper(str[1])=='O' && _totupper(str[2])=='R')
			return code=C_EOR, true;
		break;

	case 'I':
		if (_totupper(str[1])=='N')
			switch (_totupper(str[2]))
			{
			case 'A':
				return code=C_INA, true;
			case 'C':
				return code=C_INC, true;
			case 'X':
				return code=C_INX, true;
			case 'Y':
				return code=C_INY, true;
			}
		break;

	case 'J':
		if (_totupper(str[1])=='M' && _totupper(str[2])=='P')
			return code=C_JMP, true;
		else if (_totupper(str[1])=='S' && _totupper(str[2])=='R')
			return code=C_JSR, true;
		break;

	case 'L':
		if (_totupper(str[1])=='D')
		{
			if (_totupper(str[2])=='A')
				return code=C_LDA, true;
			else if (_totupper(str[2])=='X')
				return code=C_LDX, true;
			else if (_totupper(str[2])=='Y')
				return code=C_LDY, true;
		}
		else if (_totupper(str[1])=='S' && _totupper(str[2])=='R')
			return code=C_LSR, true;
		break;

	case 'N':
		if (_totupper(str[1])=='O' && _totupper(str[2])=='P')
			return code=C_NOP, true;
		break;

	case 'O':
		if (_totupper(str[1])=='R' && _totupper(str[2])=='A')
			return code=C_ORA, true;
		break;

	case 'P':
		if (_totupper(str[1])=='H')
			switch (_totupper(str[2]))
			{
			case 'A':
				return code=C_PHA, true;
			case 'P':
				return code=C_PHP, true;
			case 'X':
				return code=C_PHX, true;
			case 'Y':
				return code=C_PHY, true;
			}
		else if (_totupper(str[1])=='L')
			switch (_totupper(str[2]))
			{
			case 'A':
				return code=C_PLA, true;
			case 'P':
				return code=C_PLP, true;
			case 'X':
				return code=C_PLX, true;
			case 'Y':
				return code=C_PLY, true;
			}
		break;

	case 'R':
		switch (_totupper(str[1]))
		{
		case 'O':
			if (_totupper(str[2])=='L')
				return code=C_ROL, true;
			else if (_totupper(str[2])=='R')
				return code=C_ROR, true;
			break;
		case 'M':
			if (!bProc6502 && _totupper(str[2])=='B')
				return code=C_RMB, true;
			break;
		case 'T':
			if (_totupper(str[2])=='I')
				return code=C_RTI, true;
			else if (_totupper(str[2])=='S')
				return code=C_RTS, true;
			break;
		}
		break;

	case 'S':
		switch (_totupper(str[1]))
		{
		case 'B':
			if (_totupper(str[2])=='C')
				return code=C_SBC, true;
			break;
		case 'E':
			if (_totupper(str[2])=='C')
				return code=C_SEC, true;
			else if (_totupper(str[2])=='D')
				return code=C_SED, true;
			else if (_totupper(str[2])=='I')
				return code=C_SEI, true;
			break;
		case 'M':
			if (!bProc6502 && _totupper(str[2])=='B')
				return code=C_SMB, true;
			break;
		case 'T':
			if (_totupper(str[2])=='A')
				return code=C_STA, true;
			else if (_totupper(str[2])=='X')
				return code=C_STX, true;
			else if (_totupper(str[2])=='Y')
				return code=C_STY, true;
			else if (!bProc6502 && _totupper(str[2])=='Z')
				return code=C_STZ, true;
			break;
		}
		break;

	case 'T':
		switch (_totupper(str[1]))
		{
		case 'A':
			if (_totupper(str[2])=='X')
				return code=C_TAX, true;
			else if (_totupper(str[2])=='Y')
				return code=C_TAY, true;
			break;
		case 'R':
			if (!bProc6502 && _totupper(str[2])=='B')
				return code=C_TRB, true;
			break;
		case 'S':
			if (!bProc6502 && _totupper(str[2])=='B')
				return code=C_TSB, true;
			if (_totupper(str[2])=='X')
				return code=C_TSX, true;
			break;
		case 'X':
			if (_totupper(str[2])=='A')
				return code=C_TXA, true;
			if (_totupper(str[2])=='S')
				return code=C_TXS, true;
			break;
		case 'Y':
			if (_totupper(str[2])=='A')
				return code=C_TYA, true;
			break;
		}
		break;
	}

	return false;
}


int __cdecl CAsm6502::asm_str_key_cmp(const void *elem1, const void *elem2)
{
	return _tcsicmp(((CAsm6502::ASM_STR_KEY *)elem1)->str, ((CAsm6502::ASM_STR_KEY *)elem2)->str);
}


bool CAsm6502::asm_instr(const CString &str, InstrType &it)
{				// spr. czy 'str' jest dyrektyw¹ asemblera
	static const ASM_STR_KEY instr[]=
	{		// dyrektywy asemblera w porz¹dku alfabetycznym
		".ASCII",	I_DB,		// def byte
		".ASCIS",	I_ASCIS,	// ascii + $80 ostatni bajt
		".BYTE",	I_DB,
		".DB",		I_DB,		// def byte
		".DBYTE",	I_DD,		// def double byte
		".DCB",		I_DCB,		// declare block
		".DD",		I_DD,		// def double byte
		".DS",		I_RS,		// reserve space (define space)
		".DW",		I_DW,		// def word
		".ELSE",	I_ELSE,
		".END",		I_END,		// zakoñczenie programu (pliku)
		".ENDIF",	I_ENDIF,	// koniec .IF
		".ENDM",	I_ENDM,		// koniec .MACRO
		".ENDR",	I_ENDR,		// koniec .REPEAT
		".ERROR",	I_ERROR,	// zg³oszenie b³êdu
		".EXITM",	I_EXITM,	// zakoñczenie rozwijania makra
		".IF",		I_IF,		// asemblacja warunkowa
		".INCLUDE",	I_INCLUDE,	// w³¹czenie pliku do asemblacji
		".IO_WND",	I_IO_WND,	// I/O terminal window size
		".MACRO",	I_MACRO,	// makrodefinicja
		".OPT",		I_OPT,		// opcje asemblera
		".ORG",		I_ORG,		// origin
		".REPEAT",	I_REPEAT,	// powtórka
		".REPT",	I_REPEAT,
		".ROM_AREA",I_ROM_AREA,	// protected memory area
		".RS",		I_RS,		// reserve space
		".SET",		I_SET,		// przypisanie wartoœci
		".START",	I_START,	// pocz¹tek programu (dla symulatora)
		".STR",		I_DS,		// def string
		".STRING",	I_DS,		// def string
		".WORD",	I_DW
	};
	ASM_STR_KEY find;
	find.str = str;

	void *ret= bsearch(&find, instr, sizeof instr / sizeof(ASM_STR_KEY), 
		sizeof(ASM_STR_KEY), asm_str_key_cmp);

	if (ret)
	{
		it = ((ASM_STR_KEY *)ret)->it;
		return true;
	}

	return false;
}


	// interpretacja argumentów rozkazu procesora
CAsm6502::Stat CAsm6502::proc_instr_syntax(CLeksem &leks, CodeAdr &mode, Expr &expr,
					   Expr &expr_bit, Expr &expr_zpg)
{
	static TCHAR x_idx_reg[]= "X";
	static TCHAR y_idx_reg[]= "Y";
	Stat ret;

	switch (leks.type)
	{
	case CLeksem::L_BRACKET_L:			// nawias '('
		leks = next_leks();		// kolejny niepusty leksem
		ret = expression(leks,expr);
		if (ret)				// niepoprawne wyra¿enie?
			return ret;
		if (expr.inf==Expr::EX_LONG)
			return ERR_NUM_LONG;		// za du¿a liczba, max $FFFF
		switch (leks.type)
		{
		case CLeksem::L_SPACE:
			ASSERT(false);
			break;

		case CLeksem::L_COMMA:
			if (bProc6502 && expr.inf!=Expr::EX_BYTE && expr.inf!=Expr::EX_UNDEF)
				return ERR_NUM_NOT_BYTE;	// za du¿a liczba, max $FF
			leks = next_leks();		// kolejny niepusty leksem
			if (leks.type!=CLeksem::L_IDENT)
				return ERR_IDX_REG_X_EXPECTED;
			if ((leks.GetString())->CompareNoCase(x_idx_reg))	// nie ma rejestru X?
				return ERR_IDX_REG_X_EXPECTED;
			leks = next_leks();		// kolejny niepusty leksem
			if (leks.type != CLeksem::L_BRACKET_R)
				return ERR_BRACKET_R_EXPECTED;	// brak nawiasu ')'
			if (expr.inf==Expr::EX_LONG)
				return ERR_NUM_LONG;
			if (expr.inf==Expr::EX_WORD)
				mode = A_ABSI_X;
			else if (expr.inf==Expr::EX_BYTE)
				mode = A_ZPGI_X;
			else
				mode = A_ABSIX_OR_ZPGIX;
			leks = next_leks();		// kolejny niepusty leksem
			return OK;

		case CLeksem::L_BRACKET_R:
			leks = next_leks();		// kolejny niepusty leksem
			if (leks.type != CLeksem::L_COMMA)
			{
				switch (expr.inf)
				{
				case Expr::EX_UNDEF:		// nieokreœlona wartoœæ wyra¿enia?
					mode = A_ABSI_OR_ZPGI;		// niezdeterminowany tryb adresowania
					break;
				case Expr::EX_BYTE:		// wyra¿enie < 256 ?
					mode = A_ZPGI;
					break;
				case Expr::EX_WORD:
					mode = A_ABSI;
					break;
				default:
					ASSERT(false);
				}
				return OK;
			}
			leks = next_leks();		// kolejny niepusty leksem
			if (leks.type!=CLeksem::L_IDENT)
				return ERR_IDX_REG_Y_EXPECTED;
			if ((leks.GetString())->CompareNoCase(y_idx_reg))	// nie ma rejestru Y?
				return ERR_IDX_REG_Y_EXPECTED;
			if (expr.inf == Expr::EX_WORD)
				return ERR_INDIRECT_BYTE_EXPECTED;
			mode = A_ZPGI_Y;
			leks = next_leks();		// kolejny niepusty leksem
			return OK;

		default:
			return ERR_COMMA_OR_BRACKET_EXPECTED;
		}
		return OK;


	case CLeksem::L_HASH:			// argument natychmiastowy '#'
		leks = next_leks();		// kolejny niepusty leksem
		ret = expression(leks,expr);
		if (ret)				// niepoprawne wyra¿enie?
			return ret;

		if (expr.inf!=Expr::EX_BYTE && expr.inf!=Expr::EX_UNDEF)
			return ERR_NUM_NOT_BYTE;	// za du¿a liczba, max $FF
		mode = A_IMM;
		if (!bProc6502 && leks.type==CLeksem::L_COMMA)	// przecinek po wyra¿eniu?
		{
			leks = next_leks();		// kolejny niepusty leksem
			if (expr.inf == Expr::EX_BYTE && abs(expr.value) > 7)
				return ERR_NOT_BIT_NUM;	// z³y numer bitu
			ret = expression(leks,expr_zpg);
			if (ret)				// niepoprawne wyra¿enie?
				return ret;
			expr_bit = expr;		// wyra¿enie - numer bitu
			if (expr_zpg.inf!=Expr::EX_BYTE && expr_zpg.inf!=Expr::EX_UNDEF)
				return ERR_NUM_NOT_BYTE;	// za du¿a liczba, max $FF
			if (leks.type==CLeksem::L_COMMA)	// przecinek po wyra¿eniu?
			{
				leks = next_leks();		// kolejny niepusty leksem
				ret = expression(leks,expr);
				if (ret)				// niepoprawne wyra¿enie?
					return ret;
				mode = A_ZREL;		// tryb adr. dla BBS i BBR
			}
			else
				mode = A_ZPG2;		// tryb adr. dla RMB i SMB
		}
		return OK;


	default:					// wyra¿enie lub nic
		if (!is_expression(leks))		// pocz¹tek wyra¿enia?
		{
			mode = A_IMP_OR_ACC;
			return OK;
		}
		ret = expression(leks,expr);
		if (ret)				// niepoprawne wyra¿enie?
			return ret;
		if (expr.inf==Expr::EX_LONG)
			return ERR_NUM_LONG;
		//      if (leks.type==CLeksem::L_SPACE)
		//        leks = next_leks();
		if (leks.type!=CLeksem::L_COMMA)	// nia ma przecinka po wyra¿eniu?
		{
			switch (expr.inf)
			{
			case Expr::EX_UNDEF:		// nieokreœlona wartoœæ?
				mode = A_ABS_OR_ZPG;	// niezdeterminowany tryb adresowania
				break;
			case Expr::EX_BYTE:		// wyra¿enie < 256 ?
				mode = A_ZPG;
				break;
			case Expr::EX_WORD:
				mode = A_ABS;
				break;
			default:
				ASSERT(false);
			}
			return OK;
		}
		leks = next_leks();		// po przecinku rejestr indeksowy
		if (leks.type!=CLeksem::L_IDENT)
			return ERR_IDX_REG_EXPECTED;
		const CString &str= *leks.GetString();
		bool reg_x= false;
		if (str.CompareNoCase(x_idx_reg)==0)	// rejestr indeksowy 'X'?
			reg_x = true;
		else if (str.CompareNoCase(y_idx_reg))	// nie rejestr 'Y'?
			return ERR_IDX_REG_EXPECTED;

		switch (expr.inf)
		{
		case Expr::EX_UNDEF:		// nieokreœlona wartoœæ wyra¿enia?
			mode = reg_x ? A_ABSX_OR_ZPGX : A_ABSY_OR_ZPGY;	// niezdeterminowany tryb adresowania
			break;
		case Expr::EX_BYTE:		// wyra¿enie < 256 ?
			mode = reg_x ? A_ZPG_X : A_ZPG_Y;
			break;
		case Expr::EX_WORD:
			mode = reg_x ? A_ABS_X : A_ABS_Y;
			break;
		default:
			ASSERT(false);
		}
		leks = next_leks();		// kolejny niepusty leksem
		return OK;
	}
}

//-----------------------------------------------------------------------------

	// interpretacja dyrektywy
CAsm6502::Stat CAsm6502::asm_instr_syntax_and_generate(CLeksem &leks, InstrType it, const CString *pLabel)
{
	Stat ret;
	int def= -2;

	switch (it)
	{
	case I_ORG:		// origin
		{
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane s³owo
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			if (expr.value < 0)
				return ERR_NUM_NEGATIVE;	// oczekiwana wartoœæ nieujemna
			if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
				return ERR_NUM_LONG;
			if (program_start==~0u)		// pocz¹tek programu jeszcze nie zdefiniowany?
			{
				program_start = origin = expr.value & mem_mask;
				if (markArea && pass==2)
					markArea->SetStart(origin);
			}
			else
			{
				if (markArea && pass==2 && origin!=-1)
					markArea->SetEnd(UINT16(origin-1));
				origin = expr.value & mem_mask;
				if (markArea && pass==2)
					markArea->SetStart(origin);
			}
			if (pass==2 && listing.IsOpen())
				listing.AddCodeBytes((UINT16)origin);
			break;
		}

	case I_START:	// pocz¹tek programu dla symulatora
		{
			if (pLabel)			// jest etykieta przed .START ?
				return ERR_LABEL_NOT_ALLOWED;
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane s³owo
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return pass==1 ? OK : ERR_UNDEF_EXPR;
			if (expr.value < 0)
				return ERR_NUM_NEGATIVE;	// oczekiwana wartoœæ nieujemna
			if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
				return ERR_NUM_LONG;
			program_start = expr.value & mem_mask;
			if (listing.IsOpen())
				listing.AddValue(UINT16(program_start));
			break;
		}

	case I_DD:		// def double byte
		def++;
	case I_DW:		// def word
		def++;
	case I_DB:		// def byte
		def++;
	case I_DS:		// def string
		def++;
	case I_ASCIS:	// def ascii + $80
		{
			UINT32 cnt_org= origin;	// miejsce na bajt d³ugoœci danych (tylko .STR)
			int cnt= 0;		// d³ugoœæ danych (inf. dla .STR)
			if (def == -1)		// jeœli .STR to zarezerwowanie bajtu
			{
				ret = inc_prog_counter(1);
				if (ret)
					return ret;		// dane nie zmieszcz¹ siê w pamiêci systemu 6502
			}
			for (;;)
			{
				Expr expr;
				ret = expression(leks,expr,def<=0);	// oczekiwane wyra¿enie
				if (ret)
					return ret;
				if (expr.inf==Expr::EX_STRING)		// tekst?
				{
					ASSERT(def<=0);	// tekst tylko w .DB i .STR
					const CString &str= expr.string;
					UINT32 org= origin;
					//	  if (origin > 0xFFFF)
					//	    return ERR_UNDEF_ORIGIN;
					int len= str.GetLength();
					cnt += len;
					ret = inc_prog_counter(len);
					if (ret)
						return ret;		// ci¹g znaków nie zmieœci³ siê w pamiêci systemu 6502
					if (pass==2 && out)
					{
						for (int i=0; org<origin; org++,i++)
							(*out)[org] = UINT8(str[i]);
					}
					//	  leks = next_leks();
				}
				else if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
					return ERR_NUM_LONG;
				else if (pass==1)
				{
					if (def==0 && expr.inf==Expr::EX_WORD)
						return ERR_NUM_NOT_BYTE;	// za du¿a liczba, max $FF
					ret = inc_prog_counter(def>0 ? 2 : 1);
					if (ret)
						return ret;		// dane nie zmieszcz¹ siê w pamiêci systemu 6502
					cnt++;
				}
				else
				{
					if (expr.inf==Expr::EX_UNDEF)
						return ERR_UNDEF_EXPR;
					if (def==0 && expr.inf==Expr::EX_WORD)
						return ERR_NUM_NOT_BYTE;	// za du¿a liczba, max $FF
					UINT32 org= origin;
					//	  if (origin > 0xFFFF)
					//	    return ERR_UNDEF_ORIGIN;
					ret = inc_prog_counter(def>0 ? 2 : 1);
					if (ret)
						return ret;		// dane nie zmieszcz¹ siê w pamiêci systemu 6502
					if (out)
					{
						switch (def)
						{
						case -1:		// .str
							cnt++;
						case -2:		// .ascis
						case 0:		// .db
							(*out)[org] = UINT(expr.value & 0xFF);
							break;
						case 1:		// .dw
							(*out)[org] = UINT(expr.value & 0xFF);
							(*out)[org+1] = UINT((expr.value>>8) & 0xFF);
							break;
						case 2:		// .dd
							(*out)[org] = UINT((expr.value>>8) & 0xFF);
							(*out)[org+1] = UINT(expr.value & 0xFF);
							break;
						}
					}
				}
				if (leks.type != CLeksem::L_COMMA)	// po przecinku (jeœli jest) kolejne dane
				{
					if (def == -1)		// .STR ?
					{
						if (cnt >= 256)
							return ERR_STRING_TOO_LONG;
						if (pass==2 && out)
							(*out)[cnt_org] = UINT8(cnt);
					}
					else if (def == -2)		// .ASCIS ?
					{
						if (pass==2 && out)
							(*out)[origin-1] ^= UINT8(0x80);
					}
					return OK;			// nie ma przecinka - koniec danych
				}
				leks = next_leks();
			}
			if (pass=2 && listing.IsOpen())
				listing.AddBytes(UINT16(cnt_org),mem_mask,out->Mem(),origin-cnt_org);

		}

	case I_DCB:		// declare block
		{
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane s³owo
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			if (expr.value < 0)
				return ERR_NUM_NEGATIVE;	// oczekiwana wartoœæ nieujemna
			if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
				return ERR_NUM_LONG;
			UINT16 org= origin;
			ret = inc_prog_counter(expr.value);
			if (ret)
				return ret;

			if (leks.type != CLeksem::L_COMMA)	// po przecinku kolejne dane
				return OK;

			leks = next_leks();
			Expr init;
			ret = expression(leks,init);	// oczekiwany bajt
			if (ret)
				return ret;
			if (init.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ?
				return pass==1 ? OK : ERR_UNDEF_EXPR;
			if (init.inf!=Expr::EX_BYTE)	// za du¿a wartoœæ?
				return ERR_NUM_NOT_BYTE;
			if (pass==2 && out)
			{
				int len= origin-org;
				for (int i=0; org<origin; org++,i++)
					(*out)[org] = UINT8(init.value);
				if (len && listing.IsOpen())
					listing.AddBytes(UINT16(org-len),mem_mask,out->Mem(),len);
			}
			break;
		}

	case I_RS:		// reserve space
		{
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane s³owo
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			if (expr.value < 0)
				return ERR_NUM_NEGATIVE;	// oczekiwana wartoœæ nieujemna
			if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
				return ERR_NUM_LONG;
			if (origin > 0xFFFF)
				return ERR_UNDEF_ORIGIN;
			origin += expr.value & 0xFFFF;	// zarezerwowana przestrzeñ
			if (origin > mem_mask)
				return ERR_PC_WRAPED;		// licznik rozkazów "przewin¹³ siê"
			if (pass==2 && listing.IsOpen())
				listing.AddCodeBytes(UINT16(origin));
			break;
		}

	case I_END:		// zakoñczenie
		{
			if (!is_expression(leks))		// nie ma wyra¿enia?
				return STAT_FIN;
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane s³owo
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return pass==1 ? OK : ERR_UNDEF_EXPR;
			if (expr.value < 0)
				return ERR_NUM_NEGATIVE;	// oczekiwana wartoœæ nieujemna
			if (expr.inf==Expr::EX_LONG)	// za du¿a wartoœæ
				return ERR_NUM_LONG;
			program_start = expr.value & mem_mask;
			return STAT_FIN;
		}

	case I_ERROR:	// zg³oszenie b³êdu
		if (pLabel)			// jest etykieta przed .ERROR ?
			return ERR_LABEL_NOT_ALLOWED;

		if (leks.type==CLeksem::L_STR)
		{
			Expr expr;
			ret = expression(leks,expr,true);	// oczekiwany tekst
			if (ret)
				return ret;
			if (expr.inf!=Expr::EX_STRING)
				return ERR_STR_EXPECTED;
			user_error_text = expr.string;
		}
		else
			user_error_text.Empty();
/*
      if (leks.type==CLeksem::L_STR)
      {
        user_error_text = *leks.GetString();
	leks = next_leks();
      }
*/
		return STAT_USER_DEF_ERR;		// b³¹d u¿ytkownika

	case I_INCLUDE:	// w³¹czenie plku
		if (pLabel)			// jest etykieta przed .INCLUDE ?
			return ERR_LABEL_NOT_ALLOWED;

		if (leks.type==CLeksem::L_STR)
		{
			Expr expr;
			ret = expression(leks,expr,true);	// oczekiwany tekst
			if (ret)
				return ret;
			if (expr.inf!=Expr::EX_STRING)
				return ERR_STR_EXPECTED;

			CString strPath= expr.string;
			strPath.Replace('/', '\\');
			if (::PathIsRelative(strPath))	// if path is relative combine it with current dir
			{
				char szBuf[MAX_PATH]= {0};
				::GetCurrentDirectory(MAX_PATH, szBuf);
				char szPath[MAX_PATH];
				::PathCombine(szPath, szBuf, strPath);
				strPath = szPath;
			}
			// canonicalize the path to make sure it always looks the same
			// or debug info won't be found by simulator
			char szPath[MAX_PATH];
			::PathCanonicalize(szPath, strPath);
			include_fname = szPath;
		}
		else
			return ERR_STR_EXPECTED;	// oczekiwany ³añcuch znaków
		return STAT_INCLUDE;

	case I_IF:
		{
			if (pLabel)			// jest etykieta przed .IF ?
				return ERR_LABEL_NOT_ALLOWED;
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane wyra¿enie
			if (ret)
				return ret;
			//      leks = next_leks();
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return check_line ? OK : STAT_IF_UNDETERMINED;
			return expr.value ? STAT_IF_TRUE : STAT_IF_FALSE;
		}

	case I_ELSE:
		if (pLabel)			// jest etykieta przed .ELSE ?
			return ERR_LABEL_NOT_ALLOWED;
//      leks = next_leks();
		return STAT_ELSE;

	case I_ENDIF:
		if (pLabel)			// jest etykieta przed .ENDIF ?
			return ERR_LABEL_NOT_ALLOWED;
//      leks = next_leks();
		return STAT_ENDIF;

	case I_MACRO:			// makrodefinicja
		{
			if (!pLabel)			// nie ma etykiety przed .MACRO ?
				return ERR_MACRONAME_REQUIRED;
			if ((*pLabel)[0] == LOCAL_LABEL_CHAR)	// etykiety lokalne niedozwolone
				return ERR_BAD_MACRONAME;

			CMacroDef *pMacro= NULL;
			if (pass == 1)
			{
				pMacro = get_new_macro_entry();	// miejsce na now¹ makrodefinicjê
				ret = def_macro_name(*pLabel,CIdent(CIdent::I_MACRONAME,get_last_macro_entry_index()));
				if (ret)
					return ret;
				if (!check_line)
					pMacro->SetFileUID(text->GetFileUID());
			}
			else if (pass == 2)
			{
				ret = chk_macro_name(*pLabel);
				if (ret)
					return ret;
				return STAT_MACRO;		// makro zosta³o ju¿ zarejestrowane
			}

			ASSERT(pMacro);
			pMacro->m_strName = *pLabel;	// zapamiêtanie nazwy makra w opisie makrodefinicji

			for (bool bRequired= false; ; )
			{
				if (leks.type == CLeksem::L_IDENT)	// nazwa parametru?
				{
					if (pMacro->AddParam(*leks.GetString()) < 0)
						return ERR_PARAM_ID_REDEF;		// powtórzona nazwa parametru
				}
				else if (leks.type == CLeksem::L_MULTI)	// wielokropek?
				{
					pMacro->AddParam(MULTIPARAM);
					leks = next_leks();
					break;
				}
				else
				{
					if (bRequired)		// po przecinku wymagany paramter makra
						return ERR_PARAM_DEF_REQUIRED;
					break;
				}
				leks = next_leks();
				if (leks.type == CLeksem::L_COMMA)	// przecinek?
				{
					leks = next_leks();
					bRequired = true;
				}
				else
					break;
			}
			in_macro = pMacro;		// aktualnie nagrywane makro
			return STAT_MACRO;
		}

	case I_ENDM:			// koniec makrodefinicji
		if (pLabel)
			return ERR_LABEL_NOT_ALLOWED;
		return ERR_SPURIOUS_ENDM;		// .ENDM bez wczeœniejszego .MACRO

	case I_EXITM:			// opuszczenie makrodefinicji
		if (pLabel)
			return ERR_LABEL_NOT_ALLOWED;
		return expanding_macro ? STAT_EXITM : ERR_SPURIOUS_EXITM;


	case I_SET:				// przypisanie wartoœci zmiennej
		{
			if (!pLabel)			// nie ma etykiety przed .SET ?
				return ERR_LABEL_EXPECTED;
			Expr expr;
			ret = expression(leks,expr);	// oczekiwane wyra¿enie
			if (ret)
				return ret;
			CIdent::IdentInfo info= expr.inf==Expr::EX_UNDEF ? CIdent::I_UNDEF : CIdent::I_VALUE;
			ret = pass == 1 ? def_ident(*pLabel, CIdent(info, expr.value, true)) :
							  chk_ident_def(*pLabel, CIdent(info, expr.value, true));
			if (ret)
				return ret;
			if (pass == 2 && listing.IsOpen())
				listing.AddValue(UINT16(expr.value));
			break;
		}


	case I_REPEAT:			// powtórka wierszy
		{
			Expr expr;
			ret = expression(leks, expr);	// oczekiwane wyra¿enie
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			if (expr.value < 0 || expr.value > 0xFFFF)
				return ERR_BAD_REPT_NUM;
			reptInit = expr.value;
			return STAT_REPEAT;
		}

	case I_ENDR:			// koniec powtórki
		return ERR_SPURIOUS_ENDR;		// .ENDR bez wczeœniejszego .REPEAT


	case I_OPT:				// opcje asemblera
		{
			if (pLabel)
				return ERR_LABEL_NOT_ALLOWED;
			static const TCHAR *opts[]=
			{	"Proc6502", "Proc65c02", "Proc6501", "CaseSensitive", "CaseInsensitive" };
			for (;;)
			{
				if (leks.type == CLeksem::L_IDENT)	// nazwa opcji?
				{
					if (leks.GetString()->CompareNoCase(opts[0]) == 0)
						bProc6502 = true;
					else if (leks.GetString()->CompareNoCase(opts[1]) == 0)
						bProc6502 = false;
					else if (leks.GetString()->CompareNoCase(opts[2]) == 0)
						bProc6502 = false;
					else if (leks.GetString()->CompareNoCase(opts[3]) == 0)
						case_insensitive = false;
					else if (leks.GetString()->CompareNoCase(opts[4]) == 0)
						case_insensitive = true;
					else
						return ERR_OPT_NAME_UNKNOWN;	// nierozpoznana nazwa opcji
				}
				else
					return ERR_OPT_NAME_REQUIRED;		// oczekiwana nazwa opcji
				leks = next_leks();
				if (leks.type == CLeksem::L_COMMA)	// przecinek?
					leks = next_leks();
				else
					break;
			}
			break;
		}

	case I_ROM_AREA:	// protected memory area
		{
			Expr addr_from;
			ret = expression(leks, addr_from);		// oczekiwane s³owo
			if (ret)
				return ret;
			if (addr_from.inf == Expr::EX_UNDEF)	// nieokreœlona wartoœæ
			{
				if (pass == 2)
					return ERR_UNDEF_EXPR;
			}
			else if (addr_from.value < 0)
				return ERR_NUM_NEGATIVE;			// oczekiwana wartoœæ nieujemna
			else if (addr_from.inf == Expr::EX_LONG)		// za du¿a wartoœæ
				return ERR_NUM_LONG;

			if (leks.type != CLeksem::L_COMMA)		// po przecinku kolejne dane
				return ERR_CONST_EXPECTED;

			leks = next_leks();
			Expr addr_to;
			ret = expression(leks, addr_to);		// expected word
			if (ret)
				return ret;
			if (addr_to.inf == Expr::EX_UNDEF)		// nieokreœlona wartoœæ?
			{
				if (pass == 2)
					return ERR_UNDEF_EXPR;
			}
			else if (addr_to.value < 0)
				return ERR_NUM_NEGATIVE;			// oczekiwana wartoœæ nieujemna
			else if (addr_to.inf == Expr::EX_LONG)	// za du¿a wartoœæ?
				return ERR_NUM_LONG;

//			if (pass == 2)		// do it once (avoid first pass; it's called for line checking)
			if (!check_line)
			{
				if (addr_from.value > addr_to.value)	// valid range?
					return ERR_NO_RANGE;

				CSym6502::s_bWriteProtectArea = addr_to.value != addr_from.value;
				if (CSym6502::s_bWriteProtectArea)
				{
					CSym6502::s_uProtectFromAddr = addr_from.value;
					CSym6502::s_uProtectToAddr = addr_to.value;
				}
			}
		}
		break;

	case I_IO_WND:		// size of terminal window
		{
			Expr width;
			ret = expression(leks, width);			// oczekiwane s³owo
			if (ret)
				return ret;
			if (width.inf == Expr::EX_UNDEF)		// nieokreœlona wartoœæ
			{
				if (pass == 2)
					return ERR_UNDEF_EXPR;
			}
			else if (width.value < 0)
				return ERR_NUM_NEGATIVE;			// oczekiwana wartoœæ nieujemna
			else if (width.inf != Expr::EX_BYTE)	// za du¿a wartoœæ?
				return ERR_NUM_NOT_BYTE;

			if (leks.type != CLeksem::L_COMMA)		// po przecinku kolejne dane
				return ERR_CONST_EXPECTED;

			leks = next_leks();
			Expr height;
			ret = expression(leks, height);			// expected word
			if (ret)
				return ret;
			if (height.inf == Expr::EX_UNDEF)		// nieokreœlona wartoœæ?
			{
				if (pass == 2)
					return ERR_UNDEF_EXPR;
			}
			else if (height.value < 0)
				return ERR_NUM_NEGATIVE;			// oczekiwana wartoœæ nieujemna
			else if (height.inf != Expr::EX_BYTE)	// za du¿a wartoœæ?
				return ERR_NUM_NOT_BYTE;

			if (pass == 2)		// do it once (avoid first pass; it's called for line checking)
			{
				ASSERT(width.value >= 0 && height.value >= 0);
				if (width.value == 0 || height.value == 0)
					return ERR_NUM_ZERO;

				//TODO: potential problem if window exists
				CIOWindow::m_nInitW = width.value;
				CIOWindow::m_nInitH = height.value;
			}
		}
		break;

	default:
		ASSERT(false);
	}

	return OK;
}

//-----------------------------------------------------------------------------

const CString CSource::s_strEmpty;


// wczytanie argumentów wywo³ania makra
CAsm::Stat CMacroDef::ParseArguments(CLeksem &leks, CAsm6502 &asmb)
{
	bool get_param= true;
	bool first_param= true;
	Stat ret;
	int count= 0;

	int required= m_nParams >= 0 ? m_nParams : -m_nParams - 1;	// iloœæ wymaganych arg.

	m_strarrArgs.RemoveAll();
	m_narrArgs.RemoveAll();
	m_arrArgType.RemoveAll();
	m_nParamCount = 0;
//  leks = asmb.next_leks();

	if (m_nParams == 0)		// makro bezparametrowe?
		return OK;

	for (;;)
		if (get_param)		// ew. kolejny argument
			switch (leks.type)
			{
			case CLeksem::L_STR:		// ci¹g znaków?
				m_strarrArgs.Add(*leks.GetString());
				m_narrArgs.Add(strlen(*leks.GetString()));
				m_arrArgType.Add(STR);
				count++;
				get_param = false;		// parametr ju¿ zinterpretowany
				first_param = false;		// pierwszy parametr ju¿ wczytany
				leks = asmb.next_leks();
				break;
			default:
				if (asmb.is_expression(leks))	// wyra¿enie?
				{
					Expr expr;
					ret = asmb.expression(leks,expr);
					if (ret)
						return ret;
					if (expr.inf == Expr::EX_UNDEF)	// wartoœæ niezdeterminowana
					{
						m_strarrArgs.Add(_T(""));
						m_narrArgs.Add(0);
						m_arrArgType.Add(UNDEF_EXPR);
					}
					else if (expr.inf == Expr::EX_STRING)	// tekst
					{
						m_strarrArgs.Add(expr.string);
						m_narrArgs.Add(strlen(expr.string));
						m_arrArgType.Add(STR);
					}
					else
					{
						CString num;
						num.Format("%ld",expr.value);
						m_strarrArgs.Add(num);
						m_narrArgs.Add(expr.value);
						m_arrArgType.Add(NUM);
					}
					count++;
					get_param = false;		// parametr ju¿ zinterpretowany
				}
				else
				{
					if (count < required)
						return ERR_PARAM_REQUIRED;	// za ma³o parametrów wywo³ania makra
					if (!first_param)
						return ERR_PARAM_REQUIRED;	// po przecinku trzeba podaæ kolejny parametr
					m_nParamCount = count;
					return OK;
				}
			}
		else			// za argumentem przecinek, œrednik lub koniec
		{
			if (count==required && m_nParams>0)
			{
				m_nParamCount = count;
				return OK;		// wszystkie wymagane parametry ju¿ wczytane
			}
			switch (leks.type)
			{
			case CLeksem::L_COMMA:		// przecinek
				get_param = true;		// nastêpny parametr
				leks = asmb.next_leks();
				break;
			default:
				if (count < required)
					return ERR_PARAM_REQUIRED;	// za ma³o parametrów wywo³ania makra
				m_nParamCount = count;
				return OK;
			}
		}
}

// odszukanie parametru 'param_name' aktualnego makra
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, const CString param_name, Expr &expr, bool &found, CAsm6502 &asmb)
{
	CIdent ident;
	if (!param_names.lookup(param_name,ident))	// odszukanie parametru o danej nazwie
	{
		found = false;
		return OK;
	}
	found = true;
	leks = asmb.next_leks(false);
	return ParamLookup(leks,ident.val,expr,asmb);
}


// odszukanie wartoœci parametru numer 'param_number' aktualnego makra
CAsm::Stat CMacroDef::ParamLookup(CLeksem &leks, int param_number, Expr &expr, CAsm6502 &asmb)
{
	bool special= param_number == -1;	// zmienna %0 ? (nie parametr)
	if (leks.type == CLeksem::L_STR_ARG)	// odwo³anie do wartoœci znakowej parametru?
	{
		if (!special && (param_number >= m_nParamCount || param_number < 0))
			return ERR_EMPTY_PARAM;
		if (special)			// odwo³anie do %0$ -> co oznacza nazwê makra
			expr.string = m_strName;
		else
		{
			ASSERT(m_arrArgType.GetSize() > param_number);
			if (m_arrArgType[param_number] != STR)	// spr. czy zmienna ma wartoœæ tekstow¹
				return ERR_NOT_STR_PARAM;
			ASSERT(m_strarrArgs.GetSize() > param_number);
			expr.string = m_strarrArgs[param_number];
		}
		expr.inf = Expr::EX_STRING;
		leks = asmb.next_leks();
		return OK;
	}
	else if (leks.type == CLeksem::L_SPACE)
		leks = asmb.next_leks();

	if (special)	// odwo³anie do %0 -> iloœæ aktualnych parametrów w wywo³aniu makra
	{
		expr.inf = Expr::EX_LONG;
		expr.value = m_nParamCount;
	}
	else		// odwo³anie do aktualnego parametru
	{
		if (param_number >= m_nParamCount || param_number < 0)
			return ERR_EMPTY_PARAM;		// parametru o takim numerze nie ma
		ASSERT(m_arrArgType.GetSize() > param_number);
		switch (m_arrArgType[param_number])	// aktualny typ parametru
		{
		case NUM:		// parametr liczbowy
		case STR:		// parametr tekstowy (podawana jest jego d³ugoœæ)
			ASSERT(m_narrArgs.GetSize() > param_number);
			expr.inf = Expr::EX_LONG;
			expr.value = m_narrArgs[param_number];
			break;
		case UNDEF_EXPR:	// parametr liczbowy, wartoœæ niezdefiniowana
			ASSERT(m_narrArgs.GetSize() > param_number);
			expr.inf = Expr::EX_UNDEF;
			expr.value = 0;
			break;
		default:
			ASSERT(false);
			break;
		}
	}

	return OK;
}


// spr. sk³adni odwo³ania do parametru makra (tryb sprawdzania wiersza)
CAsm::Stat CMacroDef::AnyParamLookup(CLeksem &leks, CAsm6502 &asmb)
{
	if (leks.type == CLeksem::L_STR_ARG)	// odwo³anie do wartoœci znakowej parametru?
	{
		leks = asmb.next_leks();
		return OK;
	}
	else if (leks.type == CLeksem::L_SPACE)
		leks = asmb.next_leks();

	return OK;
}


const TCHAR* CMacroDef::GetCurrLine(CString &str)	// odczyt aktualnego wiersza makra
{
	ASSERT(m_nLineNo >= 0);
	if (m_nLineNo < GetSize())
	{
		str = GetLine(m_nLineNo++);
		return (const TCHAR *)str;
	}
	else				// koniec wierszy?
	{
		ASSERT(m_nLineNo == GetSize());
		return NULL;
	}
}

//-----------------------------------------------------------------------------


CAsm6502::Stat CAsm6502::CheckLine(const TCHAR *str, int &instr_idx_start, int &instr_idx_fin)
{
	Stat ret;
	ptr = str;  //.GetBuffer(0);

	instr_idx_start = instr_idx_fin = 0;

	try
	{
		pass = 1;
		local_area = 0;
		macro_local_area = 0;
		origin = 0;
		ret = assemble_line();
	}
	catch (CMemoryException *)
	{
		ret = ERR_OUT_OF_MEM;
	}

	if (instr_start)
	{
		instr_idx_start = instr_start - str;
		instr_idx_fin = instr_fin - str;
	}

//  str.ReleaseBuffer();
	if (ret < OK)
		ret = OK;
//  if (ret == STAT_FIN || ret == STAT_USER_DEF_ERR || ret == STAT_INCLUDE)
//    ret = OK;
	switch (ret)	// nie wszystke b³êdy s¹ b³êdami w trybie analizowania jednego wiersza
	{
	case ERR_UNDEF_EXPR:
	case ERR_UNKNOWN_INSTR:
	case ERR_SPURIOUS_ENDM:
	case ERR_SPURIOUS_EXITM:
	case ERR_SPURIOUS_ENDR:
		ret = OK;
		break;
	}
	ASSERT(ret >= OK);

	return ret;
}


CAsm6502::Stat CAsm6502::look_for_endif()		// szukanie .IF, .ENDIF lub .ELSE
{
	CLeksem leks= next_leks(false);	// kolejny leksem, byæ mo¿e pusty (L_SPACE)
	bool labelled= false;

	switch (leks.type)
	{
	case CLeksem::L_IDENT:	// etykieta
	case CLeksem::L_IDENT_N:	// etykieta numerowana
		leks = next_leks();
		if (leks.type == CLeksem::L_IDENT_N)
		{
			Expr expr(0);
			Stat ret = factor(leks,expr);
			if (ret)
				return ret;
		}
		labelled = true;
		switch (leks.type)
		{
		case CLeksem::L_LABEL:	// znak ':'
			leks = next_leks();
			if (leks.type!=CLeksem::L_ASM_INSTR)
				return OK;
			break;
		case CLeksem::L_ASM_INSTR:
			break;
		default:
			return OK;
		}
		//      leks = next_leks();
		break;

	case CLeksem::L_SPACE:	// odstêp
		leks = next_leks();
		if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
			return OK;
		break;

	case CLeksem::L_COMMENT:	// komentarz
	case CLeksem::L_CR:		// koniec wiersza
		return OK;

	case CLeksem::L_FIN:	// koniec tekstu
		return STAT_FIN;
		break;

	default:
		return ERR_UNEXP_DAT;
	}

	ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

	switch (leks.GetInstr())
	{
	case I_IF:
		return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_IF_UNDETERMINED;
	case I_ELSE:
		return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ELSE;
	case I_ENDIF:
		return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ENDIF;
	default:
		return OK;
	}
}


CAsm6502::Stat CAsm6502::assemble_line()	// interpretacja wiersza
{
	enum			// stany automatu
	{
		START,
			AFTER_LABEL,
			INSTR,
			EXPR,
			COMMENT,
			FINISH
	} state= START;
	CString label;	// pomocnicza zmienna do zapamiêtania identyfikatora
	bool labelled= false;	// flaga wyst¹pienia etykiety
	Stat ret, ret_stat= OK;
	instr_start = NULL;
	instr_fin = NULL;
	ident_start = NULL;
	ident_fin = NULL;

	CLeksem leks= next_leks(false);	// kolejny leksem, byæ mo¿e pusty (L_SPACE)

	for (;;)
	{
		switch (state)
		{
		case START:			// pocz¹tek wiersza
			switch (leks.type)
			{
			case CLeksem::L_IDENT:	// etykieta
				label = *leks.GetString();	// zapamiêtanie identyfikatora
				state = AFTER_LABEL;
				leks = next_leks();
				break;
			case CLeksem::L_IDENT_N:	// etykieta numerowana
				{
					CLeksem ident(leks);
					Expr expr(0);
					leks = next_leks();
					ret = factor(leks,expr);
					if (ret)
						return ret;
					if (!check_line)
					{
						if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
							return ERR_UNDEF_EXPR;
						ident.Format(expr.value);	// znormalizowanie postaci etykiety
						label = *ident.GetString();	// zapamiêtanie identyfikatora
					}
					else
						label = _T("x");
					state = AFTER_LABEL;
					break;
				}
			case CLeksem::L_SPACE:	// po odstêpie ju¿ nie mo¿e byæ etykiety
				state = INSTR;
				leks = next_leks();
				break;
			case CLeksem::L_COMMENT:	// komentarz
				state = COMMENT;
				break;
			case CLeksem::L_CR:		// koniec wiersza
			case CLeksem::L_FIN:		// koniec tekstu
				state = FINISH;
				break;
			default:
				return ERR_UNEXP_DAT;	// nierozpoznany napis
			}
			break;


		case AFTER_LABEL:			// wyst¹pi³a etykieta
			switch (leks.type)
			{
			case CLeksem::L_SPACE:
				ASSERT(false);
				break;
			case CLeksem::L_LABEL:	// znak ':'
				state = INSTR;
				leks = next_leks();
				break;
			case CLeksem::L_EQUAL:	// znak '='
				state = EXPR;
				leks = next_leks();
				break;
			default:
				state = INSTR;
				break;
			}
			labelled = true;
			break;


		case INSTR:			// oczekiwana instrukcja, komentarz lub nic
			if (labelled &&					// przed instr. by³a etykieta
				!(leks.type == CLeksem::L_ASM_INSTR &&	// i za etykiet¹
				(leks.GetInstr() == I_MACRO ||		// nie wystêpuje dyrektywa .MACRO
				leks.GetInstr() == I_SET)))			// ani dyrektywa .SET?
			{
				if (origin > 0xFFFF)
					return ERR_UNDEF_ORIGIN;
				ret = pass == 1 ? def_ident(label, CIdent(CIdent::I_ADDRESS, origin)) :
								  chk_ident_def(label, CIdent(CIdent::I_ADDRESS, origin));
				if (ret)
					return ret;
			}

			switch (leks.type)
			{
			case CLeksem::L_SPACE:
				ASSERT(false);
				break;
			case CLeksem::L_ASM_INSTR:	// dyrektywa asemblera
				{
					InstrType it= leks.GetInstr();
					instr_start = ident_start;	// po³o¿enie instrukcji w wierszu
					instr_fin = ident_fin;

					leks = next_leks();
					ret_stat = asm_instr_syntax_and_generate(leks,it,labelled ? &label : NULL);
					if (ret_stat > OK)		// b³¹d? (ERR_xxx)
						return ret_stat;
					if (pass==2 && out && debug &&
						ret_stat!=STAT_MACRO && ret_stat!=STAT_EXITM && it!=I_SET &&
						ret_stat!=STAT_REPEAT && ret_stat!=STAT_ENDR)
					{
						ret = generate_debug(it,text->GetLineNo(),text->GetFileUID());
						if (ret)
							return ret;
					}
					if (pass==2 && ret_stat==STAT_MACRO)
						return ret_stat;		// w drugim przejœciu omijamy .MACRO
					state = COMMENT;
					break;
				}

			case CLeksem::L_PROC_INSTR:	// rozkaz procesora
				{
					instr_start = ident_start;	// po³o¿enie instrukcji w wierszu
					instr_fin = ident_fin;
					OpCode code= leks.GetCode();	// nr rozkazu
					CodeAdr mode;
					Expr expr, expr_bit, expr_zpg;
					leks = next_leks();
					ret = proc_instr_syntax(leks,mode,expr,expr_bit,expr_zpg);
					if (ret)			// b³¹d sk³adni
						return ret;
					int len;			// d³ugoœæ rozkazu z argumentem
					ret = chk_instr_code(code,mode,expr,len);
					if (ret)
						return ret;		// b³¹d trybu adresowania lub zakresu
					if (pass==2 && out && debug)
					{
						if (origin > 0xFFFF)
							return ERR_UNDEF_ORIGIN;
						CMacroDef* pMacro= dynamic_cast<CMacroDef*>(text);
						if (pMacro && pMacro->m_bFirstCodeLine)
						{	// debug info dla pierwszego wiersza makra zawieraj¹cego instr. 6502
							pMacro->m_bFirstCodeLine = false;
							generate_debug((UINT16)origin,pMacro->GetFirstLineNo(),pMacro->GetFirstLineFileUID());
						}
						else
							generate_debug((UINT16)origin,text->GetLineNo(),text->GetFileUID());
						generate_code(code,mode,expr,expr_bit,expr_zpg);
					}
					ret = inc_prog_counter(len);
					if (ret)
						return ret;
					state = COMMENT;		// dozwolony ju¿ tylko komentarz
					break;
				}

			case CLeksem::L_IDENT:	// etykieta - tu tylko jako nazwa makra
			case CLeksem::L_IDENT_N:	// etykieta numerowana - tu tylko jako nazwa makra
				{
					if (leks.type == CLeksem::L_IDENT)
					{
						label = *leks.GetString();	// zapamiêtanie identyfikatora
						leks = next_leks();
					}
					else
					{
						CLeksem ident(leks);
						Expr expr(0);
						leks = next_leks();
						ret = factor(leks,expr);
						if (ret)
							return ret;
						if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
							return ERR_UNDEF_EXPR;
						ident.Format(expr.value);		// znormalizowanie postaci etykiety
						label = *ident.GetString();	// zapamiêtanie identyfikatora
					}
					CIdent macro;
					if (!macro_name.lookup(label,macro))	// jeœli etykiety nie ma w tablicy
						return ERR_UNKNOWN_INSTR;
					ASSERT(macros.GetSize() > macro.val && macro.val >= 0);
					CMacroDef *pMacro= &macros[macro.val];
					ret = pMacro->ParseArguments(leks,*this);	// wczytanie argumentów makra
					if (ret)
						return ret;
					pMacro->Start(text->GetLineNo(),text->GetFileUID());
					source.Push(text);		// bie¿¹ce Ÿród³o wierszy na stos
					text = expanding_macro = pMacro;
					//	    text->Start();
					macro_local_area++;		// nowy obszar lokalny etykiet makra
					//	    MacroExpandStart(pMacro);	// przejœcie do trybu rozwijania makrodefinicji
					state = COMMENT;		// dozwolony ju¿ tylko komentarz
					break;
				}

			case CLeksem::L_CR:
			case CLeksem::L_FIN:
				state = FINISH;
				break;
			case CLeksem::L_COMMENT:	// komentarz
				state = COMMENT;
				break;
			default:
				return ERR_INSTR_OR_NULL_EXPECTED;
			}
			break;


		case EXPR:			// oczekiwane wyra¿enie - wart. etykiety
			switch (leks.type)
			{
			case CLeksem::L_SPACE:
				ASSERT(false);
				break;

			default:
				Expr expr;
				ret = expression(leks, expr);// zinterpretowanie wyra¿enia
				if (ret)
					return ret;

				// is it predefined label?
				int nConstant= find_const(label);
				if (nConstant >= 0)
				{
					// assignment to io_area is fine
					if (nConstant == 1)	// io_area label?
					{
						if (expr.inf == Expr::EX_WORD || expr.inf == Expr::EX_BYTE)
						{
							if (!check_line)
//							if (pass == 2)		// do it once (avoid first pass; it's called for line checking)
								CSym6502::io_addr = UINT16(expr.value);
						}
						else if (expr.inf == Expr::EX_LONG)
							return ERR_NUM_LONG;
						else if (expr.inf == Expr::EX_UNDEF)
							; // not yet defined; this is fine
						else
							return ERR_CONST_EXPECTED;
					}
					else
					{
						err_ident= label;
						return ERR_CONST_LABEL_REDEF;
					}
				}
				else if (pass==1)		// pierwsze przejœcie?
				{
					if (expr.inf != Expr::EX_UNDEF)
						ret = def_ident(label, CIdent(CIdent::I_VALUE, expr.value));
					else
						ret = def_ident(label, CIdent(CIdent::I_UNDEF, 0));
				}
				else			// drugie przejœcie
				{
					if (expr.inf != Expr::EX_UNDEF)
						ret = chk_ident_def(label, CIdent(CIdent::I_VALUE, expr.value));
					else
						return ERR_UNDEF_EXPR;
					if (listing.IsOpen())
						listing.AddValue((UINT16)expr.value);
				}
				if (ret)
					return ret;
				state = COMMENT;
				break;
			}
			break;


		case COMMENT:			// jeszcze tylko komentarz lub koniec wiersza
			switch (leks.type)
			{
			case CLeksem::L_SPACE:
				ASSERT(false);
				//	    break;
			case CLeksem::L_COMMENT:
				return ret_stat;
			case CLeksem::L_CR:
			case CLeksem::L_FIN:
				state = FINISH;
				break;
			default:
				return ERR_DAT;
			}
			break;


		case FINISH:
			switch (leks.type)
			{
			case CLeksem::L_CR:
				return ret_stat;
			case CLeksem::L_FIN:
				return ret_stat ? ret_stat : STAT_FIN;
			default:
				return ERR_DAT;
			}
			break;
		}

	}

}


//-----------------------------------------------------------------------------


bool CAsm6502::is_expression(const CLeksem &leks)	// pocz¹tek wyra¿enia?
{
	switch (leks.type)
	{
	case CLeksem::L_NUM:			// liczba (dec, hex, bin, lub znak)
	case CLeksem::L_IDENT:			// identyfikator
	case CLeksem::L_IDENT_N:		// identyfikator numerowany
	case CLeksem::L_OPER:			// operator
	case CLeksem::L_EXPR_BRACKET_L:	// lewy nawias dla wyra¿eñ '['
	case CLeksem::L_EXPR_BRACKET_R:	// prawy nawias dla wyra¿eñ ']'
		return true;				// to pocz¹tek wyra¿enia

	default:
		return false;	// to nie pocz¹tek wyra¿enia
	}
}


int CAsm6502::find_const(const CString& str)
{
	static const TCHAR cnst1[]= "ORG";		// predefiniowana sta³a
	static const TCHAR cnst2[]= "IO_AREA";	// predefiniowana sta³a

	if (str.CompareNoCase(cnst1) == 0)
		return 0;
	if (str.CompareNoCase(cnst2) == 0)
		return 1;

	return -1;
}


CAsm6502::Stat CAsm6502::predef_const(const CString& str, Expr& expr, bool& found)
{
	int nConstant= find_const(str);

	if (nConstant == 0)
	{
		if (origin > 0xFFFF)
			return ERR_UNDEF_ORIGIN;
		expr.value = origin;	// wartoœæ licznika rozkazów
		found = true;
		return OK;
	}
	else if (nConstant == 1)
	{
		expr.value = CSym6502::io_addr;		// io simulator area
		found = true;
		return OK;
	}

	found = false;
	return OK;
}


CAsm6502::Stat CAsm6502::predef_function(CLeksem &leks, Expr &expr, bool &fn)
{
	static const TCHAR def[]= ".DEF";	// predefiniowana funkcja .DEF
	static const TCHAR ref[]= ".REF";	// predefiniowana funkcja .REF
	static const TCHAR strl[]= ".STRLEN";	// predefiniowana funkcja .STRLEN
	static const TCHAR pdef[]= ".PASSDEF";// predefiniowana funkcja .PASSDEF

	const CString &str= *leks.GetString();

	int hit= 0;
	if (str.CompareNoCase(def) == 0)
		hit = 1;
	else if (str.CompareNoCase(ref) == 0)
		hit = 2;
	else if (str.CompareNoCase(pdef) == 0)
		hit = 3;
	else if (str.CompareNoCase(strl) == 0)
		hit = -1;

	if (hit > 0)
	{
		leks = next_leks(false);
		if (leks.type != CLeksem::L_BRACKET_L)
			return ERR_BRACKET_L_EXPECTED;	// wymagany nawias '(' (bez odstêpu)
		leks = next_leks();
		CString Label;
		if (leks.type == CLeksem::L_IDENT)
		{
			Label = *leks.GetString();
			leks = next_leks();
		}
		else if (leks.type == CLeksem::L_IDENT_N)
		{
			CLeksem ident(leks);
			Expr expr(0);
			leks = next_leks();
			Stat ret = factor(leks,expr);
			if (ret)
				return ret;
			if (expr.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			ident.Format(expr.value);		// znormalizowanie postaci etykiety
			Label = *ident.GetString();	// zapamiêtanie identyfikatora
		}
		else
			return ERR_LABEL_EXPECTED;	// wymagana etykieta - argument .DEF lub .REF

		if (leks.type != CLeksem::L_BRACKET_R)
			return ERR_BRACKET_R_EXPECTED;	// wymagany nawias ')'

		if (Label[0] == LOCAL_LABEL_CHAR)	// etykiety lokalne niedozwolone
			return ERR_LOCAL_LABEL_NOT_ALLOWED;
		CIdent ident;

		if (hit == 1)			// .DEF?
		{
			if (global_ident.lookup(Label,ident) && ident.info!=CIdent::I_UNDEF)
			{
				ASSERT(ident.info != CIdent::I_INIT);
				expr.value = 1;			// 1 - etykieta zdefiniowana
			}
			else
				expr.value = 0;			// 0 - etykieta niezdefiniowana
		}
		else if (hit == 2)			// .REF?
		{
			expr.value = global_ident.lookup(Label,ident) ? 1 : 0; // 1 - jeœli etykieta jest w tablicy
		}
		else				// .PASSDEF?
		{
			if (global_ident.lookup(Label,ident) && ident.info!=CIdent::I_UNDEF)
			{
				ASSERT(ident.info != CIdent::I_INIT);
				if (pass==1)
					expr.value = 1;		// 1 - etykieta zdefiniowana
				else		// drugie przejœcie asemblacji
					expr.value = ident.checked ? 1 : 0;	// 1 - def. etykiety znaleziona w 2. przejœciu
			}
			else		// etykieta jeszcze nie zdefiniowana
				expr.value = 0;			// 0 - etykieta niezdefiniowana
		}

		leks = next_leks();
		fn = true;
		return OK;
	}
	else if (hit == -1)		// funkcja .STRLEN?
	{
		leks = next_leks(false);
		if (leks.type != CLeksem::L_BRACKET_L)
			return ERR_BRACKET_L_EXPECTED;	// wymagany nawias '(' (bez odstêpu)
		leks = next_leks();
		Expr strexpr;
		Stat ret= expression(leks,strexpr,true);
		if (ret)
			return ret;
		if (strexpr.inf != Expr::EX_STRING)
			return ERR_STR_EXPECTED;		// wymagany ³añcuch znaków jako argument
		if (leks.type != CLeksem::L_BRACKET_R)
			return ERR_BRACKET_R_EXPECTED;	// wymagany nawias ')'
		expr.value = strlen(strexpr.string);
		leks = next_leks();
		fn = true;
		return OK;
	}

	fn = false;
	return OK;
}


// wartoœæ sta³a - etykieta, parametr, liczba, predef. sta³a lub funkcja
CAsm6502::Stat CAsm6502::constant_value(CLeksem &leks, Expr &expr, bool nospace)
{
	switch (leks.type)
	{
	case CLeksem::L_NUM:		// liczba (dec, hex, bin, lub znak)
		expr.value = leks.GetValue();	// wartoœæ liczby lub znaku
		break;

	case CLeksem::L_STR:		// ci¹g znaków w cudzys³owach
		expr.string = *leks.GetString();
		expr.inf = Expr::EX_STRING;
		break;

	case CLeksem::L_IDENT:		// identyfikator
		{
			bool found= false;
			Stat ret= predef_const(*leks.GetString(), expr, found);
			if (ret)
				return ret;
			if (found)
			{
				leks = next_leks();
				return OK;
			}
			ret =  predef_function(leks,expr,found);
			if (ret)
				return ret;
			if (found)
				return OK;
			if (expanding_macro)
			{		// przeszukanie tablicy parametrów makra, jeœli jest rozwijane makro
				Stat ret= expanding_macro->ParamLookup(leks,*leks.GetString(),expr,found,*this);
				if (ret)
					return ret;
				if (found)
					return OK;
			}
			CIdent id(CIdent::I_UNDEF);	// niezdefiniowany identyfikator
			if (!add_ident(*leks.GetString(),id) && id.info!=CIdent::I_UNDEF)	// ju¿ zdefiniowany?
				expr.value = id.val;		// odczytana wartoœæ etykiety
			else
			{
				expr.inf = Expr::EX_UNDEF;	// jeszcze bez wartoœci
				if (pass==2)
					return err_ident=*leks.GetString(), ERR_UNDEF_LABEL;	// niezdefiniowana etykieta w drugim przebiegu
			}
			if (check_line)			// tryb sprawdzania jednego wiersza?
			{
				leks = next_leks(false);
				if (leks.type == CLeksem::L_STR_ARG)	// ominiêcie znaku '$' na koñcu etykiety
					leks = next_leks();
				if (leks.type == CLeksem::L_SPACE)
					leks = next_leks();
				return OK;
			}
			break;
		}

	case CLeksem::L_IDENT_N:		// identyfikator numerowany
		{
			CLeksem ident(leks);
			Expr expr2(0);
			leks = next_leks();
			Stat ret = factor(leks,expr2);
			if (ret)
				return ret;
			if (expr2.inf==Expr::EX_UNDEF)	// nieokreœlona wartoœæ
				return ERR_UNDEF_EXPR;
			ident.Format(expr2.value);	// znormalizowanie postaci etykiety

			CIdent id(CIdent::I_UNDEF);	// niezdefiniowany identyfikator
			if (!add_ident(*ident.GetString(),id) && id.info!=CIdent::I_UNDEF)	// ju¿ zdefiniowany?
				expr.value = id.val;		// odczytana wartoœæ etykiety
			else
			{
				expr.inf = Expr::EX_UNDEF;	// jeszcze bez wartoœci
				if (pass==2)
					return err_ident=*ident.GetString(), ERR_UNDEF_LABEL;	// niezdefiniowana etykieta w drugim przebiegu
			}
			return OK;
		}

	case CLeksem::L_OPER:		// operator
		if (expanding_macro && leks.GetOper() == O_MOD)	// '%' (odwo³anie do parametru makra)?
		{
			leks = next_leks(false);
			if (leks.type == CLeksem::L_SPACE)	// odstêp niedozwolony
				return ERR_PARAM_NUMBER_EXPECTED;
			Stat ret= factor(leks,expr,false);	// oczekiwany numer parametru makra
			if (ret)
				return ret;
			if (expr.inf == Expr::EX_UNDEF)
				return ERR_UNDEF_PARAM_NUMBER;	// numer parametru musi byæ zdefiniowany
			ret = expanding_macro->ParamLookup(leks,expr.value-1,expr,*this);
			if (ret)
				return ret;
			return OK;
		}
		else if (check_line && leks.GetOper() == O_MOD)	// tryb sprawdzania wiersza?
		{
			leks = next_leks(false);
			if (leks.type == CLeksem::L_SPACE)	// odstêp niedozwolony
				return ERR_PARAM_NUMBER_EXPECTED;
			Stat ret= factor(leks,expr,false);	// oczekiwany numer parametru makra
			if (ret)
				return ret;
			ret = expanding_macro->AnyParamLookup(leks,*this);
			if (ret)
				return ret;
			return OK;
		}
		else if (leks.GetOper() == O_MUL)	// '*' ?
		{
			if (origin > 0xFFFF)
				return ERR_UNDEF_ORIGIN;
			expr.value = origin;		// wartoœæ licznika rozkazów
			break;
		}
		// no break here
	default:
		return ERR_CONST_EXPECTED;
	}

	leks = next_leks(nospace);
	return OK;
}


CAsm6502::Stat CAsm6502::factor(CLeksem &leks, Expr &expr, bool nospace)
	// [~|!|-|>|<] sta³a | '['wyra¿enie']'
{
	OperType oper;
//  bool operation= false;

	if (leks.type==CLeksem::L_OPER)
	{
		oper = leks.GetOper();
		switch (oper)
		{
		case O_B_NOT:	// negacja bitowa '~'
		case O_NOT:	// negacja logiczna '!'
		case O_MINUS:	// minus unarny '-'
		case O_GT:	// górny bajt s³owa '>'
		case O_LT:	// dolny bajt s³owa '<'
			{
				//        operation = true;
				leks = next_leks();	// kolejny niepusty leksem
				Stat ret= factor(leks,expr,nospace);
				if (ret)
					return ret;
				if (expr.inf==Expr::EX_STRING)
					return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
				//        leks = next_leks(nospace);
				if (expr.inf!=Expr::EX_UNDEF)
					switch (oper)
				{
		case O_B_NOT:	// negacja bitowa '~'
			expr.value = ~expr.value;
			break;
		case O_NOT:		// negacja logiczna '!'
			expr.value = !expr.value;
			break;
		case O_MINUS:	// minus unarny '-'
			expr.value = -expr.value;
			break;
		case O_GT:		// górny bajt s³owa '>'
			expr.value = (expr.value >> 8) & 0xFF;
			break;
		case O_LT:		// dolny bajt s³owa '<'
			expr.value = expr.value & 0xFF;
			break;
				}
				return OK;
			}
		default:
			break;
		}
	}

	if (leks.type==CLeksem::L_EXPR_BRACKET_L)
	{
		leks = next_leks();		// kolejny niepusty leksem
		Stat ret= expression(leks,expr,true);
		if (ret)
			return ret;
		if (leks.type!=CLeksem::L_EXPR_BRACKET_R)
			return ERR_EXPR_BRACKET_R_EXPECTED;
		leks = next_leks(nospace);	// kolejny leksem
	}
	else
	{
		Stat ret= constant_value(leks,expr,nospace);
		if (ret)
			return ret;
	}
	return OK;
}


CAsm6502::Stat CAsm6502::mul_expr(CLeksem &leks, Expr &expr)
	// czynnik [*|/|% czynnik]
{
	Stat ret= factor(leks,expr);		// obliczenie czynnika
	if (ret)
		return ret;
//  leks = next_leks();

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;
		OperType oper= leks.GetOper();
		if (oper!=O_MUL && oper!=O_DIV && oper!=O_MOD)
			return OK;
		if (expr.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		leks = next_leks();		// ominiêcie operatora '*', '/' lub '%'

		Expr expr2(0);
		ret = factor(leks,expr2);		// kolejny czynnik
		if (ret)
			return ret;
		if (expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
		//    leks = next_leks();

		if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
			switch (oper)
		{
		case O_MUL:
			expr.value *= expr2.value;
			break;
		case O_DIV:
			if (expr2.value == 0)
				return ERR_DIV_BY_ZERO;
			expr.value /= expr2.value;
			break;
		case O_MOD:
			if (expr2.value == 0)
				return ERR_DIV_BY_ZERO;
			expr.value %= expr2.value;
			break;
		}
		else
			expr.inf = Expr::EX_UNDEF;
	}
}


CAsm6502::Stat CAsm6502::shift_expr(CLeksem &leks, Expr &expr)
	// czynnik1 [<<|>> czynnik1]
{
	Stat ret= mul_expr(leks,expr);	// obliczenie sk³adnika
	if (ret)
		return ret;		// b³¹d

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;

		bool left;
		switch (leks.GetOper())
		{
		case O_SHL:			// przesuniêcie w lewo?
			left = true;
			break;
		case O_SHR:			// przesuniêcie w prawo?
			left = false;
			break;
		default:
			return OK;
		}
		if (expr.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
		leks = next_leks();		// ominiêcie operatora '>>' lub '<<'
		Expr expr2(0);
		ret= mul_expr(leks,expr2);		// obliczenie kolejnego sk³adnika
		if (ret)
			return ret;		// b³¹d
		if (expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
		{
			if (left)
				expr.value <<= expr2.value;
			else
				expr.value >>= expr2.value;
		}
		else
			expr.inf = Expr::EX_UNDEF;
	}
}


CAsm6502::Stat CAsm6502::add_expr(CLeksem &leks, Expr &expr)
	// sk³adnik [+|- sk³adnik]
{
	Stat ret= shift_expr(leks,expr);	// obliczenie sk³adnika
	if (ret)
		return ret;		// b³¹d

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;

		bool add;
		switch (leks.GetOper())
		{
		case O_MINUS:			// odejmowanie?
			add = false;
			if (expr.inf==Expr::EX_STRING)
				return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
			break;
		case O_PLUS:			// dodawanie?
			add = true;
			break;
		default:
			return OK;
		}
		leks = next_leks();		// ominiêcie operatora '+' lub '-'
		Expr expr2(0);
		ret= shift_expr(leks,expr2);	// obliczenie kolejnego sk³adnika
		if (ret)
			return ret;		// b³¹d
		if (!add && expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony
		if ((expr.inf==Expr::EX_STRING) ^ (expr2.inf==Expr::EX_STRING))	// albo, albo
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
		{
			if (add)
			{
				if (expr.inf==Expr::EX_STRING && expr2.inf==Expr::EX_STRING)
					expr.string += expr2.string;
				else
				{
					ASSERT(expr.inf!=Expr::EX_STRING && expr2.inf!=Expr::EX_STRING);
					expr.value += expr2.value;
				}
			}
			else
				expr.value -= expr2.value;
		}
		else
			expr.inf = Expr::EX_UNDEF;
	}
}

// wyr_proste [ & | '|' | ^ wyr_proste ]
CAsm6502::Stat CAsm6502::bit_expr(CLeksem &leks, Expr &expr)
{
	Stat ret= add_expr(leks,expr);	// obliczenie wyra¿enia prostego
	if (ret)
		return ret;

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;
		OperType oper= leks.GetOper();
		if (oper!=O_B_AND && oper!=O_B_OR && oper!=O_B_XOR)
			return OK;
		if (expr.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		leks = next_leks();		// ominiêcie operatora '&', '|' lub '^'

		Expr expr2(0);
		ret = add_expr(leks,expr2);		// kolejne wyra¿enie proste
		if (ret)
			return ret;
		if (expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
			switch (oper)
		{
		case O_B_AND:
			expr.value &= expr2.value;
			break;
		case O_B_OR:
			expr.value |= expr2.value;
			break;
		case O_B_XOR:
			expr.value ^= expr2.value;
			break;
		}
		else
			expr.inf = Expr::EX_UNDEF;
	}
}


CAsm6502::Stat CAsm6502::cmp_expr(CLeksem &leks, Expr &expr)	// wyr [>|<|>=|<=|==|!= wyr]
{
	Stat ret= bit_expr(leks,expr);	// obliczenie sk³adnika
	if (ret)
		return ret;				// b³¹d

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;
		OperType oper= leks.GetOper();
		if (oper!=O_GT && oper!=O_GTE && oper!=O_LT && oper!=O_LTE && oper!=O_EQ && oper!=O_NE)
			return OK;

		leks = next_leks();			// ominiêcie operatora logicznego
		Expr expr2(0);
		ret= bit_expr(leks,expr2);		// obliczenie kolejnego sk³adnika
		if (ret)
			return ret;			// b³¹d
		if ((expr.inf==Expr::EX_STRING) ^ (expr2.inf==Expr::EX_STRING))	// albo, albo
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
			if (expr.inf==Expr::EX_STRING && expr2.inf==Expr::EX_STRING)
			{
				switch (oper)
				{
				case O_GT:
					expr.value = expr.string > expr2.string;
					break;
				case O_LT:
					expr.value = expr.string < expr2.string;
					break;
				case O_GTE:
					expr.value = expr.string >= expr2.string;
					break;
				case O_LTE:
					expr.value = expr.string <= expr2.string;
					break;
				case O_EQ:
					expr.value = expr.string != expr2.string;
					break;
				case O_NE:
					expr.value = expr.string == expr2.string;
					break;
				}
				expr.inf = Expr::EX_BYTE;
			}
			else
			{
				ASSERT(expr.inf!=Expr::EX_STRING && expr2.inf!=Expr::EX_STRING);
				switch (oper)
				{
				case O_GT:
					expr.value = expr.value > expr2.value;
					break;
				case O_LT:
					expr.value = expr.value < expr2.value;
					break;
				case O_GTE:
					expr.value = expr.value >= expr2.value;
					break;
				case O_LTE:
					expr.value = expr.value <= expr2.value;
					break;
				case O_EQ:
					expr.value = expr.value != expr2.value;
					break;
				case O_NE:
					expr.value = expr.value == expr2.value;
					break;
				}
			}
			else
				expr.inf = Expr::EX_UNDEF;
	}
}


CAsm6502::Stat CAsm6502::bool_expr_and(CLeksem &leks, Expr &expr)	// wyr [&& wyr]
{
	bool skip= false;

	Stat ret= cmp_expr(leks,expr);	// obliczenie sk³adnika
	if (ret)
		return ret;				// b³¹d

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;
		if (leks.GetOper() != O_AND)
			return OK;
		if (expr.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (expr.inf!=Expr::EX_UNDEF && expr.value==0)
			skip = true;			// ju¿ false - nie potrzeba dalej liczyæ

		leks = next_leks();			// ominiêcie operatora '&&'
		Expr expr2(0);
		ret= cmp_expr(leks,expr2);		// obliczenie kolejnego sk³adnika
		if (ret)
			return ret;			// b³¹d
		if (expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (!skip)
			if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
				expr.value = expr2.value ? 1 : 0;
			else
				expr.inf = Expr::EX_UNDEF;
	}
}


CAsm6502::Stat CAsm6502::bool_expr_or(CLeksem &leks, Expr &expr)	// wyr [|| wyr]
{
	bool skip= false;

	Stat ret= bool_expr_and(leks,expr);	// obliczenie sk³adnika
	if (ret)
		return ret;				// b³¹d

	for (;;)
	{
		if (leks.type!=CLeksem::L_OPER)	// nie operator?
			return OK;
		if (leks.GetOper() != O_OR)
			return OK;

		if (expr.inf!=Expr::EX_UNDEF && expr.value!=0)
			skip = true;			// ju¿ true - nie potrzeba dalej liczyæ

		leks = next_leks();			// ominiêcie operatora '||'
		Expr expr2(0);
		ret= bool_expr_and(leks,expr2);	// obliczenie kolejnego sk³adnika
		if (ret)
			return ret;			// b³¹d
		if (expr2.inf==Expr::EX_STRING)
			return ERR_STR_NOT_ALLOWED;	// tekst niedozwolony

		if (!skip)
			if (expr.inf!=Expr::EX_UNDEF && expr2.inf!=Expr::EX_UNDEF)	// obliczyæ wartoœæ?
				expr.value = expr2.value ? 1 : 0;
			else
				expr.inf = Expr::EX_UNDEF;
	}
}

// interpretacja wyra¿enia
CAsm6502::Stat CAsm6502::expression(CLeksem &leks, Expr &expr, bool str)
{
	expr.inf = Expr::EX_LONG;
	Stat ret= bool_expr_or(leks,expr);
	if (ret)
		return ret;
	if (expr.inf == Expr::EX_STRING)
	{
		if (!str)			// wyra¿enie znakowe dozwolone?
			return ERR_STR_NOT_ALLOWED;
	}
	else if (expr.inf != Expr::EX_UNDEF)
	{
		SINT32 value= SINT32(expr.value);
		if (value>-0x100 && value<0x100)
			expr.inf = Expr::EX_BYTE;
		else if (value>-0x10000 && value<0x10000)
			expr.inf = Expr::EX_WORD;
		else
			expr.inf = Expr::EX_LONG;
	}
	return OK;
}

//-----------------------------------------------------------------------------

CAsm6502::Stat CAsm6502::look_for_endm()	// szukanie .ENDM lub .MACRO
{
	CLeksem leks= next_leks(false);	// kolejny leksem, byæ mo¿e pusty (L_SPACE)
	bool labelled= false;

	switch (leks.type)
	{
	case CLeksem::L_IDENT:	// etykieta
	case CLeksem::L_IDENT_N:	// etykieta numerowana
		leks = next_leks();
		if (leks.type == CLeksem::L_IDENT_N)
		{
			Expr expr(0);
			Stat ret = factor(leks,expr);
			if (ret)
				return ret;
		}
		//      leks = next_leks();
		labelled = true;
		switch (leks.type)
		{
		case CLeksem::L_LABEL:	// znak ':'
			leks = next_leks();
			if (leks.type!=CLeksem::L_ASM_INSTR)
				return OK;
			break;
		case CLeksem::L_ASM_INSTR:
			break;
		default:
			return OK;
		}
		//      leks = next_leks();
		break;
		case CLeksem::L_SPACE:	// odstêp
			leks = next_leks();
			if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
				return OK;
			break;
		case CLeksem::L_COMMENT:	// komentarz
		case CLeksem::L_CR:		// koniec wiersza
			return OK;
		case CLeksem::L_FIN:	// koniec tekstu
			return STAT_FIN;
		default:
			return ERR_UNEXP_DAT;
	}

	ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

	switch (leks.GetInstr())
	{
	case I_MACRO:
		return ERR_NESTED_MACRO;	// definicja makra w makrodefinicji jest zabroniona
	case I_ENDM:
		return labelled ? ERR_LABEL_NOT_ALLOWED : STAT_ENDM;	// koniec makra
	default:
		return OK;
	}
}


CAsm6502::Stat CAsm6502::record_macro()	// wczytanie kolejnego wiersza makrodefinicji
{
	CMacroDef *pMacro= get_last_macro_entry();

	Stat ret= look_for_endm();
	if (ret > 0)
		return ret;

	if (ret != STAT_ENDM)			// wiersza z .ENDM ju¿ nie potrzeba zapamiêtywaæ
		pMacro->AddLine(current_line,text->GetLineNo());

	return ret;
}

//-----------------------------------------------------------------------------

CString CAsm6502::format_local_label(const CString &ident, int area)
{
	CString local(' ',ident.GetLength()+8);
	local.Format("%08X%s",area,(LPCTSTR)ident);
	return local;
}

  // spr. czy dana etykieta jest ju¿ zdefiniowana
bool CAsm6502::add_ident(const CString &id, CIdent &inf)
{
	CString tmp;
	const CString &ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	if (ident[0]==LOCAL_LABEL_CHAR)	// etykieta lokalna?
	{
		if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
			return macro_ident.insert(format_local_label(ident,macro_local_area),inf);
		else
			return local_ident.insert(format_local_label(ident,local_area),inf);
	}
	else					// etykieta globalna
		return global_ident.insert(ident,inf);
}


  // wprowadzenie definicji etykiety (1. przebieg asemblacji)
CAsm6502::Stat CAsm6502::def_ident(const CString &id, CIdent &inf)
{
	ASSERT(pass==1);

	if (find_const(id) >= 0)
		return err_ident=id, ERR_CONST_LABEL_REDEF;

	CString tmp;
	const CString &ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	if (ident[0]==LOCAL_LABEL_CHAR)	// etykieta lokalna?
	{
		if (expanding_macro)
		{
			if (!macro_ident.replace(format_local_label(ident,macro_local_area),inf))
				return err_ident=ident, ERR_LABEL_REDEF;	// ju¿ zdefiniowana
		}
		else if (!local_ident.replace(format_local_label(ident,local_area),inf))
			return err_ident=ident, ERR_LABEL_REDEF;	// ju¿ zdefiniowana
	}
	else					// etykieta globalna
	{
		if (!global_ident.replace(ident,inf))
			return err_ident=ident, ERR_LABEL_REDEF;	// ju¿ zdefiniowana
		//    if (inf.info == CIdent::I_ADDRESS)// etykieta z adresem odgradza etykiety lokalne
		local_area++;			// nowy obszar lokalny
	}
	return OK;
}


  // sprawdzenie czy etykieta jest zdefiniowana (2. przebieg asemblacji)
CAsm6502::Stat CAsm6502::chk_ident(const CString &id, CIdent &inf)
{
	ASSERT(pass==2);
	CString tmp;
	const CString& ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	CIdent info;
	bool exist= false;

	if (ident[0] == LOCAL_LABEL_CHAR)	// etykieta lokalna?
	{
		if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
			exist = macro_ident.lookup(format_local_label(ident,macro_local_area),info);
		else
			exist = local_ident.lookup(format_local_label(ident,local_area),info);
	}
	else					// etykieta globalna
	{
		exist = global_ident.lookup(ident,info);
		local_area++;	// nowy obszar lokalny
	}

	if (exist)	// sprawdzana etykieta znaleziona w tablicy
	{
		if (info.info == CIdent::I_UNDEF)
			return err_ident=ident, ERR_UNDEF_LABEL;	// etykieta bez definicji
		ASSERT(info.variable && inf.variable || !info.variable && !inf.variable);
		if (info.val != inf.val && !info.variable)
			return err_ident=ident, ERR_PHASE;// niezgodne wartoœci miêdzy przebiegami - b³¹d fazy
	}
	else		// sprawdzanej etykiety nie ma w tablicy
		return err_ident=ident, ERR_UNDEF_LABEL;

	inf = info;
	return OK;
}


// sprawdzenie definicji etykiety (2. przebieg asemblacji)
CAsm6502::Stat CAsm6502::chk_ident_def(const CString &id, CIdent &inf)
{
	CString tmp;
	const CString &ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	SINT32 val= inf.val;		// zapamiêtanie wartoœci
	Stat ret= chk_ident(ident, inf);
	if (ret != OK && ret != ERR_UNDEF_LABEL)
		return ret;
	if (inf.variable)		// etykieta zmiennej?
	{
		inf.val = val;		// nowa wartoœæ zmiennej
		bool ret;
		if (ident[0] == LOCAL_LABEL_CHAR)	// etykieta lokalna?
		{
			if (expanding_macro)		// etykieta lokalna w makrorozszerzeniu?
				ret = macro_ident.replace(format_local_label(ident, macro_local_area), inf);
			else
				ret = local_ident.replace(format_local_label(ident, local_area), inf);
		}
		else
			ret = global_ident.replace(ident, inf);
		ASSERT(ret);
	}
	else if (ident[0] != LOCAL_LABEL_CHAR)	// etykieta globalna sta³ej?
	{
		ASSERT(!inf.checked);
		inf.checked = true;		// potwierdzenie definicji w drugim przejœciu asemblacji
		bool ret= global_ident.replace(ident, inf);
//		ASSERT(!ret && inf.info == I_ADDRESS || ret);		// etykieta musi byæ redefiniowana
	}
	return OK;
}


CAsm6502::Stat CAsm6502::def_macro_name(const CString &id, CIdent &inf)
{
	ASSERT(pass==1);
	CString tmp;
	const CString &ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	if (!macro_name.replace(ident,inf))
		return err_ident=ident, ERR_LABEL_REDEF;	// nazwa ju¿ zdefiniowana

	return OK;
}


CAsm6502::Stat CAsm6502::chk_macro_name(const CString &id)
{
	ASSERT(pass==2);
	CString tmp;
	const CString &ident= case_insensitive ? tmp : id;
	if (case_insensitive)
		tmp=id, tmp.MakeLower();

	CIdent info;

	if (macro_name.lookup(ident,info))	// sprawdzana etykieta znaleziona w tablicy
	{
		ASSERT(info.info==CIdent::I_MACRONAME);
		return OK;
		//    if (info.val != inf.val)
		//      return err_ident=ident, ERR_PHASE;// niezgodne wartoœci miêdzy przebiegami - b³¹d fazy
	}
	else		// sprawdzanej etykiety nie ma w tablicy
		return err_ident=ident, ERR_UNDEF_LABEL;

	return OK;
}

//-----------------------------------------------------------------------------

const TCHAR* CRepeatDef::GetCurrLine(CString &str)	// odczyt aktualnego wiersza do powtórki
{
	ASSERT(m_nLineNo >= 0);
	if (m_nLineNo == GetSize())	// koniec wierszy?
	{
		if (m_nRepeat == 0)		// koniec powtórzeñ?
			return NULL;
		if (GetSize() == 0)		// puste powtórzenie (bez wierszy)?
			return NULL;
		m_nRepeat--;		// odliczanie powtórzeñ
		//    m_nRepeatLocalArea++;	// nowy obszar etykiet
		ASSERT(m_nRepeat >= 0);
		m_nLineNo = 0;
	}
	ASSERT(m_nLineNo < GetSize());
	str = GetLine(m_nLineNo++);
	return (const TCHAR *)str;
}


CAsm6502::Stat CAsm6502::record_rept(CRepeatDef *pRept)	// wczytanie kolejnego wiersza do powtórki
{
	Stat ret= look_for_repeat();
	if (ret > 0)
		return ret;

	if (ret == STAT_REPEAT)	// zagnie¿d¿one .REPEAT
	{
		ret = OK;
		reptNested++;
	}
	else if (ret == STAT_ENDR)
		if (reptNested == 0)	// koniec .REPEAT?
			return ret;
		else
		{
			reptNested--;		// koniec zagnie¿d¿onego .REPEAT
			ret = OK;
		}

	pRept->AddLine(current_line,text->GetLineNo());

	return ret;
}


CAsm6502::Stat CAsm6502::look_for_repeat()	// szukanie .ENDR lub .REPEAT
{
	CLeksem leks= next_leks(false);	// kolejny leksem, byæ mo¿e pusty (L_SPACE)

	switch (leks.type)
	{
	case CLeksem::L_IDENT:		// etykieta
	case CLeksem::L_IDENT_N:	// etykieta numerowana
		leks = next_leks();
		if (leks.type == CLeksem::L_IDENT_N)
		{
			Expr expr(0);
			Stat ret = factor(leks,expr);
			if (ret)
				return ret;
		}
		//      leks = next_leks();
		switch (leks.type)
		{
		case CLeksem::L_LABEL:	// znak ':'
			leks = next_leks();
			if (leks.type!=CLeksem::L_ASM_INSTR)
				return OK;
			break;
		case CLeksem::L_ASM_INSTR:
			break;
		default:
			return OK;
		}
		//      leks = next_leks();
		break;
		case CLeksem::L_SPACE:	// odstêp
			leks = next_leks();
			if (leks.type!=CLeksem::L_ASM_INSTR)	// nie dyrektywa asemblera?
				return OK;
			break;
		case CLeksem::L_COMMENT:	// komentarz
		case CLeksem::L_CR:		// koniec wiersza
			return OK;
		case CLeksem::L_FIN:	// koniec tekstu
			return STAT_FIN;
		default:
			return ERR_UNEXP_DAT;
	}

	ASSERT(leks.type==CLeksem::L_ASM_INSTR);	// dyrektywa asemblera

	switch (leks.GetInstr())
	{
	case I_REPEAT:
		return STAT_REPEAT;		// zagnie¿d¿one .REPEAT
	case I_ENDR:
		return STAT_ENDR;		// koniec .REPEAT
	default:
		return OK;
	}
}

//-----------------------------------------------------------------------------
/*
const TCHAR *CAsm6502::get_next_line()		// wczytanie kolejnego wiersza do asemblacji
{
  LPTSTR pstr= current_line.GetBuffer(1024+4);
  TCHAR *ret= input.read_line(pstr,1024+4);
  current_line.ReleaseBuffer(-1);
  return ret;
}
*/

void CAsm6502::asm_start()
{
	user_error_text.Empty();
	if (markArea)
		markArea->Clear();
	if (debug)
	{
		debug->ResetFileMap();
		entire_text.SetFileUID(debug);	// wygenerowanie FUID dla tekstu Ÿród³owego
	}
}

void CAsm6502::asm_fin()
{
	if (debug)
		generate_debug();
	if (markArea && markArea->IsStartSet())
		markArea->SetEnd(UINT16(origin-1));
}

void CAsm6502::asm_start_pass()
{
	source.Push(&entire_text);
	text = source.Peek(); 
	local_area = 0;
	macro_local_area = 0;
	//  input.seek_to_begin();
	text->Start();
	origin = ~0u;
	originWrapped = false;
	if (pass==2)
		debug->Empty();
}

void CAsm6502::asm_fin_pass()
{
	text->Fin();
}

CAsm6502::Stat CAsm6502::assemble()	// translacja programu
{
	Stat ret;
	bool skip= false;
	bool skip_macro= false;
//  CRepeatDef *pRept= NULL;

	try
	{
		asm_start();

		for (pass=1; pass<=2; pass++)	// dwa przejœcia asemblacji
		{
			asm_start_pass();
//      func = read.Peek();

			for (bool fin=false; !fin; )
			{
				while (!(ptr=text->GetCurrLine(current_line)))	// funkcja nie zwraca ju¿ wierszy?
				{
					if (source.Peek())		// jest jeszcze jakaœ funkcja odczytu wierszy?
					{
						text->Fin();
						expanding_macro = (CMacroDef *)source.FindMacro();
						repeating = (CRepeatDef *)source.FindRepeat();
						text = source.Pop();
					}
					else
						break;		// nie ma, zwracamy ptr==NULL na oznaczenie koñca tekstu programu
				}
				if (current_line.GetLength() > 1024)	// spr. max d³ugoœæ wiersza
					return ERR_LINE_TO_LONG;

				if (is_aborted())
					return ERR_USER_ABORT;

				if (skip)			// asemblacja warunkowa (po .IF) ?
					ret = look_for_endif();	// omijanie instrukcji a¿ do .ENDIF lub .ELSE
				else if (in_macro)		// zapamiêtywanie makra (po .MACRO) ?
					ret = record_macro();		// zapamiêtanie wiersza makra
				else if (skip_macro)		// omijanie makra (w 2. przejœciu po .MACRO) ?
					ret = look_for_endm();	// omijanie wierszy a¿ do .ENDM
				else if (pRept)			// zapamiêtywanie wiersza powtórzeñ?
					ret = record_rept(pRept);	// zapamiêtanie wiersza do powtórzeñ
				else
				{
					ret = assemble_line();	// asemblacja wiersza
					if (pass==2 && listing.IsOpen())
					{
						listing.AddSourceLine(current_line);
						listing.NextLine();
					}
				}

				switch (ret)
				{
				case STAT_INCLUDE:
					{
//	    if (text->IsMacro() || text->IsRepeat())
//	    if (typeid(text) == typeid(CMacroDef) || typeid(text) == typeid(CRepeatDef))
						CSourceText* pSrc= dynamic_cast<CSourceText*>(text);
						if (pSrc == NULL)
							return ERR_INCLUDE_NOT_ALLOWED;	// .INCLUDE w makrze/powtórce niedozwolone
						pSrc->Include(include_fname,debug);
						break;
					}

				case STAT_IF_TRUE:
				case STAT_IF_FALSE:
				case STAT_IF_UNDETERMINED:
					ret = conditional_asm.instr_if_found(ret);
					if (ret > OK)
						return ret;		// b³¹d
					skip = ret==STAT_SKIP;	// omijanie instrukcji a¿ do .ELSE lub .ENDIF?
					break;
				case STAT_ELSE:
					ret = conditional_asm.instr_else_found();
					if (ret > OK)
						return ret;		// b³¹d
					skip = ret==STAT_SKIP;	// omijanie instrukcji a¿ do .ELSE lub .ENDIF?
					break;
				case STAT_ENDIF:
					ret = conditional_asm.instr_endif_found();
					if (ret > OK)
						return ret;		// b³¹d
					skip = ret==STAT_SKIP;	// omijanie instrukcji a¿ do .ELSE lub .ENDIF?
					break;

				case STAT_MACRO:		// makrodefinicja
					if (pass == 2)		// drugie przejœcie?
						skip_macro = true;	// omijanie makrodefinicji (ju¿ zarejestrowanej)
					break;
				case STAT_ENDM:		// koniec makrodefinicji
					if (pass == 1)		// pierwsze przejœcie?
					{
						ASSERT(in_macro);
						in_macro = NULL;		// rejestracja makra zakoñczona
					}
					else
					{
						ASSERT(skip_macro);
						skip_macro = false;	// omijanie definicji makra zakoñczone
					}
					break;
				case STAT_EXITM:
					ASSERT(expanding_macro);
					while (expanding_macro != text)	// szukanie opuszczanego makra
					{
						text->Fin();
						text = source.Pop();
					}
					text->Fin();		// zakoñczenie rozwijania makra
					expanding_macro = (CMacroDef *)source.FindMacro();
					repeating = (CRepeatDef *)source.FindRepeat();
					text = source.Pop();	// poprzednie Ÿród³o wierszy
					break;

				case STAT_REPEAT:		// zarejestrowanie wierszy po .REPEAT
					pRept = new CRepeatDef(reptInit);
					if (pass == 2)
						pRept->SetFileUID(text->GetFileUID());
					break;
				case STAT_ENDR:		// koniec rejestracji, teraz powtarzanie
					//	    RepeatStart(pRept);
					source.Push(text);		// bie¿¹ce Ÿród³o wierszy na stos
					text = pRept;
					pRept = NULL;
					text->Start();
					break;

				case STAT_FIN:		// koniec pliku
					ASSERT(dynamic_cast<CSourceText*>(text));
//	    ASSERT( typeid(text) != typeid(CMacroDef) && typeid(text) != typeid(CRepeatDef));
//	    ASSERT(!text->IsMacro() && !text->IsRepeat());
					if (!static_cast<CSourceText*>(text)->TextFin()) // koniec zagnie¿d¿onego odczytu (.include) ?
					{
						if (conditional_asm.in_cond())	// w œrodku dyrektywy .IF ?
							return ERR_ENDIF_REQUIRED;
						if (in_macro)
							return ERR_ENDM_REQUIRED;
						fin = true;		// koniec przejœcia asemblacji
						ret = OK;
					}
					break;

				case OK:
					break;

				default:
					if (listing.IsOpen())	// usuniêcie listingu ze wzglêdu na b³êdy
						listing.Remove();
					return ret;			// b³¹d asemblacji
				}
			}

			asm_fin_pass();
		}

		asm_fin();

	}
	catch (CMemoryException*)
	{
		return ERR_OUT_OF_MEM;
	}
	catch (CFileException*)
	{
		return ERR_FILE_READ;
	}

	return ret;
}

//-----------------------------------------------------------------------------
static const UINT8 NA= 0xBB;


CAsm6502::Stat CAsm6502::chk_instr_code(OpCode code, CodeAdr &mode, Expr expr, int &length)
{
	UINT8 byte;
	const UINT8 (&trans)[C_ILL][A_NO_OF_MODES]= TransformTable(bProc6502);

	if (mode >= A_NO_OF_MODES)		// niezdeterminowane tryby adresowania
	{
		switch (mode)
		{
		case A_ABS_OR_ZPG:
			byte = trans[code][mode=A_ABS];	// wybieramy ABS
			break;

		case A_ABSX_OR_ZPGX:
			byte = trans[code][mode=A_ABS_X];
			if (byte==NA)			// jeœli nie ma ABS_X,
				byte = trans[code][mode=A_ZPG_X];	// to spr. ZPG_X
			break;

		case A_ABSY_OR_ZPGY:
			byte = trans[code][mode=A_ABS_Y];
			if (byte==NA)			// jeœli nie ma ABS_Y,
				byte = trans[code][mode=A_ZPG_Y];	// to spr. ZPG_Y
			break;

		case A_ABSI_OR_ZPGI:
			byte = trans[code][mode=A_ABSI];
			if (byte==NA)			// jeœli nie ma ABSI,
				byte = trans[code][mode=A_ZPGI];	// to spr. ZPGI
			break;

		case A_IMP_OR_ACC:
			byte = trans[code][mode=A_IMP];
			if (byte==NA)			// jeœli nie ma IMP,
				byte = trans[code][mode=A_ACC];	// to spr. ACC
			if (code == C_BRK)
				mode = A_IMP2;
			break;

		case A_ABSIX_OR_ZPGIX:
			byte = trans[code][mode=A_ZPGI_X];
			if (byte==NA)			// jeœli nie ma ZPGI_X,
				byte = trans[code][mode=A_ABSI_X];	// to spr. ABSI_X
			break;

		default:
			ASSERT(false);
		}
	}
	else
		byte = trans[code][mode];

	if (byte==NA)				// niedozwolony tryb adresowania?
	{
		switch (mode)	// promocja trybu adresowania ZPG na odpowiadaj¹cy mu ABS
		{
		case A_ZPG:	// zero page
			mode = A_ABS;
			if (trans[code][mode]==NA)
				mode = A_REL;	// mo¿e to REL
			break;
		case A_ZPG_X:	// zero page indexed X
			mode = A_ABS_X;
			break;
		case A_ZPG_Y:	// zero page indexed Y
			mode = A_ABS_Y;
			break;
		case A_ZPGI:	// zero page indirect
			mode = A_ABSI;
			break;
		case A_ABS:		// jest ABS ale mo¿e chodziæ o REL
			mode = A_REL;
			break;
		case A_ZPGI_X:	// (zp,X) illigal, try (abs,X) then
			mode = A_ABSI_X;
			break;
		default:
			return ERR_MODE_NOT_ALLOWED;
		}
		byte = trans[code][mode];
		if (byte==NA)			// wci¹¿ niedozwolony tryb adresowania?
			return ERR_MODE_NOT_ALLOWED;
	}

	switch (mode)				// okreœlenie d³ugoœci rozkazu
	{
	case A_IMP:		// implied
	case A_ACC:		// accumulator
		length = 1+0;
		break;
	case A_IMP2:	// implied dla BRK
		length = generateBRKExtraByte ? 1+1 : 1+0;
		break;
	case A_ZPG:		// zero page
	case A_ZPG_X:	// zero page indexed X
	case A_ZPG_Y:	// zero page indexed Y
	case A_ZPGI:	// zero page indirect
	case A_ZPGI_X:	// zero page indirect, indexed X
	case A_ZPGI_Y:	// zero page indirect, indexed Y
	case A_IMM:		// immediate
	case A_ZPG2:	// zero page (dla SMB i RMB)
		length = 1+1;
		break;
	case A_REL:		// relative
		if (pass==2)
		{
			if (origin > 0xFFFF)
				return ERR_UNDEF_ORIGIN;
			ASSERT(expr.inf==Expr::EX_WORD || expr.inf==Expr::EX_BYTE);
			SINT32 dist= expr.value - ( SINT32(origin & 0xFFFF) + 2 );
			if (dist>127 || dist<-128)
				return ERR_REL_OUT_OF_RNG;
		}
		length = 1+1;
		break;
	case A_ABS:		// absolute
	case A_ABS_X:	// absolute indexed X
	case A_ABS_Y:	// absolute indexed Y
	case A_ABSI:	// absolute indirect
	case A_ABSI_X:	// absolute indirect, indexed X
		length = 1+2;
		break;
	case A_ZREL:	// zero page / relative (BBS i BBR z 65c02)
		if (pass==2)
		{
			if (origin > 0xFFFF)
				return ERR_UNDEF_ORIGIN;
			ASSERT(expr.inf==Expr::EX_WORD || expr.inf==Expr::EX_BYTE);
			SINT32 dist= expr.value - ( SINT32(origin & 0xFFFF) + 3 );
			if (dist>127 || dist<-128)
				return ERR_REL_OUT_OF_RNG;
		}
		length = 1+1+1;
		break;

	default:
		ASSERT(false);
	}

	return OK;
}


	// wygenerowanie kodu
void CAsm6502::generate_code(OpCode code, CodeAdr mode, Expr expr, Expr expr_bit, Expr expr_zpg)
{
	ASSERT( TransformTable(bProc6502)[code][mode] != NA || mode==A_IMP2 && code==C_BRK );
	ASSERT(origin <= 0xFFFF);

	if (mode==A_IMP2 && code==C_BRK)
		(*out)[origin] = 0;
	else
		(*out)[origin] = TransformTable(bProc6502)[code][mode];	// rozkaz

	switch (mode)			// argumenty rozkazu
	{
	case A_IMP:		// implied
	case A_ACC:		// accumulator
		if (listing.IsOpen())
			listing.AddCodeBytes((UINT16)origin,(*out)[origin]);
		return;

	case A_IMP2:	// implied dla BRK
		ASSERT(origin+1 <= mem_mask);
		if (generateBRKExtraByte)
			(*out)[origin+1] = BRKExtraByte;
		if (listing.IsOpen())
			listing.AddCodeBytes((UINT16)origin,(*out)[origin],generateBRKExtraByte ? (*out)[origin+1] : -1);
		return;

	case A_ZPG:		// zero page
	case A_ZPG_X:	// zero page indexed X
	case A_ZPG_Y:	// zero page indexed Y
	case A_ZPGI:	// zero page indirect
	case A_ZPGI_X:	// zero page indirect, indexed X
	case A_ZPGI_Y:	// zero page indirect, indexed Y
	case A_IMM:		// immediate
		ASSERT(origin+1 <= mem_mask);
		ASSERT(expr.inf == Expr::EX_BYTE);
		(*out)[origin+1] = UINT8(expr.value & 0xFF);
		if (listing.IsOpen())
			listing.AddCodeBytes((UINT16)origin,(*out)[origin],(*out)[origin+1]);
		break;

	case A_REL:		// relative
		{
			ASSERT(origin+1 <= mem_mask);
			ASSERT(expr.inf==Expr::EX_WORD || expr.inf==Expr::EX_BYTE);
			SINT32 dist= expr.value - ( SINT32(origin & 0xFFFF) + 2 );
			ASSERT(dist<128 && dist>-129);
			(*out)[origin+1] = UINT8(dist & 0xFF);
			if (listing.IsOpen())
				listing.AddCodeBytes((UINT16)origin,(*out)[origin],(*out)[origin+1]);
			break;
		}

	case A_ABS:		// absolute
	case A_ABS_X:	// absolute indexed X
	case A_ABS_Y:	// absolute indexed Y
	case A_ABSI:	// absolute indirect
	case A_ABSI_X:	// absolute indirect, indexed X
		ASSERT(origin+2 <= mem_mask);
		ASSERT(expr.inf==Expr::EX_WORD || expr.inf==Expr::EX_BYTE);
		(*out)[origin+1] = UINT8(expr.value & 0xFF);
		(*out)[origin+2] = UINT8((expr.value>>8) & 0xFF);
		if (listing.IsOpen())
			listing.AddCodeBytes((UINT16)origin,(*out)[origin],(*out)[origin+1],(*out)[origin+2]);
		break;

	case A_ZPG2:	// zero page dla SMB i RMB
		{
			(*out)[origin] = TransformTable(bProc6502)[code][mode];	// rozkaz SMB lub RMB
			ASSERT(expr_bit.inf==Expr::EX_BYTE && abs(expr_bit.value) < 8);
			(*out)[origin] += UINT8(expr_bit.value << 4);	// odp. nr bitu dla rozkazu SMBn lub RMBn
			ASSERT(origin+1 <= mem_mask);
			
			ASSERT(expr_zpg.inf == Expr::EX_BYTE);	// adres argumentu (na str. zerowej)
			(*out)[origin+1] = UINT8(expr_zpg.value & 0xFF);
			if (listing.IsOpen())
				listing.AddCodeBytes((UINT16)origin,(*out)[origin],(*out)[origin+1]);
			break;
		}

	case A_ZREL:	// zero page / relative
		{
			(*out)[origin] = TransformTable(bProc6502)[code][mode];	// rozkaz BBS lub BBR
			ASSERT(expr_bit.inf==Expr::EX_BYTE && abs(expr_bit.value) < 8);
			(*out)[origin] += UINT8(expr_bit.value << 4);	// odp. nr bitu dla rozkazu BBSn lub BBRn
			ASSERT(origin+2 <= mem_mask);

			ASSERT(expr_zpg.inf == Expr::EX_BYTE);	// adres argumentu (na str. zerowej)
			(*out)[origin+1] = UINT8(expr_zpg.value & 0xFF);

			ASSERT(expr.inf==Expr::EX_WORD || expr.inf==Expr::EX_BYTE);
			SINT32 dist= expr.value - ( SINT32(origin & 0xFFFF) + 3 );
			ASSERT(dist<128 && dist>-129);
			(*out)[origin+2] = UINT8(dist & 0xFF);
			if (listing.IsOpen())
				listing.AddCodeBytes((UINT16)origin,(*out)[origin],(*out)[origin+1],(*out)[origin+2]);
			break;
		}

	default:
		ASSERT(false);
	}
}



CAsm6502::Stat CAsm6502::inc_prog_counter(int dist)
{
	ASSERT(dist>=-0xFFFF && dist<=0xFFFF);

	if (origin > 0xFFFF)
		return ERR_UNDEF_ORIGIN;
	if (originWrapped)
		return ERR_PC_WRAPED;

	origin += dist;

	if (origin == mem_mask+1)
	{
		originWrapped = true;
		origin = 0;
	}
	else if (origin > mem_mask)
		return ERR_PC_WRAPED;

	return OK;
}


//-----------------------------------------------------------------------------

CAsm6502::Stat CAsm6502::CConditionalAsm::instr_if_found(Stat condition)
{
	ASSERT(condition==STAT_IF_TRUE || condition==STAT_IF_FALSE || condition==STAT_IF_UNDETERMINED);

	bool assemble= level<0 || get_assemble();

	if (assemble && condition==STAT_IF_UNDETERMINED)
		return ERR_UNDEF_EXPR;	// oczekiwane zdefiniowane wyra¿enie

	level++;			// zmiana stanu, zapisanie go na stosie

	if (assemble && condition==STAT_IF_TRUE)
	{
		set_state(BEFORE_ELSE,true);
		return STAT_ASM;		// kolejne wiersze nale¿y poddaæ asemblacji
	}
	else
	{
		set_state(BEFORE_ELSE,false);
		return STAT_SKIP;		// kolejne wiersze nale¿y omin¹æ, a¿ do .ELSE lub .ENDIF
	}
}


CAsm6502::Stat CAsm6502::CConditionalAsm::instr_else_found()
{
	if (level<0 || get_state()!=BEFORE_ELSE)
		return ERR_SPURIOUS_ELSE;
	// zmiana stanu automatu
	if (get_assemble())			// przed .ELSE wiersze asemblowane?
		set_state(AFTER_ELSE,false);	// wiêc po .ELSE ju¿ nie
	else
	{		// wiersze przed .ELSE nie by³y asemblowane
		if (level>0 && get_prev_assemble() || level==0)	// nadrzêdne if/endif asemblowane lub
			set_state(AFTER_ELSE,true);			// nie ma nadrzêdnego if/endif
		else
			set_state(AFTER_ELSE,false);
	}
	return get_assemble() ? STAT_ASM : STAT_SKIP;
}


CAsm6502::Stat CAsm6502::CConditionalAsm::instr_endif_found()
{
	if (level<0)
		return ERR_SPURIOUS_ENDIF;
	level--;		// zmiana stanu przez usuniêcie wierzcho³ka stosu
	if (level >= 0)
		return get_assemble() ? STAT_ASM : STAT_SKIP;
	return STAT_ASM;
}

//-----------------------------------------------------------------------------
/*
int CAsm6502::get_line_no()		// numer wiersza (dla debug info)
{
  if (repeating)
    return repeating->GetLineNo();
  return expanding_macro ? expanding_macro->GetLineNo() : input.get_line_no();
}

CAsm::FileUID CAsm6502::get_file_UID()	// id pliku (dla debug info)
{
  if (repeating)
    return repeating->GetFileUID();
  return expanding_macro ? expanding_macro->GetFileUID() : input.get_file_UID();
}
*/

void CAsm6502::generate_debug(UINT16 addr, int line_no, FileUID file_UID)
{
	ASSERT(debug != NULL);
	debug->AddLine( CDebugLine(line_no,file_UID,addr,
		typeid(text) == typeid(CMacroDef) ? DBG_CODE|DBG_MACRO : DBG_CODE) );
}


CAsm::Stat CAsm6502::generate_debug(InstrType it, int line_no, FileUID file_UID)
{
	ASSERT(debug != NULL);

	switch (it)
	{
	case I_DD:		// def double byte
	case I_DW:		// def word
	case I_DB:		// def byte
	case I_DS:		// def string
	case I_DCB:		// declare block
	case I_RS:		// reserve space
	case I_ASCIS:	// ascii + $80 ostatni bajt
		{
			if (origin > 0xFFFF)
				return ERR_UNDEF_ORIGIN;
			CDebugLine dl(line_no,file_UID,(UINT16)origin,DBG_DATA);
			debug->AddLine(dl);
			break;
		}

	case I_ORG:		// origin
	case I_START:
	case I_END:		// zakoñczenie
	case I_ERROR:	// zg³oszenie b³êdu
	case I_INCLUDE:	// w³¹czenie pliku
	case I_IF:
	case I_ELSE:
	case I_ENDIF:
	case I_OPT:
	case I_ROM_AREA:
	case I_IO_WND:
		break;

	default:
		ASSERT(false);
	}

	return OK;
}


void CAsm6502::generate_debug()
{
	CIdent info;
	CString ident;
	debug->SetIdentArrSize( global_ident.GetCount() + local_ident.GetCount() );
	POSITION pos= global_ident.GetStartPosition();
	int index= 0;
	while (pos)
	{
		global_ident.GetNextAssoc(pos,ident,info);
		debug->SetIdent(index++,ident,info);
	}
	pos = local_ident.GetStartPosition();
	while (pos)
	{
		local_ident.GetNextAssoc(pos,ident,info);
		debug->SetIdent(index++,ident,info);
	}
}

//=============================================================================

/*
void CAsm6502::test()
{
//  CString str(".gucio:	LDA ($a0),y		;x");

  Stat ret= assemble();
  if (ret)
    ; // cout << get_err_msg(ret);

//  check_line(str);
}
*/

//=============================================================================

/*
int main()
{
  CAsm6502 asmx(_T("y:\\m6502\\program.m65"));

  asmx.test();

  return 0;
}
*/

//=============================================================================

CString CAsm6502::GetErrMsg(Stat stat)
{
	if ((stat<OK || stat>=ERR_LAST) && stat!=STAT_USER_DEF_ERR)
	{
		ASSERT(false);		// b³êdna wartoœæ 'stat'
		return CString(_T("???"));
	}

	CString msg,form,txt;

//  if (!text->IsPresent())	// asemblacja wiersza?
	if (check_line)
	{
		ASSERT(stat>0);
		if (form.LoadString(IDS_ASM_FORM3) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
			msg.Format(form,(int)stat,(LPCTSTR)txt);
		return msg;
	}

	switch (stat)
	{
	case OK:
		msg.LoadString(IDS_ASM_ERR_MSG_FIRST);
		break;
	case ERR_OUT_OF_MEM:
		if (form.LoadString(IDS_ASM_FORM3) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
			msg.Format(form,(int)stat,(LPCTSTR)txt);
		break;
	case ERR_FILE_READ:
		if (form.LoadString(IDS_ASM_FORM2) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
			msg.Format(form,(int)stat,(LPCTSTR)txt,(LPCTSTR)text->GetFileName());
		break;
	case ERR_UNDEF_LABEL:	// niezdefiniowana etykieta
	case ERR_PHASE:
	case ERR_LABEL_REDEF:	// etykieta ju¿ zdefiniowana
		if (form.LoadString(IDS_ASM_FORM4) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
			msg.Format(form,(int)stat,(LPCTSTR)txt,(LPCTSTR)err_ident,text->GetLineNo()+1,(LPCTSTR)text->GetFileName());
		break;
	case STAT_USER_DEF_ERR:
		if (!user_error_text.IsEmpty())
		{
			if (form.LoadString(IDS_ASM_FORM5))
				msg.Format(form,(LPCTSTR)user_error_text,text->GetLineNo()+1,(LPCTSTR)text->GetFileName());
		}
		else
		{
			if (form.LoadString(IDS_ASM_FORM6))
				msg.Format(form,text->GetLineNo()+1,(LPCTSTR)text->GetFileName());
		}
		break;
	default:
		if (form.LoadString(IDS_ASM_FORM1) && txt.LoadString(IDS_ASM_ERR_MSG_FIRST+stat))
			msg.Format(form,(int)stat,(LPCTSTR)txt,text->GetLineNo()+1,(LPCTSTR)text->GetFileName());
		break;
	}

	return msg;
}

//=============================================================================

void CAsm6502::CListing::Remove()
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	try
	{
		m_File.Remove(m_File.GetFilePath());
	}
	catch (CFileException *)
	{
	}
}


void CAsm6502::CListing::NextLine()
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	if (m_nLine != 0)
		m_File.WriteString(m_Str);
	m_nLine++;
	m_Str.Format(_T("%04d    "),m_nLine);
}


void CAsm6502::CListing::AddCodeBytes(UINT16 addr, int code1/*= -1*/, int code2/*= -1*/, int code3/*= -1*/)
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	TCHAR buf[32];
	if (code3 != -1)
		wsprintf(buf,_T("%04X  %02X %02X %02X     "),(int)addr,(int)code1,code2,code3);
	else if (code2 != -1)
		wsprintf(buf,_T("%04X  %02X %02X        "),(int)addr,(int)code1,code2);
	else if (code1 != -1)
		wsprintf(buf,_T("%04X  %02X           "),(int)addr,(int)code1);
	else
		wsprintf(buf,_T("%04X               "),(int)addr);
	m_Str += buf;
}


void CAsm6502::CListing::AddValue(UINT16 val)
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	TCHAR buf[32];
	wsprintf(buf,_T("  %04X             "),val);
	m_Str += buf;
}


void CAsm6502::CListing::AddBytes(UINT16 addr, UINT16 mask, const UINT8 mem[], int len)
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	ASSERT(len > 0);
	TCHAR buf[32];
	for (int i=0; i<len; i+=4)
	{
		switch ((len-i) % 4)
		{
		case 1:
			wsprintf(buf,_T("%04X  %02X           "),int(addr),int(mem[addr & mask]));
			break;
		case 2:
			wsprintf(buf,_T("%04X  %02X %02X        "),int(addr),int(mem[addr & mask]),
				int(mem[addr+1 & mask]));
			break;
		case 3:
			wsprintf(buf,_T("%04X  %02X %02X %02X     "),int(addr),int(mem[addr & mask]),
				int(mem[addr+1 & mask]),int(mem[addr+2 & mask]));
			break;
		case 0:
			wsprintf(buf,_T("%04X  %02X %02X %02X %02X  "),int(addr),int(mem[addr & mask]),
				int(mem[addr+1 & mask]),int(mem[addr+2 & mask]),int(mem[addr+3 & mask]));
			break;
		}
		m_Str += buf;
		addr = addr+4 & mask;
		NextLine();
	}
}


void CAsm6502::CListing::AddSourceLine(const TCHAR *line)
{
	ASSERT(m_nLine != -1);	// plik musi byæ otwarty
	m_Str += line;
}


CAsm6502::CListing::CListing(const TCHAR *fname)
{
	if (fname && *fname)
	{
		Open(fname);
		m_nLine = 0;
	}
	else
		m_nLine = -1;
}
