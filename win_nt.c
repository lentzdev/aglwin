/* AGLWIN video windows manager
 * Copyright (C) 1990-2001 Arjen G. Lentz
 *
 * This file is part of AGLWIN.
 * AGLWIN is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


/* win_nt.c - Windows95/NT dependent functions (AGL 1996) */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#ifdef FAR
#undef FAR
#endif
#include "window.h"
#undef Left	// in win_keys.h
#undef Right	// in win_keys.h


#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define MODE_BW40	   0		/* Black & white, 40 columns	     */
#define MODE_C40	   1		/* Color, 40 columns		     */
#define MODE_BW80	   2		/* Black & white, 80 columns	     */
#define MODE_C80	   3		/* Color, 80 columns		     */
#define MODE_MONO	   7		/* Monochrome, 80 columns	     */

#define SCR_BLANKED	0x0020		/* Screen is currently blanked	     */
#define SCR_STATESAVED	0x0040		/* State saved - system disabled     */

enum { VDU_UNKNOWN, VDU_MONO, VDU_CGA, VDU_EGA, VDU_VGACOLOUR, VDU_VGAMONO
};

enum { MT_NONE, MT_ATBIOS, MT_DDOS, MT_DESQVIEW, MT_TOPVIEW, MT_MULTILINK,
       MT_PCMOS, MT_WIN386, MT_WIN3, MT_WIN3E, MT_OS2
};


static SCR_PTR	work_buf;		/* pointer to work/blank buffer      */
static word	scr_flags;		/* display types and writing methods */
static word	cur_normalattrib,	/* cursor shapes normal and insert   */
		cur_insertattrib;
static HANDLE hConsoleInput = INVALID_HANDLE_VALUE;
static HANDLE hConsoleOutput = INVALID_HANDLE_VALUE;


static USHORT VioWrtCellStr (PCH pchCellString, USHORT cbCellString,
			     USHORT usRow, USHORT usColumn, USHORT hvio)
{
	DWORD dwWritten;
	BOOL fResult = TRUE;
	static char vchars[1000];
	static WORD vattrs[1000];
	COORD dwCursorPosition;
	int i, count;
	char *p;
	unsigned char *s = (unsigned char *) pchCellString;
	WORD *q;

	if (cbCellString > 2000)
	   return (1);

	dwCursorPosition.X = usColumn;
	dwCursorPosition.Y = usRow;

	count = cbCellString >> 1;
	for (p = vchars, q = vattrs, i = 0; i < count; i++) {
	    *p++ = (char) (*s++);
	    *q++ = (WORD) (*s++);
	}

	fResult = WriteConsoleOutputCharacter(hConsoleOutput, vchars, count,
					      dwCursorPosition, &dwWritten);

	if (fResult)
	   fResult = WriteConsoleOutputAttribute(hConsoleOutput, vattrs, count,
						 dwCursorPosition, &dwWritten);

	return (fResult != TRUE);
}


static USHORT VioReadCellStr (PCH pchCellString, PUSHORT pcb,
			      USHORT usRow, USHORT usColumn, USHORT hvio)
{
	DWORD dwRead;
	BOOL fResult = TRUE;
	static char vchars[1000];
	static WORD vattrs[1000];
	COORD dwCursorPosition;
	int i, count;
	char *p;
	unsigned char *s = (unsigned char *) pchCellString;
	WORD *q;

	if ((*pcb) > 2000)
	   return (1);

	dwCursorPosition.X = usColumn;
	dwCursorPosition.Y = usRow;

	count = (*pcb) >> 1;

	fResult = ReadConsoleOutputCharacter(hConsoleOutput, vchars, count,
					     dwCursorPosition, &dwRead);

	if (fResult)
	   fResult = ReadConsoleOutputAttribute(hConsoleOutput, vattrs, count,
						dwCursorPosition, &dwRead);

	for (p = vchars, q = vattrs, i = 0; i < count; i++) {
	    *s++ = *p++;
	    *s++ = *q++;
	}

	return (fResult != TRUE);
}


int _win_exp _win_fast scr_init(void)		       /* init screen system */
{
	USHORT scrsize;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	signal(SIGINT,SIG_IGN);

	scr_flags = 0;				   /* reset all screen flags */

	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsoleOutput == INVALID_HANDLE_VALUE)
	   return (0);

	hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	if (hConsoleInput == INVALID_HANDLE_VALUE)
	   return (0);

	cur_normalattrib = 0x0e0f;
	cur_insertattrib = 0x0a0f;

	if (!GetConsoleScreenBufferInfo(hConsoleOutput, &csbi))
	   return(0);

	//win_maxhor = csbi.dwSize.X;
	//win_maxver = csbi.dwSize.Y;
	win_maxhor = 80;
	win_maxver = 25;
						  /* set window 0 cursor pos */
	windows->cur_horpos = (byte) csbi.dwCursorPosition.X;
	windows->cur_verpos = (byte) csbi.dwCursorPosition.Y;

	/* now read the current screen contents into window 0 */
	windows->win_buf = (SCR_PTR) malloc(sizeof (SCR_CELL) * win_maxhor * win_maxver);
	if (windows->win_buf == NIL)
	   return (0);
	win_setbuf(windows->win_buf,0x0720,win_maxhor * win_maxver);
	scrsize = (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL));
	VioReadCellStr((PCH) windows->win_buf,
		       (PUSHORT) &scrsize, (USHORT) 0, (USHORT) 0, 0);
	work_buf = (SCR_PTR) malloc(sizeof (SCR_CELL) * win_maxhor * win_maxver);
	if (work_buf == NIL)
	   return (0);
	win_movebuf(work_buf,windows->win_buf,win_maxhor * win_maxver);

	return (1);
}/*scr_init()*/


void _win_exp _win_fast scr_deinit(void)		      /* deinit screen system */
{
	if (scr_flags & SCR_STATESAVED)
	   (void) win_restorestate();

	free(work_buf);
}/*scr_deinit()*/


void _win_exp _win_fast scr_savestate(void)  /* save entire screen/mouse state and reset */
{
	CONSOLE_CURSOR_INFO cci;
	COORD dwCursorPosition;

	if (windows == NULL || (scr_flags & SCR_STATESAVED))
	   return;

	scr_unblank();

	win_keypurge();

	VioWrtCellStr((PCH) windows->win_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, 0);

	(void) GetConsoleCursorInfo(hConsoleOutput, &cci);

	if (windows->cur_type == CUR_INSERT) {
	   cci.dwSize	= 30UL;
	   cci.bVisible = TRUE;
	}
	else if (windows->cur_type == CUR_NONE) {
	   cci.bVisible = FALSE;
	}
	else {
	   cci.dwSize	= 10UL;
	   cci.bVisible = TRUE;
	}

	SetConsoleCursorInfo (hConsoleOutput, &cci);

	dwCursorPosition.X = windows->win_left + windows->cur_left + windows->cur_horpos;
	dwCursorPosition.Y = windows->win_top + windows->cur_top + windows->cur_verpos;
	(void) SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);

	scr_flags |= SCR_STATESAVED;
}/*scr_savestate()*/


void _win_exp _win_fast scr_restorestate(void)	/* restore previously saved screen state */
{
	if (windows == NULL || !(scr_flags & SCR_STATESAVED))
	   return;

	scr_flags &= ~SCR_STATESAVED;

	VioWrtCellStr((PCH) work_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, 0);

	scr_setcur();
	scr_setpos();

	win_keypurge(); 		       /* reset keyboard ring buffer */
}/*scr_restorestate()*/


void _win_exp _win_fast scr_blank(void)        /* blank system screen and hide cursor */
{
	CONSOLE_CURSOR_INFO cci;
	COORD coord;
	DWORD Written;

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	(void) GetConsoleCursorInfo(hConsoleOutput, &cci);
	cci.bVisible = FALSE;
	SetConsoleCursorInfo(hConsoleOutput, &cci);

	coord.X = 0;
	coord.Y = 0;
	FillConsoleOutputAttribute(hConsoleOutput,
				   0U, (ULONG) (win_maxhor * win_maxver),
				   coord, &Written);

	scr_flags |= SCR_BLANKED;
}/*scr_blank()*/


void _win_exp _win_fast scr_unblank(void)	    /* unblank system screen and restore cursor */
{
	if (!(scr_flags & SCR_BLANKED))
	   return;

	VioWrtCellStr((PCH) work_buf,
		      (USHORT) (win_maxhor * win_maxver * sizeof (SCR_CELL)),
		      (USHORT) 0, (USHORT) 0, 0);

	scr_flags &= ~SCR_BLANKED;
	scr_setcur();
}/*scr_unblank()*/


void _win_exp _win_fast scr_bell(void)	       /* make bell (ASCII 07) sound one way or another */
{
	DWORD dwWritten;

	if (scr_flags & SCR_STATESAVED)
	   return;

	(void) WriteConsole(hConsoleOutput, "\007", 1UL, &dwWritten, NULL);
}/*scr_bell()*/


void _win_exp _win_fast scr_update(WIN_REC *wp, byte hor, byte ver, int len)
{
	win_movebuf(work_buf + (wp->win_top + ver) * win_maxhor +
			       wp->win_left + hor,
		    wp->win_buf + ver * wp->win_horlen + hor,len);

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	VioWrtCellStr((PCH) (wp->win_buf + ver * wp->win_horlen + hor),
		      (USHORT) (len * sizeof (SCR_CELL)),
		      (USHORT) (wp->win_top + ver),
		      (USHORT) (wp->win_left + hor), 0);
}/*scr_update()*/


void _win_exp _win_fast scr_scrollup (byte left, byte top, byte width, byte depth,
			    byte lines, byte fillattrib)
{
	SCR_CELL c = ((word) fillattrib << 8) | (byte) ' ';
	SMALL_RECT srct;
	COORD	   coord;
	CHAR_INFO  charinfo;
	byte ver;

	if (width == win_maxhor) {
	   win_movebuf(work_buf + top * win_maxhor,
		       work_buf + (top + lines) * win_maxhor,
		       width * (depth - lines));
	   win_setbuf(work_buf + ((top + depth) - lines) * win_maxhor,c,width * lines);
	}
	else {
	   for (ver = top; ver <= (top + (depth - 1)) - lines; ver++)
	       win_movebuf(work_buf + ver * win_maxhor + left,
			   work_buf + (ver + lines) * win_maxhor + left,
			   width);
	   for (ver = (top + depth) - lines; ver < top + depth; ver++)
	       win_setbuf(work_buf + ver * win_maxhor + left,c,width);
	}

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	srct.Left   = left;
	srct.Top    = top + lines;
	srct.Right  = (left + width) - 1;
	srct.Bottom = (top + depth) - 1;
	coord.X = left;
	coord.Y = top;
	charinfo.Char.AsciiChar = ' ';
	charinfo.Attributes	= fillattrib;
	(void) ScrollConsoleScreenBuffer(hConsoleOutput,&srct,NULL,
					 coord,&charinfo);
}/*scr_scrollup()*/


void _win_exp _win_fast scr_scrolldown (byte left, byte top, byte width, byte depth,
			      byte lines, byte fillattrib)
{
	SCR_CELL c = ((word) fillattrib << 8) | (byte) ' ';
	SMALL_RECT srct;
	COORD	   coord;
	CHAR_INFO  charinfo;
	byte ver;

	if (width == win_maxhor) {
	   win_movebuf(work_buf + (top + lines) * win_maxhor,
		       work_buf + top * win_maxhor,
		       width * (depth - lines));
	   win_setbuf(work_buf + top * win_maxhor,c,width * lines);
	}
	else {
	   for (ver = top + (depth - 1); ver >= top + lines; ver--)
	       win_movebuf(work_buf + ver * win_maxhor + left,
			   work_buf + (ver - lines) * win_maxhor + left,
			   width);
	   for (ver = top + lines - 1; ver >= top; ver--)
	       win_setbuf(work_buf + ver * win_maxhor + left,c,width);
	}

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	srct.Left   = left;
	srct.Top    = top;
	srct.Right  = (left + width) - 1;
	srct.Bottom = ((top + (depth)) - lines) - 1;
	coord.X = left;
	coord.Y = top + lines;
	charinfo.Char.AsciiChar = ' ';
	charinfo.Attributes	= fillattrib;
	(void) ScrollConsoleScreenBuffer(hConsoleOutput,&srct,NULL,
					 coord,&charinfo);
}/*scr_scrolldown()*/


void _win_exp _win_fast scr_setcur(void)      /* set cur shape/size to cur_status of top window */
{
	byte curtype;
	CONSOLE_CURSOR_INFO cci;

	if (scr_flags & (SCR_BLANKED | SCR_STATESAVED))
	   return;

	curtype = win_getwp(win_gettop())->cur_type;

	(void) GetConsoleCursorInfo(hConsoleOutput, &cci);

	if (curtype == CUR_INSERT) {
	   cci.dwSize	= 30UL;
	   cci.bVisible = TRUE;
	}
	else if (curtype == CUR_NONE) {
	   cci.bVisible = FALSE;
	}
	else {
	   cci.dwSize	= 10UL;
	   cci.bVisible = TRUE;
	}

	SetConsoleCursorInfo (hConsoleOutput, &cci);
}/*scr_setcur()*/


void _win_exp _win_fast scr_setpos(void)
{
	WIN_REC *wp;
	COORD dwCursorPosition;

	if (scr_flags & SCR_STATESAVED)
	   return;

	wp = win_getwp(win_gettop());
	dwCursorPosition.X = wp->win_left + wp->cur_left + wp->cur_horpos;
	dwCursorPosition.Y = wp->win_top + wp->cur_top + wp->cur_verpos;

	(void) SetConsoleCursorPosition (hConsoleOutput, dwCursorPosition);
}/*scr_setpos()*/


void _win_exp _win_fast win_setbuf(SCR_PTR dst, SCR_CELL c, int len)
{
	if (!len) return;

	while (len)
	      dst[--len] = c;
}/*win_setbuf()*/


void _win_exp _win_fast win_movebuf(SCR_PTR dst, SCR_PTR src, int len)
{
	memmove(dst,src,len * sizeof (SCR_CELL));
}/*win_movebuf()*/


void _win_exp cdecl win_timeslice(void)
{
	Sleep(1L);
}/*win_timeslice()*/


word _win_exp _win_fast win_getvdutype (void)
{
	/* How to find out under Win95/NT ?? */

	return (VDU_VGACOLOUR);
}/*win_getvdutype()*/


word _win_exp _win_fast win_getmttype (void)
{
	return (MT_WIN386);
}/*win_getmttype()*/


void _win_exp _win_fast win_setvdudirect (int direct)
{
	direct = direct;  /* just to remove warning */
}/*win_setvdudirect()*/


void _win_exp _win_fast key_fillbuf(void)
{
	word scancode;
	int iKey;
	INPUT_RECORD irBuffer;
	DWORD pcRead;

	if (scr_flags & SCR_STATESAVED)
	   return;

	while (!win_keyfull()) {

	iKey = 0;
	if (WaitForSingleObject(hConsoleInput, 0L) == 0) {
	   memset (&irBuffer, 0, sizeof (INPUT_RECORD));
	   ReadConsoleInput(hConsoleInput, &irBuffer, 1, &pcRead);
	   if (irBuffer.EventType == KEY_EVENT &&
	       irBuffer.Event.KeyEvent.bKeyDown != 0 &&
	       irBuffer.Event.KeyEvent.wRepeatCount <= 1) {
	      unsigned short vk;
	      unsigned short vs;
	      unsigned char uc;
	      BOOL fShift;
	      BOOL fAlt;
	      BOOL fCtrl;

	      vk = irBuffer.Event.KeyEvent.wVirtualKeyCode;
	      vs = irBuffer.Event.KeyEvent.wVirtualScanCode;
	      uc = irBuffer.Event.KeyEvent.uChar.AsciiChar;

	      fShift = (irBuffer.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED));
	      fAlt = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED + LEFT_ALT_PRESSED));
	      fCtrl = (irBuffer.Event.KeyEvent.dwControlKeyState & (RIGHT_CTRL_PRESSED + LEFT_CTRL_PRESSED));

	      /* The following is ugly, incomplete and nonportable.
		 It mostly handles Fkeys; Alt+Fkeys; Shift+FKeys;
		 Printable charactes; Alt+printable characters;
		 Ctrl+printable characters.
	      */

	      if (uc == 0) {				/* FKeys */
		 if (vk == 0x21) {			/* PG UP */
		    if (fCtrl)				/* Special case     */
		       vs = 0x84;			/* CTRL+PG UP	    */
		 }
		 else if (vk == 0x22) { 		/* PG DN */
		    if (fCtrl)				/* Special case     */
		       vs = 0x76;			/* CTRL+PG DN	    */
		 }
		 else if (vk == 0x23) { 		/* END */
		    if (fCtrl)				/* Special case     */
		       vs = 0x75;			/* CTRL+END	    */
		 }
		 else if (vk == 0x24) { 		/* HOME */
		    if (fCtrl)				/* Special case     */
		       vs = 0x77;			/* CTRL+HOME	    */
		 }
		 else if (vk == 0x26) { 		/* UP AR */
		    if (fCtrl)				/* Special case     */
		       vs = 0x8D;			/* CTRL+UP AR	    */
		 }
		 else if (vk == 0x28) { 		/* DN AR */
		    if (fCtrl)				/* Special case     */
		       vs = 0x91;			/* CTRL+DN AR	    */
		 }
		 else if (vk >= 0x70 && vk <= 0x79) {	/* FKeys */
		    if (fAlt)
		       vs += 0x2d;			/* Hack Alt+FKey    */
		    else if (fShift)
		       vs += 0x19;			/* Hack Shift+Fkey  */
		 }

		 if (vk > 0x20 && vk < 0x92)		/* If it's OK   */
		    iKey = (vs << 8);			/* use scan code    */
	      }
	      else {
		 if (fAlt)				/* Hack Alt Key     */
		    iKey = (vs << 8);
		 else if (fCtrl)			/* Hack Ctrl Key    */
		    iKey = (vk & 0xBF);
		 else
		    iKey = uc;
	      }
	   }
	}

	      if (iKey == 0)
		 break;
	      scancode = iKey;
	      if (!win_keyputc(scancode & 0xff ?
			       scancode & 0xff : 0x100 | (scancode >> 8)))
		    break;
	}
}/*key_fillbuf()*/


int _win_exp _win_fast mcur_enable (void)
{
	return (0);
}/*mcur_enable()*/


void _win_exp _win_fast mcur_disable (void)
{
	return;
}/*mcur_disable()*/


void _win_exp _win_fast mcur_show (void)
{
	return;
}/*mcur_show()*/


void _win_exp _win_fast mcur_hide (void)
{
	return;
}/*mcur_hide()*/


void _win_exp _win_fast mcur_reset (void)
{
	return;
}/*mcur_reset()*/

void _win_exp _win_fast mcur_setrange (byte left, byte top, byte right, byte bottom)
{
	left = left; top = top; right = right; bottom = bottom;

	return;
}/*mcur_setrange()*/


void _win_exp _win_fast mcur_getrange (byte *left, byte *top, byte *right, byte *bottom)
{
	left = left; top = top; right = right; bottom = bottom;

	return;
}/*mcur_getrange()*/


void _win_exp _win_fast mcur_setpos (byte horpos, byte verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_setpos()*/


void _win_exp _win_fast mcur_getpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_getpos()*/


word _win_exp _win_fast mcur_status (void)
{
	return (0);
}/*mcur_status()*/


word _win_exp _win_fast mcur_buttons (void)
{
	return (0);
}/*mcur_buttons()*/


WIN_IDX _win_exp _win_fast mcur_getwin (void)
{
	return (0);
}/*mcur_getwin()*/


void _win_exp _win_fast mcur_getwinpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return;
}/*mcur_getwinpos()*/


int _win_exp _win_fast mcur_getcurpos (byte *horpos, byte *verpos)
{
	horpos = horpos; verpos = verpos;

	return (0);
}/*mcur_getcurpos()*/


/* end of win_nt.c */
