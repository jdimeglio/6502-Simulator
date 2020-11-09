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

// DynamicHelp.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DynamicHelp.h"
#include "CCrystalTextBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynamicHelp

PCSTR CDynamicHelp::s_pcszWndClass= NULL;
const int nMinW= 80;
const int nMinH= 50;
CString CDynamicHelp::s_strWndClass;
static const TCHAR* s_REGISTRY_SECTION= _T("DynamicHelp");
static const TCHAR* s_REGISTRY_WIDTH= _T("Height");


CDynamicHelp::CDynamicHelp()
{
	if (s_strWndClass.IsEmpty())
		RegisterWndClass();

	m_cxLeftBorder = m_cxRightBorder = 0;
	m_cyTopBorder = m_cyBottomBorder = 0;
	m_nMRUWidth = 300;
	m_sizeDefault = CSize(m_nMRUWidth, AfxGetApp()->GetProfileInt(s_REGISTRY_SECTION, s_REGISTRY_WIDTH, 500));
	m_nHeaderHeight = 16;
}


void CDynamicHelp::RegisterWndClass()
{
	s_strWndClass = AfxRegisterWndClass(0, ::LoadCursor(NULL,IDC_ARROW), 0, 0);
}


CDynamicHelp::~CDynamicHelp()
{}


BEGIN_MESSAGE_MAP(CDynamicHelp, CControlBar)
	//{{AFX_MSG_MAP(CDynamicHelp)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_NCPAINT()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(IDCLOSE, OnCloseWnd)
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipGetText)
	ON_MESSAGE(WM_USER, OnDelayedResize)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynamicHelp message handlers

//-----------------------------------------------------------------------------

CSize CDynamicHelp::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize sizeBar;

	if (bHorz)
	{
		sizeBar.cx = nMinW;
		sizeBar.cy = nMinH;

		if (bStretch)
			sizeBar.cx = 32767;
	}
	else
	{
		sizeBar.cx = nMinW;
		sizeBar.cy = nMinH;

		if (bStretch)
			sizeBar.cy = 32767;
	}

	return sizeBar;
}

//-----------------------------------------------------------------------------


CSize CDynamicHelp::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	return CalcLayout(dwMode, nLength);
}


CSize CDynamicHelp::CalcLayout(DWORD dwMode, int nLength)
{
	CSize sizeResult(50, 50);
	if (dwMode & (LM_HORZDOCK | LM_VERTDOCK))
	{
		CRect rect;
		CWnd* pMainWnd= AfxGetMainWnd();
		if (CMDIFrameWnd* pFrameWnd= dynamic_cast<CMDIFrameWnd*>(pMainWnd))
			::GetClientRect(pFrameWnd->m_hWndMDIClient, rect);
		else
			pMainWnd->GetClientRect(rect);
		sizeResult = rect.Size();
		sizeResult += CSize(10, 10);
		if (dwMode & 0x8000)
			m_nMRUWidth = nLength;
		sizeResult.cx = m_nMRUWidth;
	}

	if (dwMode & LM_MRUWIDTH)
		SizeToolBar(m_nMRUWidth);
	else if (dwMode & LM_HORZDOCK)
		return CSize(sizeResult.cx, nMinH);
	else if (dwMode & LM_VERTDOCK)
		return CSize(m_nMRUWidth, sizeResult.cy);
	else if (nLength != -1)
		SizeToolBar(nLength, !!(dwMode & LM_LENGTHY));
	else if (m_dwStyle & CBRS_FLOATING)
		SizeToolBar(m_nMRUWidth);
	else
		SizeToolBar(dwMode & LM_HORZ ? 32767 : 0);

	sizeResult = m_sizeDefault;

	if (dwMode & LM_COMMIT)
		m_nMRUWidth = sizeResult.cx;

	return sizeResult;
}


void CDynamicHelp::SizeToolBar(int nLength, bool bVert/*= false*/)
{
	if (bVert)
		m_sizeDefault.cy = nLength;
	else
		m_sizeDefault.cx = nLength;

	if (m_sizeDefault.cx < nMinW)
		m_sizeDefault.cx = nMinW;
	if (m_sizeDefault.cy < nMinH)
		m_sizeDefault.cy = nMinH;

	InvalidateRect(NULL);
}

//-----------------------------------------------------------------------------
void CDynamicHelp::OnNcPaint()
{
	EraseNonClient();
}


void CDynamicHelp::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{}

//-----------------------------------------------------------------------------
static const TCHAR* s_pcszTitle= _T("Dynamic Help");

bool CDynamicHelp::Create(CWnd* pParentWnd, UINT nID)
{
	CControlBar::Create(s_strWndClass, s_pcszTitle,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		CRect(10, 10, 100, 100), pParentWnd, nID);

	SetBarStyle(CBRS_ALIGN_RIGHT | CBRS_FLYBY | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC);

	m_wndClose.Create(WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER,
		CRect(0,0,0,0), this, -1);
	m_wndClose.SetButtonStructSize(sizeof(TBBUTTON));
	m_wndClose.AddBitmap(1, IDB_CLOSE_TB);
	m_wndClose.SetBitmapSize(CSize(8, 8));
	TBBUTTON btn;
	btn.iBitmap = 0;
	btn.idCommand = IDCLOSE;
	btn.fsState = TBSTATE_ENABLED;
	btn.fsStyle = TBSTYLE_BUTTON;
	btn.dwData = 0;
	btn.iString = 0;
	m_wndClose.AddButtons(1, &btn);

	Resize();

	return true;
}

//-----------------------------------------------------------------------------

BOOL CDynamicHelp::OnEraseBkgnd(CDC* pDC)
{
	if (!IsFloating())
	{
		CRect rect;
		GetClientRect(rect);
		pDC->FillSolidRect(rect.right - m_nHeaderHeight - 2, rect.top, m_nHeaderHeight, m_nHeaderHeight, ::GetSysColor(COLOR_3DFACE));
	}

	return true;
}

//-----------------------------------------------------------------------------
static COLORREF s_rgbHelpBkgnd= RGB(255,255,240);

void CDynamicHelp::DoPaint(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	if (IsFloating())
	{
		pDC->FillSolidRect(rect, s_rgbHelpBkgnd);
	}
	else
	{
		pDC->FillSolidRect(rect.left, rect.top, rect.Width(), m_nHeaderHeight, ::GetSysColor(COLOR_3DFACE));

		int nW= rect.Width() - 9 - m_nHeaderHeight;
		int nX= rect.left + 5;
		if (nW > 0)
		{
			COLORREF rgbLight= RGB(255,255,255);
			COLORREF rgbDark= ::GetSysColor(COLOR_3DSHADOW);
			pDC->Draw3dRect(nX, rect.top + 5, nW, 3, rgbLight, rgbDark);
			pDC->Draw3dRect(nX, rect.top + 9, nW, 3, rgbLight, rgbDark);
		}
		int nHeight= rect.Height() - m_nHeaderHeight;
		if (nHeight > 0)
			pDC->FillSolidRect(rect.left, rect.top + m_nHeaderHeight, rect.Width(), nHeight, s_rgbHelpBkgnd);
	}
}


BOOL CDynamicHelp::OnToolTipGetText(UINT uId, NMHDR* pNmHdr, LRESULT* pResult)
{
  NMTTDISPINFO* pTTT= (NMTTDISPINFO*)pNmHdr;

  pTTT->lpszText = pTTT->szText;
  pTTT->szText[0] = 0;
  pTTT->hinst = NULL;

  *pResult = 0;
  return TRUE;
}


void CDynamicHelp::Resize()
{
	CRect rect;
	GetClientRect(rect);

	if (m_wndHelp.m_hWnd)
	{
		if (IsFloating())	// if it's floating it has a title already
		{
			// correction for afxData.cxBorder2 junk
			rect.DeflateRect(2, 2, 2, 2);

			if (m_wndClose.m_hWnd)
				m_wndClose.ShowWindow(SW_HIDE);

			m_wndHelp.SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			// it needs it's own title

			// correction for afxData.cxBorder2 junk
			rect.DeflateRect(2, 0, 2, 8);
			if (rect.bottom < rect.top)
				rect.bottom = rect.top;

			if (m_wndClose.m_hWnd)
				m_wndClose.SetWindowPos(0, rect.right - 16, rect.top + 2, 15, 14, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);

			rect.top += m_nHeaderHeight;
			m_wndHelp.SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
}


void CDynamicHelp::OnSize(UINT nType, int cx, int cy)
{
	CControlBar::OnSize(nType, cx, cy);

	Resize();
}


void CDynamicHelp::OnDestroy()
{
	AfxGetApp()->WriteProfileInt(s_REGISTRY_SECTION, s_REGISTRY_WIDTH, m_sizeDefault.cy);

	CControlBar::OnDestroy();
}


int CDynamicHelp::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwStyle= WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL;
	m_wndHelp.CWnd::Create(_T("RichEdit20A"), NULL, dwStyle, CRect(0,0,0,0), this, IDS_DYNAMIC_HELP);

	if (m_wndHelp.m_hWnd == 0)
		return -1;

	m_wndHelp.SetReadOnly();
	m_wndHelp.SetBackgroundColor(false, s_rgbHelpBkgnd);
	m_wndHelp.SendMessage(EM_SETTEXTMODE, TM_RICHTEXT | TM_SINGLECODEPAGE);
	m_wndHelp.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, 0);

//	SetContextHelp(_T("<b>LDA</b> blah blah<p><u>importante</u> reioioi khjkcxn opoiiso fjoo."));
/*
CFile file("C:\\LDA2.rtf", CFile::modeRead);
CString str;
file.Read(str.GetBuffer(file.GetLength()), file.GetLength());
str.ReleaseBuffer(file.GetLength());
SetContextHelp(str);
*/
	return 0;
}


void CDynamicHelp::SetContextHelp(const TCHAR* pcszText, const TCHAR* pcszHeader/*= 0*/)
{
	CString strText= _T(
		"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033"
		"{\\fonttbl"
			"{\\f0\\fmodern\\fprq1\\fcharset0 Courier New;}"
			"{\\f1\\fswiss\\fcharset0 Arial;}"
		"}"
		"{"
			"\\colortbl ;\\red255\\green255\\blue255;\\red192\\green192\\blue192;\\red160\\green160\\blue160;\\red224\\green224\\blue224;"
		"}"
		"\\viewkind4\\uc1\\pard\\li0");

	const TCHAR* pcszHeaderBefore= _T("\\f1\\fs20\\sa25\\sb25\\cf1\\cb3\\highlight3\\b\\ql\\~  \\~\\~\\~");
	const TCHAR* pcszHeaderAfter= _T("\\~\\~\\~  \\~\\highlight0\\par");
	const TCHAR* pcszFont= _T("\\pard\\cf0\\b0\\f0\\fs20\\ql");

	if (pcszHeader)
	{
		strText += pcszHeaderBefore;
		strText += pcszHeader;
		strText += pcszHeaderAfter;
		strText += pcszFont;
		strText += _T("\\par\\li60 ");
	}
	else
	{
		strText += pcszFont;
	}

	#define PLAIN	_T("\\f1\\fs20\\b0 ")
	CString str= pcszText;
	str.Replace(_T("{"), _T("\\{"));
	str.Replace(_T("}"), _T("\\}"));
	str.Replace(_T("#title#"), _T("\\fi350\\f0\\fs30\\b "));
	str.Replace(_T("#text#"), _T("\\par\\fi0\\f1\\fs20\\b0 "));
	str.Replace(_T("#syntax#"), PLAIN _T("\\par\\par\\sa70 Syntax:\\par\\sa0\\f0\\fs20\\b "));
	str.Replace(_T("#exmpl#"), PLAIN _T("\\par\\sa70 Example:\\par\\sa0\\f0\\fs20\\b0 "));
	str.Replace(_T("#desc#"), PLAIN _T("\\par\\sa70 Description:\\par\\sa0\\f1\\fs20\\b0 "));
	str.Replace(_T("#flags#"), PLAIN _T("\\par\\par Affects flags: \\f0\\fs24\\b0 "));
	str.Replace(_T("#modes#"), PLAIN _T("\\par\\par\\sa70 Addressing Modes:\\par\\sa0\\f0\\fs20\\b0 "));
	str.Replace(_T("<pre>"), _T("\\f0 "));
	str.Replace(_T("<small>"), _T("\\f1\\fs16 "));
	str.Replace(_T("|"), _T("\\f1 "));
	str.Replace(_T("\n"), _T("\\par "));

	strText += str;
	strText += _T("}");

	if (m_wndHelp)
	{
		CString strOld;
		m_wndHelp.GetWindowText(strOld);

		if (strOld != strText)
			m_wndHelp.SetWindowText(strText);

		CRect rect(0,0,0,0);
		m_wndHelp.GetRect(rect);
	}
}


void CDynamicHelp::DisplayHelp(const CString& strLine, int nWordStart, int nWordEnd)
{
	CString strHelp;
	const TCHAR* pcszHeader= 0;

	if (strLine.IsEmpty() || nWordStart >= nWordEnd)
		;
	else if (nWordStart == 0)	// no instruction can start here; also almost all directives are prohibited in the first column
	{
		// test io_area only
		//
	}
	else
	{
		int nComment= strLine.Find(_T(';'));
		if (nComment < 0 || nComment > nWordEnd)	// not inside a comment?
		{
			CString strWord= strLine.Mid(nWordStart, nWordEnd - nWordStart);

			extern int MatchingDirectives(const CString& strWord, CString& strOut);
			CString GetDirectiveDesc(const CString& strDirective);
			extern int MatchingInstructions(const CString& strWord, CString& strResult);
			CString GetInstructionDesc(const CString& strInstruction);

			int nMatching= MatchingInstructions(strWord, strHelp);
			if (nMatching == 1)
				strHelp = GetInstructionDesc(strHelp), pcszHeader = _T("Instruction");
			else if (nMatching > 1)
				pcszHeader = _T("Instructions");
			else
			{
				int nMatching= MatchingDirectives(strWord, strHelp);
				if (nMatching == 1)
					strHelp = GetDirectiveDesc(strHelp), pcszHeader = _T("Directive");
				else if (nMatching > 1)
					pcszHeader = _T("Directives");
			}
		}
	}

	SetContextHelp(strHelp, pcszHeader);
}


void CDynamicHelp::OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle)
{
	PostMessage(WM_USER);
}

LRESULT CDynamicHelp::OnDelayedResize(WPARAM, LPARAM)
{
	Resize();
	return 0;
}


void CDynamicHelp::OnCloseWnd()
{
	if (CFrameWnd* pFrameWnd= GetDockingFrame())
		pFrameWnd->ShowControlBar(this, false, TRUE);

}
