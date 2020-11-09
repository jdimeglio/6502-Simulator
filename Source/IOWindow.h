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

// IOWindow.h : header file
//

#ifndef _io_window_
#define _io_window_

#include "Asm.h"
#include "Broadcast.h"

/////////////////////////////////////////////////////////////////////////////
// CIOWindow frame

class CInputBuffer
{
public:
	CInputBuffer() : m_pHead(m_vchBuffer), m_pTail(m_vchBuffer)
	{}

	char GetChar();			// get next available character (returns 0 if there are no chars)
	void PutChar(char c);	// places char in the buffer (char is ignored if there is no space)
	void Paste(const char* pcText);		// paste clipboard text into buffer

private:
	enum { BUF_SIZE= 32 * 1024 };
	char m_vchBuffer[BUF_SIZE];
	char* m_pHead;
	char* m_pTail;
};


class CIOWindow : public CMiniFrameWnd
{
  UINT8 *m_pData;		// pamiêæ okna
  int m_nWidth, m_nHeight;	// rozmiar okna (kolumn x wierszy)
  int m_nCharH, m_nCharW;	// rozmiar znaków
  static CString m_strClass;
  static bool m_bRegistered;
  void RegisterWndClass();
  int m_nPosX, m_nPosY;		// po³o¿enie znaku do wypisania (i kursora)
  int m_nCursorCount;		// licznik ukryæ kursora
  bool m_bCursorOn;				// flag: cursor on/off
  bool m_bCursorVisible;		// flag: cursor currently visible
  UINT m_uTimer;
  CInputBuffer m_InputBuffer;	// keyboard input buffer

  int put(char chr, int x, int y);
  int puts(const char *str, int len, int x, int y);
  int scroll(int dy);		// przesuniêcie napisów o 'dy' wierszy
  int invalidate(int x, int y);	// obszar znaku pod (x,y) do przerysowania

  afx_msg LRESULT OnCls(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnPutC(WPARAM wParam, LPARAM /* lParam */);
  afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnInput(WPARAM /*wParam*/, LPARAM /* lParam */);
  afx_msg LRESULT OnPosition(WPARAM wParam, LPARAM lParam);

  DECLARE_DYNCREATE(CIOWindow)
public:
  static bool m_bHidden;
  CIOWindow();           // protected constructor used by dynamic creation

  // Attributes
public:
  static CFont m_Font;
  static LOGFONT m_LogFont;
  static CPoint m_WndPos;		// po³o¿enie okna
  static int m_nInitW, m_nInitH;
  static COLORREF m_rgbTextColor, m_rgbBackgndColor;

  // Operations
private:
  void CalcFontSize();			// obliczenie rozmiaru znaków
  void DrawCursor();			// draw cursor
  void DrawCursor(int nX, int nY, bool bVisible);
  void HideCursor();			// hide cursor if it's on
public:
  enum Commands				// rozkazy dla okna terminala
  { CMD_CLS = CBroadcast::WM_USER_OFFSET+100, CMD_PUTC, CMD_PUTS, CMD_IN, CMD_POSITION };
  bool Create();
  void SetSize(int w, int h, int resize= 1);
  void GetSize(int &w, int &h);
  void Resize();
  void SetWndPos(const POINT &p);
  CPoint GetWndPos();
  void Paste();

  void SetColors(COLORREF text, COLORREF backgnd);
  void GetColors(COLORREF &text, COLORREF &backgnd);

  int PutC(int chr);			// wydrukowanie znaku
  int PutChr(int chr);			// wydrukowanie znaku (verbatim)
  int PutS(const char *str, int len= -1);	// ci¹g znaków do wydrukowania
  int PutH(int n);			// wydrukowanie liczby hex (8 bitow)
  bool SetPosition(int x, int y);	// ustawienie po³o¿enia dla tekstu
  void GetPosition(int &x, int &y);	// odczyt po³o¿enia
  bool ShowCursor(bool bVisible= TRUE);	// w³¹czenie/wy³¹czenie kursora
  bool ResetCursor();			// w³¹cza kursor, zeruje licznik ukryæ
  bool Cls();				// wyczyszczenie okna
  int  Input();				// input

  bool IsWaiting() const;
  void ExitModalLoop();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CIOWindow)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
  virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~CIOWindow();
protected:

  // Generated message map functions
  //{{AFX_MSG(CIOWindow)
  afx_msg void OnPaint();
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* pMMI);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnDestroy();
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaste();
	//}}AFX_MSG
  virtual BOOL ContinueModal();
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
