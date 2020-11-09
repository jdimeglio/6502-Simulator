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

// Deasm6502Doc.h : header file
//

#ifndef _deasm_6502_doc_h_
#define _deasm_6502_doc_h_

#include "Asm.h"

/////////////////////////////////////////////////////////////////////////////
// CDeasm6502Doc document
class CContext;

class CDeasm6502Doc : public CDocument, CAsm
{
  UINT m_uStart;		// zmienne wykorzystane przy zapisie
  UINT m_uEnd;			// deasemblowanego programu
  UINT m_uLength;
  bool m_bSaveAsData;

protected:
  CDeasm6502Doc();		// protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CDeasm6502Doc)

  const CContext *m_pCtx;
  UINT16 m_uStartAddr;
  int m_nPointerAddr;		//  adres wsk. przez strza³kê (->) lub -1

  virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
  void DeassembleSave(CArchive &ar, const CContext &ctx, UINT16 start, UINT16 end, int opt);
  // Attributes
public:
  void SetContext(const CContext *pCtx);
  void SetStart(UINT16 addr, bool bDraw= TRUE);
  
  // Operations
public:
  void SetPointer(int addr, bool scroll= FALSE);
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDeasm6502Doc)
public:
  virtual void Serialize(CArchive& ar);   // overridden for document i/o
protected:
  virtual BOOL OnNewDocument();
  //}}AFX_VIRTUAL
  
  // Implementation
public:
  virtual ~CDeasm6502Doc();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CDeasm6502Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


#endif
