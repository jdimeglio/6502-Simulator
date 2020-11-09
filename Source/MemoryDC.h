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

// MemoryDC.h: interface for the CMemoryDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_)
#define AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMemoryDC : public CDC
{
public:
	CMemoryDC(CDC& dc, CWnd* pWnd, COLORREF rgbClrBack= -1);
	CMemoryDC(CDC& dc, const CRect& rect);
	virtual ~CMemoryDC();

	void BitBlt();

private:
	CDC* m_pDC;
	CBitmap m_bmpScreen;
	CPoint m_ptPos;
	CSize m_Size;

	void Init(CDC& dc, const CRect& rect, COLORREF rgbClrBack);
};

#endif // !defined(AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_)
