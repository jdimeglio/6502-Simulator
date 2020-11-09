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

// DeasmSaveOptions.h : header file
//
#include "FormatNums.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasmSaveOptions dialog

class CDeasmSaveOptions : public CDialog, CFormatNums
{
  bool m_bModify;
  void CalculateNums(int pos);
// Construction
public:
  CDeasmSaveOptions(CWnd* pParent = NULL);   // standard constructor
  
  // Dialog Data
  //{{AFX_DATA(CDeasmSaveOptions)
  enum { IDD = IDD_DEASM_SAVE_OPT };
  UINT	m_uEnd;
  UINT	m_uLength;
  UINT	m_uStart;
  BOOL	m_bSaveAsData;
  //}}AFX_DATA

  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDeasmSaveOptions)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  
  // Generated message map functions
  //{{AFX_MSG(CDeasmSaveOptions)
  afx_msg void OnChangeDeasmEnd();
  afx_msg void OnChangeDeasmLength();
  afx_msg void OnChangeDeasmStart();
  afx_msg void OnDeltaposDeasmSpinEnd(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposDeasmSpinLength(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnDeltaposDeasmSpinStart(NMHDR* pNMHDR, LRESULT* pResult);
  virtual BOOL OnInitDialog();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
