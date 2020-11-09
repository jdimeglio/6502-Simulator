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

// IdentInfoView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoView view

class CIdentInfoView : public CListView
{
  static int CALLBACK CompareLVIFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
  int m_nSortBy;

protected:
  CIdentInfoView();           // protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CIdentInfoView)
    
    // Attributes
public:
  
  // Operations
public:
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CIdentInfoView)
public:
  virtual void OnInitialUpdate();
protected:
  virtual void OnDraw(CDC* pDC);      // overridden to draw this view
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  virtual ~CIdentInfoView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CIdentInfoView)
  afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
