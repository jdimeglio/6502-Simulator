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

#ifndef _ident_h_
#define _ident_h_

struct CIdent
{
  enum IdentInfo	// info o identyfikatorze
  {
    I_INIT,
    I_UNDEF,		// identyfikator niezdefiniowany
    I_ADDRESS,		// identyfikator zawiera adres
    I_VALUE,		// identyfikator zawiera warto�� liczbow�
    I_MACRONAME,	// identyfikator jest nazw� makrodefinicji
    I_MACROADDR		// identyfikator zawiera adres w makrodefinicji
  } info;
  SINT32 val;		// warto�� liczbowa
  UINT8 checked;	// definicja identyfikatora potwierdzona w drugim przej�ciu asemblacji
  UINT8 variable;	// identyfikator zmiennej

  CIdent() : info(I_INIT), checked(FALSE), variable(FALSE)
  { }

  CIdent(IdentInfo info, SINT32 value= 0, bool variable= FALSE) :
    info(info), val(value), checked(FALSE), variable(variable)
  { }
};

#endif
