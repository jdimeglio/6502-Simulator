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
#include "Deasm.h"

extern C6502App theApp;		// dostêp do GetProcType() z CGlobal


CString CDeasm::DeasmInstr(const CmdInfo& ci, DeasmFmt flags)
{
	CString str(_T(' '), 128), fmt(_T(' '), 128);
	str.Empty();

	UINT16 addr= ci.pc;
	UINT8 cmd= ci.cmd;
	UINT16 uLen= cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];

	if (flags & DF_ADDRESS)
	{
		fmt.Format("%04X  ",(int)addr);
		str += fmt;
	}

	if (flags & DF_CODE_BYTES)
	{
		switch (uLen)
		{
		case 1:
			fmt.Format("%02X        ", int(cmd));
			break;
		case 2:
			fmt.Format("%02X %02X     ", int(cmd), int(ci.arg1));
			break;
		case 3:
			fmt.Format("%02X %02X %02X  ", int(cmd), int(ci.arg1), int(ci.arg2));
			break;
		default:
			fmt.Empty();
			ASSERT(FALSE);
		}
		str += fmt;
	}

	bool b6502= theApp.m_global.m_bProc6502 && !(flags & DF_65C02);
	str += Mnemonic(cmd, b6502, !!(flags & DF_USE_BRK));

	str += Argument(cmd, (CodeAdr)CodeToMode(b6502)[cmd], addr, ci.arg1, ci.arg2, flags & DF_LABELS, flags & DF_HELP);

	return str;
}


CString CDeasm::DeasmInstr(const CContext& ctx, DeasmFmt flags, int& ptr)
{
	ASSERT(ptr == -1 || ptr >= 0 && ptr <= 0xFFFF);

	CString str(_T(' '), 128), fmt(_T(' '), 128);
	str.Empty();
	UINT16 addr= ptr >= 0 ? ptr : ctx.pc;
	UINT8 cmd= ctx.mem[addr];
	UINT16 uLen= cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];

	if (flags & DF_ADDRESS)
	{
		fmt.Format("%04X  ",(int)addr);
		str += fmt;
	}

	if (flags & DF_CODE_BYTES)
	{
		switch (uLen)
		{
		case 1:
			fmt.Format("%02X        ", int(ctx.mem[addr]));
			break;
		case 2:
			fmt.Format("%02X %02X     ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]));
			break;
		case 3:
			fmt.Format("%02X %02X %02X  ", int(ctx.mem[addr]), int(ctx.mem[addr + 1]), int(ctx.mem[addr + 2]));
			break;
		default:
			fmt.Empty();
			ASSERT(FALSE);
		}
		str += fmt;
	}

	bool b6502= theApp.m_global.m_bProc6502 && !(flags & DF_65C02);
	str += Mnemonic(cmd, b6502);

	str += Argument(cmd, (CodeAdr)CodeToMode(b6502)[cmd], addr, ctx.mem[addr+1], ctx.mem[addr+2], flags & DF_LABELS);

	if (flags & DF_BRANCH_INFO)
	{
		bool sign= FALSE;
		switch (CodeToCommand()[cmd])
		{
		case C_BRA:
			sign = TRUE;
			break;
		case C_BPL:
			if (!ctx.negative)
				sign = TRUE;
			break;
		case C_BMI:
			if (ctx.negative)
				sign = TRUE;
			break;
		case C_BVC:
			if (!ctx.overflow)
				sign = TRUE;
			break;
		case C_BVS:
			if (ctx.overflow)
				sign = TRUE;
			break;
		case C_BCC:
			if (!ctx.carry)
				sign = TRUE;
			break;
		case C_BCS:
			if (ctx.carry)
				sign = TRUE;
			break;
		case C_BNE:
			if (!ctx.zero)
				sign = TRUE;
			break;
		case C_BEQ:
			if (ctx.zero)
				sign = TRUE;
			break;
		case C_BBS:
			{
				UINT8 zpg= ctx.mem[addr + 1];
				int bit_no= (cmd >> 4) & 0x07;
				if (ctx.mem[zpg] & UINT8(1 << bit_no))
					sign = TRUE;
				break;
			}
		case C_BBR:
			{
				UINT8 zpg= ctx.mem[addr + 1];
				int bit_no= (cmd >> 4) & 0x07;
				if (!(ctx.mem[zpg] & UINT8(1 << bit_no)))
					sign = TRUE;
				break;
			}
		}
		if (sign)
			str += " ->";		// oznaczenie aktywnego skoku
	}

	ptr = (addr + uLen) & 0xFFFF;	// adr nast. instr.

	return str;
}


CString CDeasm::Mnemonic(UINT8 code, bool bUse6502, bool bUseBrk/*= false*/)
{
	ASSERT(CodeToCommand(bUse6502)[code] <= C_ILL && CodeToCommand(bUse6502)[code] >= 0);
	TCHAR buf[16];
	UINT8 cmd= CodeToCommand(bUse6502)[code];
	if (cmd == C_ILL || (cmd == C_BRK && !bUseBrk))	// kod nielegalnego rozkazu lub BRK
		wsprintf(buf, ".DB $%02X", int(code));
	else
	{
		memcpy(buf, mnemonics + 3 * cmd, 3);
		buf[3] = _T('\0');
	}
	return CString(buf);
}


const TCHAR CDeasm::mnemonics[]=
	"LDALDXLDYSTASTXSTYSTZTAXTXATAYTYATXSTSXADCSBCCMPCPXCPYINCDECINADEAINXDEXINYDEY"
	"ASLLSRROLRORANDORAEORBITTSBTRBJMPJSRBRKBRABPLBMIBVCBVSBCCBCSBNEBEQRTSRTIPHAPLA"
	"PHXPLXPHYPLYPHPPLPCLCSECCLVCLDSEDCLISEINOP"
	"BBRBBSRMBSMB"
	"???";


CString CDeasm::Argument(UINT8 cmd, CodeAdr mode, UINT16 addr, UINT8 arg1, UINT8 arg2, bool bLabel, bool bHelp)
{
	CString str;
	addr++;
	UINT8 lo= arg1;
	UINT16 word= arg1 + (arg2 << 8);

	switch (mode)
	{
	case A_IMP:		// implied
	case A_IMP2:	// implied dla BRK
		if (bHelp)
			str = _T("         |Implied");
		break;
	case A_ACC:		// accumulator
		if (bHelp)
			str = _T("         |Accumulator");
		break;
	case A_ILL:		// wartoœæ do oznaczania nielegalnych rozkazów w symulatorze (ILLEGAL)
		break;
	case A_IMM:		// immediate
		str.Format(" #$%02X",(int)lo);
		if (bHelp)
			str += _T("    |Immediate");
		break;
	case A_ZPG:		// zero page
		str.Format(bLabel ? " z%02X" : " $%02X",(int)lo);
		if (bHelp)
			str += _T("     |Zero Page");
		break;
	case A_ABS:		// absolute
		str.Format(bLabel ? " a%04X" : " $%04X",(int)word);
		if (bHelp)
			str += _T("   |Absolute");
		break;
	case A_ABS_X:	// absolute indexed X
		str.Format(bLabel ? " a%04X,X" : " $%04X,X",(int)word);
		if (bHelp)
			str += _T(" |Absolute Indexed, X");
		break;
	case A_ABS_Y:	// absolute indexed Y
		str.Format(bLabel ? " a%04X,Y" : " $%04X,Y",(int)word);
		if (bHelp)
			str += _T(" |Absolute Indexed, Y");
		break;
	case A_ZPG_X:	// zero page indexed X
		str.Format(bLabel ? " z%02X,X" : " $%02X,X",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indexed, X");
		break;
	case A_ZPG_Y:	// zero page indexed Y
		str.Format(bLabel ? " z%02X,Y" : " $%02X,Y",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indexed, Y");
		break;
	case A_REL:		// relative
		if (bHelp)
			str = _T(" label  |Relative");
		else
			str.Format(bLabel ? " e%04X" : " $%04X", int( lo & 0x80 ? addr+1 - (0x100 - lo) : addr+1 + lo ));
		break;
	case A_ZPGI:	// zero page indirect
		str.Format(bLabel ? " (z%02X)" : " ($%02X)",(int)lo);
		if (bHelp)
			str += _T("   |Zero Page Indirect");
		break;
	case A_ZPGI_X:	// zero page indirect, indexed X
		str.Format(bLabel ? " (z%02X,X)" : " ($%02X,X)",(int)lo);
		if (bHelp)
			str += _T(" |Zero Page Indexed X, Indirect");
		break;
	case A_ZPGI_Y:	// zero page indirect, indexed Y
		str.Format(bLabel ? " (z%02X),Y" : " ($%02X),Y",(int)lo);
		if (bHelp)
			str += _T(" |Zero Page Indirect, Indexed Y");
		break;
	case A_ABSI:	// absolute indirect
		str.Format(bLabel ? " (a%04X)" : " ($%04X)",(int)word);
		if (bHelp)
			str += _T(" |Absolute Indirect");
		break;
	case A_ABSI_X:	// absolute indirect, indexed X
		str.Format(bLabel ? " (a%04X,X)" : " ($%04X,X)",(int)word);
		if (bHelp)
			str += _T(" |Absolute Indexed X, Indirect");
		break;
	case A_ZPG2:	// zero page dla RMB i SMB
		{
			unsigned int bit_no= (cmd>>4) & 0x07;
			str.Format(bLabel ? " #%u,z%02X" : " #%u,$%02X",(unsigned int)bit_no,(int)lo);
			if (bHelp)
				str += _T(" |Memory Bit Manipulation");
			break;
		}
	case A_ZREL:	// zero page / relative dla BBS i BBR
		{
			UINT8 hi= arg2; //ctx.mem[addr + 1];
			unsigned int bit_no= (cmd >> 4) & 0x07;
			str.Format(bLabel ? " #%u,z%02X,e%04X" : " #%u,$%02X,$%04X", bit_no, int(lo), int( hi & 0x80 ? addr+2 - (0x100 - hi) : addr+2 + hi ));
			if (bHelp)
				str += _T(" |Relative Bit Branch");
			break;
		}
	default:
		ASSERT(FALSE);
	}

	return str;
}


CString CDeasm::ArgumentValue(const CContext &ctx, int cmd_addr /*= -1*/)
{
	CString str(_T("-"));
	UINT8 arg;
	UINT16 addr,tmp;

	ASSERT(cmd_addr==-1 || cmd_addr>=0 && cmd_addr<=0xFFFF);	// b³êdny adres

	if (cmd_addr == -1)
		cmd_addr = ctx.pc;

//  UINT8 cmd= ctx.mem[cmd_addr];
	UINT8 mode= CodeToMode()[ctx.mem[cmd_addr]];
	cmd_addr = (cmd_addr + 1) & 0xFFFF;

	switch (mode)
	{
	case A_IMP:
	case A_IMP2:
	case A_ACC:
		break;

	case A_IMM:
		return SetValInfo(ctx.mem[cmd_addr]);

	case A_REL:
		arg = ctx.mem[cmd_addr];
		if (arg & 0x80)	// skok do ty³u
			str.Format(_T("PC-$%02X"),int(0x100 - arg));
		else		// skok do przodu
			str.Format(_T("PC+$%02X"),int(arg));
		return str;

	case A_ZPGI:
		arg = ctx.mem[cmd_addr];	// adres komórki na str. 0
		addr = ctx.mem[arg];		// adres wsk. przez komórki
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPG:
	case A_ZPG2:
		addr = ctx.mem[cmd_addr];
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ZPG_X:
		addr = (ctx.mem[cmd_addr]+ctx.x) & 0xFF;
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ZPG_Y:
		addr = (ctx.mem[cmd_addr]+ctx.y) & 0xFF;
		return SetMemZPGInfo((UINT8)addr, ctx.mem[addr]);

	case A_ABS:
		addr = ctx.mem[cmd_addr];	// m³odszy bajt adresu
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ABSI:
		addr = ctx.mem[cmd_addr];	// m³odszy bajt adresu
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		tmp = ctx.mem[addr];		// liczba pod adresem
		tmp += UINT16( ctx.mem[addr + 1] ) << 8;
		//      tmp &= ctx.mem_mask;
		return SetMemInfo(tmp, ctx.mem[tmp]);

	case A_ABSI_X:
		addr = ctx.mem[cmd_addr] + ctx.x;	// m³odszy bajt adresu + przesuniêcie X
		if (theApp.m_global.GetProcType() && (cmd_addr & 0xFF)==0xFF)    // m³odszy bajt == 0xFF?
			addr += UINT16( ctx.mem[cmd_addr - 0xFF] ) << 8;  // zgodnie z b³êdem w 6502
		else
			addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		tmp = ctx.mem[addr];		// liczba pod adresem
		tmp += UINT16( ctx.mem[addr + 1] ) << 8;
		//      tmp &= ctx.mem_mask;
		return SetMemInfo(tmp, ctx.mem[tmp]);

	case A_ABS_X:
		addr = ctx.mem[cmd_addr] + ctx.x;	// m³odszy bajt adresu i przesuniêcie X
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ABS_Y:
		addr = ctx.mem[cmd_addr] + ctx.y;	// m³odszy bajt adresu i przesuniêcie Y
		addr += UINT16( ctx.mem[cmd_addr + 1] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPGI_X:
		arg = ctx.mem[cmd_addr];	// adres komórki na str. 0
		arg = (arg + ctx.x) & 0xFF;
		addr = ctx.mem[arg];		// adres wsk. przez komórki
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZPGI_Y:
		arg = ctx.mem[cmd_addr];	// adres komórki na str. 0
		addr = ctx.mem[arg] + ctx.y;	// adres wsk. przez komórki i przesuniêcie Y
		addr += UINT16( ctx.mem[(arg+1)&0xFF] ) << 8;
		//      addr &= ctx.mem_mask;
		return SetMemInfo(addr, ctx.mem[addr]);

	case A_ZREL:
		{
			CString tmp= SetMemZPGInfo((UINT8)cmd_addr, ctx.mem[cmd_addr]);
			arg = ctx.mem[cmd_addr + 1];
			if (arg & 0x80)	// skok do ty³u
				str.Format(_T("; PC-$%02X"),int(0x100 - arg));
			else		// skok do przodu
				str.Format(_T("; PC+$%02X"),int(arg));
			return tmp+str;
		}

	case A_ILL:
		str.Empty();
		break;

	default:
		ASSERT(FALSE);
		str.Empty();
	}

	return str;
}


CString CDeasm::SetMemInfo(UINT16 addr, UINT8 val)	// opis komórki pamiêci
{
  CString str;
  str.Format("[%04X]: $%02X, %d, '%c', %s", int(addr), int(val), val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}


CString CDeasm::SetMemZPGInfo(UINT8 addr, UINT8 val)	// opis komórki strony zerowej pamiêci
{
  CString str;
  str.Format("[%02X]: $%02X, %d, '%c', %s", int(addr), int(val), val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}


CString CDeasm::SetValInfo(UINT8 val)	// opis wartoœci 'val'
{
  CString str;
  str.Format("%d, '%c', %s", val&0xFF, val ? (char)val : (char)' ', (const TCHAR *)Binary(val));
  return str;
}


CString CDeasm::Binary(UINT8 val)
{
  CString bin(_T(' '),8);

  bin.SetAt(0, val & 0x80 ? _T('1') : _T('0') );
  bin.SetAt(1, val & 0x40 ? _T('1') : _T('0') );
  bin.SetAt(2, val & 0x20 ? _T('1') : _T('0') );
  bin.SetAt(3, val & 0x10 ? _T('1') : _T('0') );
  bin.SetAt(4, val & 0x08 ? _T('1') : _T('0') );
  bin.SetAt(5, val & 0x04 ? _T('1') : _T('0') );
  bin.SetAt(6, val & 0x02 ? _T('1') : _T('0') );
  bin.SetAt(7, val & 0x01 ? _T('1') : _T('0') );

  return bin;
}


// odszukanie adresu rozkazu poprzedzaj¹cego dany rozkaz
int CDeasm::FindPrevAddr(UINT16 &addr, const CContext &ctx, int cnt/*= 1*/)
{
  ASSERT(cnt >= 0);
  if (cnt <= 0)
    return 0;

  if (cnt > 1)
  {
    int len= max(10, cnt * 3) + 2;
    UINT16 start= int(addr) - len > 0 ? addr - len : 0;
    start &= ~1;	// parzysty adres
    CWordArray addresses;
    addresses.SetSize(len + 4);
    UINT8 cmd;
    int ret= 0;
    int i;

    for (i = 0; start < addr; i++)
    {
      addresses[i] = start;
      cmd = ctx.mem[start];
//      start += mode_to_len[CodeToMode()[cmd]];
      start += cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]];
//      start &= ctx.mem_mask;
    }
    if (start == addr)
      ret = 1;
    else
      ret = -1;
    addr = addresses[i - cnt];
    return ret;
  }
  else
  {
    UINT16 start= int(addr)-10 > 0 ? addr-10 : 0;
    UINT16 prev= start;
    int ret= 0;
//    ASSERT(addr <= ctx.mem_mask);		// niepoprawny adres; za du¿y
    UINT8 cmd;

    while (start < addr)
    {
      prev = start;
      cmd = ctx.mem[start];
      if (cmd == 0)		// rozkaz BRK?
	start++;		// zwiêkszamy tylko o 1, chocia¿ normalnie BRK zwiêksza o 2
      else
	start += mode_to_len[CodeToMode()[cmd]];

//      start &= ctx.mem_mask;
    }
    cmd = ctx.mem[prev];
/*
    if (cmd == 0 && ctx.mem[prev - 1] == 0)	// dwa zera przed rozkazem?
    {
      prev--;				// cofamy adres o 2
      ret = 1;				// bo jest to jeden rozkaz BRK
    }
    else
*/
    if (prev == addr)
      ret = 0;	// jesteœmy na pocz¹tku - nie ma przesuniêcia
    else if ((prev + (cmd == 0 ? 1 : mode_to_len[CodeToMode()[cmd]])) == addr)
      ret = 1;	// jest przesuniêcie o jeden wiersz
    else
      ret = -1;	// jest przesuniêcie, ale wp³ynê³o na zmianê kolejnych rozkazów

    addr = prev;
    return ret;
  }
}


// odszukanie adresu rozkazu nastêpuj¹cego po 'cnt'-ym rozkazie od 'addr'
int CDeasm::FindNextAddr(UINT16 &addr, const CContext &ctx, int cnt/*= 1*/)
{
//  ASSERT(addr <= ctx.mem_mask);		// niepoprawny adres; za du¿y

  int ret= 0;
  UINT16 next= addr;

  for (UINT16 address= addr; cnt; cnt--)
  {
    next = address;
    next += ctx.mem[address] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[address]]];
    ASSERT(next != address);

    if (next < addr)
      ret = 0;	// "przewiniêcie siê" adresu
    else
      ret = 1;	// nastêpny adres znaleziony

    address = next;
  }

  addr = next;
  return ret;
}


// spr. o ile wierszy nale¿y przesun¹æ zawartoœæ okna aby dotrzeæ od 'addr' do 'dest'
int CDeasm::FindDelta(UINT16 &addr, UINT16 dest, const CContext &ctx, int max_lines)
{
  if (dest == addr)
    return 0;

  if (dest < addr)
  {
    UINT16 start= dest;
    int i;
    for (i=0; start < addr; i++)
    {
//      start += mode_to_len[CodeToMode()[ ctx.mem[start] ]];
      start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
//      start &= ctx.mem_mask;
      if (i >= max_lines)
	break;
    }
    i = start == addr ? i : -i;
    addr = dest;
    return i;
  }
  else
  {
    UINT16 start= addr;
    int i;
    for (i=0; start < dest; i++)
    {
//      start += mode_to_len[CodeToMode()[ ctx.mem[start] ]];
      start += ctx.mem[start] == 0 ? 1 : mode_to_len[CodeToMode()[ctx.mem[start]]];
//      start &= ctx.mem_mask;
      if (i >= max_lines)
	break;
    }
    i = start == addr ? i : -i;
    addr = dest;
    return i;
  }
}
