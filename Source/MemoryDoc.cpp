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

// MemoryDoc.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "MemoryDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc

IMPLEMENT_DYNCREATE(CMemoryDoc, CDocument)

CMemoryDoc::CMemoryDoc()
{
	m_uStackPtr = 0x1ff;
	m_uAddress = 0;
	m_pMem = 0;
}

BOOL CMemoryDoc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;
  return TRUE;
}

CMemoryDoc::~CMemoryDoc()
{
}


BEGIN_MESSAGE_MAP(CMemoryDoc, CDocument)
  //{{AFX_MSG_MAP(CMemoryDoc)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc diagnostics

#ifdef _DEBUG
void CMemoryDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CMemoryDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc serialization

void CMemoryDoc::Serialize(CArchive& ar)
{
  if (ar.IsStoring())
  {
    // TODO: add storing code here
  }
  else
  {
    // TODO: add loading code here
  }
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryDoc commands

BOOL CMemoryDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace/*= TRUE*/)
{
	// nop
	return false;
}
