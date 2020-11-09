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

// SyntaxExample.cpp : implementation file
//

#include "stdafx.h"
#include "SyntaxExample.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSyntaxExample

CSyntaxExample::CSyntaxExample()
{
	m_nTabStop = 8;
	m_hEditorFont = 0;

	m_rgbBackground = RGB(255,255,255);
	m_rgbText = m_rgbInstruction = m_rgbDirective =
		m_rgbComment = m_rgbNumber = m_rgbString = m_rgbSelection = RGB(0,0,0);

	m_vbBold[0] = m_vbBold[1] = m_vbBold[2] = m_vbBold[3] = m_vbBold[4] = false;
}

CSyntaxExample::~CSyntaxExample()
{
}


BEGIN_MESSAGE_MAP(CSyntaxExample, CButton)
	//{{AFX_MSG_MAP(CSyntaxExample)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSyntaxExample message handlers

void CSyntaxExample::DrawItem(LPDRAWITEMSTRUCT pDrawItemStruct)
{
	if (pDrawItemStruct->itemAction != ODA_DRAWENTIRE)
		return;

	CRect rect;
	GetClientRect(rect);
	CDC* pDC= CDC::FromHandle(pDrawItemStruct->hDC);
	if (pDC == 0)
		return;

	//Redraw(pDC, pDrawItemStruct->rcItem, false);
	rect.DeflateRect(1, 1);
	pDC->FillSolidRect(rect, m_rgbBackground);

	static const TCHAR* vszText[]=
	{
		_T("\4; Example code \7snippet"),
		_T("\1IO_PutC\t= \5$e001"),
		_T("\t\3.ORG \5$0600"),
		_T("\t\2LDA\1 #\6'a'"),
		_T("\t\2STA\1 IO_PutC"),
		_T("\1str\t\3.STR \6\"Hello World\""),
		_T("\t\2LDA\1 #>str"),
		_T("\t\2LDX\1 #<str"),
		_T("\t\2JSR\1 PrintS"),
		_T("\t\2BRK"),
		0
	};

	CFont fntBold;
	HGDIOBJ hOldFont= 0;
	if (m_hEditorFont)
	{
		hOldFont = ::SelectObject(*pDC, m_hEditorFont);

		LOGFONT lf;
		if (::GetObject(m_hEditorFont, sizeof lf, &lf))
		{
			lf.lfWeight = FW_BOLD;
			fntBold.CreateFontIndirect(&lf);
		}
	}

	int nDy= 16;
	int nDx= 16;
	TEXTMETRIC tm;
	if (pDC->GetTextMetrics(&tm))
	{
		nDy = tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
		nDx = tm.tmAveCharWidth;
	}

	pDC->SetBkMode(OPAQUE);

	int nTabStop= m_nTabStop ? m_nTabStop : 1;

	int y= rect.top;
	for (const char** ppcszText= vszText; *ppcszText != 0; ++ppcszText)
	{
		const TCHAR* pcszText= *ppcszText;
		int nLength= _tcslen(pcszText);

		int x= rect.left;
		int idx= 0;
		COLORREF rgbText= m_rgbText;
		COLORREF rgbBack= m_rgbBackground;
		HFONT hFont= m_hEditorFont;
		bool bColorChange= true;
		for (int i= 0; i < nLength; ++i)
		{
			TCHAR c= pcszText[i];

			if (c == '\t')
			{
				x += (nTabStop - idx % nTabStop) * nDx;
				continue;
			}

			if (c < ' ')	// special control char?
			{
				rgbBack = m_rgbBackground;
				switch (c)
				{
				case 1: rgbText = m_rgbText;		hFont = m_hEditorFont; break;
				case 2: rgbText = m_rgbInstruction;	hFont = m_vbBold[0] ? fntBold : m_hEditorFont; break;
				case 3: rgbText = m_rgbDirective;	hFont = m_vbBold[1] ? fntBold : m_hEditorFont; break;
				case 4: rgbText = m_rgbComment;		hFont = m_vbBold[2] ? fntBold : m_hEditorFont; break;
				case 5: rgbText = m_rgbNumber;		hFont = m_vbBold[3] ? fntBold : m_hEditorFont; break;
				case 6: rgbText = m_rgbString;		hFont = m_vbBold[4] ? fntBold : m_hEditorFont; break;
				case 7: rgbBack = m_rgbSelection;	break;
				default:
					ASSERT(false);
					break;
				}
				bColorChange = true;
				continue;
			}
			else
				++idx;

			if (bColorChange)
			{
				if (hFont)
					::SelectObject(*pDC, hFont);
				pDC->SetTextColor(rgbText);
				pDC->SetBkColor(rgbBack);
				bColorChange = false;
			}

			pDC->TextOut(x, y, &c, 1);
			x += nDx;
		}

		y += nDy;

	}

	if (hOldFont)
		::SelectObject(*pDC, hOldFont);

	COLORREF rgbFrame= RGB(192,192,192);
	rect.InflateRect(1, 1);
	pDC->Draw3dRect(rect, rgbFrame, rgbFrame);
}


BOOL CSyntaxExample::OnEraseBkgnd(CDC* pDC)
{
	return true;
}
