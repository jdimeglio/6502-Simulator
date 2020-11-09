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


class CDeasm : public CAsm, public CObject
{
	static const TCHAR mnemonics[];

	CString SetMemZPGInfo(UINT8 addr, UINT8 val);	// opis komórki strony zerowej pamiêci
	CString SetMemInfo(UINT16 addr, UINT8 val);	// opis komórki pamiêci
	CString SetValInfo(UINT8 val);		// opis wartoœci 'val'

public:
	CDeasm()
	{ }
	~CDeasm()
	{ }

	CString DeasmInstr(const CContext& ctx, DeasmFmt flags, int& ptr);
	CString DeasmInstr(const CmdInfo& ci, DeasmFmt flags);
	CString ArgumentValue(const CContext &ctx, int ptr= -1);

	CString Mnemonic(UINT8 code, bool bUse6502, bool bUseBrk= false);
	CString Argument(UINT8 cmd, CodeAdr mode, UINT16 addr, UINT8 arg1, UINT8 arg2, bool bLabel= false, bool bHelp= false);
	CString Binary(UINT8 val);
	int FindPrevAddr(UINT16 &addr, const CContext &ctx, int cnt= 1);
	int FindNextAddr(UINT16 &addr, const CContext &ctx, int cnt= 1);
	int FindDelta(UINT16 &addr, UINT16 dest, const CContext &ctx, int max_lines);
};
