////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextView.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CCrystalTextView class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use or modify this code to the following restrictions:
//	- Acknowledge me somewhere in your about box, simple "Parts of code by.."
//	will be enough. If you can't (or don't want to), contact me personally.
//	- LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	17-Feb-99
//	FIX:	missing UpdateCaret() in CCrystalTextView::SetFont
//	FIX:	missing UpdateCaret() in CCrystalTextView::RecalcVertScrollBar
//	FIX:	mistype in CCrystalTextView::RecalcPageLayouts + instead of +=
//	FIX:	removed condition 'm_nLineHeight < 20' in
//		CCrystalTextView::CalcLineCharDim(). This caused painting defects
//		when using very small fonts.
//
//	FEATURE:	Some experiments with smooth scrolling, controlled by
//		m_bSmoothScroll member variable, by default turned off.
//		See ScrollToLine function for implementation details.
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//	21-Feb-99
//		Paul Selormey, James R. Twine
//	+	FEATURE: description for Undo/Redo actions
//	+	FEATURE: multiple MSVC-like bookmarks
//	+	FEATURE: 'Disable backspace at beginning of line' option
//	+	FEATURE: 'Disable drag-n-drop editing' option
//
//	+	FIX:  ResetView() now virtual
//	+	FEATURE: Added OnEditOperation() virtual: base for auto-indent,
//		smart indent etc.
////////////////////////////////////////////////////////////////////////////
// 25-May-2002
//		Michal Kowalski
//	Collapsible blocks
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "editcmd.h"
#include "editreg.h"
#include "CCrystalTextView.h"
#include "CCrystalTextBuffer.h"
#include "CFindTextDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TAB_CHARACTER				_T('\xBB')
#define SPACE_CHARACTER				_T('\xB7')
#define DEFAULT_PRINT_MARGIN		1000				//	10 millimeters

#define SMOOTH_SCROLL_FACTOR		6
#define	CRYSTAL_TIMER_DRAGSEL		1001

////////////////////////////////////////////////////////////////////////////
// CCrystalTextView

IMPLEMENT_DYNCREATE(CCrystalTextView, CView)

HINSTANCE CCrystalTextView::s_hResourceInst = NULL;

BEGIN_MESSAGE_MAP(CCrystalTextView, CView)
	//{{AFX_MSG_MAP(CCrystalTextView)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_WM_RBUTTONDOWN()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_FIND, OnEditFind)
	ON_COMMAND(ID_EDIT_REPEAT, OnEditRepeat)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REPEAT, OnUpdateEditRepeat)
	ON_COMMAND(ID_EDIT_FIND_PREVIOUS, OnEditFindPrevious)
	ON_UPDATE_COMMAND_UI(ID_EDIT_FIND_PREVIOUS, OnUpdateEditFindPrevious)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_EDIT_VIEW_TABS, OnEditViewTabs)
	ON_UPDATE_COMMAND_UI(ID_EDIT_VIEW_TABS, OnUpdateEditViewTabs)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_EDIT_CHAR_LEFT, OnCharLeft)
	ON_COMMAND(ID_EDIT_EXT_CHAR_LEFT, OnExtCharLeft)
	ON_COMMAND(ID_EDIT_CHAR_RIGHT, OnCharRight)
	ON_COMMAND(ID_EDIT_EXT_CHAR_RIGHT, OnExtCharRight)
	ON_COMMAND(ID_EDIT_WORD_LEFT, OnWordLeft)
	ON_COMMAND(ID_EDIT_EXT_WORD_LEFT, OnExtWordLeft)
	ON_COMMAND(ID_EDIT_WORD_RIGHT, OnWordRight)
	ON_COMMAND(ID_EDIT_EXT_WORD_RIGHT, OnExtWordRight)
	ON_COMMAND(ID_EDIT_LINE_UP, OnLineUp)
	ON_COMMAND(ID_EDIT_EXT_LINE_UP, OnExtLineUp)
	ON_COMMAND(ID_EDIT_LINE_DOWN, OnLineDown)
	ON_COMMAND(ID_EDIT_EXT_LINE_DOWN, OnExtLineDown)
	ON_COMMAND(ID_EDIT_SCROLL_UP, ScrollUp)
	ON_COMMAND(ID_EDIT_SCROLL_DOWN, ScrollDown)
	ON_COMMAND(ID_EDIT_PAGE_UP, OnPageUp)
	ON_COMMAND(ID_EDIT_EXT_PAGE_UP, OnExtPageUp)
	ON_COMMAND(ID_EDIT_PAGE_DOWN, OnPageDown)
	ON_COMMAND(ID_EDIT_EXT_PAGE_DOWN, OnExtPageDown)
	ON_COMMAND(ID_EDIT_LINE_END, OnLineEnd)
	ON_COMMAND(ID_EDIT_EXT_LINE_END, OnExtLineEnd)
	ON_COMMAND(ID_EDIT_HOME, OnHome)
	ON_COMMAND(ID_EDIT_EXT_HOME, OnExtHome)
	ON_COMMAND(ID_EDIT_TEXT_BEGIN, OnTextBegin)
	ON_COMMAND(ID_EDIT_EXT_TEXT_BEGIN, OnExtTextBegin)
	ON_COMMAND(ID_EDIT_TEXT_END, OnTextEnd)
	ON_COMMAND(ID_EDIT_EXT_TEXT_END, OnExtTextEnd)
	//	Standard printing commands
	ON_COMMAND(ID_FILE_PAGE_SETUP, OnFilePageSetup)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	//	Status
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_CRLF, OnUpdateIndicatorCRLF)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INDICATOR_POSITION, OnUpdateIndicatorPosition)
	//	Bookmarks
	ON_COMMAND_RANGE(ID_EDIT_TOGGLE_BOOKMARK0, ID_EDIT_TOGGLE_BOOKMARK9, OnToggleBookmark)
	ON_COMMAND_RANGE(ID_EDIT_GO_BOOKMARK0, ID_EDIT_GO_BOOKMARK9, OnGoBookmark)
	ON_COMMAND(ID_EDIT_CLEAR_BOOKMARKS, OnClearBookmarks)
	// More Bookmarks
	ON_COMMAND(ID_EDIT_TOGGLE_BOOKMARK,     OnToggleBookmark)
	ON_COMMAND(ID_EDIT_GOTO_NEXT_BOOKMARK,  OnNextBookmark)
	ON_COMMAND(ID_EDIT_GOTO_PREV_BOOKMARK,  OnPrevBookmark)
	ON_COMMAND(ID_EDIT_CLEAR_ALL_BOOKMARKS, OnClearAllBookmarks)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_NEXT_BOOKMARK,  OnUpdateNextBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_GOTO_PREV_BOOKMARK,  OnUpdatePrevBookmark)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR_ALL_BOOKMARKS, OnUpdateClearAllBookmarks)
	// Collapsible blocks
	ON_COMMAND(ID_EDIT_TOGGLE_BLOCK, OnToggleCollapsibleBlock)
	ON_COMMAND(ID_EDIT_COLLAPSE_ALL, OnCollapseAllBlocks)
	ON_COMMAND(ID_EDIT_EXPAND_ALL, OnExpandAllBlocks)
END_MESSAGE_MAP()

#define EXPAND_PRIMITIVE(impl, func)	\
	void CCrystalTextView::On##func() { impl(FALSE); }	\
	void CCrystalTextView::OnExt##func() { impl(TRUE); }
	EXPAND_PRIMITIVE(MoveLeft, CharLeft)
	EXPAND_PRIMITIVE(MoveRight, CharRight)
	EXPAND_PRIMITIVE(MoveWordLeft, WordLeft)
	EXPAND_PRIMITIVE(MoveWordRight, WordRight)
	EXPAND_PRIMITIVE(MoveUp, LineUp)
	EXPAND_PRIMITIVE(MoveDown, LineDown)
	EXPAND_PRIMITIVE(MovePgUp, PageUp)
	EXPAND_PRIMITIVE(MovePgDn, PageDown)
	EXPAND_PRIMITIVE(MoveHome, Home)
	EXPAND_PRIMITIVE(MoveEnd, LineEnd)
	EXPAND_PRIMITIVE(MoveCtrlHome, TextBegin)
	EXPAND_PRIMITIVE(MoveCtrlEnd, TextEnd)
#undef EXPAND_PRIMITIVE


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextView construction/destruction

CCrystalTextView::CCrystalTextView()
{
	m_bLastSearch = false;
	m_dwLastSearchFlags = 0;
	m_pszLastFindWhat = 0;
	m_bMultipleSearch = false;
	m_bCursorHidden = false;
	m_pCacheBitmap = 0;
	m_nLineHeight = m_nCharWidth = 0;
	m_nTabSize = 8;
	m_bViewTabs = false;
	m_bSelMargin = true;
	m_nScreenLines = m_nScreenChars = 0;
	m_nMaxLineLength = 0;
	m_nIdealCharPos = 0;
	m_bFocused = false;
	m_ptAnchor = CPoint(0, 0);
	//LOGFONT m_lfBaseFont;
	m_apFonts[0] = m_apFonts[1] = m_apFonts[2] = m_apFonts[3] = 0;
	m_pdwParseCookies = 0;
	m_nParseArraySize = 0;
	m_nActualLengthArraySize = 0;
	m_pnActualLineLength = 0;
	m_bPreparingToDrag = false;
	m_bDraggingText = false;
	m_bDragSelection = m_bWordSelection = m_bLineSelection = false;
	m_nDragSelTimer = 0;
	m_ptDrawSelStart = m_ptDrawSelEnd = CPoint(0, 0);
	m_ptCursorPos = CPoint(0, 0);
	m_ptSelStart = m_ptSelEnd = CPoint(0, 0);
	m_bBookmarkExist = false;
	m_pIcons = 0;
	m_pTextBuffer = 0;
	m_hAccel = 0;
	m_bVertScrollBarLocked = m_bHorzScrollBarLocked = false;
	m_ptDraggedTextBegin = m_ptDraggedTextEnd = CPoint(0, 0);
	m_bShowInactiveSelection = false;
	m_bDisableDragAndDrop = true;
	m_nPrintPages = 0;
	m_pnPages = 0;
	m_pPrintFont = 0;
	m_nPrintLineHeight = 0;
	m_bPrintHeader = m_bPrintFooter = false;
	m_ptPageArea.SetRectEmpty();
	m_rcPrintArea.SetRectEmpty();
	m_nTopLine = m_nOffsetChar = 0;
	m_bSmoothScroll = true;

//		AFX_ZERO_INIT_OBJECT(CView);	// TODO: remove this crap
	ResetView();
}

CCrystalTextView::~CCrystalTextView()
{
	ASSERT(m_hAccel == NULL);
	ASSERT(m_pCacheBitmap == NULL);
	ASSERT(m_pTextBuffer == NULL);		//	Must be correctly detached
	if (m_pszLastFindWhat != NULL)
		free(m_pszLastFindWhat);
	if (m_pdwParseCookies != NULL)
		delete m_pdwParseCookies;
	if (m_pnActualLineLength != NULL)
		delete m_pnActualLineLength;
}

BOOL CCrystalTextView::PreCreateWindow(CREATESTRUCT& cs)
{
	CWnd* pParentWnd= CWnd::FromHandlePermanent(cs.hwndParent);

	if (pParentWnd == NULL || ! pParentWnd->IsKindOf(RUNTIME_CLASS(CSplitterWnd)))
	{
		//	View must always create its own scrollbars,
		//	if only it's not used within splitter
		cs.style |= (WS_HSCROLL | WS_VSCROLL);
	}
	cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS);
	return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextView drawing

void CCrystalTextView::GetSelection(CPoint &ptStart, CPoint &ptEnd)
{
	PrepareSelBounds();
	ptStart = m_ptDrawSelStart;
	ptEnd = m_ptDrawSelEnd;
}

CCrystalTextBuffer *CCrystalTextView::LocateTextBuffer()
{
	return NULL;
}

int CCrystalTextView::GetLineActualLength(int nLineIndex)
{
	int nLineCount = GetLineCount();
	ASSERT(nLineCount > 0);
	ASSERT(nLineIndex >= 0 && nLineIndex < nLineCount);
	if (m_pnActualLineLength == NULL)
	{
		m_pnActualLineLength = new int[nLineCount];
		memset(m_pnActualLineLength, 0xff, sizeof(int) * nLineCount);
		m_nActualLengthArraySize = nLineCount;
	}

	if (m_pnActualLineLength[nLineIndex] >= 0)
		return m_pnActualLineLength[nLineIndex];

	//	Actual line length is not determined yet, let's calculate a little
	int nActualLength = 0;
	int nLength = GetLineLength(nLineIndex);
	if (nLength > 0)
	{
		LPCTSTR pszLine = GetLineChars(nLineIndex);
		LPTSTR pszChars = (LPTSTR) _alloca(sizeof(TCHAR) * (nLength + 1));
		memcpy(pszChars, pszLine, sizeof(TCHAR) * nLength);
		pszChars[nLength] = 0;
		LPTSTR pszCurrent = pszChars;

		int nTabSize = GetTabSize();
		for (;;)
		{
#ifdef _UNICODE
			LPTSTR psz = wcschr(pszCurrent, L'\t');
#else
			LPTSTR psz = strchr(pszCurrent, '\t');
#endif
			if (psz == NULL)
			{
				nActualLength += (pszChars + nLength - pszCurrent);
				break;
			}

			nActualLength += (psz - pszCurrent);
			nActualLength += (nTabSize - nActualLength % nTabSize);
			pszCurrent = psz + 1;
		}
	}

	m_pnActualLineLength[nLineIndex] = nActualLength;
	return nActualLength;
}

void CCrystalTextView::ScrollToChar(int nNewOffsetChar, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
	//	For now, ignoring bNoSmoothScroll and m_bSmoothScroll
	if (m_nOffsetChar != nNewOffsetChar)
	{
		int nScrollChars = m_nOffsetChar - nNewOffsetChar;
		m_nOffsetChar = nNewOffsetChar;
		CRect rcScroll;
		GetClientRect(&rcScroll);
		rcScroll.left += GetMarginWidth();
		ScrollWindow(nScrollChars * GetCharWidth(), 0, &rcScroll, &rcScroll);
		UpdateWindow();
		if (bTrackScrollBar)
			RecalcHorzScrollBar(TRUE);
	}
}

void CCrystalTextView::ScrollToLine(int nNewTopLine, BOOL bNoSmoothScroll /*= FALSE*/, BOOL bTrackScrollBar /*= TRUE*/)
{
	if (m_nTopLine != nNewTopLine)
	{
		if (bNoSmoothScroll || ! m_bSmoothScroll)
		{
			int nScrollLines= -CalcVisibleLines(m_nTopLine, nNewTopLine);
			m_nTopLine = nNewTopLine;
			ScrollWindow(0, nScrollLines * GetLineHeight());
			UpdateWindow();
			if (bTrackScrollBar)
				RecalcVertScrollBar(TRUE);
		}
		else
		{
			//	Do smooth scrolling
			int nLineHeight = GetLineHeight();
			if (m_nTopLine > nNewTopLine)
			{
				int nIncrement = (m_nTopLine - nNewTopLine) / SMOOTH_SCROLL_FACTOR + 1;
				while (m_nTopLine != nNewTopLine)
				{
					int nTopLine = m_nTopLine - nIncrement;
					if (nTopLine < nNewTopLine)
						nTopLine = nNewTopLine;
					int nScrollLines = nTopLine - m_nTopLine;
					m_nTopLine = nTopLine;
					ScrollWindow(0, - nLineHeight * nScrollLines);
					UpdateWindow();
					if (bTrackScrollBar)
						RecalcVertScrollBar(TRUE);
				}
			}
			else
			{
				int nIncrement = (nNewTopLine - m_nTopLine) / SMOOTH_SCROLL_FACTOR + 1;
				while (m_nTopLine != nNewTopLine)
				{
					int nTopLine = m_nTopLine + nIncrement;
					if (nTopLine > nNewTopLine)
						nTopLine = nNewTopLine;
					int nScrollLines = nTopLine - m_nTopLine;
					m_nTopLine = nTopLine;
					ScrollWindow(0, - nLineHeight * nScrollLines);
					UpdateWindow();
					if (bTrackScrollBar)
						RecalcVertScrollBar(TRUE);
				}
			}
		}
	}
}

void CCrystalTextView::ExpandChars(LPCTSTR pszChars, int nOffset, int nCount, CString &line)
{
	if (nCount <= 0)
	{
		line = _T("");
		return;
	}

	int nTabSize = GetTabSize();

	int nActualOffset = 0;
	for (int I = 0; I < nOffset; I ++)
	{
		if (pszChars[I] == _T('\t'))
			nActualOffset += (nTabSize - nActualOffset % nTabSize);
		else
			nActualOffset ++;
	}

	pszChars += nOffset;
	int nLength = nCount;

	int nTabCount = 0;
	int I;
	for (I = 0; I < nLength; I ++)
	{
		if (pszChars[I] == _T('\t'))
			nTabCount ++;
	}

	LPTSTR pszBuf = line.GetBuffer(nLength + nTabCount * (nTabSize - 1) + 1);
	int nCurPos = 0;
	if (nTabCount > 0 || m_bViewTabs)
	{
		for (I = 0; I < nLength; I ++)
		{
			if (pszChars[I] == _T('\t'))
			{
				int nSpaces = nTabSize - (nActualOffset + nCurPos) % nTabSize;
				if (m_bViewTabs)
				{
					pszBuf[nCurPos ++] = TAB_CHARACTER;
					nSpaces --;
				}
				while (nSpaces > 0)
				{
					pszBuf[nCurPos ++] = _T(' ');
					nSpaces --;
				}
			}
			else
			{
				if (pszChars[I] == _T(' ') && m_bViewTabs)
					pszBuf[nCurPos] = SPACE_CHARACTER;
				else
					pszBuf[nCurPos] = pszChars[I];
				nCurPos ++;
			}
		}
	}
	else
	{
		memcpy(pszBuf, pszChars, sizeof(TCHAR) * nLength);
		nCurPos = nLength;
	}
	pszBuf[nCurPos] = 0;
	line.ReleaseBuffer();
}


void CCrystalTextView::DrawLineHelperImpl(CDC *pdc, CPoint &ptOrigin, const CRect &rcClip,
									 LPCTSTR pszChars, int nOffset, int nCount)
{
	ASSERT(nCount >= 0);
	if (nCount > 0)
	{
		CString line;
		ExpandChars(pszChars, nOffset, nCount, line);
		int nWidth = rcClip.right - ptOrigin.x;
		if (nWidth > 0)
		{
			int nCharWidth= GetCharWidth();
			int nCount = line.GetLength();
			int nCountFit = nWidth / nCharWidth + 1;
			if (nCount > nCountFit)
				nCount = nCountFit;
			std::vector<int> vWidths(nCount, nCharWidth);
			VERIFY(pdc->ExtTextOut(ptOrigin.x, ptOrigin.y, ETO_CLIPPED, &rcClip, line, nCount, &vWidths.front()));
		}
		ptOrigin.x += GetCharWidth() * line.GetLength();
	}
}

void CCrystalTextView::DrawLineHelper(CDC *pdc, CPoint &ptOrigin, const CRect &rcClip, int nColorIndex,
									 LPCTSTR pszChars, int nOffset, int nCount, CPoint ptTextPos)
{
	if (nCount > 0)
	{
		if (m_bFocused || m_bShowInactiveSelection)
		{
			int nSelBegin = 0, nSelEnd = 0;
			if (m_ptDrawSelStart.y > ptTextPos.y)
			{
				nSelBegin = nCount;
			}
			else
			if (m_ptDrawSelStart.y == ptTextPos.y)
			{
				nSelBegin = m_ptDrawSelStart.x - ptTextPos.x;
				if (nSelBegin < 0)
					nSelBegin = 0;
				if (nSelBegin > nCount)
					nSelBegin = nCount;
			}
			if (m_ptDrawSelEnd.y > ptTextPos.y)
			{
				nSelEnd = nCount;
			}
			else
			if (m_ptDrawSelEnd.y == ptTextPos.y)
			{
				nSelEnd = m_ptDrawSelEnd.x - ptTextPos.x;
				if (nSelEnd < 0)
					nSelEnd = 0;
				if (nSelEnd > nCount)
					nSelEnd = nCount;
			}

			ASSERT(nSelBegin >= 0 && nSelBegin <= nCount);
			ASSERT(nSelEnd >= 0 && nSelEnd <= nCount);
			ASSERT(nSelBegin <= nSelEnd);

			//	Draw part of the text before selection
			if (nSelBegin > 0)
			{
				DrawLineHelperImpl(pdc, ptOrigin, rcClip, pszChars, nOffset, nSelBegin);
			}
			if (nSelBegin < nSelEnd)
			{
				COLORREF crOldBk = pdc->SetBkColor(GetColor(COLORINDEX_SELBKGND));
//				COLORREF crOldText = pdc->SetTextColor(GetColor(COLORINDEX_SELTEXT));
				DrawLineHelperImpl(pdc, ptOrigin, rcClip, pszChars, nOffset + nSelBegin, nSelEnd - nSelBegin);
				pdc->SetBkColor(crOldBk);
//				pdc->SetTextColor(crOldText);
			}
			if (nSelEnd < nCount)
			{
				DrawLineHelperImpl(pdc, ptOrigin, rcClip, pszChars, nOffset + nSelEnd, nCount - nSelEnd);
			}
		}
		else
		{
			DrawLineHelperImpl(pdc, ptOrigin, rcClip, pszChars, nOffset, nCount);
		}
	}
}

void CCrystalTextView::GetLineColors(int nLineIndex, COLORREF &crBkgnd,
					COLORREF &crText, BOOL &bDrawWhitespace)
{
	DWORD dwLineFlags = GetLineFlags(nLineIndex);
	bDrawWhitespace = TRUE;
	crText = RGB(255, 255, 255);
	if (dwLineFlags & LF_EXECUTION)
	{
		crBkgnd = RGB(0, 128, 0);
		return;
	}
	if (dwLineFlags & LF_BREAKPOINT)
	{
		crBkgnd = RGB(255, 0, 0);
		return;
	}
	if (dwLineFlags & LF_INVALID_BREAKPOINT)
	{
		crBkgnd = RGB(128, 128, 0);
		return;
	}
	crBkgnd = CLR_NONE;
	crText = CLR_NONE;
	bDrawWhitespace = FALSE;
}

DWORD CCrystalTextView::GetParseCookie(int nLineIndex)
{
	int nLineCount = GetLineCount();
	if (m_pdwParseCookies == NULL)
	{
		m_nParseArraySize = nLineCount;
		m_pdwParseCookies = new DWORD[nLineCount];
		memset(m_pdwParseCookies, 0xff, nLineCount * sizeof(DWORD));
	}

	if (nLineIndex < 0)
		return 0;
	if (m_pdwParseCookies[nLineIndex] != (DWORD) -1)
		return m_pdwParseCookies[nLineIndex];

	int L = nLineIndex;
	while (L >= 0 && m_pdwParseCookies[L] == (DWORD) -1)
		L --;
	L ++;

	int nBlocks;
	while (L <= nLineIndex)
	{
		DWORD dwCookie = 0;
		if (L > 0)
			dwCookie = m_pdwParseCookies[L - 1];
		ASSERT(dwCookie != (DWORD) -1);
		m_pdwParseCookies[L] = ParseLine(dwCookie, L, NULL, nBlocks);
		ASSERT(m_pdwParseCookies[L] != (DWORD) -1);
		L ++;
	}

	return m_pdwParseCookies[nLineIndex];
}

void CCrystalTextView::DrawSingleLine(CDC *pdc, const CRect &rc, int nLineIndex)
{
	ASSERT(nLineIndex >= -1 && nLineIndex < GetLineCount());

	if (nLineIndex == -1)
	{
		//	Draw line beyond the text
		pdc->FillSolidRect(rc, GetColor(COLORINDEX_WHITESPACE));
		return;
	}
/*	else if (IsFirstLineOfCollapsedBlock(nLineIndex))
	{
		// first line of collapsed block
		DrawEllipsis(pdc, rc);
		return;
	} */

	//	Acquire the background color for the current line
	BOOL bDrawWhitespace = FALSE;
	COLORREF crBkgnd, crText;
	GetLineColors(nLineIndex, crBkgnd, crText, bDrawWhitespace);
	if (crBkgnd == CLR_NONE)
		crBkgnd = GetColor(COLORINDEX_BKGND);

	int nLength = GetLineLength(nLineIndex);
	if (nLength == 0)
	{
		// MiK
		if (IsFirstLineOfCollapsedBlock(nLineIndex))
		{
			DrawEllipsis(pdc, rc);
		}
		else
		{
			//	Draw the empty line
			CRect rect = rc;
			if ((m_bFocused || m_bShowInactiveSelection) && IsInsideSelBlock(CPoint(0, nLineIndex)))
			{
				pdc->FillSolidRect(rect.left, rect.top, GetCharWidth(), rect.Height(), GetColor(COLORINDEX_SELBKGND));
				rect.left += GetCharWidth();
			}
			pdc->FillSolidRect(rect, bDrawWhitespace ? crBkgnd : GetColor(COLORINDEX_WHITESPACE));
		}

		// parse line: give it a chance to mark collapsible blocks
		GetParseCookie(nLineIndex);

		return;
	}

	//	Parse the line
	LPCTSTR pszChars = GetLineChars(nLineIndex);
	DWORD dwCookie = GetParseCookie(nLineIndex - 1);
	TEXTBLOCK *pBuf = (TEXTBLOCK *) _alloca(sizeof(TEXTBLOCK) * nLength * 3);
	int nBlocks = 0;
	m_pdwParseCookies[nLineIndex] = ParseLine(dwCookie, nLineIndex, pBuf, nBlocks);
	ASSERT(m_pdwParseCookies[nLineIndex] != (DWORD) -1);

	//	Draw the line text
	CPoint origin(rc.left - m_nOffsetChar * GetCharWidth(), rc.top);
	pdc->SetBkColor(crBkgnd);
	if (crText != CLR_NONE)
		pdc->SetTextColor(crText);
	BOOL bColorSet = FALSE;

	if (nBlocks > 0)
	{
		ASSERT(pBuf[0].m_nCharPos >= 0 && pBuf[0].m_nCharPos <= nLength);
		if (crText == CLR_NONE)
			pdc->SetTextColor(GetColor(COLORINDEX_NORMALTEXT));
		pdc->SelectObject(GetFont(GetItalic(COLORINDEX_NORMALTEXT), GetBold(COLORINDEX_NORMALTEXT)));
		DrawLineHelper(pdc, origin, rc, COLORINDEX_NORMALTEXT, pszChars, 0, pBuf[0].m_nCharPos, CPoint(0, nLineIndex));
		for (int I = 0; I < nBlocks - 1; I ++)
		{
			ASSERT(pBuf[I].m_nCharPos >= 0 && pBuf[I].m_nCharPos <= nLength);
			if (crText == CLR_NONE)
				pdc->SetTextColor(GetColor(pBuf[I].m_nColorIndex));
			pdc->SelectObject(GetFont(GetItalic(pBuf[I].m_nColorIndex), GetBold(pBuf[I].m_nColorIndex)));
			DrawLineHelper(pdc, origin, rc, pBuf[I].m_nColorIndex, pszChars,
							pBuf[I].m_nCharPos, pBuf[I + 1].m_nCharPos - pBuf[I].m_nCharPos,
							CPoint(pBuf[I].m_nCharPos, nLineIndex));
		}
		ASSERT(pBuf[nBlocks - 1].m_nCharPos >= 0 && pBuf[nBlocks - 1].m_nCharPos <= nLength);
		if (crText == CLR_NONE)
			pdc->SetTextColor(GetColor(pBuf[nBlocks - 1].m_nColorIndex));
		pdc->SelectObject(GetFont(GetItalic(pBuf[nBlocks - 1].m_nColorIndex),
							GetBold(pBuf[nBlocks - 1].m_nColorIndex)));
		DrawLineHelper(pdc, origin, rc, pBuf[nBlocks - 1].m_nColorIndex, pszChars,
							pBuf[nBlocks - 1].m_nCharPos, nLength - pBuf[nBlocks - 1].m_nCharPos,
							CPoint(pBuf[nBlocks - 1].m_nCharPos, nLineIndex));
	}
	else
	{
		if (crText == CLR_NONE)
			pdc->SetTextColor(GetColor(COLORINDEX_NORMALTEXT));
		pdc->SelectObject(GetFont(GetItalic(COLORINDEX_NORMALTEXT), GetBold(COLORINDEX_NORMALTEXT)));
		DrawLineHelper(pdc, origin, rc, COLORINDEX_NORMALTEXT, pszChars, 0, nLength, CPoint(0, nLineIndex));
	}

	//	Draw whitespaces to the left of the text
	CRect frect = rc;
	if (origin.x > frect.left)
		frect.left = origin.x;
	if (frect.right > frect.left)
	{
		if ((m_bFocused || m_bShowInactiveSelection) && IsInsideSelBlock(CPoint(nLength, nLineIndex)))
		{
			pdc->FillSolidRect(frect.left, frect.top, GetCharWidth(), frect.Height(),
												GetColor(COLORINDEX_SELBKGND));
			frect.left += GetCharWidth();
		}
		if (frect.right > frect.left)
			pdc->FillSolidRect(frect, bDrawWhitespace ? crBkgnd : GetColor(COLORINDEX_WHITESPACE));

		// MiK
		if (IsFirstLineOfCollapsedBlock(nLineIndex))
		{
			DrawEllipsis(pdc, frect);
		}
	}
}

COLORREF CCrystalTextView::GetColor(int nColorIndex)
{
	switch (nColorIndex)
	{
	case COLORINDEX_WHITESPACE:
	case COLORINDEX_BKGND:
		return ::GetSysColor(COLOR_WINDOW);
	case COLORINDEX_NORMALTEXT:
		return ::GetSysColor(COLOR_WINDOWTEXT);
	case COLORINDEX_SELMARGIN:
		{
			COLORREF rgbGray= ::GetSysColor(COLOR_3DFACE);
			return RGB(0x80 + GetRValue(rgbGray) / 2, 0x80 + GetGValue(rgbGray) / 2, 0x80 + GetBValue(rgbGray) / 2);
		}
//		return RGB(240,240,240); //::GetSysColor(COLOR_SCROLLBAR);
	case COLORINDEX_PREPROCESSOR:
		return RGB(128, 0, 128);
	case COLORINDEX_COMMENT:
		return RGB(128, 128, 128);
	//	[JRT]: Enabled Support For Numbers...
	case COLORINDEX_NUMBER:
		return RGB(0, 0, 255);
	//	[JRT]: Support For C/C++ Operators
	case COLORINDEX_OPERATOR:
		return RGB(128, 0, 0);
	case COLORINDEX_KEYWORD:
		return RGB(0, 0, 160);
	case COLORINDEX_SELBKGND:
		return RGB(192, 192, 224);
	case COLORINDEX_SELTEXT:
		return RGB(255, 255, 255);
	case COLORINDEX_STRING:
		return RGB(0, 128, 128);
	case COLORINDEX_ELLIPSIS:
		return RGB(128, 128, 128);
	}

	return RGB(0, 0, 0);
}

DWORD CCrystalTextView::GetLineFlags(int nLineIndex)
{
	if (m_pTextBuffer == NULL)
		return 0;
	return m_pTextBuffer->GetLineFlags(nLineIndex);
}

void CCrystalTextView::DrawMargin(CDC *pdc, const CRect &rect, int nLineIndex)
{
	if (!m_bSelMargin)
	{
		pdc->FillSolidRect(rect, GetColor(COLORINDEX_BKGND));
		return;
	}

	pdc->FillSolidRect(rect, GetColor(COLORINDEX_SELMARGIN));

	if (nLineIndex < 0)
		return;

	int nImageIndex= -1;

	DWORD dwLineFlags= GetLineFlags(nLineIndex);

	static const DWORD adwFlags[] =
	{
		LF_EXECUTION,
		LF_BREAKPOINT,
		LF_COMPILATION_ERROR,
		LF_BOOKMARK(1),
		LF_BOOKMARK(2),
		LF_BOOKMARK(3),
		LF_BOOKMARK(4),
		LF_BOOKMARK(5),
		LF_BOOKMARK(6),
		LF_BOOKMARK(7),
		LF_BOOKMARK(8),
		LF_BOOKMARK(9),
		LF_BOOKMARK(0),
		LF_BOOKMARKS,
		LF_INVALID_BREAKPOINT
	};

	for (int I = 0; I <= sizeof(adwFlags) / sizeof(adwFlags[0]); I ++)
	{
		if ((dwLineFlags & adwFlags[I]) != 0)
		{
			nImageIndex = I;
			break;
		}
	}

	if (nImageIndex >= 0)
		DrawMarginIcon(pdc, rect, nImageIndex);

	// MiK: draw expand/collapse mark on top of other marks

	int nIndex= nLineIndex - m_nTopLine;
	bool bInsideBlock= nIndex >= 0 && nIndex < m_vLineInsideBlock.size() ? m_vLineInsideBlock[nIndex] : false;

	nImageIndex = sizeof(adwFlags) / sizeof(adwFlags[0]);

	if (bInsideBlock)
		DrawMarginIcon(pdc, rect, nImageIndex + 3);

	if (dwLineFlags & (LF_COLLAPSIBLE_BLOCK_START | LF_COLLAPSIBLE_BLOCK_END))
	{
		int nImg= 0;

		if (dwLineFlags & LF_COLLAPSIBLE_BLOCK_START && dwLineFlags & LF_COLLAPSED_BLOCK)
			nImg = 2;
		else if (dwLineFlags & LF_COLLAPSIBLE_BLOCK_END)
			nImg = 1;

		DrawMarginIcon(pdc, rect, nImageIndex + nImg);

	}

	DrawMarginMarker(nLineIndex, pdc, rect);
}


void CCrystalTextView::DrawMarginMarker(int nLine, CDC* pDC, const CRect &rect)
{}


void CCrystalTextView::DrawMarginIcon(CDC* pDC, const CRect &rect, int nImageIndex)
{
	if (nImageIndex >= 0)
	{
		if (m_pIcons == NULL)
		{ 
			m_pIcons = new CImageList;
			VERIFY(m_pIcons->Create(IDR_MARGIN_ICONS, 16, 16, RGB(255, 0, 255)));
		}

		CPoint pt(rect.left + 2, rect.top + (rect.Height() - 16) / 2);
		VERIFY(m_pIcons->Draw(pDC, nImageIndex, pt, ILD_TRANSPARENT));
	}
}

BOOL CCrystalTextView::IsInsideSelBlock(CPoint ptTextPos)
{
	ASSERT_VALIDTEXTPOS(ptTextPos);
	if (ptTextPos.y < m_ptDrawSelStart.y)
		return FALSE;
	if (ptTextPos.y > m_ptDrawSelEnd.y)
		return FALSE;
	if (ptTextPos.y < m_ptDrawSelEnd.y && ptTextPos.y > m_ptDrawSelStart.y)
		return TRUE;
	if (m_ptDrawSelStart.y < m_ptDrawSelEnd.y)
	{
		if (ptTextPos.y == m_ptDrawSelEnd.y)
			return ptTextPos.x < m_ptDrawSelEnd.x;
		ASSERT(ptTextPos.y == m_ptDrawSelStart.y);
		return ptTextPos.x >= m_ptDrawSelStart.x;
	}
	ASSERT(m_ptDrawSelStart.y == m_ptDrawSelEnd.y);
	return ptTextPos.x >= m_ptDrawSelStart.x && ptTextPos.x < m_ptDrawSelEnd.x;
}

BOOL CCrystalTextView::IsInsideSelection(const CPoint &ptTextPos)
{
	PrepareSelBounds();
	return IsInsideSelBlock(ptTextPos);
}

void CCrystalTextView::PrepareSelBounds()
{
	if (m_ptSelStart.y < m_ptSelEnd.y ||
			(m_ptSelStart.y == m_ptSelEnd.y && m_ptSelStart.x < m_ptSelEnd.x))
	{
		m_ptDrawSelStart = m_ptSelStart;
		m_ptDrawSelEnd = m_ptSelEnd;
	}
	else
	{
		m_ptDrawSelStart = m_ptSelEnd;
		m_ptDrawSelEnd = m_ptSelStart;
	}
}

void CCrystalTextView::OnDraw(CDC* pdc)
{
	CRect rcClient;
	GetClientRect(rcClient);

	int nLineCount = GetLineCount();
	int nLineHeight = GetLineHeight();
	PrepareSelBounds();

	CDC cacheDC;
	cacheDC.CreateCompatibleDC(pdc);
	if (cacheDC.m_hDC == 0)
		return;
	if (m_pCacheBitmap == NULL)
	{
		m_pCacheBitmap = new CBitmap;
		VERIFY(m_pCacheBitmap->CreateCompatibleBitmap(pdc, rcClient.Width(), nLineHeight));
	}
	CBitmap* pOldBitmap= cacheDC.SelectObject(m_pCacheBitmap);

	CRect rcLine;
	rcLine = rcClient;
	rcLine.bottom = rcLine.top + nLineHeight;
	CRect rcCacheMargin(0, 0, GetMarginWidth(), nLineHeight);
	CRect rcCacheLine(GetMarginWidth(), 0, rcLine.Width(), nLineHeight);

	// block vertical lines support
	FindBlockBoundaries(m_nTopLine, rcClient.Height() / nLineHeight + 1);

	for (int nCurrentLine = m_nTopLine; rcLine.top < rcClient.bottom; ++nCurrentLine)
	{
		int nLineIndex= nCurrentLine < nLineCount ? nCurrentLine : -1;

		if (IsLineHidden(nCurrentLine))
		{
			// hidden lines inside collapsed block
			continue;	// do not draw hidden lines
		}
		else	// normal (visible) lines
		{
			if (pdc->RectVisible(rcLine))
			{
				DrawMargin(&cacheDC, rcCacheMargin, nLineIndex);
				DrawSingleLine(&cacheDC, rcCacheLine, nLineIndex);

				VERIFY(pdc->BitBlt(rcLine.left, rcLine.top, rcLine.Width(), rcLine.Height(), &cacheDC, 0, 0, SRCCOPY));
			}
		}

		rcLine.OffsetRect(0, nLineHeight);
	}

	cacheDC.SelectObject(pOldBitmap);
	cacheDC.DeleteDC();
}


// This fn tries to determine if lines displayed lie inside collapsible text blocks
//
// (it's sensitive to spurious block ends; just like whole collapsing code; try
// a few closing brackets '}' without corresponding '{' ones to see what happens
// with lines below)
//
void CCrystalTextView::FindBlockBoundaries(int nTopLine, int nLines)
{
	// this vector will tell me whether line N lies inside text block
	m_vLineInsideBlock.clear();

	if (nLines < 1 || m_pTextBuffer == NULL)
		return;

	int nLineCount= m_pTextBuffer->GetLineCount();
	if (nTopLine >= nLineCount)
		return;

	int nLastLine= FindVisibleLine(nTopLine, nLines);
	int nCount= nLastLine - nTopLine + 1;
	m_vLineInsideBlock.resize(nCount);

	// text block nesting depth
	int nLevel= 0;

	ASSERT(nTopLine >= 0);

	int nLineFrom = nTopLine;

	std::vector<bool>::iterator itInside= m_vLineInsideBlock.begin();
	std::vector<bool>::iterator itEnd=	  m_vLineInsideBlock.end();

	for (int nLine= nTopLine, nIndex= 0; nLines > 0; ++itInside)
	{
		DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

		if (dwFlags & LF_COLLAPSIBLE_BLOCK_END)
			--nLevel;

		if (nLevel <= 0 && nLineFrom > 0)
		{
			// look outside (this is up); are we still inside a block?

			int nLineIndex= nLineFrom - 1;

			for (; nLineIndex >= 0; --nLineIndex)
			{
				DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLineIndex);

				if (dwFlags & LF_COLLAPSIBLE_BLOCK_END)
					--nLevel;

				if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
					if (++nLevel > 0)
						break;
			}

			nLineFrom = nLineIndex;
		}

		ASSERT(itInside < itEnd);
		*itInside = nLevel > 0;

		if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
			++nLevel;

		if (!IsLineHidden(nLine))
			--nLines;

		if (++nLine >= nLineCount)
			break;

		if (itInside == itEnd)
			break;
	}

	std::fill(itInside, itEnd, false);
}


void CCrystalTextView::ResetView()
{
	m_nTopLine = 0;
	m_nOffsetChar = 0;
	m_nLineHeight = -1;
	m_nCharWidth = -1;
	m_nTabSize = 4;
	m_nMaxLineLength = -1;
	m_nScreenLines = -1;
	m_nScreenChars = -1;
	m_nIdealCharPos = -1;
	m_ptAnchor.x = 0;
	m_ptAnchor.y = 0;
	if (m_pIcons != NULL)
	{
		delete m_pIcons;
		m_pIcons = NULL;
	}
	for (int I = 0; I < 4; I ++)
	{
		if (m_apFonts[I] != NULL)
		{
			m_apFonts[I]->DeleteObject();
			delete m_apFonts[I];
			m_apFonts[I] = NULL;
		}
	}
	if (m_pdwParseCookies != NULL)
	{
		delete m_pdwParseCookies;
		m_pdwParseCookies = NULL;
	}
	if (m_pnActualLineLength != NULL)
	{
		delete m_pnActualLineLength;
		m_pnActualLineLength = NULL;
	}
	m_nParseArraySize = 0;
	m_nActualLengthArraySize = 0;
	m_ptCursorPos.x = 0;
	m_ptCursorPos.y = 0;
	m_ptSelStart = m_ptSelEnd = m_ptCursorPos;
	m_bDragSelection = FALSE;
	m_bVertScrollBarLocked = FALSE;
	m_bHorzScrollBarLocked = FALSE;
	if (::IsWindow(m_hWnd))
		UpdateCaret();
	m_bLastSearch = FALSE;
	m_bShowInactiveSelection = FALSE;
	m_bPrintHeader = FALSE;
	m_bPrintFooter = TRUE;

	m_bBookmarkExist  = FALSE;	// More bookmarks
	m_bMultipleSearch = FALSE;	// More search
}

void CCrystalTextView::UpdateCaret()
{
	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
	if (m_bFocused && ! m_bCursorHidden &&
		CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x) >= m_nOffsetChar)
	{
		CreateSolidCaret(2, GetLineHeight());
		SetCaretPos(TextToClient(m_ptCursorPos));
		ShowCaret();
	}
	else
	{
		HideCaret();
	}

	CaretMoved();
}


int CCrystalTextView::GetTabSize()
{
	ASSERT(m_nTabSize >= 0 && m_nTabSize <= 64);
	return m_nTabSize;
}

void CCrystalTextView::SetTabSize(int nTabSize)
{
	ASSERT(nTabSize >= 0 && nTabSize <= 64);
	if (m_nTabSize != nTabSize)
	{
		m_nTabSize = nTabSize;
		if (m_pnActualLineLength != NULL)
		{
			delete m_pnActualLineLength;
			m_pnActualLineLength = NULL;
		}
		m_nActualLengthArraySize = 0;
		m_nMaxLineLength = -1;
		RecalcHorzScrollBar();
		Invalidate();
		UpdateCaret();
	}
}

CFont *CCrystalTextView::GetFont(BOOL bItalic /*= FALSE*/, BOOL bBold /*= FALSE*/)
{
	int nIndex = 0;
	if (bBold)
		nIndex |= 1;
	if (bItalic)
		nIndex |= 2;

	if (m_apFonts[nIndex] == NULL)
	{
		m_apFonts[nIndex] = new CFont;
		m_lfBaseFont.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
		m_lfBaseFont.lfItalic = (BYTE) bItalic;
		if (! m_apFonts[nIndex]->CreateFontIndirect(&m_lfBaseFont))
		{
			delete m_apFonts[nIndex];
			m_apFonts[nIndex] = NULL;
			return CView::GetFont();
		}

	}
	return m_apFonts[nIndex];
}

void CCrystalTextView::CalcLineCharDim()
{
	CDC *pdc = GetDC();
	CFont *pOldFont = pdc->SelectObject(GetFont());
	CSize szCharExt = pdc->GetTextExtent(_T("X"));
	m_nLineHeight = szCharExt.cy;
	if (m_nLineHeight < 1)
		m_nLineHeight = 1;
	m_nCharWidth = szCharExt.cx;
	/*
	TEXTMETRIC tm;
	if (pdc->GetTextMetrics(&tm))
		m_nCharWidth -= tm.tmOverhang;
	*/
	pdc->SelectObject(pOldFont);
	ReleaseDC(pdc);
}

int CCrystalTextView::GetLineHeight()
{
	if (m_nLineHeight == -1)
		CalcLineCharDim();
	return m_nLineHeight;
}

int CCrystalTextView::GetCharWidth()
{
	if (m_nCharWidth == -1)
		CalcLineCharDim();
	return m_nCharWidth;
}

int CCrystalTextView::GetMaxLineLength()
{
	if (m_nMaxLineLength == -1)
	{
		m_nMaxLineLength = 0;
		int nLineCount = GetLineCount();
		for (int I = 0; I < nLineCount; I ++)
		{
			int nActualLength = GetLineActualLength(I);
			if (m_nMaxLineLength < nActualLength)
				m_nMaxLineLength = nActualLength;
		}
	}
	return m_nMaxLineLength;
}

CCrystalTextView *CCrystalTextView::GetSiblingView(int nRow, int nCol)
{
	CSplitterWnd *pSplitter = GetParentSplitter(this, FALSE);
	if (pSplitter == NULL)
		return NULL;
	CWnd *pWnd = CWnd::FromHandlePermanent(
		::GetDlgItem(pSplitter->m_hWnd, pSplitter->IdFromRowCol(nRow, nCol)));
	if (pWnd == NULL || ! pWnd->IsKindOf(RUNTIME_CLASS(CCrystalTextView)))
		return NULL;
	return (CCrystalTextView *) pWnd;
}

void CCrystalTextView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	AttachToBuffer(NULL);

	CSplitterWnd *pSplitter = GetParentSplitter(this, FALSE);
	if (pSplitter != NULL)
	{
		//	See CSplitterWnd::IdFromRowCol() implementation
		int nRow = (GetDlgCtrlID() - AFX_IDW_PANE_FIRST) / 16;
		int nCol = (GetDlgCtrlID() - AFX_IDW_PANE_FIRST) % 16;
		ASSERT(nRow >= 0 && nRow < pSplitter->GetRowCount());
		ASSERT(nCol >= 0 && nCol < pSplitter->GetColumnCount());

		if (nRow > 0)
		{
			CCrystalTextView *pSiblingView = GetSiblingView(0, nCol);
			if (pSiblingView != NULL && pSiblingView != this)
			{
				m_nOffsetChar = pSiblingView->m_nOffsetChar;
				ASSERT(m_nOffsetChar >= 0 && m_nOffsetChar <= GetMaxLineLength());
			}
		}

		if (nCol > 0)
		{
			CCrystalTextView *pSiblingView = GetSiblingView(nRow, 0);
			if (pSiblingView != NULL && pSiblingView != this)
			{
				m_nTopLine = pSiblingView->m_nTopLine;
				ASSERT(m_nTopLine >= 0 && m_nTopLine < GetLineCount());
			}
		}
	}

	// MiK: parse all to update collapsible blocks
	GetParseCookie(GetLineCount() - 1);
}


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextView printing

void CCrystalTextView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);

	if (pInfo != NULL)
	{
		pInfo->m_bContinuePrinting = TRUE;
		if (m_pnPages != NULL && (int) pInfo->m_nCurPage > m_nPrintPages)
			pInfo->m_bContinuePrinting = FALSE;
	}
}

BOOL CCrystalTextView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

int CCrystalTextView::PrintLineHeight(CDC *pdc, int nLine)
{
	ASSERT(nLine >= 0 && nLine < GetLineCount());
	ASSERT(m_nPrintLineHeight > 0);
	int nLength = GetLineLength(nLine);
	if (nLength == 0)
		return m_nPrintLineHeight;

	CString line;
	LPCTSTR pszChars = GetLineChars(nLine);
	ExpandChars(pszChars, 0, nLength, line);
	CRect rcPrintArea = m_rcPrintArea;
	pdc->DrawText(line, &rcPrintArea, DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK | DT_CALCRECT);
	return rcPrintArea.Height();
}

void CCrystalTextView::GetPrintHeaderText(int nPageNum, CString &text)
{
	ASSERT(m_bPrintHeader);
	text = _T("");
}

void CCrystalTextView::GetPrintFooterText(int nPageNum, CString &text)
{
	ASSERT(m_bPrintFooter);
	text.Format(_T("Page %d/%d"), nPageNum, m_nPrintPages);
}

void CCrystalTextView::PrintHeader(CDC *pdc, int nPageNum)
{
	CRect rcHeader = m_rcPrintArea;
	rcHeader.bottom = rcHeader.top;
	rcHeader.top -= (m_nPrintLineHeight + m_nPrintLineHeight / 2);

	CString text;
	GetPrintHeaderText(nPageNum, text);
	if (! text.IsEmpty())
		pdc->DrawText(text, &rcHeader, DT_CENTER | DT_NOPREFIX | DT_TOP | DT_SINGLELINE);
}

void CCrystalTextView::PrintFooter(CDC *pdc, int nPageNum)
{
	CRect rcFooter = m_rcPrintArea;
	rcFooter.top = rcFooter.bottom;
	rcFooter.bottom += (m_nPrintLineHeight + m_nPrintLineHeight / 2);

	CString text;
	GetPrintFooterText(nPageNum, text);
	if (! text.IsEmpty())
		pdc->DrawText(text, &rcFooter, DT_CENTER | DT_NOPREFIX | DT_BOTTOM | DT_SINGLELINE);
}

void CCrystalTextView::RecalcPageLayouts(CDC *pdc, CPrintInfo *pInfo)
{
	m_ptPageArea = pInfo->m_rectDraw;
	m_ptPageArea.NormalizeRect();

	m_nPrintLineHeight = pdc->GetTextExtent(_T("X")).cy;

	m_rcPrintArea = m_ptPageArea;
	CSize szTopLeft, szBottomRight;
	CWinApp *pApp = AfxGetApp();
	ASSERT(pApp != NULL);
	szTopLeft.cx = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_LEFT, DEFAULT_PRINT_MARGIN);
	szBottomRight.cx = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_RIGHT, DEFAULT_PRINT_MARGIN);
	szTopLeft.cy = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_TOP, DEFAULT_PRINT_MARGIN);
	szBottomRight.cy = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_BOTTOM, DEFAULT_PRINT_MARGIN);
	pdc->HIMETRICtoLP(&szTopLeft);
	pdc->HIMETRICtoLP(&szBottomRight);
	m_rcPrintArea.left += szTopLeft.cx;
	m_rcPrintArea.right -= szBottomRight.cx;
	m_rcPrintArea.top += szTopLeft.cy;
	m_rcPrintArea.bottom -= szBottomRight.cy;
	if (m_bPrintHeader)
		m_rcPrintArea.top += m_nPrintLineHeight + m_nPrintLineHeight / 2;
	if (m_bPrintFooter)
		m_rcPrintArea.bottom += m_nPrintLineHeight + m_nPrintLineHeight / 2;

	int nLimit = 32;
	m_nPrintPages = 1;
	m_pnPages = new int[nLimit];
	m_pnPages[0] = 0;

	int nLineCount = GetLineCount();
	int nLine = 1;
	int y = m_rcPrintArea.top + PrintLineHeight(pdc, 0);
	while (nLine < nLineCount)
	{
		int nHeight = PrintLineHeight(pdc, nLine);
		if (y + nHeight <= m_rcPrintArea.bottom)
		{
			y += nHeight;
		}
		else
		{
			ASSERT(nLimit >= m_nPrintPages);
			if (nLimit <= m_nPrintPages)
			{
				nLimit += 32;
				int *pnNewPages = new int[nLimit];
				memcpy(pnNewPages, m_pnPages, sizeof(int) * m_nPrintPages);
				delete m_pnPages;
				m_pnPages = pnNewPages;
			}
			ASSERT(nLimit > m_nPrintPages);
			m_pnPages[m_nPrintPages ++] = nLine;
			y = m_rcPrintArea.top + nHeight;
		}
		nLine ++;
	}
}

void CCrystalTextView::OnBeginPrinting(CDC *pdc, CPrintInfo *pInfo)
{
	ASSERT(m_pnPages == NULL);
	ASSERT(m_pPrintFont == NULL);
	CFont *pDisplayFont = GetFont();

	LOGFONT lf;
	pDisplayFont->GetLogFont(&lf);

	CDC *pDisplayDC = GetDC();
	lf.lfHeight = MulDiv(lf.lfHeight, pdc->GetDeviceCaps(LOGPIXELSY), pDisplayDC->GetDeviceCaps(LOGPIXELSY) * 2);
	lf.lfWidth = MulDiv(lf.lfWidth, pdc->GetDeviceCaps(LOGPIXELSX), pDisplayDC->GetDeviceCaps(LOGPIXELSX) * 2);
	ReleaseDC(pDisplayDC);

	m_pPrintFont = new CFont;
	if (! m_pPrintFont->CreateFontIndirect(&lf))
	{
		delete m_pPrintFont;
		m_pPrintFont = NULL;
		return;
	}

	pdc->SelectObject(m_pPrintFont);
}

void CCrystalTextView::OnEndPrinting(CDC *pdc, CPrintInfo *pInfo)
{
	if (m_pPrintFont != NULL)
	{
		delete m_pPrintFont;
		m_pPrintFont = NULL;
	}
	if (m_pnPages != NULL)
	{
		delete m_pnPages;
		m_pnPages = NULL;
	}
	m_nPrintPages = 0;
	m_nPrintLineHeight = 0;
}

void CCrystalTextView::OnPrint(CDC* pdc, CPrintInfo* pInfo) 
{
	if (m_pnPages == NULL)
	{
		RecalcPageLayouts(pdc, pInfo);
		ASSERT(m_pnPages != NULL);
	}

	ASSERT(pInfo->m_nCurPage >= 1 && (int) pInfo->m_nCurPage <= m_nPrintPages);
	int nLine = m_pnPages[pInfo->m_nCurPage - 1];
	int nEndLine = GetLineCount();
	if ((int) pInfo->m_nCurPage < m_nPrintPages)
		nEndLine = m_pnPages[pInfo->m_nCurPage];
	TRACE(_T("Printing page %d of %d, lines %d - %d\n"), pInfo->m_nCurPage, m_nPrintPages,
						nLine, nEndLine - 1);

	if (m_bPrintHeader)
		PrintHeader(pdc, pInfo->m_nCurPage);
	if (m_bPrintFooter)
		PrintFooter(pdc, pInfo->m_nCurPage);

	int y = m_rcPrintArea.top;
	for (; nLine < nEndLine; nLine ++)
	{
		int nLineLength = GetLineLength(nLine);
		if (nLineLength == 0)
		{
			y += m_nPrintLineHeight;
			continue;
		}

		CRect rcPrintRect = m_rcPrintArea;
		rcPrintRect.top = y;
		LPCTSTR pszChars = GetLineChars(nLine);
		CString line;
		ExpandChars(pszChars, 0, nLineLength, line);
		y += pdc->DrawText(line, &rcPrintRect, DT_LEFT | DT_NOPREFIX | DT_TOP | DT_WORDBREAK);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextView message handlers

int CCrystalTextView::GetLineCount()
{
	if (m_pTextBuffer == NULL)
		return 1;		//	Single empty line
	int nLineCount = m_pTextBuffer->GetLineCount();
	ASSERT(nLineCount > 0);
	return nLineCount;
}

int CCrystalTextView::GetLineLength(int nLineIndex)
{
	if (m_pTextBuffer == NULL)
		return 0;
	return m_pTextBuffer->GetLineLength(nLineIndex);
}

LPCTSTR CCrystalTextView::GetLineChars(int nLineIndex)
{
	if (m_pTextBuffer == NULL)
		return NULL;
	return m_pTextBuffer->GetLineChars(nLineIndex);
}

void CCrystalTextView::AttachToBuffer(CCrystalTextBuffer *pBuf /*= NULL*/)
{
	if (m_pTextBuffer != NULL)
		m_pTextBuffer->RemoveView(this);
	if (pBuf == NULL)
	{
		pBuf = LocateTextBuffer();
		//	...
	}
	m_pTextBuffer = pBuf;
	if (m_pTextBuffer != NULL)
		m_pTextBuffer->AddView(this);
	ResetView();

	//	Init scrollbars
	CScrollBar *pVertScrollBarCtrl = GetScrollBarCtrl(SB_VERT);
	if (pVertScrollBarCtrl != NULL)
		pVertScrollBarCtrl->EnableScrollBar(GetScreenLines() >= GetLineCount() ?
											ESB_DISABLE_BOTH : ESB_ENABLE_BOTH);
	CScrollBar *pHorzScrollBarCtrl = GetScrollBarCtrl(SB_HORZ);
	if (pHorzScrollBarCtrl != NULL)
		pHorzScrollBarCtrl->EnableScrollBar(GetScreenChars() >= GetMaxLineLength() ?
											ESB_DISABLE_BOTH : ESB_ENABLE_BOTH);

	//	Update scrollbars
	RecalcVertScrollBar();
	RecalcHorzScrollBar();
}

void CCrystalTextView::DetachFromBuffer()
{
	if (m_pTextBuffer != NULL)
	{
		m_pTextBuffer->RemoveView(this);
		m_pTextBuffer = NULL;
		ResetView();
	}
}

int CCrystalTextView::GetScreenLines()
{
	if (m_nScreenLines == -1)
	{
		CRect rect;
		GetClientRect(&rect);
		m_nScreenLines = rect.Height() / GetLineHeight();
	}
	return m_nScreenLines;
}

BOOL CCrystalTextView::GetItalic(int nColorIndex)
{
	return FALSE;
}

BOOL CCrystalTextView::GetBold(int nColorIndex)
{
//	if (nColorIndex == COLORINDEX_KEYWORD)
//		return true;

	return FALSE;
}

int CCrystalTextView::GetScreenChars()
{
	if (m_nScreenChars == -1)
	{
		CRect rect;
		GetClientRect(&rect);
		m_nScreenChars = (rect.Width() - GetMarginWidth()) / GetCharWidth();
	}
	return m_nScreenChars;
}

void CCrystalTextView::OnDestroy() 
{
	DetachFromBuffer();
	m_hAccel = NULL;

	CView::OnDestroy();

	for (int I = 0; I < 4; I ++)
	{
		if (m_apFonts[I] != NULL)
		{
			m_apFonts[I]->DeleteObject();
			delete m_apFonts[I];
			m_apFonts[I] = NULL;
		}
	}
	if (m_pCacheBitmap != NULL)
	{
		delete m_pCacheBitmap;
		m_pCacheBitmap = NULL;
	}
}

BOOL CCrystalTextView::OnEraseBkgnd(CDC *pdc) 
{
	return TRUE;
}

void CCrystalTextView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (m_pCacheBitmap != NULL)
	{
		m_pCacheBitmap->DeleteObject();
		delete m_pCacheBitmap;
		m_pCacheBitmap = NULL;
	}
	m_nScreenLines = -1;
	m_nScreenChars = -1;
	RecalcVertScrollBar();
	RecalcHorzScrollBar();
}

void CCrystalTextView::UpdateSiblingScrollPos(BOOL bHorz)
{
	CSplitterWnd *pSplitterWnd = GetParentSplitter(this, FALSE);
	if (pSplitterWnd != NULL)
	{
		//	See CSplitterWnd::IdFromRowCol() implementation for details
		int nCurrentRow = (GetDlgCtrlID() - AFX_IDW_PANE_FIRST) / 16;
		int nCurrentCol = (GetDlgCtrlID() - AFX_IDW_PANE_FIRST) % 16;
		ASSERT(nCurrentRow >= 0 && nCurrentRow < pSplitterWnd->GetRowCount());
		ASSERT(nCurrentCol >= 0 && nCurrentCol < pSplitterWnd->GetColumnCount());

		if (bHorz)
		{
			int nCols = pSplitterWnd->GetColumnCount();
			for (int nCol = 0; nCol < nCols; nCol ++)
			{
				if (nCol != nCurrentCol)	//	We don't need to update ourselves
				{
					CCrystalTextView *pSiblingView = GetSiblingView(nCurrentRow, nCol);
					if (pSiblingView != NULL)
						pSiblingView->OnUpdateSibling(this, TRUE);
				}
			}
		}
		else
		{
			int nRows = pSplitterWnd->GetRowCount();
			for (int nRow = 0; nRow < nRows; nRow ++)
			{
				if (nRow != nCurrentRow)	//	We don't need to update ourselves
				{
					CCrystalTextView *pSiblingView = GetSiblingView(nRow, nCurrentCol);
					if (pSiblingView != NULL)
						pSiblingView->OnUpdateSibling(this, FALSE);
				}
			}
		}
	}
}

void CCrystalTextView::OnUpdateSibling(CCrystalTextView *pUpdateSource, BOOL bHorz)
{
	if (pUpdateSource != this)
	{
		ASSERT(pUpdateSource != NULL);
		ASSERT_KINDOF(CCrystalTextView, pUpdateSource);
		if (bHorz)
		{
			ASSERT(pUpdateSource->m_nTopLine >= 0);
			ASSERT(pUpdateSource->m_nTopLine < GetLineCount());
			if (pUpdateSource->m_nTopLine != m_nTopLine)
			{
				ScrollToLine(pUpdateSource->m_nTopLine, TRUE, FALSE);
				UpdateCaret();
			}
		}
		else
		{
			ASSERT(pUpdateSource->m_nOffsetChar >= 0);
			ASSERT(pUpdateSource->m_nOffsetChar < GetMaxLineLength());
			if (pUpdateSource->m_nOffsetChar != m_nOffsetChar)
			{
				ScrollToChar(pUpdateSource->m_nOffsetChar, TRUE, FALSE);
				UpdateCaret();
			}
		}
	}
}

void CCrystalTextView::RecalcVertScrollBar(BOOL bPositionOnly /*= FALSE*/)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	if (bPositionOnly)
	{
		si.fMask = SIF_POS;
//		si.nPos = m_nTopLine;
		// MiK
		si.nPos = CalcVisibleLines(0, m_nTopLine);
	}
	else
	{
		int nVisLines= CalcVisibleLines();
		if (GetScreenLines() >= nVisLines /*MiK GetLineCount()*/ && m_nTopLine > 0)
		{
			m_nTopLine = 0;
			Invalidate();
			UpdateCaret();
		}
		si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nMin = 0;
		si.nMax = nVisLines - 1;
		si.nPage = GetScreenLines();
		si.nPos = CalcVisibleLines(0, m_nTopLine);
	}
	VERIFY(SetScrollInfo(SB_VERT, &si));
}

void CCrystalTextView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CView::OnVScroll(nSBCode, nPos, pScrollBar);

	//	Note we cannot use nPos because of its 16-bit nature
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	VERIFY(GetScrollInfo(SB_VERT, &si));

	int nPageLines = GetScreenLines();
	int nLineCount = GetLineCount();

	int nNewTopLine= 0;
	bool bSmoothScrolling= false;

	switch (nSBCode)
	{
	case SB_TOP:
		nNewTopLine = 0;
		break;
	case SB_BOTTOM:
		nNewTopLine = FindVisibleLine(nLineCount - 1, -nPageLines + 1); //nLineCount - nPageLines + 1;
		break;
	case SB_LINEUP:
		nNewTopLine = FindVisibleLine(m_nTopLine, -1);
		break;
	case SB_LINEDOWN:
		nNewTopLine = FindVisibleLine(m_nTopLine, 1);
		break;
	case SB_PAGEUP:
		nNewTopLine = FindVisibleLine(m_nTopLine, -si.nPage + 1);
		break;
	case SB_PAGEDOWN:
		nNewTopLine = FindVisibleLine(m_nTopLine, si.nPage - 1);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nNewTopLine = FindVisibleLine(0, si.nTrackPos);
		break;
	case 98:	// MiK: mouse wheel
		nNewTopLine = FindVisibleLine(m_nTopLine, -3);
		bSmoothScrolling = true;
		break;
	case 99:
		nNewTopLine = FindVisibleLine(m_nTopLine, 3);
		bSmoothScrolling = true;
		break;
	default:
		return;
	}

	if (nNewTopLine < 0)
		nNewTopLine = 0;
	if (nNewTopLine >= nLineCount)
		nNewTopLine = nLineCount - 1;

	ScrollToLine(nNewTopLine, !bSmoothScrolling);
}

void CCrystalTextView::RecalcHorzScrollBar(BOOL bPositionOnly /*= FALSE*/)
{
	//	Again, we cannot use nPos because it's 16-bit
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	if (bPositionOnly)
	{
		si.fMask = SIF_POS;
		si.nPos = m_nOffsetChar;
	}
	else
	{
		if (GetScreenChars() >= GetMaxLineLength() && m_nOffsetChar > 0)
		{
			m_nOffsetChar = 0;
			Invalidate();
			UpdateCaret();
		}
		si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nMin = 0;
		si.nMax = GetMaxLineLength() - 1;
		si.nPage = GetScreenChars();
		si.nPos = m_nOffsetChar;
	}
	VERIFY(SetScrollInfo(SB_HORZ, &si));
}

void CCrystalTextView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CView::OnHScroll(nSBCode, nPos, pScrollBar);

	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	VERIFY(GetScrollInfo(SB_HORZ, &si));

	int nPageChars = GetScreenChars();
	int nMaxLineLength = GetMaxLineLength();

	int nNewOffset;
	switch (nSBCode)
	{
	case SB_LEFT:
		nNewOffset = 0;
		break;
	case SB_BOTTOM:
		nNewOffset = nMaxLineLength - nPageChars + 1;
		break;
	case SB_LINEUP:
		nNewOffset = m_nOffsetChar - 1;
		break;
	case SB_LINEDOWN:
		nNewOffset = m_nOffsetChar + 1;
		break;
	case SB_PAGEUP:
		nNewOffset = m_nOffsetChar - si.nPage + 1;
		break;
	case SB_PAGEDOWN:
		nNewOffset = m_nOffsetChar + si.nPage - 1;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nNewOffset = si.nTrackPos;
		break;
	default:
		return;
	}

	if (nNewOffset >= nMaxLineLength)
		nNewOffset = nMaxLineLength - 1;
	if (nNewOffset < 0)
		nNewOffset = 0;
	ScrollToChar(nNewOffset, TRUE);
	UpdateCaret();
}

BOOL CCrystalTextView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT)
	{
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		if (pt.x < GetMarginWidth())
		{
			::SetCursor(::LoadCursor(GetResourceHandle(), MAKEINTRESOURCE(IDR_MARGIN_CURSOR)));
		}
		else
		{
			CPoint ptText = ClientToText(pt);
			PrepareSelBounds();
			if (IsInsideSelBlock(ptText))
			{
				//	[JRT]:	Support For Disabling Drag and Drop...
				if (!m_bDisableDragAndDrop)				// If Drag And Drop Not Disabled
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));	// Set To Arrow Cursor
			}
			else
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
		}
		return TRUE;
	}
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

CPoint CCrystalTextView::ClientToText(const CPoint &point)
{
	int nLineCount = GetLineCount();

	CPoint pt;
	pt.y = FindVisibleLine(m_nTopLine, point.y / GetLineHeight());
	if (pt.y >= nLineCount)
		pt.y = nLineCount - 1;
	if (pt.y < 0)
		pt.y = 0;

	int nLength = 0;
	LPCTSTR pszLine = NULL;
	if (pt.y >= 0 && pt.y < nLineCount)
	{
		nLength = GetLineLength(pt.y);
		pszLine = GetLineChars(pt.y);
	}

	int nPos = m_nOffsetChar + (point.x - GetMarginWidth()) / GetCharWidth();
	if (nPos < 0)
		nPos = 0;

	int nIndex = 0, nCurPos = 0;
	int nTabSize = GetTabSize();
	while (nIndex < nLength)
	{
		if (pszLine[nIndex] == _T('\t'))
			nCurPos += (nTabSize - nCurPos % nTabSize);
		else
			nCurPos ++;

		if (nCurPos > nPos)
			break;

		nIndex ++;
	}

	ASSERT(nIndex >= 0 && nIndex <= nLength);
	pt.x = nIndex;
	return pt;
}

#ifdef _DEBUG
void CCrystalTextView::AssertValidTextPos(const CPoint &point)
{
	if (GetLineCount() > 0)
	{
		ASSERT(m_nTopLine >= 0 && m_nOffsetChar >= 0);
		ASSERT(point.y >= 0 && point.y < GetLineCount());
		ASSERT(point.x >= 0 && point.x <= GetLineLength(point.y));
	}
}
#endif

CPoint CCrystalTextView::TextToClient(const CPoint &point)
{
	ASSERT_VALIDTEXTPOS(point);
	int nLength = GetLineLength(point.y);
	LPCTSTR pszLine = GetLineChars(point.y);

	CPoint pt;
	pt.y = CalcVisibleLines(m_nTopLine, point.y) * GetLineHeight();
	pt.x = 0;
	int nTabSize = GetTabSize();
	for (int nIndex = 0; nIndex < point.x; nIndex ++)
	{
		if (pszLine[nIndex] == _T('\t'))
			pt.x += (nTabSize - pt.x % nTabSize);
		else
			pt.x ++;
	}

	pt.x = (pt.x - m_nOffsetChar) * GetCharWidth() + GetMarginWidth();
	return pt;
}

void CCrystalTextView::InvalidateLines(int nLine1, int nLine2, BOOL bInvalidateMargin /*= FALSE*/)
{
	bInvalidateMargin = TRUE;
	if (nLine2 == -1)
	{
		CRect rcInvalid;
		GetClientRect(&rcInvalid);
		if (! bInvalidateMargin)
			rcInvalid.left += GetMarginWidth();
//		rcInvalid.top = (nLine1 - m_nTopLine) * GetLineHeight();
		rcInvalid.top = CalcVisibleLines(m_nTopLine, nLine1) * GetLineHeight();
		InvalidateRect(&rcInvalid, FALSE);
	}
	else
	{
		if (nLine2 < nLine1)
		{
			int nTemp = nLine1;
			nLine1 = nLine2;
			nLine2 = nTemp;
		}
		CRect rcInvalid;
		GetClientRect(&rcInvalid);
		if (! bInvalidateMargin)
			rcInvalid.left += GetMarginWidth();
		rcInvalid.top = CalcVisibleLines(m_nTopLine, nLine1) * GetLineHeight();
		rcInvalid.bottom = (CalcVisibleLines(m_nTopLine, nLine2) + 1) * GetLineHeight();
		InvalidateRect(&rcInvalid, FALSE);
	}
}

void CCrystalTextView::SetSelection(const CPoint &ptStart, const CPoint &ptEnd)
{
	ASSERT_VALIDTEXTPOS(ptStart);
	ASSERT_VALIDTEXTPOS(ptEnd);
	if (m_ptSelStart == ptStart)
	{
		if (m_ptSelEnd != ptEnd)
			InvalidateLines(ptEnd.y, m_ptSelEnd.y);
	}
	else
	{
		InvalidateLines(ptStart.y, ptEnd.y);
		InvalidateLines(m_ptSelStart.y, m_ptSelEnd.y);
	}
	m_ptSelStart = ptStart;
	m_ptSelEnd = ptEnd;
}

void CCrystalTextView::AdjustTextPoint(CPoint &point)
{
	point.x += GetCharWidth() / 2;	//todo
}

void CCrystalTextView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);

	m_bFocused = TRUE;
	if (m_ptSelStart != m_ptSelEnd)
		InvalidateLines(m_ptSelStart.y, m_ptSelEnd.y);
	UpdateCaret();
}

DWORD CCrystalTextView::ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems)
{
	return 0;
}

int CCrystalTextView::CalculateActualOffset(int nLineIndex, int nCharIndex)
{
//	ASSERT(!IsLineHidden(nLineIndex));
	int nLength = GetLineLength(nLineIndex);
	ASSERT(nCharIndex >= 0 && nCharIndex <= nLength);
	LPCTSTR pszChars = GetLineChars(nLineIndex);
	int nOffset = 0;
	int nTabSize = GetTabSize();
	for (int I = 0; I < nCharIndex; I ++)
	{
		if (pszChars[I] == _T('\t'))
			nOffset += (nTabSize - nOffset % nTabSize);
		else
			nOffset ++;
	}
	return nOffset;
}

int CCrystalTextView::ApproxActualOffset(int nLineIndex, int nOffset)
{
	if (nOffset == 0)
		return 0;

	int nLength = GetLineLength(nLineIndex);
	LPCTSTR pszChars = GetLineChars(nLineIndex);
	int nCurrentOffset = 0;
	int nTabSize = GetTabSize();
	for (int I = 0; I < nLength; I ++)
	{
		if (pszChars[I] == _T('\t'))
			nCurrentOffset += (nTabSize - nCurrentOffset % nTabSize);
		else
			nCurrentOffset ++;
		if (nCurrentOffset >= nOffset)
		{
			if (nOffset <= nCurrentOffset - nTabSize / 2)
				return I;
			return I + 1;
		}
	}
	return nLength;
}

void CCrystalTextView::EnsureVisible(CPoint pt)
{
	ASSERT(!IsLineHidden(pt.y));

	//	Scroll vertically
	int nLineCount = GetLineCount();
	int nNewTopLine = m_nTopLine;
	// MiK: bottom line (last visible)
	int nBottomLine= FindVisibleLine(nNewTopLine, GetScreenLines() - 1);
	if (pt.y > nBottomLine) // MiK (pt.y >= nNewTopLine + GetScreenLines())
	{
		nNewTopLine = FindVisibleLine(pt.y, -GetScreenLines() + 1); //MiK pt.y - GetScreenLines() + 1;
	}
	if (pt.y < nNewTopLine)
	{
		nNewTopLine = pt.y;
	}

	if (nNewTopLine < 0)
		nNewTopLine = 0;
	if (nNewTopLine >= nLineCount)
		nNewTopLine = nLineCount - 1;

	if (m_nTopLine != nNewTopLine)
	{
		ScrollToLine(nNewTopLine);
		UpdateSiblingScrollPos(TRUE);
	}

	//	Scroll horizontally
	int nActualPos = CalculateActualOffset(pt.y, pt.x);
	int nNewOffset = m_nOffsetChar;
	if (nActualPos > nNewOffset + GetScreenChars())
	{
		nNewOffset = nActualPos - GetScreenChars();
	}
	if (nActualPos < nNewOffset)
	{
		nNewOffset = nActualPos;
	}

	if (nNewOffset >= GetMaxLineLength())
		nNewOffset = GetMaxLineLength() - 1;
	if (nNewOffset < 0)
		nNewOffset = 0;

	if (m_nOffsetChar != nNewOffset)
	{
		ScrollToChar(nNewOffset);
		UpdateCaret();
		UpdateSiblingScrollPos(FALSE);
	}
}

void CCrystalTextView::OnKillFocus(CWnd* pNewWnd) 
{
	CView::OnKillFocus(pNewWnd);

	m_bFocused = FALSE;
	UpdateCaret();
	if (m_ptSelStart != m_ptSelEnd)
		InvalidateLines(m_ptSelStart.y, m_ptSelEnd.y);
	if (m_bDragSelection)
	{
		ReleaseCapture();
		KillTimer(m_nDragSelTimer);
		m_bDragSelection = FALSE;
	}
}

void CCrystalTextView::OnSysColorChange() 
{
	CView::OnSysColorChange();
	Invalidate();
}

void CCrystalTextView::GetText(const CPoint &ptStart, const CPoint &ptEnd, CString &text)
{
	if (m_pTextBuffer != NULL)
		m_pTextBuffer->GetText(ptStart.y, ptStart.x, ptEnd.y, ptEnd.x, text);
	else
		text = _T("");
}

CString CCrystalTextView::GetCurLine()
{
	int nLine= m_ptCursorPos.y;

	if (m_pTextBuffer == 0 || nLine >= GetLineCount())
		return _T("");

	int nLength= GetLineLength(nLine);
	if (nLength == 0)
		return _T("");

	CString str;
	m_pTextBuffer->GetText(nLine, 0, nLine, nLength, str);
	return str;
}


void CCrystalTextView::UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext,
								  DWORD dwFlags, int nLineIndex /*= -1*/)
{
	if (dwFlags & UPDATE_RESET)
	{
		ResetView();
		RecalcVertScrollBar();
		RecalcHorzScrollBar();
		// MiK: parse all to update collapsible blocks
		GetParseCookie(GetLineCount() - 1);
		NotifyTextChanged();
		return;
	}

	int nLineCount = GetLineCount();
	ASSERT(nLineCount > 0);
	ASSERT(nLineIndex >= -1 && nLineIndex < nLineCount);
	if ((dwFlags & UPDATE_SINGLELINE) != 0)
	{
		ASSERT(nLineIndex != -1);
		//	All text below this line should be reparsed
		if (m_pdwParseCookies != NULL)
		{
			ASSERT(m_nParseArraySize == nLineCount);
			memset(m_pdwParseCookies + nLineIndex, 0xff, sizeof(DWORD) * (m_nParseArraySize - nLineIndex));
		}
		//	This line'th actual length must be recalculated
		if (m_pnActualLineLength != NULL)
		{
			ASSERT(m_nActualLengthArraySize == nLineCount);
			m_pnActualLineLength[nLineIndex] = -1;
		}
		//	Repaint the lines
		InvalidateLines(nLineIndex, -1, TRUE);
	}
	else
	{
		if (nLineIndex == -1)
			nLineIndex = 0;		//	Refresh all text
		//	All text below this line should be reparsed
		if (m_pdwParseCookies != NULL)
		{
			if (m_nParseArraySize != nLineCount)
			{
				//	Reallocate cookies array
				DWORD *pdwNewArray = new DWORD[nLineCount];
				if (nLineIndex > 0)
					memcpy(pdwNewArray, m_pdwParseCookies, sizeof(DWORD) * nLineIndex);
				delete m_pdwParseCookies;
				m_nParseArraySize = nLineCount;
				m_pdwParseCookies = pdwNewArray;
			}
			memset(m_pdwParseCookies + nLineIndex, 0xff, sizeof(DWORD) * (m_nParseArraySize - nLineIndex));
		}
		//	Recalculate actual length for all lines below this
		if (m_pnActualLineLength != NULL)
		{
			if (m_nActualLengthArraySize != nLineCount)
			{
				//	Reallocate actual length array
				int *pnNewArray = new int[nLineCount];
				if (nLineIndex > 0)
					memcpy(pnNewArray, m_pnActualLineLength, sizeof(int) * nLineIndex);
				delete m_pnActualLineLength;
				m_nActualLengthArraySize = nLineCount;
				m_pnActualLineLength = pnNewArray;
			}
			memset(m_pnActualLineLength + nLineIndex, 0xff, sizeof(DWORD) * (m_nActualLengthArraySize - nLineIndex));
		}
		//	Repaint the lines
		InvalidateLines(nLineIndex, -1, TRUE);
	}

	//	All those points must be recalculated and validated
	if (pContext != NULL)
	{
		pContext->RecalcPoint(m_ptCursorPos);
		pContext->RecalcPoint(m_ptSelStart);
		pContext->RecalcPoint(m_ptSelEnd);
		pContext->RecalcPoint(m_ptAnchor);
		ASSERT_VALIDTEXTPOS(m_ptCursorPos);
		ASSERT_VALIDTEXTPOS(m_ptSelStart);
		ASSERT_VALIDTEXTPOS(m_ptSelEnd);
		ASSERT_VALIDTEXTPOS(m_ptAnchor);
		if (m_bDraggingText)
		{
			pContext->RecalcPoint(m_ptDraggedTextBegin);
			pContext->RecalcPoint(m_ptDraggedTextEnd);
			ASSERT_VALIDTEXTPOS(m_ptDraggedTextBegin);
			ASSERT_VALIDTEXTPOS(m_ptDraggedTextEnd);
		}
		CPoint ptTopLine(0, m_nTopLine);
		pContext->RecalcPoint(ptTopLine);
		ASSERT_VALIDTEXTPOS(ptTopLine);
		m_nTopLine = ptTopLine.y;
		UpdateCaret();
	}

	//	Recalculate vertical scrollbar, if needed
	if ((dwFlags & UPDATE_VERTRANGE) != 0)
	{
		if (! m_bVertScrollBarLocked)
			RecalcVertScrollBar();
	}

	//	Recalculate horizontal scrollbar, if needed
	if ((dwFlags & UPDATE_HORZRANGE) != 0)
	{
		m_nMaxLineLength = -1;
		if (! m_bHorzScrollBarLocked)
			RecalcHorzScrollBar();
	}

	if (dwFlags & (UPDATE_SINGLELINE | UPDATE_VERTRANGE))
		NotifyTextChanged();
}

HINSTANCE CCrystalTextView::GetResourceHandle()
{
#ifdef CRYSEDIT_RES_HANDLE
	return CRYSEDIT_RES_HANDLE;
#else
	if (s_hResourceInst != NULL)
		return s_hResourceInst;
	return AfxGetResourceHandle();
#endif
}

int CCrystalTextView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Note: CSampleDoc overwrites this logfont
	memset(&m_lfBaseFont, 0, sizeof(m_lfBaseFont));
	lstrcpy(m_lfBaseFont.lfFaceName, _T("Atari Mono"));
	m_lfBaseFont.lfHeight = 0;
	m_lfBaseFont.lfWeight = FW_NORMAL;
	m_lfBaseFont.lfItalic = FALSE;
	m_lfBaseFont.lfCharSet = DEFAULT_CHARSET;
	m_lfBaseFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	m_lfBaseFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_lfBaseFont.lfQuality = DEFAULT_QUALITY;
	m_lfBaseFont.lfPitchAndFamily = DEFAULT_PITCH;

	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ASSERT(m_hAccel == NULL);
	m_hAccel = ::LoadAccelerators(GetResourceHandle(), MAKEINTRESOURCE(IDR_DEFAULT_ACCEL));
	ASSERT(m_hAccel != NULL);
	return 0;
}

void CCrystalTextView::SetAnchor(const CPoint &ptNewAnchor)
{
	ASSERT_VALIDTEXTPOS(ptNewAnchor);
	m_ptAnchor = ptNewAnchor;
}

void CCrystalTextView::OnEditOperation(int nAction, LPCTSTR pszText)
{
}

BOOL CCrystalTextView::PreTranslateMessage(MSG *pMsg) 
{
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		if (m_hAccel != NULL)
		{
			if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
				return TRUE;
		}
	}

	return CView::PreTranslateMessage(pMsg);
}

CPoint CCrystalTextView::GetCursorPos()
{
	return m_ptCursorPos;
}

void CCrystalTextView::SetCursorPos(const CPoint &ptCursorPos)
{
	ASSERT_VALIDTEXTPOS(ptCursorPos);
	m_ptCursorPos = ptCursorPos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	UpdateCaret();
}

void CCrystalTextView::SetSelectionMargin(BOOL bSelMargin)
{
	if (m_bSelMargin != bSelMargin)
	{
		m_bSelMargin = bSelMargin;
		if (::IsWindow(m_hWnd))
		{
			m_nScreenChars = -1;
			Invalidate();
			RecalcHorzScrollBar();
		}
	}
}

void CCrystalTextView::GetFont(LOGFONT &lf)
{
	lf = m_lfBaseFont;
}

void CCrystalTextView::SetFont(const LOGFONT &lf)
{
	m_lfBaseFont = lf;
	m_nScreenLines = -1;
	m_nScreenChars = -1;
	m_nCharWidth = -1;
	m_nLineHeight = -1;
	if (m_pCacheBitmap != NULL)
	{
		m_pCacheBitmap->DeleteObject();
		delete m_pCacheBitmap;
		m_pCacheBitmap = NULL;
	}
	for (int I = 0; I < 4; I ++)
	{
		if (m_apFonts[I] != NULL)
		{
			m_apFonts[I]->DeleteObject();
			delete m_apFonts[I];
			m_apFonts[I] = NULL;
		}
	}
	if (::IsWindow(m_hWnd))
	{
		RecalcVertScrollBar();
		RecalcHorzScrollBar();
		UpdateCaret();
		Invalidate();
	}
}

void CCrystalTextView::OnUpdateIndicatorPosition(CCmdUI* pCmdUI)
{
	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
	CString stat;
	int nXPos= CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	stat.Format(_T("Ln %d, Col %d"), m_ptCursorPos.y + 1, nXPos + 1);
	pCmdUI->SetText(stat);
}

void CCrystalTextView::OnUpdateIndicatorCRLF(CCmdUI* pCmdUI)
{
	if (m_pTextBuffer != NULL)
	{
		int crlfMode = m_pTextBuffer->GetCRLFMode();
		switch (crlfMode)
		{
		case CRLF_STYLE_DOS:
			pCmdUI->SetText(_T("DOS"));
			pCmdUI->Enable(TRUE);
			break;
		case CRLF_STYLE_UNIX:
			pCmdUI->SetText(_T("UNIX"));
			pCmdUI->Enable(TRUE);
			break;
		case CRLF_STYLE_MAC:
			pCmdUI->SetText(_T("MAC"));
			pCmdUI->Enable(TRUE);
			break;
		default:
			pCmdUI->SetText(NULL);
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->SetText(NULL);
		pCmdUI->Enable(FALSE);
	}
}

void CCrystalTextView::OnToggleBookmark(UINT nCmdID)
{
	int nBookmarkID = nCmdID - ID_EDIT_TOGGLE_BOOKMARK0;
	ASSERT(nBookmarkID >= 0 && nBookmarkID <= 9);
	if (m_pTextBuffer != NULL)
	{
		DWORD dwFlags = GetLineFlags(m_ptCursorPos.y);
		DWORD dwMask = LF_BOOKMARK(nBookmarkID);
		m_pTextBuffer->SetLineFlag(m_ptCursorPos.y, dwMask, (dwFlags & dwMask) == 0);
	}
}

void CCrystalTextView::OnGoBookmark(UINT nCmdID)
{
	int nBookmarkID = nCmdID - ID_EDIT_GO_BOOKMARK0;
	ASSERT(nBookmarkID >= 0 && nBookmarkID <= 9);
	if (m_pTextBuffer != NULL)
	{
		int nLine = m_pTextBuffer->GetLineWithFlag(LF_BOOKMARK(nBookmarkID));
		if (nLine >= 0)
			GoToLine(nLine);
	}
}

void CCrystalTextView::OnClearBookmarks()
{
	if (m_pTextBuffer != NULL)
	{
		for (int nBookmarkID = 0; nBookmarkID <= 9; nBookmarkID++)
		{
			int nLine = m_pTextBuffer->GetLineWithFlag(LF_BOOKMARK(nBookmarkID));
			if (nLine >= 0)
			{
				m_pTextBuffer->SetLineFlag(nLine, LF_BOOKMARK(nBookmarkID), FALSE);
			}
		}
		
	}
}

void CCrystalTextView::ShowCursor()
{
	m_bCursorHidden = FALSE;
	UpdateCaret();
}

void CCrystalTextView::HideCursor()
{
	m_bCursorHidden = TRUE;
	UpdateCaret();
}

DROPEFFECT CCrystalTextView::GetDropEffect()
{
	return DROPEFFECT_COPY;
}

void CCrystalTextView::OnDropSource(DROPEFFECT de)
{
	ASSERT(de == DROPEFFECT_COPY);
}

HGLOBAL CCrystalTextView::PrepareDragData()
{
	PrepareSelBounds();
	if (m_ptDrawSelStart == m_ptDrawSelEnd)
		return NULL;

	CString text;
	GetText(m_ptDrawSelStart, m_ptDrawSelEnd, text);
	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, lstrlen(text) + 1);
	if (hData == NULL)
		return NULL;

	LPSTR pszData = (LPSTR) ::GlobalLock(hData);
	USES_CONVERSION;
	strcpy(pszData, T2A(text.GetBuffer(0)));
	text.ReleaseBuffer();
	::GlobalUnlock(hData);

	m_ptDraggedTextBegin = m_ptDrawSelStart;
	m_ptDraggedTextEnd = m_ptDrawSelEnd;
	return hData;
}

static int FindStringHelper(LPCTSTR pszFindWhere, LPCTSTR pszFindWhat, BOOL bWholeWord)
{
	ASSERT(pszFindWhere != NULL);
	ASSERT(pszFindWhat != NULL);
	int nCur = 0;
	int nLength = lstrlen(pszFindWhat);
	for (;;)
	{
#ifdef _UNICODE
		LPCTSTR pszPos = wcsstr(pszFindWhere, pszFindWhat);
#else
		LPCTSTR pszPos = strstr(pszFindWhere, pszFindWhat);
#endif
		if (pszPos == NULL)
			return -1;
		if (! bWholeWord)
			return nCur + (pszPos - pszFindWhere);
		if (pszPos > pszFindWhere && (isalnum(pszPos[-1]) || pszPos[-1] == _T('_')))
		{
			nCur += (pszPos - pszFindWhere);
			pszFindWhere = pszPos + 1;
			continue;
		}
		if (isalnum(pszPos[nLength]) || pszPos[nLength] == _T('_'))
		{
			nCur += (pszPos - pszFindWhere + 1);
			pszFindWhere = pszPos + 1;
			continue;
		}
		return nCur + (pszPos - pszFindWhere);
	}
	ASSERT(FALSE);		// Unreachable
	return -1;
}


BOOL CCrystalTextView::HighlightText(const CPoint &ptStartPos, int nLength)
{
	ASSERT_VALIDTEXTPOS(ptStartPos);

	if (IsLineHidden(ptStartPos.y))			// MiK: expand block if line is hidden
		CollapseBlock(ptStartPos.y);

	m_ptCursorPos = ptStartPos;
	m_ptCursorPos.x += nLength;
	ASSERT_VALIDTEXTPOS(m_ptCursorPos);		//	Probably 'nLength' is bigger than expected...
	m_ptAnchor = m_ptCursorPos;
	SetSelection(ptStartPos, m_ptCursorPos);
	UpdateCaret();
	EnsureVisible(m_ptCursorPos);
	return TRUE;
}

BOOL CCrystalTextView::FindText(LPCTSTR pszText, const CPoint &ptStartPos, DWORD dwFlags, 
								BOOL bWrapSearch, CPoint *pptFoundPos)
{
	int nLineCount = GetLineCount();
	return FindTextInBlock(pszText, ptStartPos, CPoint(0, 0),
			CPoint(GetLineLength(nLineCount - 1), nLineCount - 1),
			dwFlags, bWrapSearch, pptFoundPos);
}

BOOL CCrystalTextView::FindTextInBlock(LPCTSTR pszText, const CPoint &ptStartPosition, 
									   const CPoint &ptBlockBegin, const CPoint &ptBlockEnd,
										DWORD dwFlags, BOOL bWrapSearch, CPoint *pptFoundPos)
{
	CPoint ptCurrentPos = ptStartPosition;

	ASSERT(pszText != NULL && lstrlen(pszText) > 0);
	ASSERT_VALIDTEXTPOS(ptCurrentPos);
	ASSERT_VALIDTEXTPOS(ptBlockBegin);
	ASSERT_VALIDTEXTPOS(ptBlockEnd);
	ASSERT(ptBlockBegin.y < ptBlockEnd.y || ptBlockBegin.y == ptBlockEnd.y && 
		ptBlockBegin.x <= ptBlockEnd.x);
	if (ptBlockBegin == ptBlockEnd)
		return FALSE;

	if (ptCurrentPos.y < ptBlockBegin.y || ptCurrentPos.y == ptBlockBegin.y && 
		ptCurrentPos.x < ptBlockBegin.x)
		ptCurrentPos = ptBlockBegin;

	CString what = pszText;
	if ((dwFlags & FIND_MATCH_CASE) == 0)
		what.MakeUpper();

	if (dwFlags & FIND_DIRECTION_UP)
	{
		//	Let's check if we deal with whole text.
		//	At this point, we cannot search *up* in selection
		ASSERT(ptBlockBegin.x == 0 && ptBlockBegin.y == 0);
		ASSERT(ptBlockEnd.x == GetLineLength(GetLineCount() - 1) &&
				ptBlockEnd.y == GetLineCount() - 1);

		//	Proceed as if we have whole text search.
		for (;;)
		{
			while (ptCurrentPos.y >= 0)
			{
				int nLineLength = GetLineLength(ptCurrentPos.y);
				nLineLength -= ptCurrentPos.x;
				if (nLineLength <= 0)
				{
					ptCurrentPos.x = 0;
					ptCurrentPos.y --;
					continue;
				}

				LPCTSTR pszChars = GetLineChars(ptCurrentPos.y);
				pszChars += ptCurrentPos.x;

				CString line;
				lstrcpyn(line.GetBuffer(nLineLength + 1), pszChars, nLineLength + 1);
				line.ReleaseBuffer();
				if ((dwFlags & FIND_MATCH_CASE) == 0)
					line.MakeUpper();

				int nPos = ::FindStringHelper(line, what, (dwFlags & FIND_WHOLE_WORD) != 0);
				if (nPos >= 0)		//	Found text!
				{
					ptCurrentPos.x += nPos;
					*pptFoundPos = ptCurrentPos;
					return TRUE;
				}

				ptCurrentPos.x = 0;
				ptCurrentPos.y --;
			}

			//	Beginning of text reached
			if (! bWrapSearch)
				return FALSE;

			//	Start again from the end of text
			bWrapSearch = FALSE;
			ptCurrentPos = CPoint(0, GetLineCount() - 1);
		}
	}
	else
	{
		for (;;)
		{
			while (ptCurrentPos.y <= ptBlockEnd.y)
			{
				int nLineLength = GetLineLength(ptCurrentPos.y);
				nLineLength -= ptCurrentPos.x;
				if (nLineLength <= 0)
				{
					ptCurrentPos.x = 0;
					ptCurrentPos.y ++;
					continue;
				}

				LPCTSTR pszChars = GetLineChars(ptCurrentPos.y);
				pszChars += ptCurrentPos.x;

				//	Prepare necessary part of line
				CString line;
				lstrcpyn(line.GetBuffer(nLineLength + 1), pszChars, nLineLength + 1);
				line.ReleaseBuffer();
				if ((dwFlags & FIND_MATCH_CASE) == 0)
					line.MakeUpper();

				//	Perform search in the line
				int nPos = ::FindStringHelper(line, what, (dwFlags & FIND_WHOLE_WORD) != 0);
				if (nPos >= 0)
				{
					ptCurrentPos.x += nPos;
					//	Check of the text found is outside the block.
					if (ptCurrentPos.y == ptBlockEnd.y && ptCurrentPos.x >= ptBlockEnd.x)
						break;

					*pptFoundPos = ptCurrentPos;
					return TRUE;
				}

				//	Go further, text was not found
				ptCurrentPos.x = 0;
				ptCurrentPos.y ++;
			}

			//	End of text reached
			if (! bWrapSearch)
				return FALSE;

			//	Start from the beginning
			bWrapSearch = FALSE;
			ptCurrentPos = ptBlockBegin;
		}
	}

	ASSERT(FALSE);		// Unreachable
	return FALSE;
}

void CCrystalTextView::OnEditFind() 
{
	CWinApp *pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	CFindTextDlg dlg(this);
	if (m_bLastSearch)
	{
		//	Get the latest search parameters
		dlg.m_bMatchCase = (m_dwLastSearchFlags & FIND_MATCH_CASE) != 0;
		dlg.m_bWholeWord = (m_dwLastSearchFlags & FIND_WHOLE_WORD) != 0;
		dlg.m_nDirection = (m_dwLastSearchFlags & FIND_DIRECTION_UP) != 0 ? 0 : 1;
		if (m_pszLastFindWhat != NULL)
			dlg.m_sText = m_pszLastFindWhat;
	}
	else
	{
		//	Take search parameters from registry
		dlg.m_bMatchCase = pApp->GetProfileInt(REG_FIND_SUBKEY, REG_MATCH_CASE, FALSE);
		dlg.m_bWholeWord = pApp->GetProfileInt(REG_FIND_SUBKEY, REG_WHOLE_WORD, FALSE);
		dlg.m_nDirection = 1;		//	Search down
		dlg.m_sText = pApp->GetProfileString(REG_FIND_SUBKEY, REG_FIND_WHAT, _T(""));
	}

	//	Take the current selection, if any
	if (IsSelection())
	{
		CPoint ptSelStart, ptSelEnd;
		GetSelection(ptSelStart, ptSelEnd);		if (ptSelStart.y == ptSelEnd.y)
		{
			LPCTSTR pszChars = GetLineChars(ptSelStart.y);
			int nChars = ptSelEnd.x - ptSelStart.x;
			lstrcpyn(dlg.m_sText.GetBuffer(nChars + 1), pszChars + ptSelStart.x, nChars + 1);
			dlg.m_sText.ReleaseBuffer();
		}
	}

	//	Execute Find dialog
	dlg.m_ptCurrentPos = m_ptCursorPos;		//	Search from cursor position
	m_bShowInactiveSelection = TRUE;
	dlg.DoModal();
	m_bShowInactiveSelection = FALSE;

	//	Save search parameters for 'F3' command
	m_bLastSearch = TRUE;
	if (m_pszLastFindWhat != NULL)
		free(m_pszLastFindWhat);
#ifdef _UNICODE
	m_pszLastFindWhat = _wcsdup(dlg.m_sText);
#else
	m_pszLastFindWhat = strdup(dlg.m_sText);
#endif

	m_dwLastSearchFlags = 0;
	if (dlg.m_bMatchCase)
		m_dwLastSearchFlags |= FIND_MATCH_CASE;
	if (dlg.m_bWholeWord)
		m_dwLastSearchFlags |= FIND_WHOLE_WORD;
	if (dlg.m_nDirection == 0)
		m_dwLastSearchFlags |= FIND_DIRECTION_UP;

	//	Save search parameters to registry
	pApp->WriteProfileInt(REG_FIND_SUBKEY, REG_MATCH_CASE, dlg.m_bMatchCase);
	pApp->WriteProfileInt(REG_FIND_SUBKEY, REG_WHOLE_WORD, dlg.m_bWholeWord);
	pApp->WriteProfileString(REG_FIND_SUBKEY, REG_FIND_WHAT, dlg.m_sText);
}

void CCrystalTextView::OnEditRepeat()
{
	if (m_bLastSearch)
	{
		CPoint ptFoundPos;
		if (!FindText(m_pszLastFindWhat, m_ptCursorPos, m_dwLastSearchFlags, TRUE, &ptFoundPos))
		{
			CString prompt;
			prompt.Format(IDS_EDIT_TEXT_NOT_FOUND, m_pszLastFindWhat);
			AfxMessageBox(prompt);
			return;
		}
		HighlightText(ptFoundPos, lstrlen(m_pszLastFindWhat));
		m_bMultipleSearch = TRUE;       // More search       
	}
}

void CCrystalTextView::OnUpdateEditRepeat(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bLastSearch);
}

void CCrystalTextView::OnEditFindPrevious() 
{
	DWORD dwSaveSearchFlags = m_dwLastSearchFlags;
	if ((m_dwLastSearchFlags & FIND_DIRECTION_UP) != 0)
		m_dwLastSearchFlags &= ~FIND_DIRECTION_UP;
	else
		m_dwLastSearchFlags |= FIND_DIRECTION_UP;
	OnEditRepeat();
	m_dwLastSearchFlags = dwSaveSearchFlags;
}

void CCrystalTextView::OnUpdateEditFindPrevious(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bLastSearch);
}

void CCrystalTextView::OnFilePageSetup()
{
	CWinApp *pApp = AfxGetApp();
	ASSERT(pApp != NULL);

	CPageSetupDialog dlg;
	dlg.m_psd.Flags &= ~PSD_INTHOUSANDTHSOFINCHES;
	dlg.m_psd.Flags |= PSD_INHUNDREDTHSOFMILLIMETERS | PSD_DISABLEORIENTATION | PSD_DISABLEPAPER;
	dlg.m_psd.rtMargin.left = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_LEFT, DEFAULT_PRINT_MARGIN);
	dlg.m_psd.rtMargin.right = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_RIGHT, DEFAULT_PRINT_MARGIN);
	dlg.m_psd.rtMargin.top = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_TOP, DEFAULT_PRINT_MARGIN);
	dlg.m_psd.rtMargin.bottom = pApp->GetProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_BOTTOM, DEFAULT_PRINT_MARGIN);
	if (dlg.DoModal() == IDOK)
	{
		pApp->WriteProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_LEFT, dlg.m_psd.rtMargin.left);
		pApp->WriteProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_RIGHT, dlg.m_psd.rtMargin.right);
		pApp->WriteProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_TOP, dlg.m_psd.rtMargin.top);
		pApp->WriteProfileInt(REG_PAGE_SUBKEY, REG_MARGIN_BOTTOM, dlg.m_psd.rtMargin.bottom);
	}
}

void CCrystalTextView::OnToggleBookmark()
{
	if (m_pTextBuffer != NULL)
	{
		DWORD dwFlags = GetLineFlags(m_ptCursorPos.y);
		DWORD dwMask  = LF_BOOKMARKS;
		m_pTextBuffer->SetLineFlag(m_ptCursorPos.y, dwMask, (dwFlags & dwMask) == 0, FALSE);
	}
	int nLine = m_pTextBuffer->GetLineWithFlag(LF_BOOKMARKS);
	if (nLine >= 0)
		m_bBookmarkExist = TRUE;
	else
		m_bBookmarkExist = FALSE;
}


// MiK: move cursor to given line (expand block if this line is hidden)
//
void CCrystalTextView::GoToLine(int nLineIndex, int nCharOffset/*= 0*/)
{
	ASSERT(nLineIndex >= 0);

	if (nLineIndex >= 0)
	{
		if (IsLineHidden(nLineIndex))
			CollapseBlock(nLineIndex);

		CPoint pt(nCharOffset, nLineIndex);
		ASSERT_VALIDTEXTPOS(pt);
		SetCursorPos(pt);
		SetSelection(pt, pt);
		SetAnchor(pt);
		EnsureVisible(pt);
	}
}


void CCrystalTextView::OnNextBookmark()
{
	if (m_pTextBuffer != NULL)
	{
		int nLine = m_pTextBuffer->FindNextBookmarkLine(m_ptCursorPos.y);
		if (nLine >= 0)
			GoToLine(nLine);
	}
}

void CCrystalTextView::OnPrevBookmark()
{
	if (m_pTextBuffer != NULL)
	{
		int nLine = m_pTextBuffer->FindPrevBookmarkLine(m_ptCursorPos.y);
		if (nLine >= 0)
			GoToLine(nLine);
	}
}

void CCrystalTextView::OnClearAllBookmarks()
{
	if (m_pTextBuffer != NULL)
	{
		int nLineCount = GetLineCount();
		for (int I = 0; I < nLineCount; I ++)
		{
			if (m_pTextBuffer->GetLineFlags(I) & LF_BOOKMARKS)
				m_pTextBuffer->SetLineFlag(I, LF_BOOKMARKS, FALSE);
		}
		m_bBookmarkExist = FALSE;
	}							 
}

void CCrystalTextView::OnUpdateNextBookmark(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBookmarkExist);
}

void CCrystalTextView::OnUpdatePrevBookmark(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBookmarkExist);
}

void CCrystalTextView::OnUpdateClearAllBookmarks(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_bBookmarkExist);
}

BOOL CCrystalTextView::GetViewTabs()
{
	return m_bViewTabs;
}

void CCrystalTextView::SetViewTabs(BOOL bViewTabs)
{
	if (bViewTabs != m_bViewTabs)
	{
		m_bViewTabs = bViewTabs;
		if (::IsWindow(m_hWnd))
			Invalidate();
	}
}

void CCrystalTextView::OnEditViewTabs()
{
	SetViewTabs(!m_bViewTabs);
}

void CCrystalTextView::OnUpdateEditViewTabs(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
	pCmdUI->SetCheck(m_bViewTabs);
}


BOOL CCrystalTextView::GetSelectionMargin()
{
	return m_bSelMargin;
}

int CCrystalTextView::GetMarginWidth()
{
	return m_bSelMargin ? 20 : 1;
}

BOOL CCrystalTextView::GetSmoothScroll() const
{
	return m_bSmoothScroll;
}

void CCrystalTextView::SetSmoothScroll(BOOL bSmoothScroll)
{
	m_bSmoothScroll = bSmoothScroll;
}

//	[JRT]
BOOL CCrystalTextView::GetDisableDragAndDrop() const
{
	return m_bDisableDragAndDrop;
}

//	[JRT]
void CCrystalTextView::SetDisableDragAndDrop(BOOL bDDAD)
{
	m_bDisableDragAndDrop = bDDAD;
}


BOOL CCrystalTextView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta)
		OnVScroll(zDelta > 0 ? 98 : 99, 0, 0);
//	return CView::OnMouseWheel(nFlags, zDelta, pt);
	return true;
}


void CCrystalTextView::MoveLeft(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		m_ptCursorPos = m_ptDrawSelStart;
	}
	else
	{
		if (m_ptCursorPos.x == 0)
		{
			if (m_ptCursorPos.y > 0)
			{
//				m_ptCursorPos.y --;
				m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, -1);
				m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
			}
		}
		else
			m_ptCursorPos.x --;
	}
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveRight(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		m_ptCursorPos = m_ptDrawSelEnd;
	}
	else
	{
		if (m_ptCursorPos.x == GetLineLength(m_ptCursorPos.y))
		{
			if (m_ptCursorPos.y < GetLineCount() - 1)
			{
				//m_ptCursorPos.y ++;
				int nNextLine= FindVisibleLine(m_ptCursorPos.y, 1);
				if (nNextLine != m_ptCursorPos.y)
				{
					m_ptCursorPos.y = nNextLine;
					m_ptCursorPos.x = 0;
				}
			}
		}
		else
			m_ptCursorPos.x ++;
	}
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveWordLeft(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		MoveLeft(bSelect);
		return;
	}

	if (m_ptCursorPos.x == 0)
	{
		if (m_ptCursorPos.y == 0)
			return;
//		m_ptCursorPos.y --;
		m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, -1);
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}

	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nPos = m_ptCursorPos.x;
	while (nPos > 0 && isspace(pszChars[nPos - 1]))
		nPos --;

	if (nPos > 0)
	{
		nPos --;
		if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
		{
			while (nPos > 0 && (isalnum(pszChars[nPos - 1]) || pszChars[nPos - 1] == _T('_')))
				nPos --;
		}
		else
		{
			while (nPos > 0 && ! isalnum(pszChars[nPos - 1])
						&& pszChars[nPos - 1] != _T('_') && ! isspace(pszChars[nPos - 1]))
				nPos --;
		}
	}

	m_ptCursorPos.x = nPos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveWordRight(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		MoveRight(bSelect);
		return;
	}

	if (m_ptCursorPos.x == GetLineLength(m_ptCursorPos.y))
	{
		if (m_ptCursorPos.y == GetLineCount() - 1)
			return;
//		m_ptCursorPos.y ++;
		m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, 1);
		m_ptCursorPos.x = 0;
	}

	int nLength = GetLineLength(m_ptCursorPos.y);
	if (m_ptCursorPos.x == nLength)
	{
		MoveRight(bSelect);
		return;
	}

	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nPos = m_ptCursorPos.x;
	if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
	{
		while (nPos < nLength && isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
			nPos ++;
	}
	else
	{
		while (nPos < nLength && ! isalnum(pszChars[nPos])
						&& pszChars[nPos] != _T('_') && ! isspace(pszChars[nPos]))
			nPos ++;
	}

	while (nPos < nLength && isspace(pszChars[nPos]))
		nPos ++;

	m_ptCursorPos.x = nPos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveUp(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
		m_ptCursorPos = m_ptDrawSelStart;

	if (m_ptCursorPos.y > 0)
	{
		if (m_nIdealCharPos == -1)
			m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
//		m_ptCursorPos.y --;
		m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, -1);
		m_ptCursorPos.x = ApproxActualOffset(m_ptCursorPos.y, m_nIdealCharPos);
		if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
			m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveDown(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
		m_ptCursorPos = m_ptDrawSelEnd;

	if (m_ptCursorPos.y < GetLineCount() - 1)
	{
		if (m_nIdealCharPos == -1)
			m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
//		m_ptCursorPos.y ++;
		m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, 1);
		m_ptCursorPos.x = ApproxActualOffset(m_ptCursorPos.y, m_nIdealCharPos);
		if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
			m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveHome(BOOL bSelect)
{
	int nLength = GetLineLength(m_ptCursorPos.y);
	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nHomePos = 0;
	while (nHomePos < nLength && isspace(pszChars[nHomePos]))
		nHomePos ++;
	if (nHomePos == nLength || m_ptCursorPos.x == nHomePos)
		m_ptCursorPos.x = 0;
	else
		m_ptCursorPos.x = nHomePos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveEnd(BOOL bSelect)
{
	m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MovePgUp(BOOL bSelect)
{
	int nNewTopLine = FindVisibleLine(m_nTopLine, -GetScreenLines() + 1);
//	int nNewTopLine = m_nTopLine - GetScreenLines() + 1;
	if (nNewTopLine < 0)
		nNewTopLine = 0;
	if (m_nTopLine != nNewTopLine)
	{
		ScrollToLine(nNewTopLine);
		UpdateSiblingScrollPos(TRUE);
	}

//	m_ptCursorPos.y -= GetScreenLines() - 1;
	m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, -GetScreenLines() + 1);
	if (m_ptCursorPos.y < 0)
		m_ptCursorPos.y = 0;
	if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);	//todo: no vertical scroll
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MovePgDn(BOOL bSelect)
{
	int nNewTopLine = FindVisibleLine(m_nTopLine, GetScreenLines() - 1);
//	int nNewTopLine = m_nTopLine + GetScreenLines() - 1;
	if (nNewTopLine >= GetLineCount())
		nNewTopLine = GetLineCount() - 1;
	if (m_nTopLine != nNewTopLine)
	{
		ScrollToLine(nNewTopLine);
		UpdateSiblingScrollPos(TRUE);
	}

//	m_ptCursorPos.y += GetScreenLines() - 1;
	m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, GetScreenLines() - 1);
	if (m_ptCursorPos.y >= GetLineCount())
		m_ptCursorPos.y = GetLineCount() - 1;
	if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);	//todo: no vertical scroll
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveCtrlHome(BOOL bSelect)
{
	m_ptCursorPos.x = 0;
	m_ptCursorPos.y = 0;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveCtrlEnd(BOOL bSelect)
{
	m_ptCursorPos.y = FindVisibleLine(GetLineCount() - 1, 1);
	m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::ScrollUp()
{
	if (m_nTopLine > 0)
	{
		ScrollToLine(FindVisibleLine(m_nTopLine, -1));
		UpdateSiblingScrollPos(TRUE);
	}
}

void CCrystalTextView::ScrollDown()
{
	if (m_nTopLine < GetLineCount() - 1)
	{
		ScrollToLine(FindVisibleLine(m_nTopLine, 1));
		UpdateSiblingScrollPos(TRUE);
	}
}

void CCrystalTextView::ScrollLeft()
{
	if (m_nOffsetChar > 0)
	{
		ScrollToChar(m_nOffsetChar - 1);
		UpdateCaret();
	}
}

void CCrystalTextView::ScrollRight()
{
	if (m_nOffsetChar < GetMaxLineLength() - 1)
	{
		ScrollToChar(m_nOffsetChar + 1);
		UpdateCaret();
	}
}

CPoint CCrystalTextView::WordToRight(CPoint pt)
{
	ASSERT_VALIDTEXTPOS(pt);
	int nLength = GetLineLength(pt.y);
	LPCTSTR pszChars = GetLineChars(pt.y);
	while (pt.x < nLength)
	{
		if (! isalnum(pszChars[pt.x]) && pszChars[pt.x] != _T('_'))
			break;
		pt.x ++;
	}
	ASSERT_VALIDTEXTPOS(pt);
	return pt;
}

CPoint CCrystalTextView::WordToLeft(CPoint pt)
{
	ASSERT_VALIDTEXTPOS(pt);
	LPCTSTR pszChars = GetLineChars(pt.y);
	while (pt.x > 0)
	{
		if (! isalnum(pszChars[pt.x - 1]) && pszChars[pt.x - 1] != _T('_'))
			break;
		pt.x --;
	}
	ASSERT_VALIDTEXTPOS(pt);
	return pt;
}

void CCrystalTextView::SelectAll()
{
	int nLineCount = GetLineCount();
	m_ptCursorPos.x = GetLineLength(nLineCount - 1);
	m_ptCursorPos.y = nLineCount - 1;
	SetSelection(CPoint(0, 0), m_ptCursorPos);
	UpdateCaret();
}

void CCrystalTextView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CView::OnLButtonDown(nFlags, point);

	BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL bControl = GetKeyState(VK_CONTROL) & 0x8000;

	if (point.x < GetMarginWidth())
	{
		AdjustTextPoint(point);

		// MiK: click on the [-] sign?
		int nLineIndex= ClientToText(point).y;
		DWORD dwFlag= GetLineFlags(nLineIndex);
		if (dwFlag & LF_COLLAPSIBLE_BLOCK_START)
		{
			CollapseBlock(nLineIndex);
		}
		else if (bControl)
		{
			SelectAll();
		}
		else
		{
			m_ptCursorPos = ClientToText(point);
			m_ptCursorPos.x = 0;				//	Force beginning of the line
			if (! bShift)
				m_ptAnchor = m_ptCursorPos;

			CPoint ptStart, ptEnd;
			ptStart = m_ptAnchor;
			if (ptStart.y == GetLineCount() - 1)
				ptStart.x = GetLineLength(ptStart.y);
			else
			{
				ptStart.y ++;
				ptStart.x = 0;
			}

			ptEnd = m_ptCursorPos;
			ptEnd.x = 0;

			m_ptCursorPos = ptEnd;
			UpdateCaret();
			EnsureVisible(m_ptCursorPos);
			SetSelection(ptStart, ptEnd);

			SetCapture();
			m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
			ASSERT(m_nDragSelTimer != 0);
			m_bWordSelection = FALSE;
			m_bLineSelection = TRUE;
			m_bDragSelection = TRUE;
		}
	}
	else
	{
		CPoint ptText = ClientToText(point);
		PrepareSelBounds();
		//	[JRT]:	Support For Disabling Drag and Drop...
		if ((IsInsideSelBlock(ptText)) &&				// If Inside Selection Area
				(!m_bDisableDragAndDrop))				// And D&D Not Disabled
		{
			m_bPreparingToDrag = TRUE;
		}
		else
		{
			AdjustTextPoint(point);
			m_ptCursorPos = ClientToText(point);
			if (! bShift)
				m_ptAnchor = m_ptCursorPos;

			CPoint ptStart, ptEnd;
			if (bControl)
			{
				if (m_ptCursorPos.y < m_ptAnchor.y ||
					m_ptCursorPos.y == m_ptAnchor.y && m_ptCursorPos.x < m_ptAnchor.x)
				{
					ptStart = WordToLeft(m_ptCursorPos);
					ptEnd = WordToRight(m_ptAnchor);
				}
				else
				{
					ptStart = WordToLeft(m_ptAnchor);
					ptEnd = WordToRight(m_ptCursorPos);
				}
			}
			else
			{
				ptStart = m_ptAnchor;
				ptEnd = m_ptCursorPos;
			}

			m_ptCursorPos = ptEnd;
			UpdateCaret();
			EnsureVisible(m_ptCursorPos);
			SetSelection(ptStart, ptEnd);

			SetCapture();
			m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
			ASSERT(m_nDragSelTimer != 0);
			m_bWordSelection = bControl;
			m_bLineSelection = FALSE;
			m_bDragSelection = TRUE;
		}
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CView::OnMouseMove(nFlags, point);

	if (m_bDragSelection)
	{
		BOOL bOnMargin = point.x < GetMarginWidth();

		AdjustTextPoint(point);
		CPoint ptNewCursorPos = ClientToText(point);

		CPoint ptStart, ptEnd;
		if (m_bLineSelection)
		{
			if (bOnMargin)
			{
				if (ptNewCursorPos.y < m_ptAnchor.y ||
					ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
				{
					ptEnd = m_ptAnchor;
					if (ptEnd.y == GetLineCount() - 1)
					{
						ptEnd.x = GetLineLength(ptEnd.y);
					}
					else
					{
						ptEnd.y ++;
						ptEnd.x = 0;
					}
					ptNewCursorPos.x = 0;
					m_ptCursorPos = ptNewCursorPos;
				}
				else
				{
					ptEnd = m_ptAnchor;
					ptEnd.x = 0;
					m_ptCursorPos = ptNewCursorPos;
					if (ptNewCursorPos.y == GetLineCount() - 1)
					{
						ptNewCursorPos.x = GetLineLength(ptNewCursorPos.y);
					}
					else
					{
						ptNewCursorPos.y ++;
						ptNewCursorPos.x = 0;
					}
					m_ptCursorPos.x = 0;
				}
				UpdateCaret();
				SetSelection(ptNewCursorPos, ptEnd);
				return;
			}

			//	Moving to normal selection mode
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			m_bLineSelection = m_bWordSelection = FALSE;
		}

		if (m_bWordSelection)
		{
			if (ptNewCursorPos.y < m_ptAnchor.y ||
				ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
			{
				ptStart = WordToLeft(ptNewCursorPos);
				ptEnd = WordToRight(m_ptAnchor);
			}
			else
			{
				ptStart = WordToLeft(m_ptAnchor);
				ptEnd = WordToRight(ptNewCursorPos);
			}
		}
		else
		{
			ptStart = m_ptAnchor;
			ptEnd = ptNewCursorPos;
		}

		m_ptCursorPos = ptEnd;
		UpdateCaret();
		SetSelection(ptStart, ptEnd);
	}

	if (m_bPreparingToDrag)
	{
		m_bPreparingToDrag = FALSE;
		HGLOBAL hData = PrepareDragData();
		if (hData != NULL)
		{
			if (m_pTextBuffer != NULL)
				m_pTextBuffer->BeginUndoGroup();

			COleDataSource ds;
			ds.CacheGlobalData(CF_TEXT, hData);
			m_bDraggingText = TRUE;
			DROPEFFECT de = ds.DoDragDrop(GetDropEffect());
			if (de != DROPEFFECT_NONE)
				OnDropSource(de);
			m_bDraggingText = FALSE;

			if (m_pTextBuffer != NULL)
				m_pTextBuffer->FlushUndoGroup(this);
		}
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnLButtonUp(nFlags, point);

	if (m_bDragSelection)
	{
		AdjustTextPoint(point);
		CPoint ptNewCursorPos = ClientToText(point);

		CPoint ptStart, ptEnd;
		if (m_bLineSelection)
		{
			CPoint ptEnd;
			if (ptNewCursorPos.y < m_ptAnchor.y ||
				ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
			{
				ptEnd = m_ptAnchor;
				if (ptEnd.y == GetLineCount() - 1)
				{
					ptEnd.x = GetLineLength(ptEnd.y);
				}
				else
				{
					ptEnd.y ++;
					ptEnd.x = 0;
				}
				ptNewCursorPos.x = 0;
				m_ptCursorPos = ptNewCursorPos;
			}
			else
			{
				ptEnd = m_ptAnchor;
				ptEnd.x = 0;
				if (ptNewCursorPos.y == GetLineCount() - 1)
				{
					ptNewCursorPos.x = GetLineLength(ptNewCursorPos.y);
				}
				else
				{
					ptNewCursorPos.y ++;
					ptNewCursorPos.x = 0;
				}
				m_ptCursorPos = ptNewCursorPos;
			}
			m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, 0);	// MiK
			m_ptCursorPos.x = min(ptEnd.x, GetLineLength(m_ptCursorPos.y));
			EnsureVisible(m_ptCursorPos);
			UpdateCaret();
			SetSelection(ptNewCursorPos, ptEnd);
		}
		else
		{
			if (m_bWordSelection)
			{
				if (ptNewCursorPos.y < m_ptAnchor.y ||
					ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
				{
					ptStart = WordToLeft(ptNewCursorPos);
					ptEnd = WordToRight(m_ptAnchor);
				}
				else
				{
					ptStart = WordToLeft(m_ptAnchor);
					ptEnd = WordToRight(ptNewCursorPos);
				}
			}
			else
			{
				ptStart = m_ptAnchor;
				ptEnd = m_ptCursorPos;
			}

//			m_ptCursorPos = ptEnd;
			m_ptCursorPos.y = FindVisibleLine(ptEnd.y, 0);	// MiK
			m_ptCursorPos.x = min(ptEnd.x, GetLineLength(m_ptCursorPos.y));
			EnsureVisible(m_ptCursorPos);
			UpdateCaret();
			SetSelection(ptStart, ptEnd);
		}

		ReleaseCapture();
		KillTimer(m_nDragSelTimer);
		m_bDragSelection = FALSE;
	}

	if (m_bPreparingToDrag)
	{
		m_bPreparingToDrag = FALSE;

		AdjustTextPoint(point);
		m_ptCursorPos = ClientToText(point);
		m_ptCursorPos.y = FindVisibleLine(m_ptCursorPos.y, 0);	// MiK
		m_ptCursorPos.x = min(m_ptCursorPos.x, GetLineLength(m_ptCursorPos.y));
		EnsureVisible(m_ptCursorPos);
		UpdateCaret();
		SetSelection(m_ptCursorPos, m_ptCursorPos);
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnTimer(UINT nIDEvent) 
{
	CView::OnTimer(nIDEvent);

	if (nIDEvent == CRYSTAL_TIMER_DRAGSEL)
	{
		ASSERT(m_bDragSelection);
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		CRect rcClient;
		GetClientRect(&rcClient);

		BOOL bChanged = FALSE;

		//	Scroll vertically, if necessary
		int nNewTopLine = m_nTopLine;
		int nLineCount = GetLineCount();
		if (pt.y < rcClient.top)
		{
			//nNewTopLine --;
			nNewTopLine = FindVisibleLine(nNewTopLine, -1);
			if (pt.y < rcClient.top - GetLineHeight())
				nNewTopLine -= 2;
		}
		else
		if (pt.y >= rcClient.bottom)
		{
//			nNewTopLine ++;
			nNewTopLine = FindVisibleLine(nNewTopLine, 1);
			if (pt.y >= rcClient.bottom + GetLineHeight())
				nNewTopLine += 2;
		}

		if (nNewTopLine < 0)
			nNewTopLine = 0;
		if (nNewTopLine >= nLineCount)
			nNewTopLine = nLineCount - 1;

		if (m_nTopLine != nNewTopLine)
		{
			ScrollToLine(nNewTopLine);
			UpdateSiblingScrollPos(TRUE);
			bChanged = TRUE;
		}

		//	Scroll horizontally, if necessary
		int nNewOffsetChar = m_nOffsetChar;
		int nMaxLineLength = GetMaxLineLength();
		if (pt.x < rcClient.left)
			nNewOffsetChar --;
		else
		if (pt.x >= rcClient.right)
			nNewOffsetChar ++;

		if (nNewOffsetChar >= nMaxLineLength)
			nNewOffsetChar = nMaxLineLength - 1;
		if (nNewOffsetChar < 0)
			nNewOffsetChar = 0;

		if (m_nOffsetChar != nNewOffsetChar)
		{
			ScrollToChar(nNewOffsetChar);
			UpdateCaret();
			UpdateSiblingScrollPos(FALSE);
			bChanged = TRUE;
		}

		//	Fix changes
		if (bChanged)
		{
			AdjustTextPoint(pt);
			CPoint ptNewCursorPos = ClientToText(pt);
			if (ptNewCursorPos != m_ptCursorPos)
			{
				m_ptCursorPos = ptNewCursorPos;
				UpdateCaret();
			}
			SetSelection(m_ptAnchor, m_ptCursorPos);
		}
	}
}

void CCrystalTextView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDblClk(nFlags, point);

	if (! m_bDragSelection)
	{
		AdjustTextPoint(point);

		m_ptCursorPos = ClientToText(point);
		m_ptAnchor = m_ptCursorPos;

		CPoint ptStart, ptEnd;
		if (m_ptCursorPos.y < m_ptAnchor.y ||
			m_ptCursorPos.y == m_ptAnchor.y && m_ptCursorPos.x < m_ptAnchor.x)
		{
			ptStart = WordToLeft(m_ptCursorPos);
			ptEnd = WordToRight(m_ptAnchor);
		}
		else
		{
			ptStart = WordToLeft(m_ptAnchor);
			ptEnd = WordToRight(m_ptCursorPos);
		}

		m_ptCursorPos = ptEnd;
		UpdateCaret();
		EnsureVisible(m_ptCursorPos);
		SetSelection(ptStart, ptEnd);

		SetCapture();
		m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
		ASSERT(m_nDragSelTimer != 0);
		m_bWordSelection = TRUE;
		m_bLineSelection = FALSE;
		m_bDragSelection = TRUE;
	}
}

void CCrystalTextView::OnEditCopy() 
{
	Copy();
}

void CCrystalTextView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ptSelStart != m_ptSelEnd);
}

void CCrystalTextView::OnEditSelectAll() 
{
	SelectAll();
}

void CCrystalTextView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

void CCrystalTextView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CPoint pt = point;
	AdjustTextPoint(pt);
	pt = ClientToText(pt);
	if (! IsInsideSelBlock(pt))
	{
		m_ptAnchor = m_ptCursorPos = pt;
		SetSelection(m_ptCursorPos, m_ptCursorPos);
		EnsureVisible(m_ptCursorPos);
		UpdateCaret();
	}

	CView::OnRButtonDown(nFlags, point);
}

BOOL CCrystalTextView::IsSelection()
{
	return m_ptSelStart != m_ptSelEnd;
}

void CCrystalTextView::Copy()
{
	if (m_ptSelStart == m_ptSelEnd)
		return;

	PrepareSelBounds();
	CString text;
	GetText(m_ptDrawSelStart, m_ptDrawSelEnd, text);
	PutToClipboard(text);
}

BOOL CCrystalTextView::TextInClipboard()
{
	return IsClipboardFormatAvailable(CF_TEXT);
}

BOOL CCrystalTextView::PutToClipboard(LPCTSTR pszText)
{
	if (pszText == NULL || lstrlen(pszText) == 0)
		return FALSE;

	CWaitCursor wc;
	BOOL bOK = FALSE;
	if (OpenClipboard())
	{
		EmptyClipboard();
		HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, lstrlen(pszText) + 1);
		if (hData != NULL)
		{
			LPSTR pszData = (LPSTR) ::GlobalLock(hData);
			USES_CONVERSION;
			strcpy(pszData, T2A((LPTSTR) pszText));
			GlobalUnlock(hData);
			bOK = SetClipboardData(CF_TEXT, hData) != NULL;
		}
		CloseClipboard();
	}
	return bOK;
}

BOOL CCrystalTextView::GetFromClipboard(CString &text)
{
	BOOL bSuccess = FALSE;
	if (OpenClipboard())
	{
		HGLOBAL hData = GetClipboardData(CF_TEXT);
		if (hData != NULL)
		{
			LPSTR pszData = (LPSTR) GlobalLock(hData);
			if (pszData != NULL)
			{
				text = pszData;
				GlobalUnlock(hData);
				bSuccess = TRUE;
			}
		}
		CloseClipboard();
	}
	return bSuccess;
}


void CCrystalTextView::MarkCollapsibleBlockLine(int nLineIndex, bool bStart)
{
	ASSERT(nLineIndex >= 0);
	m_pTextBuffer->SetLineFlag(nLineIndex, LF_COLLAPSIBLE_BLOCK_START, bStart, false);
	m_pTextBuffer->SetLineFlag(nLineIndex, LF_COLLAPSIBLE_BLOCK_END, !bStart, false);
}

void CCrystalTextView::ClearCollapsibleBlockMark(int nLineIndex)
{
	ASSERT(nLineIndex >= 0);
	DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLineIndex);
	DWORD dwBlock= LF_COLLAPSIBLE_BLOCK_START | LF_COLLAPSIBLE_BLOCK_END;
	if (dwFlags & dwBlock)
	{
		if (dwFlags & LF_COLLAPSED_BLOCK)	// line is hidden?
			VERIFY(CollapseBlock(nLineIndex));

		m_pTextBuffer->SetLineFlag(nLineIndex, LF_COLLAPSIBLE_BLOCK_START, false, false);
		m_pTextBuffer->SetLineFlag(nLineIndex, LF_COLLAPSIBLE_BLOCK_END, false, false);
	}
}


void CCrystalTextView::OnToggleCollapsibleBlock()
{
	if (!CollapseBlock(m_ptCursorPos.y))
		MessageBeep(MB_OK);
}

void CCrystalTextView::OnCollapseAllBlocks()
{
	CollapseAllBlocks(true);
}

void CCrystalTextView::OnExpandAllBlocks()
{
	CollapseAllBlocks(false);
}


// collapse/expand all blocks
//
void CCrystalTextView::CollapseAllBlocks(bool bCollapse)
{
	if (m_pTextBuffer == NULL)
		return;

	int nLineCount= m_pTextBuffer->GetLineCount();
	int nCurrentLine= m_ptCursorPos.y;

	for (int nLine= 0; nLine < nLineCount; ++nLine)
	{
		// find block beginning
		DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

		if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
		{
			int nLineFrom= -1;
			int nLineTo= -1;

			// find end of current block (note: there may be no end;
			// then just go on and try to locate next block)
			if (FindCollapsibleBlock(nLine, nLineFrom, nLineTo))
			{
				ASSERT(nLine == nLineFrom);

				// here we do not care about nested blocks, (un)cover everything

				if (bCollapse)
					m_pTextBuffer->SetLinesFlags(nLineFrom, nLineTo, LF_COLLAPSED_BLOCK, 0);
				else
					m_pTextBuffer->SetLinesFlags(nLineFrom, nLineTo, 0, LF_COLLAPSED_BLOCK);

				// move current line to the first line of collapsed block if it lies inside
				// just folded (hidden) block
				if (bCollapse && nCurrentLine >= nLineFrom && nCurrentLine <= nLineTo)
					nCurrentLine = nLineFrom;

				nLine = nLineTo;
			}
		}
	}

	// update views
	m_pTextBuffer->UpdateViews(0, 0, UPDATE_HORZRANGE | UPDATE_VERTRANGE, 0);

	// TODO: preserve top line
//	if (bCollapse)
	{
		// cancel selection & move cursor out of hidden block
		GoToLine(nCurrentLine);
	}
}


// find beginning and end of block containing 'nLineInsideBlock' and
// collase/expand this block; fn handles nested blocks
//
bool CCrystalTextView::CollapseBlock(int nLineInsideBlock)
{
	int nLineFrom= -1;
	int nLineTo= -1;
	if (FindCollapsibleBlock(nLineInsideBlock, nLineFrom, nLineTo))
		return CollapseBlock(nLineFrom, nLineTo);

	return false;
}

// find beginning and end of block containing 'nLineInsideBlock'
//
bool CCrystalTextView::FindCollapsibleBlock(int nLineInsideBlock, int& nLineFrom, int& nLineTo)
{
	if (m_pTextBuffer == NULL)
		return false;

	int nLineCount= m_pTextBuffer->GetLineCount();
	ASSERT(nLineInsideBlock >= 0 && nLineInsideBlock < nLineCount);

	nLineFrom = -1;
	int nLevel= 0;
	for (int nLine= nLineInsideBlock; nLine >= 0; --nLine)
	{
		DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

		if (dwFlags & LF_COLLAPSIBLE_BLOCK_END)
			if (nLine < nLineInsideBlock)
				++nLevel;	// nested block

		if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
		{
			if (nLevel == 0)
			{
				nLineFrom = nLine;
				break;
			}
			--nLevel;
		}
	}
	if (nLineFrom < 0)
		return false;

	nLevel = 0;
	nLineTo = -1;
	{
		for (int nLine= nLineInsideBlock; nLine < nLineCount; ++nLine)
		{
			DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

			if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
				if (nLine > nLineInsideBlock)
					++nLevel;	// nested block

			if (dwFlags & LF_COLLAPSIBLE_BLOCK_END)
			{
				if (nLevel == 0)
				{
					nLineTo = nLine;
					break;
				}
				--nLevel;
			}
		}
	}
	if (nLineTo < 0)
		return false;

	if (nLineFrom >= nLineTo)
		return false;				// no single-line blocks
/*
	if (nLineTo == nLineCount - 1)	// block span to the end of text?
	{
		--nLineTo;					// last line has to be visible...
		if (nLineTo < 1)
			return false;
	} */

	return true;
}


bool CCrystalTextView::CollapseBlock(int nLineFrom, int nLineTo)
{
	if (m_pTextBuffer == NULL)
		return false;

	int nLineCount= m_pTextBuffer->GetLineCount();

	if (nLineFrom < 0 || nLineFrom >= nLineCount || nLineTo < 0 || nLineTo >= nLineCount || nLineFrom > nLineTo)
	{
		ASSERT(false);
		return false;
	}

	DWORD dwFlagsFrom= m_pTextBuffer->GetLineFlags(nLineFrom);
	DWORD dwFlagsTo= m_pTextBuffer->GetLineFlags(nLineTo);

	if ((dwFlagsFrom & LF_COLLAPSIBLE_BLOCK_START) == 0 || (dwFlagsTo & LF_COLLAPSIBLE_BLOCK_END) == 0)
	{
		ASSERT(false);
		return false;
	}

	bool bHide= !(dwFlagsFrom & LF_COLLAPSED_BLOCK);

	if (bHide)
		m_pTextBuffer->SetLinesFlags(nLineFrom, nLineTo, LF_COLLAPSED_BLOCK, 0);
	else
		m_pTextBuffer->SetLinesFlags(nLineFrom, nLineTo, 0, LF_COLLAPSED_BLOCK);
/*
	// mark first and last line
	m_pTextBuffer->SetLineFlag(nLineFrom, LF_COLLAPSED_BLOCK, bHide, false);
	m_pTextBuffer->SetLineFlag(nLineTo, LF_COLLAPSED_BLOCK, bHide, false);

	// mark lines inside block, but leave nested blocks intact
	int nLevel= 0;
	for (int nLine= nLineFrom + 1; nLine < nLineTo; ++nLine)
	{
		DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);
		if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)
			++nLevel;
		if (dwFlags & LF_COLLAPSIBLE_BLOCK_END)
			--nLevel;

		if (nLevel == 0)
			m_pTextBuffer->SetLineFlag(nLine, LF_COLLAPSED_BLOCK, bHide, false);
	}
	ASSERT(nLevel == 0);
*/
	// update views
	m_pTextBuffer->UpdateViews(0, 0, UPDATE_HORZRANGE | UPDATE_VERTRANGE, 0);

	if (bHide)
	{
		// cancel selection & move cursor out of hidden block
		GoToLine(nLineFrom);
	}

	return true;
}


bool CCrystalTextView::IsLineCollapsed(int nLine)
{
	if (m_pTextBuffer == NULL || nLine >= m_pTextBuffer->GetLineCount())
		return false;

	DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

	return !!(dwFlags & LF_COLLAPSED_BLOCK);
}


bool CCrystalTextView::IsLineHidden(int nLine)
{
	if (m_pTextBuffer == NULL || nLine >= m_pTextBuffer->GetLineCount())
		return false;

	DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

	if (dwFlags & LF_COLLAPSED_BLOCK)	// hidden block?
	{
		if (dwFlags & LF_COLLAPSIBLE_BLOCK_START)	// first line in a block?
		{
			if (nLine > 0)
			{
				// check if previous line is visible
				DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine - 1);
				if (dwFlags & LF_COLLAPSED_BLOCK)	// not visible?
				{
					// TODO: pathological case of adjacent blocks

					return true;	// hide this first line as well
				}
			}
			return false;	// first line in block is usually visible (unless it's nested block)
		}

		return true;
	}

	return false;	// normal (visible) lines
}


bool CCrystalTextView::IsFirstLineOfCollapsedBlock(int nLine)
{
	if (m_pTextBuffer == NULL || nLine >= m_pTextBuffer->GetLineCount())
		return false;

	DWORD dwFlags= m_pTextBuffer->GetLineFlags(nLine);

	if (dwFlags & LF_COLLAPSED_BLOCK && dwFlags & LF_COLLAPSIBLE_BLOCK_START)
		return true;

	return false;
}


void CCrystalTextView::DrawEllipsis(CDC* pDC, const CRect& rcLine)
{
	pDC->FillSolidRect(rcLine, GetColor(COLORINDEX_WHITESPACE));
	pDC->SelectObject(GetFont(false, false));
	pDC->SetTextColor(GetColor(COLORINDEX_ELLIPSIS));
	pDC->TextOut(rcLine.left, rcLine.top, _T("..."), 3);
}


int CCrystalTextView::FindVisibleLine(int nLine, int nDelta)
{
	int nLineCount= GetLineCount();
	if (nLineCount == 0)
		return -1;

	int nInc= nDelta > 0 ? 1 : -1;

	for (; ; nLine += nInc)
	{
		if (nLine < 0)
			return 0;

		if (nLine >= nLineCount)
		{
			if (IsLineHidden(nLineCount - 1))	// last line hidden?
				return FindVisibleLine(nLineCount - 1, 0);
			return nLineCount - 1;
		}

		// skip hidden lines
		if (IsLineHidden(nLine))
			continue;

		if (nDelta == 0)
			break;

		if (nDelta > 0)
			--nDelta;
		else
			++nDelta;
	}

	return nLine;
}


int CCrystalTextView::CalcVisibleLines()
{
	return CalcVisibleLines(0, GetLineCount());
}


int CCrystalTextView::CalcVisibleLines(int nFromLine, int nToLine)
{
	bool bSwap= false;
	if (nFromLine > nToLine)
	{
		std::swap(nFromLine, nToLine);
		bSwap = true;
	}

	ASSERT(nFromLine >= 0 && nToLine >= 0 && nFromLine <= nToLine);
	int nLineCount= GetLineCount();
	ASSERT(nToLine <= nLineCount);

	int nVisCount= 0;

	for (int nLine= nFromLine; nLine < nToLine; ++nLine)
		if (!IsLineHidden(nLine))
			nVisCount++;

	return bSwap ? -nVisCount : nVisCount;
}


void CCrystalTextView::ExpandCurrentLine()
{
	ExpandLine(GetCursorPos().y);
}

void CCrystalTextView::ExpandLine(int nLine)
{
	if (IsLineCollapsed(nLine))
	{
		// expand block (CollapseBlock toggles it back to expanded state)
		CollapseBlock(nLine);
	}
}


void CCrystalTextView::NotifyTextChanged()
{}


void CCrystalTextView::CaretMoved()
{
	int nLine= m_ptCursorPos.y;

	if (m_pTextBuffer == 0 || nLine >= GetLineCount())
		return;

	int nStart= 0;
	int nEnd= 0;
	CString strLine;

	int nLength= GetLineLength(nLine);
	if (nLength != 0)
	{
		m_pTextBuffer->GetText(nLine, 0, nLine, nLength, strLine);
		nLength = strLine.GetLength();

		int x= m_ptCursorPos.x;

		if (x >= 0 && x <= nLength)
		{
			nStart = x;

			while (nStart > 0 && strLine[nStart - 1] > ' ')
				--nStart;

			nEnd = x;
			while (nEnd < nLength && strLine[nEnd] > ' ')
				++nEnd;
		}
	}

	CaretMoved(strLine, nStart, nEnd);
}


void CCrystalTextView::CaretMoved(const CString& strLine, int nWordStart, int nWordEnd)
{}
