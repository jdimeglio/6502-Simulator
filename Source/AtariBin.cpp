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

// AtariBin.cpp: implementation of the CAtariBin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AtariBin.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAtariBin::CAtariBin()
{}

CAtariBin::~CAtariBin()
{}


bool CAtariBin::LoadAtaBinFormat(CArchive& ar, COutputMem& mem, CMarkArea& area, int& prog_start)
{
	WORD wTemp;
	ar >> wTemp;
	if (wTemp != 0xFFFF)
		return false;

	WORD wBegin= 0;

	for (;;)
	{
		WORD wFrom, wTo;
		ar >> wFrom;
		ar >> wTo;
		if (wTo < wFrom)
			return false;

		if (wBegin == 0)
			wBegin = wFrom;

		area.SetStart(wFrom);
		area.SetEnd(wTo);

		mem.Load(ar, wFrom, wTo);

		if (ar.GetFile()->GetLength() >= ar.GetFile()->GetPosition() && ar.IsBufferEmpty())
			break;
	}

	int nStart= mem[0x2e0] + mem[0x2e1] * 256;		// run address
	if (nStart == 0)
		nStart = mem[0x2e2] + mem[0x2e3] * 256;		// init address
	if (nStart == 0)
		nStart = wBegin;	// beginning of first block
	if (nStart != 0)
		prog_start = nStart;

	return true;
}
