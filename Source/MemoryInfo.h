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

// MemoryInfo.h : header file
//

#include "MemoryView.h"
#include "MemoryDoc.h"


/////////////////////////////////////////////////////////////////////////////
// CMemoryInfo frame
class COutputMem;

class CMemoryInfo : public CMiniFrameWnd
{
  COutputMem *m_pMem;
  UINT16 m_uAddr;
  CMemoryDoc m_Doc;

  static CString m_strClass;
  static bool m_bRegistered;
  void RegisterWndClass();
  void init();

  DECLARE_DYNCREATE(CMemoryInfo)

  // Attributes
public:
  CMemoryInfo();
  virtual ~CMemoryInfo();
  CRect m_WndRect;
  bool m_bHidden;

  // Operations
public:
  enum ViewType { VIEW_MEMORY, VIEW_ZEROPAGE, VIEW_STACK };
  bool Create(COutputMem *pMem, UINT16 uAddr, ViewType bView);

  void InvalidateView(UINT16 uStackPtr= 0);
  void Invalidate();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMemoryInfo)
protected:
  virtual void PostNcDestroy();
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);
  // Generated message map functions
  //{{AFX_MSG(CMemoryInfo)
  afx_msg void OnDestroy();
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnClose();
  //}}AFX_MSG
  afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
