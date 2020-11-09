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

// IdentInfoFrame.h : header file
//

#include "ChildFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoFrame frame

class CIdentInfoFrame : public CMiniFrameWnd //CFrameWnd
{
  DECLARE_DYNCREATE(CIdentInfoFrame)
protected:
  CIdentInfoFrame();           // protected constructor used by dynamic creation
  
  // Attributes
public:

  // Operations
public:
  
  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CIdentInfoFrame)
	//}}AFX_VIRTUAL
  
  // Implementation
protected:
//  CIdentInfoFrame();
  virtual ~CIdentInfoFrame();
  
  // Generated message map functions
  //{{AFX_MSG(CIdentInfoFrame)
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
