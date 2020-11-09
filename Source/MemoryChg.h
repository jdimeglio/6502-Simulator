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

#if !defined(AFX_MEMORYCHG_H__82E05821_16FE_11D1_A91F_444553540000__INCLUDED_)
#define AFX_MEMORYCHG_H__82E05821_16FE_11D1_A91F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MemoryChg.h : header file
//

#include "OutputMem.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg dialog

class CMemoryChg : public CDialog
{
  COutputMem& m_Mem;

  void Modify();
// Construction
public:
  CMemoryChg(COutputMem& mem, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(CMemoryChg)
  enum { IDD = IDD_MEMORY_CHG };
  UINT	m_uAddr;
  int	m_nData;
  int	m_nByte;
  BOOL	m_bSigned;
  //}}AFX_DATA


// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMemoryChg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(CMemoryChg)
  afx_msg void OnChangeMemoryAddr();
  afx_msg void OnMemorySigned();
  afx_msg void OnMemoryByte();
  afx_msg void OnMemoryWord();
  virtual void OnOK();
  afx_msg void OnMemoryChg();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMORYCHG_H__82E05821_16FE_11D1_A91F_444553540000__INCLUDED_)
