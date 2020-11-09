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

// MemoryDoc.h : header file
//

#ifndef _MemoryDoc_
#define _MemoryDoc_

#include "Asm.h"

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc document
class COutputMem;


class CMemoryDoc : public CDocument
{
protected:
//  CMemoryDoc();           // protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CMemoryDoc)

	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);

    // Attributes
public:
  UINT16 m_uAddress;
  UINT16 m_uStackPtr;
  COutputMem* m_pMem;

  // Operations
public:
  CMemoryDoc();

  void SetData(COutputMem* pMem, UINT16 uAddress)
  {
    m_uAddress = uAddress;
    m_pMem = pMem;
  }

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMemoryDoc)
public:
  virtual void Serialize(CArchive& ar);   // overridden for document i/o
protected:
  virtual BOOL OnNewDocument();
  //}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~CMemoryDoc();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

  // Generated message map functions
protected:
  //{{AFX_MSG(CMemoryDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif
