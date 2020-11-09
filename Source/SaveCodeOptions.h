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

// SaveCodeOptions.h : header file
//
#include "FormatNums.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeOptions dialog

class CSaveCodeOptions : public CDialog, CFormatNums
{
  bool m_bModify;
  void CalculateNums(int pos);
  // Construction
public:
  CSaveCodeOptions(CWnd* pParent = NULL);   // standard constructor
  
  // Dialog Data
  //{{AFX_DATA(CSaveCodeOptions)
  enum { IDD = IDD_SAVE_CODE_OPT_1 };
  UINT	m_uEnd;
  UINT	m_uLength;
  UINT	m_uStart;
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CSaveCodeOptions)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  
  // Generated message map functions
  //{{AFX_MSG(CSaveCodeOptions)
  afx_msg void OnDeltaposSpinStart(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposSpinEnd(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposSpinLength(NMHDR* pNMHDR, LRESULT* pResult);
  virtual BOOL OnInitDialog();
  afx_msg void OnChangeFieldStart();
  afx_msg void OnChangeFieldEnd();
  afx_msg void OnChangeFieldLength();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
