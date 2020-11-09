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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
//#include "6502.h"
#include "MainFrm.h"
#include "DialAsmStat.h"
#include "Options.h"
#include "DialEditBrkp.h"
#include "SaveCode.h"
#include "LoadCode.h"
#include "Deasm6502View.h"
#include "Splash.h"
#include "AFXPRIV.H"	// do podmiany LoadBarState()
#include "6502View.h"
#include "6502Doc.h"
#include "IntRequestGeneratorDlg.h"
#include "editcmd.h"
#include "DockBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//-----------------------------------------------------------------------------
// okna MainFrame, RegisterBar, IOWindow, MemoryView, ZeroPageView, IdentInfo
const HWND * /*const*/ CMainFrame::m_hWindows[]= {0,0,0,0,0,0,0,0,0,0};

WNDPROC CMainFrame::m_pfnOldProc;
CBitmap CMainFrame::m_bmpCode;		// obrazki w StatusBar
CBitmap CMainFrame::m_bmpDebug;
//-----------------------------------------------------------------------------
const TCHAR CMainFrame::REG_ENTRY_LAYOUT[]= _T("Bars\\Layout");
const TCHAR CMainFrame::REG_ENTRY_MAINFRM[]= _T("MainFrame");
const TCHAR CMainFrame::REG_POSX[]= _T("XPos");
const TCHAR CMainFrame::REG_POSY[]= _T("YPos");
const TCHAR CMainFrame::REG_SIZX[]= _T("Width");
const TCHAR CMainFrame::REG_SIZY[]= _T("Height");
const TCHAR CMainFrame::REG_STATE[]= _T("Maximize");

void CMainFrame::ConfigSettings(bool load)
{
	static const LOGFONT LogFont=
	{
		-12,// LONG lfHeight;
		0,	// LONG lfWidth;
		0,	// LONG lfEscapement;
		0,	// LONG lfOrientation;
		0,	// LONG lfWeight;
		0,	// BYTE lfItalic;
		0,	// BYTE lfUnderline;
		0,	// BYTE lfStrikeOut;
		0,	// BYTE lfCharSet;
		0,	// BYTE lfOutPrecision;
		0,	// BYTE lfClipPrecision;
		0,	// BYTE lfQuality;
		FIXED_PITCH,	// BYTE lfPitchAndFamily;
		"Fixedsys"	// CHAR lfFaceName[LF_FACESIZE];
	};

	static const TCHAR ENTRY_SYM[]= _T("Simulator");
	static const TCHAR SYM_FIN[]= _T("Finish");
	static const TCHAR SYM_IO_ADDR[]= _T("IOAddress");
	static const TCHAR SYM_IO_ENABLED[]= _T("IOEnabled");
	static const TCHAR SYM_PROT_MEM[]= _T("ProtectMem");
	static const TCHAR SYM_PROT_MEM_FROM[]= _T("ProtectMemFrom");
	static const TCHAR SYM_PROT_MEM_TO[]= _T("ProtectMemTo");
	static const TCHAR SYM_WND_X[]= _T("TerminalXPos");
	static const TCHAR SYM_WND_Y[]= _T("TerminalYPos");
	static const TCHAR SYM_WND_W[]= _T("TerminalWidth");
	static const TCHAR SYM_WND_H[]= _T("TerminalHeight");

	static const TCHAR ENTRY_ASM[]= _T("Assembler");
	static const TCHAR ASM_CASE[]= _T("CaseSensitive");
	static const TCHAR ASM_GEN_LST[]= _T("GenerateListing");
	static const TCHAR ASM_LST_FILE[]= _T("ListingFile");
	static const TCHAR ASM_GEN_BYTE[]= _T("GenerateBRKExtraByte");
	static const TCHAR ASM_BRK_BYTE[]= _T("BRKExtraByte");

	static const TCHAR ENTRY_EDIT[]= _T("Editor");
	static const TCHAR EDIT_FONT[]= _T("Font");
	static const TCHAR EDIT_TAB_STEP[]= _T("TabStep");
	static const TCHAR EDIT_AUTO_INDENT[]= _T("AutoIndent");
	static const TCHAR EDIT_SYNTAX_CHECK[]= _T("SyntaxChecking");
	static const TCHAR EDIT_CAPITALS[]= _T("AutoUppercase");
	static const TCHAR EDIT_FILENEW[]= _T("FileNew");

	static const TCHAR ENTRY_DEASM[]= _T("Deassembler");
	static const TCHAR DEASM_ADDR_COLOR[]= _T("AddrColor");
	static const TCHAR DEASM_CODE_COLOR[]= _T("CodeColor");
	static const TCHAR DEASM_SHOW_CODE[]= _T("ShowCode");

	static const TCHAR ENTRY_GEN[]= _T("General");
	static const TCHAR GEN_PROC[]= _T("ProcType");
	static const TCHAR GEN_BUS_WIDTH[]= _T("BusWidth");
	static const TCHAR GEN_PTR[]= _T("PointerColor");
	static const TCHAR GEN_BRKP[]= _T("BreakpointColor");
	static const TCHAR GEN_ERR[]= _T("ErrorColor");

	static const TCHAR ENTRY_VIEW[]= _T("View");
	static const TCHAR VIEW_IDENTS_X[]= _T("IdentsXPos");
	static const TCHAR VIEW_IDENTS_Y[]= _T("IdentsYPos");
	static const TCHAR VIEW_IDENTS_W[]= _T("IdentsW");
	static const TCHAR VIEW_IDENTS_H[]= _T("IdentsH");
	static const TCHAR VIEW_MEMO_X[]= _T("MemoryXPos");
	static const TCHAR VIEW_MEMO_Y[]= _T("MemoryYPos");
	static const TCHAR VIEW_MEMO_W[]= _T("MemoryW");
	static const TCHAR VIEW_MEMO_H[]= _T("MemoryH");
	static const TCHAR VIEW_MEMO_HID[]= _T("MemoryWndHidden");
	static const TCHAR VIEW_ZMEM_X[]= _T("ZeroPageXPos");
	static const TCHAR VIEW_ZMEM_Y[]= _T("ZeroPageYPos");
	static const TCHAR VIEW_ZMEM_W[]= _T("ZeroPageW");
	static const TCHAR VIEW_ZMEM_H[]= _T("ZeroPageH");
	static const TCHAR VIEW_ZMEM_HID[]= _T("ZeroPageWndHidden");
	static const TCHAR VIEW_STACK_X[]= _T("StackXPos");
	static const TCHAR VIEW_STACK_Y[]= _T("StackYPos");
	static const TCHAR VIEW_STACK_W[]= _T("StackW");
	static const TCHAR VIEW_STACK_H[]= _T("StackH");
	static const TCHAR VIEW_STACK_HID[]= _T("StackWndHidden");
	static const TCHAR VIEW_LOG_X[]= _T("LogWndXPos");
	static const TCHAR VIEW_LOG_Y[]= _T("LogWndYPos");
	static const TCHAR VIEW_LOG_W[]= _T("LogWndW");
	static const TCHAR VIEW_LOG_H[]= _T("LogWndH");
	static const TCHAR VIEW_LOG_HID[]= _T("LogWndHidden");
	static const TCHAR VIEW_IO_HID[]= _T("IOWndHidden");
	static const TCHAR VIEW_REGS_HID[]= _T("RegsWndHidden");
	static const TCHAR VIEW_FONT_ED[]= _T("FontEditor");
	static const TCHAR VIEW_ED_TCOL[]= _T("EditorTextColor");
	static const TCHAR VIEW_ED_BCOL[]= _T("EditorBkgndColor");
	static const TCHAR VIEW_FONT_SYM[]= _T("FontIOSymWnd");
	static const TCHAR VIEW_SYM_TCOL[]= _T("IOSymWndTextColor");
	static const TCHAR VIEW_SYM_BCOL[]= _T("IOSymWndBkgndColor");
	static const TCHAR VIEW_FONT_DEASM[]= _T("FontDeasm");
	static const TCHAR VIEW_DEASM_TCOL[]= _T("DeasmInstrColor");
	static const TCHAR VIEW_DEASM_BCOL[]= _T("DeasmBkgndColor");
	static const TCHAR VIEW_FONT_MEMO[]= _T("FontMemory");
	static const TCHAR VIEW_MEMO_TCOL[]= _T("MemoryTextColor");
	static const TCHAR VIEW_MEMO_BCOL[]= _T("MemoryBkgndColor");
	static const TCHAR VIEW_FONT_ZERO[]= _T("FontZeroPage");
	static const TCHAR VIEW_ZERO_TCOL[]= _T("ZeroPageTextColor");
	static const TCHAR VIEW_ZERO_BCOL[]= _T("ZeroPageBkgndColor");
	static const TCHAR VIEW_FONT_STACK[]= _T("FontStack");
	static const TCHAR VIEW_STACK_TCOL[]= _T("StackTextColor");
	static const TCHAR VIEW_STACK_BCOL[]= _T("StackBkgndColor");

	static const TCHAR * const idents[]=
	{ VIEW_FONT_ED, VIEW_FONT_SYM, VIEW_FONT_DEASM, VIEW_FONT_MEMO, VIEW_FONT_ZERO, VIEW_FONT_STACK };
	static const TCHAR * const tcolors[]=
	{ VIEW_ED_TCOL, VIEW_SYM_TCOL, VIEW_DEASM_TCOL, VIEW_MEMO_TCOL, VIEW_ZERO_TCOL, VIEW_STACK_TCOL };
	static const TCHAR * const bcolors[]=
	{ VIEW_ED_BCOL, VIEW_SYM_BCOL, VIEW_DEASM_BCOL, VIEW_MEMO_BCOL, VIEW_ZERO_BCOL, VIEW_STACK_BCOL };
	static const TCHAR * const syntax_colors[]=
	{ _T("ColorInstruction"), _T("ColorDirective"), _T("ColorComment"), _T("ColorNumber"), _T("ColorString"), _T("ColorSelection"), 0 };
	static const TCHAR * const syntax_font[]=
	{ _T("FontInstruction"), _T("FontDirective"), _T("FontComment"), _T("FontNumber"), _T("FontString"), 0 };

	CWinApp *pApp = AfxGetApp();

	if (load)		// odczyt?
	{
		theApp.m_global.SetSymFinish( (CAsm::Finish)(pApp->GetProfileInt(ENTRY_SYM, SYM_FIN, 0)) );
		CSym6502::io_addr    = pApp->GetProfileInt(ENTRY_SYM, SYM_IO_ADDR, 0xE000);
		CSym6502::io_enabled = pApp->GetProfileInt(ENTRY_SYM, SYM_IO_ENABLED, 1);
		CSym6502::s_bWriteProtectArea = !!pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM, 0);
		CSym6502::s_uProtectFromAddr = pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM_FROM, 0xC000);
		CSym6502::s_uProtectToAddr = pApp->GetProfileInt(ENTRY_SYM, SYM_PROT_MEM_TO, 0xCFFF);
		CPoint pos;
		pos.x = pApp->GetProfileInt(ENTRY_SYM, SYM_WND_X, 200);
		pos.y = pApp->GetProfileInt(ENTRY_SYM, SYM_WND_Y, 200);
		m_IOWindow.SetWndPos(pos);
		m_IOWindow.SetSize( pApp->GetProfileInt(ENTRY_SYM, SYM_WND_W, 40),
			pApp->GetProfileInt(ENTRY_SYM, SYM_WND_H, 12) );
//    m_IOWindow.SetColors( (COLORREF)pApp->GetProfileInt(ENTRY_SYM, SYM_WND_TEXT_COL, int(RGB(0, 0, 0))),
//      pApp->GetProfileInt(ENTRY_SYM, SYM_WND_BK_COL, int(RGB(255,255,255))) );

		CDeasm6502View::m_rgbAddress   = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_ADDR_COLOR, (int)RGB(127,127,127)));
		CDeasm6502View::m_rgbCode      = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_CODE_COLOR, (int)RGB(191,191,191)));
		//    CDeasm6502View::m_rgbInstr   = COLORREF(pApp->GetProfileInt(ENTRY_DEASM, DEASM_INSTR_COLOR, (int)RGB(0,0,0)));
		CDeasm6502View::m_bDrawCode    = pApp->GetProfileInt(ENTRY_DEASM, DEASM_SHOW_CODE, 1);

		CMarks::m_rgbPointer           = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_PTR, (int)RGB(255,255,0)));
		CMarks::m_rgbBreakpoint        = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_BRKP, (int)RGB(0,0,160)));
		CMarks::m_rgbError             = COLORREF(pApp->GetProfileInt(ENTRY_GEN, GEN_ERR, (int)RGB(255,0,0)));
		theApp.m_global.SetProcType( (bool) pApp->GetProfileInt(ENTRY_GEN, GEN_PROC, 1) );
		CSym6502::bus_width            = pApp->GetProfileInt(ENTRY_GEN, GEN_BUS_WIDTH, 16);
		theApp.m_global.m_bGenerateListing = (bool) pApp->GetProfileInt(ENTRY_ASM, ASM_GEN_LST, false);
		theApp.m_global.m_strListingFile   = pApp->GetProfileString(ENTRY_ASM, ASM_LST_FILE, NULL);
		CAsm6502::generateBRKExtraByte = (bool) pApp->GetProfileInt(ENTRY_ASM, ASM_GEN_BYTE, 1);
		CAsm6502::BRKExtraByte         = (UINT8)pApp->GetProfileInt(ENTRY_ASM, ASM_BRK_BYTE, 0);

		CSrc6502View::m_nTabStep       = pApp->GetProfileInt(ENTRY_EDIT, EDIT_TAB_STEP, 8);
		CSrc6502View::m_bAutoIndent    = pApp->GetProfileInt(ENTRY_EDIT, EDIT_AUTO_INDENT, 1);
		CSrc6502View::m_bAutoSyntax    = pApp->GetProfileInt(ENTRY_EDIT, EDIT_SYNTAX_CHECK, 1);
		CSrc6502View::m_bAutoUppercase = pApp->GetProfileInt(ENTRY_EDIT, EDIT_CAPITALS, 1);
		C6502App::m_bFileNew           = pApp->GetProfileInt(ENTRY_EDIT, EDIT_FILENEW, 0);

		CIdentInfo::m_WndRect.left     = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_X, 200);
		CIdentInfo::m_WndRect.top      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_Y, 200);
		CIdentInfo::m_WndRect.right    = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_W, 400);
		CIdentInfo::m_WndRect.bottom   = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IDENTS_H, 400);
		CIdentInfo::m_WndRect.bottom  += CIdentInfo::m_WndRect.top;
		CIdentInfo::m_WndRect.right   += CIdentInfo::m_WndRect.left;

		m_Memory.m_WndRect.left        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_X, 220);
		m_Memory.m_WndRect.top         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_Y, 220);
		m_Memory.m_WndRect.right       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_W, 400);
		m_Memory.m_WndRect.bottom      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_H, 400);
		m_Memory.m_WndRect.bottom     += m_Memory.m_WndRect.top;
		m_Memory.m_WndRect.right      += m_Memory.m_WndRect.left;

		m_ZeroPage.m_WndRect.left      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_X, 240);
		m_ZeroPage.m_WndRect.top       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_Y, 240);
		m_ZeroPage.m_WndRect.right     = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_W, 400);
		m_ZeroPage.m_WndRect.bottom    = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_H, 400);
		m_ZeroPage.m_WndRect.bottom   += m_ZeroPage.m_WndRect.top;
		m_ZeroPage.m_WndRect.right    += m_ZeroPage.m_WndRect.left;

		m_Stack.m_WndRect.left         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_X, 260);
		m_Stack.m_WndRect.top          = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_Y, 260);
		m_Stack.m_WndRect.right        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_W, 300);
		m_Stack.m_WndRect.bottom       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_H, 400);
		m_Stack.m_WndRect.bottom      += m_Stack.m_WndRect.top;
		m_Stack.m_WndRect.right       += m_Stack.m_WndRect.left;

		m_wndLog.m_WndRect.left        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_X, 280);
		m_wndLog.m_WndRect.top         = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_Y, 280);
		m_wndLog.m_WndRect.right       = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_W, 300);
		m_wndLog.m_WndRect.bottom      = pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_H, 400);
		m_wndLog.m_WndRect.right       += m_wndLog.m_WndRect.left;
		m_wndLog.m_WndRect.bottom      += m_wndLog.m_WndRect.top;

		CIOWindow::m_bHidden           = pApp->GetProfileInt(ENTRY_VIEW, VIEW_IO_HID, 0);
		CRegisterBar::m_bHidden        = pApp->GetProfileInt(ENTRY_VIEW, VIEW_REGS_HID, 0);

		m_Memory.m_bHidden             = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_HID, false) != 0;
		m_ZeroPage.m_bHidden           = pApp->GetProfileInt(ENTRY_VIEW, VIEW_ZMEM_HID, false) != 0;
		m_Stack.m_bHidden              = pApp->GetProfileInt(ENTRY_VIEW, VIEW_STACK_HID, false) != 0;
		m_wndLog.m_bHidden             = !!pApp->GetProfileInt(ENTRY_VIEW, VIEW_LOG_HID, false) != 0;

//    CMemoryInfo::m_bHidden = pApp->GetProfileInt(ENTRY_VIEW, VIEW_MEMO_HID, false) != 0;

		for (int i=0; fonts[i]; i++)	// odczyt info o fontach w programie
		{
			*fonts[i] = LogFont;	// domyœlny font
			LPBYTE ptr= NULL;
			UINT bytes= sizeof *fonts[i];
			pApp->GetProfileBinary(ENTRY_VIEW, idents[i], &ptr, &bytes);
			if (ptr)
			{
				if (bytes == sizeof *fonts[i])
					memcpy(fonts[i], ptr, sizeof *fonts[i]);
				delete [] ptr;
			}
			static const COLORREF defaults[]=	      // domyœlne kolory t³a dla okien
			{ // VIEW_ED_BCOL, VIEW_SYM_BCOL, VIEW_DEASM_BCOL, VIEW_MEMO_BCOL, VIEW_ZERO_BCOL
				RGB(255,255,255), RGB(255,255,224), RGB(255,255,255),
					RGB(240,255,240), RGB(255,240,240), RGB(255,255,255), RGB(240,240,240)
			};
			*text_color[i] = COLORREF(pApp->GetProfileInt(ENTRY_VIEW, tcolors[i], RGB(0,0,0)));
			*bkgnd_color[i] = COLORREF(pApp->GetProfileInt(ENTRY_VIEW, bcolors[i], defaults[i]));
		}

		for (int clr= 0; syntax_colors[clr]; ++clr)
			*color_syntax[clr] = pApp->GetProfileInt(ENTRY_VIEW, syntax_colors[clr], *color_syntax[clr]);

		for (int style= 0; syntax_font[style]; ++style)
			*syntax_font_style[style] = pApp->GetProfileInt(ENTRY_VIEW, syntax_font[style], *syntax_font_style[style]);

    //    pApp->GetProfileInt(ENTRY_ASM,ASM_CASE,1);

	}
	else			// zapis
	{
		pApp->WriteProfileInt(ENTRY_SYM, SYM_FIN, theApp.m_global.GetSymFinish());
		pApp->WriteProfileInt(ENTRY_SYM, SYM_IO_ADDR, CSym6502::io_addr);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_IO_ENABLED, CSym6502::io_enabled);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM, CSym6502::s_bWriteProtectArea);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM_FROM, CSym6502::s_uProtectFromAddr);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_PROT_MEM_TO, CSym6502::s_uProtectToAddr);
		CPoint pos= m_IOWindow.GetWndPos();
		pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_X, pos.x);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_Y, pos.y);
		int w, h;
		m_IOWindow.GetSize(w, h);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_W, w);
		pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_H, h);
		COLORREF txt, bk;
		m_IOWindow.GetColors(txt, bk);
//    pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_TEXT_COL, (int)txt);
//    pApp->WriteProfileInt(ENTRY_SYM, SYM_WND_BK_COL, (int)bk);
//    pApp->WriteProfileBinary(ENTRY_SYM, SYM_WND_FONT,  LPBYTE(&m_IOWindow.m_LogFont), UINT(sizeof m_IOWindow.m_LogFont));

		pApp->WriteProfileInt(ENTRY_DEASM, DEASM_ADDR_COLOR, (int)CDeasm6502View::m_rgbAddress);
		pApp->WriteProfileInt(ENTRY_DEASM, DEASM_CODE_COLOR, (int)CDeasm6502View::m_rgbCode);
		//    pApp->WriteProfileInt(ENTRY_DEASM, DEASM_INSTR_COLOR, (int)CDeasm6502View::m_rgbInstr);
		pApp->WriteProfileInt(ENTRY_DEASM, DEASM_SHOW_CODE, (int)CDeasm6502View::m_bDrawCode);

		pApp->WriteProfileInt(ENTRY_GEN, GEN_PROC, (int)theApp.m_global.GetProcType());
		pApp->WriteProfileInt(ENTRY_GEN, GEN_BUS_WIDTH, CSym6502::bus_width);
		pApp->WriteProfileInt(ENTRY_GEN, GEN_PTR, (int)CMarks::m_rgbPointer);
		pApp->WriteProfileInt(ENTRY_GEN, GEN_BRKP, (int)CMarks::m_rgbBreakpoint);
		pApp->WriteProfileInt(ENTRY_GEN, GEN_ERR, (int)CMarks::m_rgbError);

//    pApp->WriteProfileBinary(ENTRY_EDIT, EDIT_FONT,  LPBYTE(&CSrc6502View::m_LogFont), UINT(sizeof CSrc6502View::m_LogFont));
		pApp->WriteProfileInt(ENTRY_EDIT, EDIT_TAB_STEP, CSrc6502View::m_nTabStep);
		pApp->WriteProfileInt(ENTRY_EDIT, EDIT_AUTO_INDENT, CSrc6502View::m_bAutoIndent);
		pApp->WriteProfileInt(ENTRY_EDIT, EDIT_SYNTAX_CHECK, CSrc6502View::m_bAutoSyntax);
		pApp->WriteProfileInt(ENTRY_EDIT, EDIT_CAPITALS, CSrc6502View::m_bAutoUppercase);
		pApp->WriteProfileInt(ENTRY_EDIT, EDIT_FILENEW, C6502App::m_bFileNew);

		pApp->WriteProfileInt(ENTRY_ASM, ASM_CASE, 1);
		pApp->WriteProfileInt(ENTRY_ASM, ASM_GEN_LST, theApp.m_global.m_bGenerateListing);
		pApp->WriteProfileString(ENTRY_ASM, ASM_LST_FILE, theApp.m_global.m_strListingFile);
		pApp->WriteProfileInt(ENTRY_ASM, ASM_GEN_BYTE, CAsm6502::generateBRKExtraByte);
		pApp->WriteProfileInt(ENTRY_ASM, ASM_BRK_BYTE, CAsm6502::BRKExtraByte);

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_X, CIdentInfo::m_WndRect.left);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_Y, CIdentInfo::m_WndRect.top);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_W, CIdentInfo::m_WndRect.Width());
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IDENTS_H, CIdentInfo::m_WndRect.Height());

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_X, m_Memory.m_WndRect.left);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_Y, m_Memory.m_WndRect.top);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_W, m_Memory.m_WndRect.Width());
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_H, m_Memory.m_WndRect.Height());

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_X, m_ZeroPage.m_WndRect.left);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_Y, m_ZeroPage.m_WndRect.top);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_W, m_ZeroPage.m_WndRect.Width());
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_H, m_ZeroPage.m_WndRect.Height());

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_X, m_Stack.m_WndRect.left);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_Y, m_Stack.m_WndRect.top);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_W, m_Stack.m_WndRect.Width());
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_H, m_Stack.m_WndRect.Height());

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_X, m_wndLog.m_WndRect.left);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_Y, m_wndLog.m_WndRect.top);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_W, m_wndLog.m_WndRect.Width());
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_H, m_wndLog.m_WndRect.Height());

		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_IO_HID,    CIOWindow::m_bHidden);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_REGS_HID,  CRegisterBar::m_bHidden);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_MEMO_HID,  m_Memory.m_bHidden);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_ZMEM_HID,  m_ZeroPage.m_bHidden);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_STACK_HID, m_Stack.m_bHidden);
		pApp->WriteProfileInt(ENTRY_VIEW, VIEW_LOG_HID,   m_wndLog.m_bHidden);

		for (int i= 0; fonts[i]; i++)
		{
			pApp->WriteProfileBinary(ENTRY_VIEW, idents[i], LPBYTE(fonts[i]), UINT(sizeof *fonts[i]));
			pApp->WriteProfileInt(ENTRY_VIEW, tcolors[i], int(*text_color[i]));
			pApp->WriteProfileInt(ENTRY_VIEW, bcolors[i], int(*bkgnd_color[i]));
		}

		for (int clr= 0; syntax_colors[clr]; ++clr)
			pApp->WriteProfileInt(ENTRY_VIEW, syntax_colors[clr], int(*color_syntax[clr]));

		for (int style= 0; syntax_font[style]; ++style)
			pApp->WriteProfileInt(ENTRY_VIEW, syntax_font[style], *syntax_font_style[style]);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_SYM_ASSEMBLE, OnAssemble)
	ON_UPDATE_COMMAND_UI(ID_SYM_ASSEMBLE, OnUpdateAssemble)
	ON_UPDATE_COMMAND_UI(ID_SYM_DEBUG, OnUpdateSymDebug)
	ON_COMMAND(ID_SYM_DEBUG, OnSymDebug)
	ON_COMMAND(ID_SYM_STEP_INTO, OnSymStepInto)
	ON_UPDATE_COMMAND_UI(ID_SYM_STEP_INTO, OnUpdateSymStepInto)
	ON_COMMAND(ID_SYM_SKIP_INSTR, OnSymSkipInstr)
	ON_UPDATE_COMMAND_UI(ID_SYM_SKIP_INSTR, OnUpdateSymSkipInstr)
	ON_COMMAND(ID_SYM_BREAKPOINT, OnSymBreakpoint)
	ON_UPDATE_COMMAND_UI(ID_SYM_BREAKPOINT, OnUpdateSymBreakpoint)
	ON_COMMAND(ID_SYM_BREAK, OnSymBreak)
	ON_UPDATE_COMMAND_UI(ID_SYM_BREAK, OnUpdateSymBreak)
	ON_COMMAND(ID_SYM_GO, OnSymGo)
	ON_UPDATE_COMMAND_UI(ID_SYM_GO, OnUpdateSymGo)
	ON_COMMAND(ID_SYM_OPTIONS, OnOptions)
	ON_UPDATE_COMMAND_UI(ID_SYM_OPTIONS, OnUpdateOptions)
	ON_COMMAND(ID_SYM_GO_LINE, OnSymGoToLine)
	ON_UPDATE_COMMAND_UI(ID_SYM_GO_LINE, OnUpdateSymGoToLine)
	ON_COMMAND(ID_SYM_SKIP_TO_LINE, OnSymSkipToLine)
	ON_UPDATE_COMMAND_UI(ID_SYM_SKIP_TO_LINE, OnUpdateSymSkipToLine)
	ON_COMMAND(ID_SYM_GO_RTS, OnSymGoToRts)
	ON_UPDATE_COMMAND_UI(ID_SYM_GO_RTS, OnUpdateSymGoToRts)
	ON_COMMAND(ID_SYM_STEP_OVER, OnSymStepOver)
	ON_UPDATE_COMMAND_UI(ID_SYM_STEP_OVER, OnUpdateSymStepOver)
	ON_COMMAND(ID_SYM_EDIT_BREAKPOINT, OnSymEditBreakpoint)
	ON_UPDATE_COMMAND_UI(ID_SYM_EDIT_BREAKPOINT, OnUpdateSymEditBreakpoint)
	ON_COMMAND(ID_SYM_RESTART, OnSymRestart)
	ON_UPDATE_COMMAND_UI(ID_SYM_RESTART, OnUpdateSymRestart)
	ON_COMMAND(ID_SYM_ANIMATE, OnSymAnimate)
	ON_UPDATE_COMMAND_UI(ID_SYM_ANIMATE, OnUpdateSymAnimate)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REGISTERBAR, OnUpdateIdViewRegisterbar)
	ON_COMMAND(ID_FILE_SAVE_CODE, OnFileSaveCode)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_CODE, OnUpdateFileSaveCode)
	ON_COMMAND(ID_VIEW_DEASM, OnViewDeasm)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DEASM, OnUpdateViewDeasm)
	ON_COMMAND(ID_VIEW_IDENT, OnViewIdents)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IDENT, OnUpdateViewIdents)
	ON_COMMAND(ID_VIEW_MEMORY, OnViewMemory)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MEMORY, OnUpdateViewMemory)
	ON_COMMAND(ID_EDITOR_OPT, OnEditorOpt)
	ON_UPDATE_COMMAND_UI(ID_EDITOR_OPT, OnUpdateEditorOpt)
	ON_COMMAND(ID_VIEW_IO_WINDOW, OnViewIOWindow)
	ON_UPDATE_COMMAND_UI(ID_VIEW_IO_WINDOW, OnUpdateViewIOWindow)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_OPEN_CODE, OnFileLoadCode)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN_CODE, OnUpdateFileLoadCode)
	ON_COMMAND(ID_DEASM_OPTIONS, OnDeasmOptions)
	ON_UPDATE_COMMAND_UI(ID_DEASM_OPTIONS, OnUpdateDeasmOptions)
	ON_COMMAND(ID_VIEW_REGISTERBAR, OnViewRegisterWnd)
	ON_WM_SYSCOLORCHANGE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZEROPAGEBAR, OnUpdateViewZeropage)
	ON_COMMAND(ID_VIEW_ZEROPAGEBAR, OnViewZeropage)
	ON_UPDATE_COMMAND_UI(ID_MEMORY_OPTIONS, OnUpdateMemoryOptions)
	ON_COMMAND(ID_MEMORY_OPTIONS, OnMemoryOptions)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_STACK, OnViewStack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STACK, OnUpdateViewStack)
	ON_COMMAND(ID_SYM_GEN_IRQ, OnSymGenIRQ)
	ON_UPDATE_COMMAND_UI(ID_SYM_GEN_IRQ, OnUpdateSymGenIRG)
	ON_COMMAND(ID_SYM_GEN_NMI, OnSymGenNMI)
	ON_UPDATE_COMMAND_UI(ID_SYM_GEN_NMI, OnUpdateSymGenNMI)
	ON_COMMAND(ID_SYM_GEN_RST, OnSymGenReset)
	ON_UPDATE_COMMAND_UI(ID_SYM_GEN_RST, OnUpdateSymGenReset)
	ON_COMMAND(ID_SYM_GEN_INT, OnSymGenIntDlg)
	ON_UPDATE_COMMAND_UI(ID_SYM_GEN_INT, OnUpdateSymGenIntDlg)
	ON_COMMAND(ID_VIEW_LOG, OnViewLog)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LOG, OnUpdateViewLog)
	ON_COMMAND(ID_HELP_DYNAMIC, OnHelpDynamic)
	ON_UPDATE_COMMAND_UI(ID_HELP_DYNAMIC, OnUpdateHelpDynamic)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
//  ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
	ON_MESSAGE(WM_USER+9998, OnUpdateState)
	ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebugger)
	ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebugger)
	ON_MESSAGE(CBroadcast::WM_USER_PROG_MEM_CHANGED, OnChangeCode)
END_MESSAGE_MAP()


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	0,
	0,
#ifdef USE_CRYSTAL_EDIT
	ID_EDIT_INDICATOR_POSITION,
	ID_INDICATOR_OVR,
#endif
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_nLastPage = 0;	// ostatnio wywo³ana strona (zak³adka) w pude³ku opcji

	int i= 0;
	m_hWindows[i++] = &m_hWnd;
	m_hWindows[i++] = &m_wndRegisterBar.m_hWnd;
	m_hWindows[i++] = &m_IOWindow.m_hWnd;
	m_hWindows[i++] = &m_Memory.m_hWnd;
	m_hWindows[i++] = &m_ZeroPage.m_hWnd;
	m_hWindows[i++] = &m_Idents.m_hWnd;
	m_hWindows[i++] = &m_Stack.m_hWnd;
	m_hWindows[i++] = &m_wndLog.m_hWnd;
	m_hWindows[i++] = NULL;

	m_uTimer = 0;
}

CMainFrame::~CMainFrame()
{
//  if (m_Idents)
//    delete m_Idents;
}

//-----------------------------------------------------------------------------

const DWORD CMainFrame::dwDockBarMapEx[4][2] =
{
	{ AFX_IDW_DOCKBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_DOCKBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_DOCKBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_DOCKBAR_RIGHT,    CBRS_RIGHT  },
};

void CMainFrame::EnableDockingEx(DWORD dwDockStyle)
{
	// must be CBRS_ALIGN_XXX or CBRS_FLOAT_MULTI only
	ASSERT((dwDockStyle & ~(CBRS_ALIGN_ANY|CBRS_FLOAT_MULTI)) == 0);

	m_pFloatingFrameClass = RUNTIME_CLASS(CMiniDockFrameWnd);
	for (int i = 0; i < 4; i++)
	{
		if (dwDockBarMapEx[i][1] & dwDockStyle & CBRS_ALIGN_ANY)
		{
			CDockBar* pDock = (CDockBar*)GetControlBar(dwDockBarMapEx[i][0]);
			if (pDock == NULL)
			{
				pDock = new CDockBarEx;
				if (!pDock->Create(this,
					WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CHILD|WS_VISIBLE |
						dwDockBarMapEx[i][1], dwDockBarMapEx[i][0]))
				{
					AfxThrowResourceException();
				}
			}
		}
	}
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	CString strName;
	strName.LoadString(IDS_TOOLBAR);
	m_wndToolBar.SetWindowText(strName);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	{		// dodanie pola do wyœwietlania aktualnego wiersza i kolumny
		UINT uID;
		UINT uStyle;
		int nWidth;
		CRect rectArea(0,0,0,0);

		if (m_strFormat.LoadString(IDS_ROW_COLUMN))
		{
			CString str;
			str.Format(m_strFormat,99999,999);
			m_wndStatusBar.GetPaneInfo(1, uID, uStyle, nWidth);
#ifdef USE_CRYSTAL_EDIT
			m_wndStatusBar.SetPaneInfo(1, uID, SBPS_NOBORDERS | SBPS_DISABLED, 1);
#else
			CClientDC dc(&m_wndStatusBar);
			dc.SelectObject(m_wndStatusBar.GetFont());
			dc.DrawText(str, -1, rectArea, DT_SINGLELINE | DT_CALCRECT);
			m_wndStatusBar.SetPaneInfo(1, uID, uStyle, rectArea.Width());
#endif
		}

		m_wndStatusBar.GetPaneInfo(2, uID, uStyle, nWidth);
		m_wndStatusBar.SetPaneInfo(2, uID, uStyle, 16);	// szerokoœæ obrazka

		m_bmpCode.LoadMappedBitmap(IDB_CODE);
		m_bmpDebug.LoadMappedBitmap(IDB_DEBUG);

		m_pfnOldProc = (WNDPROC)::SetWindowLong(m_wndStatusBar.m_hWnd,GWL_WNDPROC,(LONG)StatusBarWndProc);
	}

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	EnableDockingEx(CBRS_ALIGN_ANY); //CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	// TODO: Change the value of ID_VIEW_REGISTERBAR to an appropriate value:
	//   1. Open the file resource.h
	//   2. Find the definition for the symbol ID_VIEW_REGISTERBAR
	//   3. Change the value of the symbol. Use a value in the range
	//      0xE804 to 0xE81A that is not already used by another symbol

	// CG: The following block was inserted by the 'Dialog Bar' component
	{
		// Initialize dialog bar m_wndRegisterBar
		if (!m_wndRegisterBar.Create(this, 
			CBRS_RIGHT | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_HIDE_INPLACE | CBRS_SIZE_FIXED, ID_VIEW_REGISTERBAR))
		{
			TRACE0("Failed to create dialog bar m_wndRegisterBar\n");
			return -1;		// fail to create
		}
		// m_wndRegisterBar.SetBarStyle(CBRS_BORDER_TOP|CBRS_BORDER_BOTTOM|CBRS_BORDER_LEFT|CBRS_BORDER_RIGHT);
		m_wndRegisterBar.EnableDocking(0); //CBRS_ALIGN_ANY);
		DockControlBar(&m_wndRegisterBar);
		FloatControlBar(&m_wndRegisterBar, CPoint(100, 100));
//		EnableDocking(0); //CBRS_ALIGN_ANY);
		//    DockControlBar(&m_wndRegisterBar);
		//    FloatControlBar(&m_wndRegisterBar,CPoint(10,10));
	}

	if (!m_wndHelpBar.Create(this, AFX_IDW_CONTROLBAR_LAST))
	{
		TRACE0("Failed to create help bar\n");
		return -1;		// fail to create
	}
	m_wndHelpBar.EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);
	m_wndHelpBar.SetWindowText("Dynamic Help");
	DockControlBar(&m_wndHelpBar);


//	LoadBarState(REG_ENTRY_LAYOUT);
	bool bEmptyInfo= true;
	{
		CDockState state;
		state.LoadState(REG_ENTRY_LAYOUT);

		for (int i=0; i < state.m_arrBarInfo.GetSize(); i++)
		{
			CControlBarInfo* pInfo= (CControlBarInfo*)(state.m_arrBarInfo[i]);
			if (pInfo->m_nBarID == ID_VIEW_REGISTERBAR)
				pInfo->m_bVisible = false;	// registerBar zawsze ukryty po starcie aplikacji
		}
		bEmptyInfo = state.m_arrBarInfo.GetSize() == 0;
		SetDockState(state);
	}
	if (bEmptyInfo)		// pierwsze uruchomienie aplikacji w systemie?
	{
		CPoint point(32,32);	// pocz¹tkowe po³o¿enie
		CMiniDockFrameWnd* pDockFrame = CreateFloatingFrame(CBRS_ALIGN_TOP);
		ASSERT(pDockFrame != NULL);
		pDockFrame->SetWindowPos(NULL, point.x, point.y, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		if (pDockFrame->m_hWndOwner == NULL)
			pDockFrame->m_hWndOwner = m_wndRegisterBar.m_hWnd;

		CDockBar* pDockBar = (CDockBar*)pDockFrame->GetDlgItem(AFX_IDW_DOCKBAR_FLOAT);
		ASSERT(pDockBar != NULL);
		ASSERT_KINDOF(CDockBar, pDockBar);

		ASSERT(m_wndRegisterBar.m_pDockSite == this);
		// if this assertion occurred it is because the parent of pBar was not
		//  initially this CFrameWnd when pBar's OnCreate was called
		// (this control bar should have been created with a different
		//  parent initially)

		pDockBar->DockControlBar(&m_wndRegisterBar);
		pDockFrame->RecalcLayout(true);
		pDockFrame->ShowWindow(SW_HIDE);
		//    pDockFrame->UpdateWindow();
		m_wndRegisterBar.ModifyStyle(WS_VISIBLE,0);
	}

	for (int i =0; cfonts[i]; i++)	// utworzenie fontów
	{
		//    cfonts[i]->DeleteObject();
		cfonts[i]->CreateFontIndirect(fonts[i]);
	}

	m_Memory.Create(theApp.m_global.GetMem(), theApp.m_global.GetStartAddr(), CMemoryInfo::VIEW_MEMORY);
	m_ZeroPage.Create(theApp.m_global.GetMem(), 0x00, CMemoryInfo::VIEW_ZEROPAGE);
	m_Stack.Create(theApp.m_global.GetMem(), 0x00, CMemoryInfo::VIEW_STACK);
	m_wndLog.Create();

	return 0;
}

//-----------------------------------------------------------------------------

LRESULT CALLBACK CMainFrame::StatusBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CWnd *pWnd= FromHandlePermanent(hWnd);
//  ASSERT (pWnd->IsKindOf(RUNTIME_CLASS(CStatusBar)));

	switch (msg)
	{
	case WM_PAINT:
		{
			LRESULT ret= (*CMainFrame::m_pfnOldProc)(hWnd,msg,wParam,lParam);
			if (ret == 0)
			{
				bool bCode;
				if (theApp.m_global.IsDebugger())		// jest dzia³aj¹cy debugger ?
					bCode = false;
				else if (theApp.m_global.IsCodePresent())	// jest kod programu?
					bCode = true;
				else
					return ret;				// ani kodu programu, ani debuggera
				CRect rect;
				(*m_pfnOldProc)(hWnd,SB_GETRECT,2,(LPARAM)(RECT *)rect);	// miejsce na obrazek - wymiary
				int borders[3];
				(*m_pfnOldProc)(hWnd,SB_GETBORDERS,0,(LPARAM)borders);		// gruboœæ obwódki
				rect.DeflateRect(borders[0]+1,borders[1]-1);
				CClientDC dc(pWnd);
				if (dc)
				{
					CDC memDC;
					memDC.CreateCompatibleDC(&dc);
					if (memDC)
					{
						CBitmap *pOldBmp= memDC.SelectObject(bCode ? &m_bmpCode : &m_bmpDebug);
						dc.BitBlt(rect.left+2, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
						memDC.SelectObject(pOldBmp);
					}
				}
			}
			return ret;
		}

	default:
		return (*CMainFrame::m_pfnOldProc)(hWnd,msg,wParam,lParam);
	}
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CMainFrame::OnStartDebugger(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	RECT rect;
	m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
	m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch³y do przerysowania
	return 0;
}


afx_msg LRESULT CMainFrame::OnExitDebugger(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	RECT rect;
	m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
	m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch³y do przerysowania
	return 0;
}


afx_msg LRESULT CMainFrame::OnChangeCode(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	RECT rect;
	m_wndStatusBar.SendMessage(SB_GETRECT,2,(LPARAM)&rect);	// miejsce na obrazek - wymiary
	m_wndStatusBar.InvalidateRect(&rect);		// obrazek pch³y do przerysowania
	return 0;
}

//-----------------------------------------------------------------------------

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	CWinApp *pApp = AfxGetApp();

	CRect desk;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, desk, 0);

	cs.x = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_POSX, 50);
	cs.y = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_POSY, 50);
	cs.cx = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_SIZX, desk.Width() - 100);
	cs.cy = pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_SIZY, desk.Height() - 100);

	// prevent from appearing outside desk area
	if (cs.x < desk.left)
		cs.x = desk.left;
	if (cs.y < desk.top)
		cs.y = desk.top;
	if (cs.x + cs.cx > desk.right)
		cs.x = desk.right - min(cs.cx, desk.Width());
	if (cs.y + cs.cy > desk.bottom)
		cs.y = desk.bottom - min(cs.cy, desk.Height());

	if (pApp->GetProfileInt(REG_ENTRY_MAINFRM, REG_STATE, 0))	// maximize?
		C6502App::m_bMaximize = true;
	//  C6502App::m_bFileNew = pApp->GetProfileInt(REG_ENTRY_MAINFRM,REG_FILENEW,1);	// new file
	ConfigSettings(true);		// odczyt ustawieñ

	return CMDIFrameWnd::PreCreateWindow(cs);
}

//-----------------------------------------------------------------------------
/*
HMENU CMainFrame::GetWindowMenuPopup(HMENU hMenuBar)
  // find which popup is the "Window" menu
{
  if (hMenuBar == NULL)
    return NULL;

  ASSERT(::IsMenu(hMenuBar));

  int iItem = ::GetMenuItemCount(hMenuBar);
  while (iItem--)
  {
    HMENU hMenuPop = ::GetSubMenu(hMenuBar, iItem);
    if (hMenuPop != NULL)
    {
      int iItemMax = ::GetMenuItemCount(hMenuPop);
      for (int iItemPop = 0; iItemPop < iItemMax; iItemPop++)
      {
	UINT nID = GetMenuItemID(hMenuPop, iItemPop);
	if (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST)
	  return hMenuPop;
        HMENU hMenuSubPop = ::GetSubMenu(hMenuPop, iItemPop);
        if (hMenuSubPop != NULL)
	{
	  int iItemSubMax = ::GetMenuItemCount(hMenuSubPop);
	  for (int iItemSubPop = 0; iItemSubPop < iItemSubMax; iItemSubPop++)
	  {
	    UINT nID = GetMenuItemID(hMenuSubPop, iItemSubPop);
	    if (nID >= AFX_IDM_WINDOW_FIRST && nID <= AFX_IDM_WINDOW_LAST)
	      return hMenuSubPop;
	  }
	}
      }
    }
  }

  // no default menu found
  TRACE0("Warning: GetWindowMenuPopup failed!\n");
  return NULL;
}
*/

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose()
{
	if (theApp.m_global.IsDebugger() && theApp.m_global.IsProgramRunning())
	{
		if (m_IOWindow.IsWaiting())
		{
			m_IOWindow.ExitModalLoop();
			return;
		}
		if (AfxMessageBox(IDS_MAINFRM_PROG_RUNNING,MB_YESNO) == IDYES)
		{
			theApp.m_global.AbortProg();
			if (m_IOWindow.m_hWnd != 0)
				m_IOWindow.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
			if (m_wndRegisterBar.m_hWnd != 0)
				m_wndRegisterBar.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
			if (m_Idents.m_hWnd != 0)
				m_Idents.SendMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
		}
		else
			return;
	}

	CWinApp* pApp = AfxGetApp();

	SaveBarState(REG_ENTRY_LAYOUT);		   // zapisanie po³o¿enia pasków narzêdzi

	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
	{						   // zapisanie po³o¿enia okna g³ównego
		CRect wnd(wp.rcNormalPosition);
		pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_POSX,wnd.left);
		pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_POSY,wnd.top);
		pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_SIZX,wnd.Width());
		pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_SIZY,wnd.Height());
		pApp->WriteProfileInt(REG_ENTRY_MAINFRM,REG_STATE,wp.showCmd==SW_SHOWMAXIMIZED ? 1 : 0);
	}

	CMDIFrameWnd::OnClose();
}


void CMainFrame::OnDestroy() 
{
	if (m_uTimer)
		KillTimer(m_uTimer);
	ConfigSettings(false);		// zapis ustawieñ
	CMDIFrameWnd::OnDestroy();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnAssemble() 
{
	CSrc6502View *pView= GetCurrentView();
	if (pView==NULL)
		return;

	if (m_IOWindow.IsWaiting())
	{
		m_IOWindow.SetFocus();
		return;
	}

	SendMessageToViews(WM_USER_REMOVE_ERR_MARK);

	if (theApp.m_global.IsDebugger())	// dzia³a debugger?
	{
		if (AfxMessageBox(IDS_STOP_DEBUG,MB_OKCANCEL) != IDOK)
			return;
		ExitDebugMode();			// wyjœcie z trybu debuggera
	}

	if (CDocument* pDocument= pView->GetDocument())
	{
		// before assembly start set current dir to the document directory,
		// so include directive will find included files
		//
		const CString& strPath= pDocument->GetPathName();
		if (!strPath.IsEmpty())
		{
			CString strDir= strPath.Left(strPath.ReverseFind('\\'));
			::SetCurrentDirectory(strDir);
		}
	}

	CDialAsmStat dial(pView);

	dial.DoModal();

//  dial.Create();

//  for (int i=0; i<1000; i++)
//    dial.SetValues(i,1);

}


void CMainFrame::OnUpdateAssemble(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetCurrentDocument() != NULL &&	// jest aktywny dokument?
		!m_IOWindow.IsWaiting());
}


CSrc6502Doc *CMainFrame::GetCurrentDocument()
{
	CMDIChildWnd *pWnd= MDIGetActive();
	if (pWnd==NULL)
		return NULL;

	CDocument *pDoc= pWnd->GetActiveDocument();	// aktywny dokument
	if (pDoc==NULL || !pDoc->IsKindOf( RUNTIME_CLASS(CSrc6502Doc) ))
		return NULL;

	return static_cast<CSrc6502Doc*>(pDoc);
}


CSrc6502View *CMainFrame::GetCurrentView()
{
	CMDIChildWnd *pWnd= MDIGetActive();
	if (pWnd==NULL)
		return NULL;

	CView *pView= pWnd->GetActiveView();	// aktywne okno 'view'
	if (pView==NULL || !pView->IsKindOf( RUNTIME_CLASS(CSrc6502View) ))
		return NULL;

	return static_cast<CSrc6502View*>(pView);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateSymDebug(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest zasemblowany program?
	pCmdUI->SetCheck(theApp.m_global.IsDebugger());	// debugger uruchomiony
}

void CMainFrame::OnSymDebug()		// uruchomienie debuggera
{
	if (theApp.m_global.IsDebugger())   // ju¿ uruchomiony?
	{
		OnSymDebugStop();
	}
	else
	{
		if (!theApp.m_global.IsCodePresent())
			return;
		theApp.m_global.StartDebug();
//		m_wndToolBar.OnInitialUpdate();
		DelayedUpdateAll();
		StartIntGenerator();
	}
/*
  if (theApp.m_global.GetSimulator() == NULL)
    return;
  m_wndRegisterBar.Update( theApp.m_global.GetSimulator()->GetContext(), theApp.m_global.GetStatMsg() );

  if (m_IOWindow.m_hWnd != 0)
  {
    m_IOWindow.SendMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
    m_IOWindow.SendMessage(CIOWindow::CMD_CLS);
  }
  if (m_wndRegisterBar.m_hWnd != 0)
    m_wndRegisterBar.PostMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
  if (m_Idents.m_hWnd != 0)
    m_Idents.PostMessage(CBroadcast::WM_USER_START_DEBUGGER,0,0);
*/
}

//-----------------------------------------------------------------------------
/*
void CMainFrame::ClearPositionText()
{
  m_wndStatusBar.SetPaneText(1,NULL);
}
*/

void CMainFrame::SetPositionText(int row, int col)
{
  CString strPosition;
  strPosition.Format(m_strFormat,row,col);
  m_wndStatusBar.SetPaneText(1,strPosition);
  m_wndStatusBar.UpdateWindow();
}

/*
void CMainFrame::SetRowColumn(CEdit &edit)
{
  int idx= edit.LineIndex();
  if (idx == -1)
    return;
  int row= edit.LineFromChar(idx);

  SetPositionText(row+1,0);
}
*/

//-----------------------------------------------------------------------------

void CMainFrame::OnSymBreakpoint() 
{
	CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
//  ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

	if (!theApp.m_global.IsDebugger() || pView==NULL)
		return;

	if (pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)))
	{
		int line= pView->GetCurrLineNo();	// bie¿¹cy wiersz
		// ustawienie miejsca przerwania w kodzie wynikowym odpowiadaj¹cym bie¿¹cemu wierszowi
		CAsm::Breakpoint bp= theApp.m_global.SetBreakpoint( line, pView->GetDocument()->GetPathName() );
		if (bp != CAsm::BPT_NO_CODE)
		{
			if (bp != CAsm::BPT_NONE)
				AddBreakpoint(pView, line, bp);
			else
				RemoveBreakpoint(pView, line);
		}
		else
			AfxMessageBox(IDS_SRC_NO_CODE);
	}
	else if (pView->IsKindOf(RUNTIME_CLASS(CDeasm6502View)))
	{
		;
	}
	else
	{
		ASSERT(false);	// aktywne okno nierozpoznane
		return;
	}
}

void CMainFrame::OnUpdateSymBreakpoint(CCmdUI* pCmdUI) 
{  
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    GetActiveFrame()->GetActiveView() );		// i aktywny dokument?
}


void CMainFrame::AddBreakpoint(CSrc6502View* pView, int nLine, CAsm::Breakpoint bp)
{
	if (pView == 0)
		return;

	CDocument* pDoc= pView->GetDocument();
	POSITION pos= pDoc->GetFirstViewPosition();
	while (pos != NULL)
		if (CSrc6502View* pSrcView= dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
			pSrcView->AddBreakpoint(nLine, bp);
}

void CMainFrame::RemoveBreakpoint(CSrc6502View* pView, int nLine)
{
	if (pView == 0)
		return;

	CDocument* pDoc= pView->GetDocument();
	POSITION pos= pDoc->GetFirstViewPosition();
	while (pos != NULL)
		if (CSrc6502View* pSrcView= dynamic_cast<CSrc6502View*>(pDoc->GetNextView(pos)))
			pSrcView->RemoveBreakpoint(nLine);
}


void CMainFrame::OnSymEditBreakpoint()
{
	CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
	ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

	if (!theApp.m_global.IsDebugger() || pView==NULL)
		return;
	int line= pView->GetCurrLineNo();	// bie¿¹cy wiersz

	// pobranie parametrów przerwania w kodzie wynikowym odpowiadaj¹cym bie¿¹cemu wierszowi
	CAsm::Breakpoint bp= theApp.m_global.GetBreakpoint( line, pView->GetDocument()->GetPathName() );
	if (bp != CAsm::BPT_NO_CODE)
	{
		CDialEditBreakpoint edit_bp(bp,this);
		if (edit_bp.DoModal() != IDOK)
			return;
		bp = edit_bp.GetBreakpoint();
		if ((bp & CAsm::BPT_MASK) == CAsm::BPT_NONE)
		{
			theApp.m_global.ClrBreakpoint( line, pView->GetDocument()->GetPathName() );
			pView->RemoveBreakpoint(line);
		}
		else
		{
			theApp.m_global.ModifyBreakpoint( line, pView->GetDocument()->GetPathName(), bp );
			AddBreakpoint(pView, line,bp);
		}
	}
	else
		AfxMessageBox(IDS_SRC_NO_CODE);
}

void CMainFrame::OnUpdateSymEditBreakpoint(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
		GetActiveFrame()->GetActiveView() );		// i aktywny dokument?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymBreak()
{
	if (!theApp.m_global.IsProgramRunning())
		return;	
	theApp.m_global.GetSimulator()->Break();		// przerwanie dzia³aj¹cego programu
	DelayedUpdateAll();

	AfxGetMainWnd()->SetFocus();		// restore focus (so it's not in i/o window)
}

void CMainFrame::OnUpdateSymBreak(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(theApp.m_global.IsProgramRunning());	// jest dzia³aj¹cy program i debugger?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymSkipInstr()	// ominiêcie bie¿¹cej instrukcji
{
	if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
		theApp.m_global.IsProgramFinished() )
		return;
	theApp.m_global.GetSimulator()->SkipInstr();
}

void CMainFrame::OnUpdateSymSkipInstr(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
		!theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
		!theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGo()		// uruchomienie programu
{
	if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
		theApp.m_global.IsProgramFinished() )
		return;
	theApp.m_global.GetSimulator()->Run();
}

void CMainFrame::OnUpdateSymGo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
		!theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
		!theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGoToLine()	// uruchomienie do wiersza
{
	CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
	ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

	if (pView==NULL || !theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
		theApp.m_global.IsProgramFinished() )
		return;

	int line= pView->GetCurrLineNo();
	CAsm::DbgFlag flg= theApp.m_global.GetLineDebugFlags(line,pView->GetDocument()->GetPathName());
	if (flg == CAsm::DBG_EMPTY || (flg & CAsm::DBG_MACRO))	// wiersz bez kodu wynikowego?
	{
		AfxMessageBox(IDS_SRC_NO_CODE2);
		return;
	}
	else if (flg & CAsm::DBG_DATA)			// wiersz z danymi zamiast rozkazów?
	{
		if (AfxMessageBox(IDS_SRC_DATA,MB_YESNO) != IDYES)
			return;
	}

	theApp.m_global.SetTempExecBreakpoint(line,pView->GetDocument()->GetPathName());
	theApp.m_global.GetSimulator()->Run();	// uruchomienie po ustawieniu tymczasowego przerwania
}

void CMainFrame::OnUpdateSymGoToLine(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() &&		// niezakoñczony program
    GetActiveFrame()->GetActiveView() &&		// i aktywny dokument?
    GetActiveFrame()->GetActiveView()->IsKindOf(RUNTIME_CLASS(CSrc6502View)) );
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymSkipToLine()	// przestawienie PC na bie¿¹cy wiersz
{
  CSrc6502View *pView= (CSrc6502View *)( GetActiveFrame()->GetActiveView() );
  ASSERT(pView==NULL || pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)));

  if (pView==NULL || !theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
    theApp.m_global.IsProgramFinished() )
    return;
  int line= pView->GetCurrLineNo();
  CAsm::DbgFlag flg= theApp.m_global.GetLineDebugFlags(line,pView->GetDocument()->GetPathName());
  if (flg == CAsm::DBG_EMPTY || (flg & CAsm::DBG_MACRO))	// wiersz bez kodu wynikowego?
  {
    AfxMessageBox(IDS_SRC_NO_CODE3);
    return;
  }
  else if (flg & CAsm::DBG_DATA)			// wiersz z danymi zamiast rozkazów?
  {
    if (AfxMessageBox(IDS_SRC_DATA,MB_YESNO) != IDYES)
      return;
  }
  UINT16 addr= theApp.m_global.GetLineCodeAddr(line,pView->GetDocument()->GetPathName());
  theApp.m_global.GetSimulator()->SkipToAddr(addr);
}

void CMainFrame::OnUpdateSymSkipToLine(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() &&		// niezakoñczony program
    GetActiveFrame()->GetActiveView() &&		// i aktywny dokument?
    GetActiveFrame()->GetActiveView()->IsKindOf(RUNTIME_CLASS(CSrc6502View)) );
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymGoToRts()		// uruchomienie do powrotu z podprogramu
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
    theApp.m_global.IsProgramFinished() )
    return;
  theApp.m_global.GetSimulator()->RunTillRet();
}

void CMainFrame::OnUpdateSymGoToRts(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymStepInto()	// wykonanie bie¿¹cej instrukcji
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
    theApp.m_global.IsProgramFinished() )
    return;
  theApp.m_global.GetSimulator()->StepInto();
  DelayedUpdateAll();
}

void CMainFrame::OnUpdateSymStepInto(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymStepOver() 
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
    theApp.m_global.IsProgramFinished() )
    return;
  theApp.m_global.GetSimulator()->StepOver();
}

void CMainFrame::OnUpdateSymStepOver(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymRestart() 
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() )
    return;
  theApp.m_global.RestartProgram();
  DelayedUpdateAll();
}

void CMainFrame::OnUpdateSymRestart(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() );		// oraz zatrzymany program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymAnimate() 
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning() ||
    theApp.m_global.IsProgramFinished() )
    return;
  theApp.m_global.GetSimulator()->Animate();
}

void CMainFrame::OnUpdateSymAnimate(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
    !theApp.m_global.IsProgramRunning() &&		// oraz zatrzymany
    !theApp.m_global.IsProgramFinished() );		// i niezakoñczony program?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSymDebugStop()
{
  if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramRunning())
  {
    MessageBeep(-2);
    return;
  }
  theApp.m_global.ExitDebugger();
  StopIntGenerator();
/*
  if (m_IOWindow.m_hWnd != 0)
    m_IOWindow.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
  if (m_wndRegisterBar.m_hWnd != 0)
    m_wndRegisterBar.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
  if (m_Idents.m_hWnd != 0)
    m_Idents.PostMessage(CBroadcast::WM_USER_EXIT_DEBUGGER,0,0);
*/
}

//void CMainFrame::OnUpdateSymDebugStop(CCmdUI* pCmdUI)
//{
//  pCmdUI->Enable( theApp.m_global.IsDebugger() &&	// jest dzia³aj¹cy debugger
//    !theApp.m_global.IsProgramRunning() );		// oraz zatrzymany program?
//}

//=============================================================================

afx_msg LRESULT CMainFrame::OnUpdateState(WPARAM wParam, LPARAM lParam)
{
  if (theApp.m_global.IsDebugger())
  {
    theApp.m_global.GetSimulator()->Update((CAsm::SymStat)wParam, lParam != 0);
    DelayedUpdateAll();
  }
  return 0;
}

//---------------------------- Opcje programu ---------------------------------

int CMainFrame::Options(int page)
{
	COptions dial(this,page);

	dial.m_EditPage.m_bAutoIndent		= CSrc6502View::m_bAutoIndent;
	dial.m_EditPage.m_nTabStep			= CSrc6502View::m_nTabStep;
	dial.m_EditPage.m_bAutoSyntax		= CSrc6502View::m_bAutoSyntax;
	dial.m_EditPage.m_bAutoUppercase	= CSrc6502View::m_bAutoUppercase;
	dial.m_EditPage.m_bFileNew			= C6502App::m_bFileNew;

	dial.m_SymPage.m_nIOAddress			= CSym6502::io_addr;
	dial.m_SymPage.m_bIOEnable			= CSym6502::io_enabled;
	dial.m_SymPage.m_nFinish			= theApp.m_global.GetSymFinish();
	m_IOWindow.GetSize(dial.m_SymPage.m_nWndWidth,dial.m_SymPage.m_nWndHeight);
	dial.m_SymPage.m_bProtectMemory		= CSym6502::s_bWriteProtectArea;
	dial.m_SymPage.m_nProtFromAddr		= CSym6502::s_uProtectFromAddr;
	dial.m_SymPage.m_nProtToAddr		= CSym6502::s_uProtectToAddr;
	//  m_IOWindow.GetColors(dial.m_SymPage.m_rgbTextColor,dial.m_SymPage.m_rgbBackgndColor);

	dial.m_DeasmPage.m_rgbAddress = CDeasm6502View::m_rgbAddress;
	dial.m_DeasmPage.m_rgbCode = CDeasm6502View::m_rgbCode;
	//  dial.m_DeasmPage.m_rgbInstr = CDeasm6502View::m_rgbInstr;
	dial.m_DeasmPage.m_ShowCode = CDeasm6502View::m_bDrawCode;

	dial.m_MarksPage.m_nProc6502 = !theApp.m_global.GetProcType();
	dial.m_MarksPage.m_uBusWidth = CSym6502::bus_width;
	dial.m_MarksPage.m_rgbPointer = CMarks::m_rgbPointer;
	dial.m_MarksPage.m_rgbBreakpoint = CMarks::m_rgbBreakpoint;
	dial.m_MarksPage.m_rgbError = CMarks::m_rgbError;

	dial.m_AsmPage.m_nCaseSensitive = CAsm6502::case_insensitive;
	dial.m_AsmPage.m_nAsmInstrWithDot = 0;
	dial.m_AsmPage.m_bGenerateListing = theApp.m_global.m_bGenerateListing;
	dial.m_AsmPage.m_strListingFile = theApp.m_global.m_strListingFile;
	dial.m_AsmPage.m_bGenerateBRKExtraByte = CAsm6502::generateBRKExtraByte;
	dial.m_AsmPage.m_uBrkExtraByte = CAsm6502::BRKExtraByte;

	int i;
	for (i = 0; text_color[i]; i++)	// odczyt kolorów
	{
		dial.m_ViewPage.m_Text[i].text = *text_color[i];
		dial.m_ViewPage.m_Text[i].bkgnd = *bkgnd_color[i];
	}


	if (dial.DoModal() != IDOK)
		return dial.GetLastActivePage();


	C6502App::m_bFileNew = dial.m_EditPage.m_bFileNew;
//	CSrc6502View::m_bAutoIndent = dial.m_EditPage.m_bAutoIndent;
	CSrc6502View::m_bAutoSyntax = dial.m_EditPage.m_bAutoSyntax;
	CSrc6502View::m_bAutoUppercase = dial.m_EditPage.m_bAutoUppercase;
	if (dial.m_EditPage.m_bColorChanged)
	{
		for (int nColor= 0; nColor <= 5; ++nColor)
			CSrc6502View::m_vrgbColorSyntax[nColor] = *dial.m_EditPage.GetColorElement(nColor);
		for (int nStyle= 0; nStyle <= 4; ++nStyle)
			CSrc6502View::m_vbyFontStyle[nStyle] = *dial.m_EditPage.GetFontStyle(nStyle);
	}

	CSym6502::io_addr    = dial.m_SymPage.m_nIOAddress;
	CSym6502::io_enabled = dial.m_SymPage.m_bIOEnable;
	theApp.m_global.SetSymFinish((CAsm::Finish)dial.m_SymPage.m_nFinish);
	m_IOWindow.SetSize(dial.m_SymPage.m_nWndWidth, dial.m_SymPage.m_nWndHeight, -1);
	CSym6502::s_bWriteProtectArea	= !!dial.m_SymPage.m_bProtectMemory;
	CSym6502::s_uProtectFromAddr	= dial.m_SymPage.m_nProtFromAddr;
	CSym6502::s_uProtectToAddr		= dial.m_SymPage.m_nProtToAddr;
	if (CSym6502::s_uProtectFromAddr > CSym6502::s_uProtectToAddr)
		std::swap(CSym6502::s_uProtectToAddr, CSym6502::s_uProtectFromAddr);
	//  m_IOWindow.SetColors(dial.m_SymPage.m_rgbTextColor,dial.m_SymPage.m_rgbBackgndColor);

	CDeasm6502View::m_rgbAddress = dial.m_DeasmPage.m_rgbAddress;
	CDeasm6502View::m_rgbCode    = dial.m_DeasmPage.m_rgbCode;
	//  CDeasm6502View::m_rgbInstr = dial.m_DeasmPage.m_rgbInstr;
	CDeasm6502View::m_bDrawCode  = dial.m_DeasmPage.m_ShowCode;

	theApp.m_global.SetProcType(!dial.m_MarksPage.m_nProc6502);
	CSym6502::bus_width     = dial.m_MarksPage.m_uBusWidth;
	CMarks::m_rgbPointer    = dial.m_MarksPage.m_rgbPointer;
	CMarks::m_rgbBreakpoint = dial.m_MarksPage.m_rgbBreakpoint;
	CMarks::m_rgbError      = dial.m_MarksPage.m_rgbError;

	theApp.m_global.m_bGenerateListing = dial.m_AsmPage.m_bGenerateListing;
	theApp.m_global.m_strListingFile   = dial.m_AsmPage.m_strListingFile;
	CAsm6502::generateBRKExtraByte     = dial.m_AsmPage.m_bGenerateBRKExtraByte;
	CAsm6502::BRKExtraByte             = dial.m_AsmPage.m_uBrkExtraByte;
	CAsm6502::case_insensitive         = dial.m_AsmPage.m_nCaseSensitive;

	if (dial.m_EditPage.m_nTabStep != CSrc6502View::m_nTabStep ||
		dial.m_EditPage.m_bColorChanged ||
		!!dial.m_EditPage.m_bAutoIndent != CSrc6502View::m_bAutoIndent)
	{
		CSrc6502View::m_nTabStep = dial.m_EditPage.m_nTabStep;
		CSrc6502View::m_bAutoIndent = dial.m_EditPage.m_bAutoIndent;
		RedrawAllViews();
	}
/*
  if (dial.m_EditPage.m_bFontChanged || dial.m_EditPage.m_nTabStep != CSrc6502View::m_nTabStep ||
    dial.m_DeasmPage.m_bColorChanged || dial.m_MarksPage.m_bColorChanged)
  {
    CSrc6502View::m_nTabStep = dial.m_EditPage.m_nTabStep;
    RedrawAllViews(dial.m_EditPage.m_bFontChanged);
  }
*/
	for (i=0; text_color[i]; i++)	// zapis kolorów
	{
		*text_color[i] = dial.m_ViewPage.m_Text[i].text;
		*bkgnd_color[i] = dial.m_ViewPage.m_Text[i].bkgnd;
		if (dial.m_ViewPage.m_Text[i].changed & 2)	// zmieniony font?
		{
			dial.m_ViewPage.m_Text[i].font.GetLogFont(fonts[i]);
			cfonts[i]->DeleteObject();
			cfonts[i]->CreateFontIndirect(fonts[i]);
		}
		if (dial.m_ViewPage.m_Text[i].changed)	// zmieniony font lub kolory?
			switch (i)
			{
			case 0:		// edytor
				RedrawAllViews(dial.m_ViewPage.m_Text[i].changed & 2);
				break;
			case 1:		// symulator
				if (m_IOWindow.m_hWnd)
					m_IOWindow.Resize();
				break;
			case 2:		// debugger
				RedrawAllViews(dial.m_ViewPage.m_Text[i].changed & 2);
				break;
			case 3:		// pamiêæ 6502
				if (m_Memory.m_hWnd)	// jest ju¿ okno?
					m_Memory.Invalidate();
				break;
			case 4:		// strona zerowa
				if (m_ZeroPage.m_hWnd)
					m_ZeroPage.Invalidate();
				break;
			case 5:		// stack
				if (m_Stack.m_hWnd)
					m_Stack.Invalidate();
				break;
			default:
				ASSERT(false);
			}
	}

	// przerysowanie okien deasemblera
	POSITION posDoc= theApp.m_pDocDeasmTemplate->GetFirstDocPosition();
	while (posDoc != NULL)	// s¹ okna z deasemblera?
	{
		CDocument* pDoc= theApp.m_pDocDeasmTemplate->GetNextDoc(posDoc);
		pDoc->UpdateAllViews(NULL);
	}

	return dial.GetLastActivePage();
}


void CMainFrame::OnOptions() 
{
	m_nLastPage = Options(m_nLastPage);
}


void CMainFrame::OnUpdateOptions(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(true);
}


int CMainFrame::RedrawAllViews(int chgHint)	// 'Invalidate' wszystkich okien
{
	CWinApp *pApp= AfxGetApp();
	POSITION posTempl= pApp->GetFirstDocTemplatePosition();
	while (posTempl != NULL)
	{
		CDocTemplate *pTempl= pApp->GetNextDocTemplate(posTempl);
		POSITION posDoc= pTempl->GetFirstDocPosition();
		while (posDoc != NULL)
		{
			CDocument *pDoc= pTempl->GetNextDoc(posDoc);
			POSITION posView = pDoc->GetFirstViewPosition();
			while (posView != NULL)
			{
				CView* pView = pDoc->GetNextView(posView);
				if (pView->IsKindOf(RUNTIME_CLASS(CSrc6502View)))
				{
					CSrc6502View* pSrcView= static_cast<CSrc6502View*>(pView);
#ifdef USE_CRYSTAL_EDIT
					pSrcView->SetTabSize(CSrc6502View::m_nTabStep);
					pSrcView->SetAutoIndent(CSrc6502View::m_bAutoIndent);
#else
					int nTabStep= CSrc6502View::m_nTabStep * 4;
					pSrcView->GetEditCtrl().SetTabStops(nTabStep);
#endif
					pSrcView->SelectEditFont();
				}
				pView->Invalidate();
				//	pView->UpdateWindow();
			}
		}
		//      GetNextDoc(posDoc)->UpdateAllViews(NULL);
	}
	return 0;
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewRegisterWnd()
{
  ShowControlBar(&m_wndRegisterBar, !m_wndRegisterBar.IsVisible(), false);
//  m_wndToolBar.OnInitialUpdate();
/*
  if (theApp.m_global.IsDebugger())		// jest program?
  {
    if (m_wndRegisterBar.m_hWnd != 0)	// jest ju¿ okno?
      m_wndRegisterBar.ShowWindow((m_wndRegisterBar.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
    else
      m_wndRegisterBar.Create(this);
    m_wndRegisterBar.Update(theApp.m_global.GetSimulator()->GetContext(), theApp.m_global.GetStatMsg());
  }
  else		// nie ma programu
    if (m_wndRegisterBar.m_hWnd != 0)
      m_wndRegisterBar.ShowWindow(SW_HIDE);
*/
}


void CMainFrame::OnUpdateIdViewRegisterbar(CCmdUI* pCmdUI)
{
//  OnUpdateControlBarMenu(pCmdUI);
  pCmdUI->Enable( theApp.m_global.IsDebugger() );	// jest dzia³aj¹cy debugger
  CControlBar* pBar = GetControlBar(pCmdUI->m_nID);
  if (pBar != NULL)
    pCmdUI->SetCheck(/*m_wndRegisterBar.m_hWnd &&*/ (pBar->GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnFileSaveCode()
{
  CString filter;
  filter.LoadString(IDS_SAVE_CODE);
  CSaveCode dlg(_T("Binary Code"),filter,this);
  if (dlg.DoModal() == IDOK)
    dlg.SaveCode();
}

void CMainFrame::OnUpdateFileSaveCode(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest kod programu?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewDeasm() 
{
  if (!theApp.m_global.IsDebugger())	// nie ma dzia³aj¹cego debuggera?
    return;

  theApp.m_global.CreateDeasm();
}

void CMainFrame::OnUpdateViewDeasm(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() );	// jest dzia³aj¹cy debugger?
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewIdents() 
{
	if (theApp.m_global.IsDebugInfoPresent())	// jest zasemblowany program?
	{
		if (m_Idents.m_hWnd != 0)	// jest ju¿ okno?
			m_Idents.ShowWindow((m_Idents.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
		else
		{
			m_Idents.Create(theApp.m_global.GetDebug());
			m_Idents.ShowWindow(SW_SHOWNA);
		}
	}
	else		// nie ma zasemblowanego programu
	{
		if (m_Idents.m_hWnd != 0)
			m_Idents.ShowWindow(SW_HIDE);
	}

//	m_wndToolBar.OnInitialUpdate();
}

void CMainFrame::OnUpdateViewIdents(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(theApp.m_global.IsDebugInfoPresent());	// jest zasemblowany program?
  pCmdUI->SetCheck(/*m_Idents != NULL &&*/ m_Idents.m_hWnd != 0 && (m_Idents.GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewMemory() 
{
	if (theApp.m_global.IsCodePresent())		// jest program?
	{
		if (m_Memory.m_hWnd != 0)	// jest ju¿ okno?
			m_Memory.ShowWindow((m_Memory.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
		else
		{
			m_Memory.Create(theApp.m_global.GetMem(), theApp.m_global.GetStartAddr(), CMemoryInfo::VIEW_MEMORY);
			m_Memory.ShowWindow(SW_SHOWNA);
		}
	}
	else		// nie ma programu
		if (m_Memory.m_hWnd != 0)
			m_Memory.ShowWindow(SW_HIDE);

//	m_wndToolBar.OnInitialUpdate();
}

void CMainFrame::OnUpdateViewMemory(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest program?
  pCmdUI->SetCheck(m_Memory.m_hWnd != 0 && (m_Memory.GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnEditorOpt() 
{
  m_nLastPage = Options(2);		// opcje edytora
}

void CMainFrame::OnUpdateEditorOpt(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(true);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewIOWindow() 
{
	if (!theApp.m_global.IsDebugger())	// nie ma dzia³aj¹cego debuggera?
		return;

	if (!m_IOWindow.m_hWnd)	// nie ma okna?
	{
		m_IOWindow.Create();
		m_IOWindow.ShowWindow(SW_NORMAL);
	}
	else
		m_IOWindow.ShowWindow((m_IOWindow.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);

//	m_wndToolBar.OnInitialUpdate();
}


void CMainFrame::OnUpdateViewIOWindow(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable( theApp.m_global.IsDebugger() );	// jest dzia³aj¹cy debugger?
//  pCmdUI->Enable( true );	// jest dzia³aj¹cy debugger?
  pCmdUI->SetCheck(m_IOWindow.m_hWnd != 0 && (m_IOWindow.GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnFileLoadCode()
{
  CString filter;
  filter.LoadString(IDS_LOAD_CODE);
  CLoadCode dlg(_T(""),filter,this);
  if (dlg.DoModal() == IDOK)
    dlg.LoadCode();
}


void CMainFrame::OnUpdateFileLoadCode(CCmdUI* pCmdUI)
{
  pCmdUI->Enable( true );
}

//-----------------------------------------------------------------------------

void CMainFrame::ExitDebugMode()
{
  if (theApp.m_global.IsProgramRunning())
  {
    if (m_IOWindow.IsWaiting())
      m_IOWindow.ExitModalLoop();
    theApp.m_global.GetSimulator()->AbortProg();	// przerwanie dzia³aj¹cego programu
  }
  OnSymDebugStop();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnDeasmOptions()
{
  m_nLastPage = Options(3);		// opcje deasemblera
}

void CMainFrame::OnUpdateDeasmOptions(CCmdUI* pCmdUI)
{	
  pCmdUI->Enable(true);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnSysColorChange()
{
  CMDIFrameWnd::OnSysColorChange();

  m_bmpCode.DeleteObject();
  m_bmpCode.LoadMappedBitmap(IDB_CODE);
  m_bmpDebug.DeleteObject();
  m_bmpDebug.LoadMappedBitmap(IDB_DEBUG);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateViewZeropage(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest program?
  pCmdUI->SetCheck(m_ZeroPage.m_hWnd != 0 && (m_ZeroPage.GetStyle() & WS_VISIBLE) != 0);
}


void CMainFrame::OnViewZeropage() 
{
	if (theApp.m_global.IsCodePresent())		// jest program?
	{
		if (m_ZeroPage.m_hWnd != 0) 	// jest ju¿ okno?
			m_ZeroPage.ShowWindow((m_ZeroPage.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
		else
		{
			m_ZeroPage.Create(theApp.m_global.GetMem(), 0, CMemoryInfo::VIEW_ZEROPAGE);
			m_ZeroPage.ShowWindow(SW_SHOWNA);
		}
	}
	else		// nie ma programu
	{
		if (m_ZeroPage.m_hWnd != 0)
			m_ZeroPage.ShowWindow(SW_HIDE);
	}

//	m_wndToolBar.OnInitialUpdate();
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewLog()
{
	if (theApp.m_global.IsDebugger())		// is simulator present?
	{
		if (m_wndLog.m_hWnd != 0) 	// jest ju¿ okno?
			m_wndLog.ShowWindow((m_wndLog.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
		else
		{
			m_wndLog.Create();
			m_wndLog.ShowWindow(SW_SHOWNA);
		}
	}
	else		// nie ma programu
		if (m_wndLog.m_hWnd != 0)
			m_wndLog.ShowWindow(SW_HIDE);
}

void CMainFrame::OnUpdateViewLog(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(theApp.m_global.IsDebugger());	// is simulator present?
	pCmdUI->SetCheck(m_wndLog.m_hWnd != 0 && (m_wndLog.GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnViewStack()
{
	if (theApp.m_global.IsCodePresent())		// jest program?
	{
		if (m_Stack.m_hWnd != 0) 	// jest ju¿ okno?
			m_Stack.ShowWindow((m_Stack.GetStyle() & WS_VISIBLE) ? SW_HIDE : SW_NORMAL);
		else
		{
			m_Stack.Create(theApp.m_global.GetMem(), 0, CMemoryInfo::VIEW_STACK);
			m_Stack.ShowWindow(SW_SHOWNA);
		}
	}
	else		// nie ma programu
		if (m_Stack.m_hWnd != 0)
			m_Stack.ShowWindow(SW_HIDE);
}

void CMainFrame::OnUpdateViewStack(CCmdUI* pCmdUI)
{
  pCmdUI->Enable(theApp.m_global.IsCodePresent());	// jest program?
  pCmdUI->SetCheck(m_Stack.m_hWnd != 0 && (m_Stack.GetStyle() & WS_VISIBLE) != 0);
}

//-----------------------------------------------------------------------------

void CMainFrame::OnUpdateMemoryOptions(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(true);
}

void CMainFrame::OnMemoryOptions() 
{
  m_nLastPage = Options(5);		// opcje wygl¹du okna pamiêci
}

//-----------------------------------------------------------------------------

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
  if (m_Stack.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return true;
  if (m_ZeroPage.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return true;
  if (m_Memory.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return true;
  if (m_IOWindow.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
    return true;

  // If the object(s) in the extended command route don't handle
  // the command, then let the base class OnCmdMsg handle it.
  return CMDIFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

//-----------------------------------------------------------------------------

void CMainFrame::UpdateAll()
{
	if (m_IOWindow.m_hWnd)
		m_IOWindow.InvalidateRect(NULL, false);
	if (m_Memory.m_hWnd)
		m_Memory.Invalidate();
	if (m_ZeroPage.m_hWnd)
		m_ZeroPage.Invalidate();
	if (m_Stack.m_hWnd)
	{
		if (CSym6502* pSimulator= theApp.m_global.GetSimulator())
			m_Stack.InvalidateView(pSimulator->GetContext()->s + 0x100);
		else
			m_Stack.Invalidate();
	}
	if (m_wndLog.m_hWnd)
		if (CSym6502* pSimulator= theApp.m_global.GetSimulator())
			m_wndLog.SetText(pSimulator->GetLog());
		else
			m_wndLog.Invalidate();
}

void CMainFrame::DelayedUpdateAll()
{
//	if (m_uTimer == 0)
		m_uTimer = SetTimer(100, 200, NULL);

	if (m_uTimer == 0)
		UpdateAll();
}

void CMainFrame::OnTimer(UINT nIDEvent)
{
	KillTimer(m_uTimer);
	m_uTimer = 0;
	UpdateAll();

//  CMDIFrameWnd::OnTimer(nIDEvent);
}


void CMainFrame::SymGenInterrupt(CSym6502::IntType eInt)
{
	if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramFinished())
		return;
	theApp.m_global.GetSimulator()->Interrupt(eInt);
}

void CMainFrame::UpdateSymGenInterrupt(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(theApp.m_global.IsDebugger() && !theApp.m_global.IsProgramFinished());
}

void CMainFrame::OnSymGenIRQ()
{
	SymGenInterrupt(CSym6502::IRQ);
}

void CMainFrame::OnUpdateSymGenIRG(CCmdUI* pCmdUI)
{
	UpdateSymGenInterrupt(pCmdUI);
}

void CMainFrame::OnSymGenNMI() 
{
	SymGenInterrupt(CSym6502::NMI);
}

void CMainFrame::OnUpdateSymGenNMI(CCmdUI* pCmdUI)
{
	UpdateSymGenInterrupt(pCmdUI);
}

void CMainFrame::OnSymGenReset()
{
	SymGenInterrupt(CSym6502::RST);
}

void CMainFrame::OnUpdateSymGenReset(CCmdUI* pCmdUI)
{
	UpdateSymGenInterrupt(pCmdUI);
}


void CALLBACK EXPORT TimerProc(
	HWND hWnd,		// handle of CWnd that called SetTimer
	UINT nMsg,		// WM_TIMER
	UINT nIDEvent,	// timer identification
	DWORD dwTime)	// system time
{
	if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramFinished())
		return;

	if (!theApp.m_global.IsProgramRunning())
		return;		// if program is not running do not send int request

	switch (nIDEvent)
	{
	case CSym6502::IRQ:
		theApp.m_global.GetSimulator()->Interrupt(CSym6502::IRQ);
		break;

	case CSym6502::NMI:
		theApp.m_global.GetSimulator()->Interrupt(CSym6502::NMI);
		break;
	}
}


// Interrupt Request Generator dialog window
//
void CMainFrame::OnSymGenIntDlg()
{
	CIntRequestGeneratorDlg dlg;

	dlg.m_bGenerateIRQ = theApp.m_global.m_IntGenerator.m_bGenerateIRQ;
	dlg.m_uIRQTimeLapse = theApp.m_global.m_IntGenerator.m_nIRQTimeLapse;

	dlg.m_bGenerateNMI = theApp.m_global.m_IntGenerator.m_bGenerateNMI;
	dlg.m_uNMITimeLapse = theApp.m_global.m_IntGenerator.m_nNMITimeLapse;

	if (dlg.DoModal() != IDOK)
		return;

	StopIntGenerator();

	theApp.m_global.m_IntGenerator.m_bGenerateIRQ = dlg.m_bGenerateIRQ;
	theApp.m_global.m_IntGenerator.m_nIRQTimeLapse = dlg.m_uIRQTimeLapse;

	theApp.m_global.m_IntGenerator.m_bGenerateNMI = dlg.m_bGenerateNMI;
	theApp.m_global.m_IntGenerator.m_nNMITimeLapse = dlg.m_uNMITimeLapse;

	StartIntGenerator();
}


void CMainFrame::OnUpdateSymGenIntDlg(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}


void CMainFrame::StopIntGenerator()
{
	KillTimer(CSym6502::IRQ);
	KillTimer(CSym6502::NMI);
}

void CMainFrame::StartIntGenerator()
{
	if (!theApp.m_global.IsDebugger() || theApp.m_global.IsProgramFinished())
		return;

	if (theApp.m_global.m_IntGenerator.m_bGenerateIRQ)
	{
		int nId= SetTimer(CSym6502::IRQ, theApp.m_global.m_IntGenerator.m_nIRQTimeLapse, &TimerProc);
		ASSERT(nId);
	}

	if (theApp.m_global.m_IntGenerator.m_bGenerateNMI)
	{
		int nId= SetTimer(CSym6502::NMI, theApp.m_global.m_IntGenerator.m_nNMITimeLapse, &TimerProc);
		ASSERT(nId);
	}
}


void CMainFrame::ShowDynamicHelp(const CString& strLine, int nWordStart, int nWordEnd)
{
	m_wndHelpBar.DisplayHelp(strLine, nWordStart, nWordEnd);
}


void CMainFrame::OnHelpDynamic()
{
	ShowControlBar(&m_wndHelpBar, !m_wndHelpBar.IsVisible(), false);
}

void CMainFrame::OnUpdateHelpDynamic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndHelpBar.IsVisible() ? 1 : 0);
}
