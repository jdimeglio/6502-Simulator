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

// IntGenerator.h: interface for the CIntGenerator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTGENERATOR_H__95B80699_804F_41D1_9BFE_B7618C2D1151__INCLUDED_)
#define AFX_INTGENERATOR_H__95B80699_804F_41D1_9BFE_B7618C2D1151__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIntGenerator  
{
public:
	CIntGenerator();
	~CIntGenerator();

	int m_nIRQTimeLapse;
	bool m_bGenerateIRQ;

	int m_nNMITimeLapse;
	bool m_bGenerateNMI;
};

#endif // !defined(AFX_INTGENERATOR_H__95B80699_804F_41D1_9BFE_B7618C2D1151__INCLUDED_)
