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

// Odczyt i zapis kodu wynikowego w postaci Intel-HEX

#include "StdAfx.h"
#include "MotorolaSRecord.h"
#include "resource.h"
#include <TCHAR.h>
#include "MarkArea.h"
#include "Sym6502.h"


void CMotorolaSRecord::SaveHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int prog_start/*= -1*/)
{
  TCHAR buf[80], *ptr;

  for (UINT part=0; part<area.GetSize(); part++)
  {
    int start,end;

    area.GetPartition(part,start,end);
    ASSERT(start >= 0 && start <= 0xFFFF);
    ASSERT(end >= 0 && end <= 0xFFFF);
    ASSERT(start <= end);

    const int STEP= 0x10;
    for (int i=start; i<=end; i+=STEP)
    {
      int sum= 0;				// suma kontrolna
      int lim= min(i+STEP-1, end);
	// pocz¹tek wiersza: iloœæ danych, adres (hi, lo), zero
      int cnt= i+STEP<=end ? STEP : end-i+1;	// iloœæ bajtów do wys³ania (w wierszu)
      ptr = buf + wsprintf(buf,_T("S1%02X%02X%02X"),cnt+3,(i>>8)&0xFF,i&0xFF);
      sum += cnt+3 + ((i>>8)&0xFF) + (i&0xFF);
      for (int j=i; j<=lim; j++)
      {
        ptr += wsprintf(ptr,_T("%02X"),mem[j]);
	sum += mem[j];
      }		// suma wszystkich bajtów w wierszu musi byæ równa zeru
      ptr += wsprintf(ptr,_T("%02X\r\n"),~sum & 0xFF);	// wygenerowanie bajtu kontrolnego
      archive.Write(buf,sizeof(TCHAR)*(ptr-buf));
    }
  }
  if (prog_start == -1)
    prog_start = 0;
  {
    ASSERT(prog_start >= 0 && prog_start <= 0xFFFF);
    int sum= ~( ((prog_start>>8)&0xFF) + (prog_start&0xFF) + 3 ) & 0xFF;
    ptr = buf + wsprintf(buf,_T("S9%02X%02X%02X%02X\r\n"),3,
      (prog_start>>8)&0xFF, prog_start&0xFF, sum);
  }

  archive.Write(buf,sizeof(TCHAR)*(ptr-buf));
}

//-----------------------------------------------------------------------------

UINT CMotorolaSRecord::geth(const TCHAR *&ptr, UINT &sum)	// interpretacja dwucyfrowej liczby hex
{
  UINT res= 0;
  for (int i=0; i<2; i++)
  {
    res <<= 4;
    switch (*ptr++)
    {
      case _T('0'):
	break;
      case _T('1'):
	res++;
	break;
      case _T('2'):
	res += 2;
	break;
      case _T('3'):
	res += 3;
	break;
      case _T('4'):
	res += 4;
	break;
      case _T('5'):
	res += 5;
	break;
      case _T('6'):
	res += 6;
	break;
      case _T('7'):
	res += 7;
	break;
      case _T('8'):
	res += 8;
	break;
      case _T('9'):
	res += 9;
	break;
      case _T('A'):
	res += 10;
	break;
      case _T('B'):
	res += 11;
	break;
      case _T('C'):
	res += 12;
	break;
      case _T('D'):
	res += 13;
	break;
      case _T('E'):
	res += 14;
	break;
      case _T('F'):
	res += 15;
	break;
      case _T('a'):
	res += 10;
	break;
      case _T('b'):
	res += 11;
	break;
      case _T('c'):
	res += 12;
	break;
      case _T('d'):
	res += 13;
	break;
      case _T('e'):
	res += 14;
	break;
      case _T('f'):
	res += 15;
	break;
      default:
	throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
    }
  }
  sum += res;		// do liczenia sumy kontrolnej
  sum &= 0xFF;
  return res;
}

//-----------------------------------------------------------------------------

void CMotorolaSRecord::LoadHexFormat(CArchive &archive, COutputMem &mem, CMarkArea &area, int &prog_start)
{
  TCHAR buf[256];

  for (row=1; ; row++)
  {
    if (!archive.ReadString(buf,sizeof buf))
      break;

    if (_tcsclen(buf) == sizeof(buf)-1)		// za d³ugi wiersz?
      CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row);

    const TCHAR *ptr= buf;
    if (*ptr++ != _T('S'))	// nierozpoznany format?
      throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
    UINT info= *ptr++;		// bajt informacyjny
    UINT sum= 0;		// zmienna do liczenia sumy kontrolnej
    UINT cnt= geth(ptr,sum);	// iloœæ bajtów danych
    if (cnt < 3)		// nierozpoznany format?
      throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
    cnt -= 3;
    UINT addr= geth(ptr,sum);
    addr <<= 8;
    addr += geth(ptr,sum);	// adres danych
    switch (info)
    {
      case '1':		// kod programu (S1)
      {
	if (cnt)
	  area.SetStart(addr);
	for (UINT i=0; i<cnt; i++)
	{
	  if (addr > 0xFFFF)	// za du¿y adres?
	    throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
	  mem[addr++] = (UINT8)geth(ptr,sum);
	}
	geth(ptr,sum);		// bajt sumy kontrolnej
	if (sum != 0xFF)	// b³¹d sumy kontrolnej
	  throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM,row));
	if (cnt)
	  area.SetEnd(addr-1);
	break;
      }

      case '9':		// koniec, ew. adres uruchomienia (S9)
	if (cnt)	// nieoczekiwane dane?
	  throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
	geth(ptr,sum);		// bajt sumy kontrolnej
	if (sum != 0xFF)	// b³¹d sumy kontrolnej?
	  throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_CHKSUM,row));
	if (addr > 0xFFFF)	// za du¿y adres?
	  throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
	prog_start = (int)addr;
	break;

      default:		// nieznana wartoœæ
	throw (new CMotorolaSRecordException(CMotorolaSRecordException::E_FORMAT,row));
    }
  }

}

//-----------------------------------------------------------------------------

/*virtual*/ BOOL CMotorolaSRecord::CMotorolaSRecordException::GetErrorMessage(LPTSTR lpszError,
  UINT nMaxError, PUINT pnHelpContext/*= NULL*/)
{
  CString msg;
  TCHAR num[16];
  if (pnHelpContext != NULL)
    *pnHelpContext = 0;
  wsprintf(num,_T("%u"),row);
  switch (error)
  {
    case E_BAD_FORMAT:		// b³êdny format pliku
      msg.LoadString(IDS_INTEL_HEX_ERR_2);
      break;
    case E_CHKSUM:		// b³¹d sumy kontrolnej
      AfxFormatString1(msg,IDS_INTEL_HEX_ERR_1,num);
      break;
    case E_FORMAT:		// b³êdny format danych
      AfxFormatString1(msg,IDS_INTEL_HEX_ERR_3,num);
      break;
    default:
      return FALSE;
  }
  _tcsnccpy(lpszError,msg,nMaxError);
  return TRUE;
}
