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
#include "ConfigSettings.h"
#include "IOWindow.h"
#include "Deasm6502View.h"
#include "MemoryView.h"
#include "ZeroPageView.h"
#include "ConfigSettings.h"
#include "MainFrm.h"
#include "6502View.h"
#include "StackView.h"


LOGFONT* const CConfigSettings::fonts[]=
{
	&CSrc6502View::m_LogFont,
	&CIOWindow::m_LogFont,
	&CDeasm6502View::m_LogFont,
	&CMemoryView::m_LogFont,
	&CZeroPageView::m_LogFont,
	&CStackView::m_LogFont,
	NULL
};

CFont* const CConfigSettings::cfonts[]=	// struktury CFont w ca³ym programie
{
	&CSrc6502View::m_Font,
	&CIOWindow::m_Font,
	&CDeasm6502View::m_Font,
	&CMemoryView::m_Font,
	&CZeroPageView::m_Font,
	&CStackView::m_Font,
	NULL
};

COLORREF* CConfigSettings::text_color[]=
{
	&CSrc6502View::m_rgbTextColor,
	&CIOWindow::m_rgbTextColor,
	&CDeasm6502View::m_rgbInstr,
	&CMemoryView::m_rgbTextColor,
	&CZeroPageView::m_rgbTextColor,
	&CStackView::m_rgbTextColor,
	NULL
};


COLORREF* CConfigSettings::bkgnd_color[]=
{
	&CSrc6502View::m_rgbBkgndColor,
	&CIOWindow::m_rgbBackgndColor,
	&CDeasm6502View::m_rgbBkgnd,
	&CMemoryView::m_rgbBkgndColor,
	&CZeroPageView::m_rgbBkgndColor,
	&CStackView::m_rgbBkgndColor,
	NULL
};


COLORREF* CConfigSettings::color_syntax[]=
{
	&CSrc6502View::m_vrgbColorSyntax[0],
	&CSrc6502View::m_vrgbColorSyntax[1],
	&CSrc6502View::m_vrgbColorSyntax[2],
	&CSrc6502View::m_vrgbColorSyntax[3],
	&CSrc6502View::m_vrgbColorSyntax[4],
	&CSrc6502View::m_vrgbColorSyntax[5],
	0
};


BYTE* CConfigSettings::syntax_font_style[]=
{
	&CSrc6502View::m_vbyFontStyle[0],
	&CSrc6502View::m_vbyFontStyle[1],
	&CSrc6502View::m_vbyFontStyle[2],
	&CSrc6502View::m_vbyFontStyle[3],
	&CSrc6502View::m_vbyFontStyle[4],
	&CSrc6502View::m_vbyFontStyle[5],
	0
};
