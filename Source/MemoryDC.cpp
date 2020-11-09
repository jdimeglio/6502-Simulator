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

// MemoryDC.cpp: implementation of the CMemoryDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryDC::CMemoryDC(CDC& dc, CWnd* pWnd, COLORREF rgbClrBack/*= -1*/)
{
	CRect rect;
	pWnd->GetClientRect(rect);
	Init(dc, rect, rgbClrBack);
}

CMemoryDC::CMemoryDC(CDC& dc, const CRect& rect)
{
	Init(dc, rect, -1);
}


void CMemoryDC::BitBlt()
{
	POINT ptOrg= GetWindowOrg();
	CPoint ptVpOrg= GetViewportOrg();
	ptVpOrg -= m_ptPos;
	SetViewportOrg(0, 0);
	m_pDC->BitBlt(ptOrg.x - ptVpOrg.x, ptOrg.y - ptVpOrg.y, m_Size.cx, m_Size.cy, this, ptOrg.x, ptOrg.y, SRCCOPY);
}

CMemoryDC::~CMemoryDC()
{
	DeleteDC();		// delete in this order!
	m_bmpScreen.DeleteObject();
}



void CMemoryDC::Init(CDC& dc, const CRect& rect, COLORREF rgbClrBack)
{
	m_Size = rect.Size();
	m_ptPos = rect.TopLeft();
	m_pDC = &dc;

	m_bmpScreen.CreateCompatibleBitmap(&dc, m_Size.cx, m_Size.cy);
	if (m_bmpScreen.m_hObject == 0)
	{
		DWORD dw= ::GetLastError();
		return;
	}

	CreateCompatibleDC(&dc);
	SelectObject(&m_bmpScreen);
	if (rgbClrBack != -1)
		FillSolidRect(0, 0, m_Size.cx, m_Size.cy, rgbClrBack);

	POINT ptOrg= dc.GetWindowOrg();
	SetWindowOrg(ptOrg);

	POINT ptVpOrg= dc.GetViewportOrg();
	SetViewportOrg(-rect.left + ptVpOrg.x, -rect.top + ptVpOrg.y);

	SetBrushOrg(rect.left % 8, rect.top % 8);
}
