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


/* con_emu.c - terminal emulation functions */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "window.h"


#ifdef __PROTO__
static void near _win_fast con_putraw	 (byte c);
static void near _win_fast con_putcooked (byte c);
static void near _win_fast con_putansi	 (byte c);
static void near _win_fast con_putvt52	 (byte c);
static void near _win_fast con_putavatar (byte c);
#endif


static void near _win_fast con_putraw(c)    /* raw output, only CR & LF xlat */
byte c;
{
	switch (c) {
	       case 0:	/* NUL character, do or print nothing */
		    break;

	       case 10: /* linefeed, move down one line, bottom = scroll */
		    wp->cur_verpos++;
		    /* fallthrough to CR 13 */

	       case 13: /* move cursor to first position of current line */
		    wp->cur_horpos = 0;
		    break;

	       default: /* print char and advance cursor, scroll if needed */
		    *(wp->win_buf +
		      (wp->cur_top + wp->cur_verpos) * wp->win_horlen +
		      wp->cur_left + wp->cur_horpos) =
				     ((word) wp->chr_attrib << 8) | c;
		    if (*(win_tpl +
			  (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
			  wp->win_left + wp->cur_left + wp->cur_horpos) == win_idx)
		       scr_update(wp,wp->cur_left + wp->cur_horpos,
					  wp->cur_top + wp->cur_verpos,1);

		    wp->cur_horpos++;
		    break;
	}
}/*con_putraw()*/


static void near _win_fast con_putcooked(c)		    /* normal output */
byte c;
{
	switch (c) {
	       case 7:	/* BELL character, make BIOS beep once */
		    scr_bell();
		    break;

	       case 8:	  /* BACKSPACE, move back and delete one char */
	       case 127:  /* DELETE, ditto */
		    if (wp->cur_horpos > 0) {
		       wp->cur_horpos--;
		       *(wp->win_buf +
			 (wp->cur_top + wp->cur_verpos) * wp->win_horlen +
			 wp->cur_left + wp->cur_horpos) =
				((word) wp->chr_attrib << 8) | (byte) ' ';
		       if (*(win_tpl +
			     (wp->win_top + wp->cur_top + wp->cur_verpos) * win_maxhor +
			     wp->win_left + wp->cur_left + wp->cur_horpos) == win_idx)
			  scr_update(wp,wp->cur_left + wp->cur_horpos,
				     wp->cur_top + wp->cur_verpos,1);
		    }
		    break;

	       case 9:	/* TAB, every eighth char move to next !(horpos % 8) */
		    do con_putraw((byte) ' ');
		    while (++wp->cur_horpos < wp->cur_horlen &&
			   wp->cur_horpos % 8);
		    break;

	       case 11: /* HOME, move cursor to top left of window */
		    wp->cur_horpos = wp->cur_verpos = 0;
		    break;

	       case 12: /* FORM FEED, clear scr and move cur to win top left */
		    win_cls(win_idx);
		    break;

	       case 28: /* cursor right */
		    if (wp->cur_horpos < (wp->cur_horlen - 1))
		       wp->cur_horpos++;
		    break;

	       case 29: /* cursor left */
		    if (wp->cur_horpos > 0)
		       wp->cur_horpos--;
		    break;

	       case 30: /* cursor up */
		    if (wp->cur_verpos > 0)
		       wp->cur_verpos--;
		    break;

	       case 31: /* cursor down */
		    if (wp->cur_verpos < (wp->cur_verlen - 1))
		       wp->cur_verpos++;
		    break;

	       default:
		    con_putraw(c);
		    break;
	}
}/*con_putcooked()*/


static void near _win_fast con_putansi(c)   /* ANSI-IBM and VT-100 emulation */
byte c;
{
	if (!wp->con_seqlen)
	   goto norm;

	if (wp->con_seqlen == 1) {
	   switch (c) {
		  case '[':
		  case '(':
		  case ')':
		       wp->con_seqbuf[wp->con_seqlen++] = c;
		       wp->con_seqbuf[wp->con_seqlen] = 0;
		       return;

		  case '7':  /* DECSC DEC Save Cursor Parameters */
		       wp->con_horsav  = wp->cur_horpos;
		       wp->con_versav  = wp->cur_verpos;
		       wp->chr_attrsav = wp->chr_attrib;
		       break;

		  case '8':  /* DECRC Restore Cursor Parameters */
		       wp->cur_horpos = wp->con_horsav;
		       wp->cur_verpos = wp->con_versav;
		       wp->chr_attrib = wp->chr_attrsav;
		       break;

		  case 'c':  /* RIS Reset to Initial State */
		       win_addattrib(win_idx,0);
		       win_cls(win_idx);
		       break;

		  case 'E': /* NEL Next Line */
		       wp->cur_verpos++;
		       wp->cur_horpos = 0;
		       break;

		  default:  /* Invalid or unsupported sequence char */
		       goto reset;
	   }

	   goto done;
	}

	if (wp->con_seqlen == 2 && strchr("()",wp->con_seqbuf[1])) {
	   /* ignore for the moment */
	   goto done;
	}

	switch (c) {
	       case '=':  /* ignore */
	       case '?':
		    return;
	       case ';':  /* next parm */
		    if (wp->con_seqlen >= MAX_SEQ)
		       goto reset;
		    wp->con_seqbuf[++wp->con_seqlen] = 0;
		    return;

	       case '0': case '1': case '2': case '3': case '4':   /* digits */
	       case '5': case '6': case '7': case '8': case '9':
		    wp->con_seqbuf[wp->con_seqlen] *= 10;
		    wp->con_seqbuf[wp->con_seqlen] += (c - '0');
		    return;

	       case 'H':  /* CUP Cursor Position	      */
	       case 'f':  /* HVP Horizontal Vertical Position */
	       case 'j':  /* QuickBBS-ST can do this one !?   */
		    wp->cur_horpos = wp->cur_verpos = 0;
		    if (wp->con_seqlen >= 2) {
		       if (wp->con_seqbuf[2]) {
			  wp->cur_verpos = wp->con_seqbuf[2] - 1;
			  if (wp->cur_verpos >= wp->cur_verlen)
			     wp->cur_verpos = wp->cur_verlen - 1;
		       }
		       if (wp->con_seqlen == 3 && wp->con_seqbuf[3]) {
			  wp->cur_horpos = wp->con_seqbuf[3] - 1;
			  if (wp->cur_horpos >= wp->cur_horlen)
			     wp->cur_horpos = wp->cur_horlen - 1;
		       }
		    }
		    break;

	       case 'A':  /* CUU Cursor Up */
		    if (wp->cur_verpos > 0 && wp->con_seqlen == 2) {
		       if (wp->con_seqbuf[2]) {
			  int i;
			  i = (int) wp->cur_verpos - (int) wp->con_seqbuf[2];
			  wp->cur_verpos = i < 0 ? 0 : i;
		       }
		       else
			  wp->cur_verpos--;
		    }
		    break;

	       case 'B':  /* CUD Cursor Down */
		    if (wp->cur_verpos < (wp->cur_verlen - 1) &&
			wp->con_seqlen == 2) {
		       if (wp->con_seqbuf[2]) {
			  wp->cur_verpos += wp->con_seqbuf[2];
			  if (wp->cur_verpos >= wp->cur_verlen)
			     wp->cur_verpos = wp->cur_verlen - 1;
		       }
		       else
			  wp->cur_verpos++;
		    }
		    break;

	       case 'C':  /* CUF Cursor Forward */
		    if (wp->cur_horpos < (wp->cur_horlen - 1) &&
			wp->con_seqlen == 2) {
		       if (wp->con_seqbuf[2]) {
			  wp->cur_horpos += wp->con_seqbuf[2];
			  if (wp->cur_horpos >= wp->cur_horlen)
			     wp->cur_horpos = wp->cur_horlen - 1;
		       }
		       else
			  wp->cur_horpos++;
		    }
		    break;

	       case 'D':  /* CUB Cursor Backward */
		    if (wp->cur_horpos > 0 && wp->con_seqlen == 2) {
		       if (wp->con_seqbuf[2]) {
			  int i;
			  i = (int) wp->cur_horpos - (int) wp->con_seqbuf[2];
			  wp->cur_horpos = i < 0 ? 0 : i;
		       }
		       else
			  wp->cur_horpos--;
		    }
		    break;

	       case 'n':  /* DSR Device Status Report */
		    if (wp->con_seqlen == 2 && wp->con_seqbuf[2] == 6) {
		       WIN_IDX winsav;
		       byte oldkeyemu;
		       char cpr_str[20];

		       winsav = win_idx;
			   oldkeyemu = win_getkeyemu(win_gettop());
			   sprintf(cpr_str,"\033[%u;%uR",
			       wp->cur_verpos + 1, wp->cur_horpos + 1);
		       win_setkeyemu(win_gettop(),KEY_RAW);
		       win_keyputs(cpr_str);
		       win_setkeyemu(win_gettop(),oldkeyemu);
		       win_setwp(winsav);
		    }
		    break;

	       case 'r':  /* Set top/bottom of scrolling window */
		    /* ignore for the moment */
		    break;

	       case 's':  /* SCP Save Cursor Position */
		    if (wp->con_seqlen == 2 && !wp->con_seqbuf[2]) {
		       wp->con_horsav = wp->cur_horpos;
		       wp->con_versav = wp->cur_verpos;
		    }
		    break;

	       case 'u':  /* RCP Restore Cursor Position */
		    if (wp->con_seqlen == 2 && !wp->con_seqbuf[2]) {
		       wp->cur_horpos = wp->con_horsav;
		       wp->cur_verpos = wp->con_versav;
		    }
		    break;

	       case 'J':  /* ED Erase in Display */
		    if (wp->con_seqlen > 2) break;
		    switch (wp->con_seqbuf[2]) {
			   case 0:  if (wp->cur_verpos < (wp->cur_verlen - 1)) {
				       wp->cur_verpos++;
				       win_clreos(win_idx);
				       wp->cur_verpos--;
				    }
				    win_clreol(win_idx);
				    break;
			   case 1:  if (wp->cur_verpos > 0) {
				       wp->cur_verpos--;
				       win_clrbos(win_idx);
				       wp->cur_verpos++;
				    }
				    win_clrbol(win_idx);
				    break;
			   case 2:  win_cls(win_idx);
				    break;
			   default: break;
		    }
		    break;

	       case 'K':  /* EL Erase in Line */
		    if (wp->con_seqlen > 2) break;
		    switch (wp->con_seqbuf[2]) {
			   case 0:  win_clreol(win_idx);
				    break;
			   case 1:  win_clrbol(win_idx);
				    break;
			   case 2:  win_clrbol(win_idx);
				    win_clreol(win_idx);
				    break;
			   default: break;
		    }
		    break;

	       case 'm':  /* SGR Set Graphics Rendition */
		    { byte i;
		      for (i = 2; i <= wp->con_seqlen; i++)
			  win_addattrib(win_idx,wp->con_seqbuf[i]);
		    }
		    break;

	       case 'h':  /* SM Set Mode */
		    if (wp->con_seqlen == 2 && wp->con_seqbuf[2] == 7)
		       win_setconemu(win_idx,win_getconemu(win_idx) | CON_WRAP);
		    break;

	       case 'I':  /* RM Reset Mode */
		    if (wp->con_seqlen == 2 && wp->con_seqbuf[2] == 7)
		       win_setconemu(win_idx,win_getconemu(win_idx) & ~CON_WRAP);
		    break;

	       case 'L':  /* IL Insert line */
		    if (wp->con_seqlen == 2)
		       win_insline(win_idx,wp->con_seqbuf[2] ?
					   wp->con_seqbuf[2] : 1);
		    break;

	       case 'M':  /* DL Delete line */
		    if (wp->con_seqlen == 2)
		       win_delline(win_idx,wp->con_seqbuf[2] ?
					   wp->con_seqbuf[2] : 1);
		    break;

	       case 'S':  /* Scroll up */
		    if (wp->con_seqlen == 2)
		       win_scrollup(win_idx,wp->con_seqbuf[2] ?
					    wp->con_seqbuf[2] : 1);
		    break;

	       case 'T':  /* Scroll down */
		    if (wp->con_seqlen == 2)
		       win_scrolldown(win_idx,wp->con_seqbuf[2] ?
					      wp->con_seqbuf[2] : 1);
		    break;

	       default:  /* Invalid or unsupported sequence char */
		    goto reset;
	}

done:	wp->con_seqlen = 0;
	return;

reset:	wp->con_seqlen = 0;

norm:	switch (c) {
	       case 27:  /* ANSI escape character */
		    wp->con_seqbuf[wp->con_seqlen++] = c;
		    break;

	       case 15:  /* SI	Shift Out */
	       case 14:  /* SO	Shift In  */
		    /* ignore for the moment */
		    break;

	       case 10: /* linefeed, move down one line, bottom = scroll */
		    /* different from normal: don't change horizontal pos */
		    /* ANSI specs say no CR, so... we adhere to that ;-) */
		    wp->cur_verpos++;
		    break;

	       case  8:  /* BACKSPACE, move back and DO NOT delete one char */
		    if (wp->cur_horpos > 0)
		       wp->cur_horpos--;
		    break;

	       case  9: /* TAB, every eighth char move to next !(horpos % 8) */
		    do wp->cur_horpos++;
		    while (wp->cur_horpos < wp->cur_horlen &&
			   wp->cur_horpos % 8);
		    break;

	       default:
		    if ((wp->con_emu & CON_INSERT) && c > 31)
		       win_inschar(win_idx);

		    con_putcooked(c);
		    break;
	}
}/*con_putansi()*/


static void near _win_fast con_putvt52(c)		     /* VT-52 emulation */
byte c;
{
	if (!wp->con_seqlen)
	   goto norm;

	if (wp->con_seqlen == 1) {
	   wp->con_seqbuf[wp->con_seqlen++] = c;
	   switch (c) {
		  case 'A':  /* Cursor Up */
		       if (wp->cur_verpos > 0)
			  wp->cur_verpos--;
		       break;

		  case 'B':  /* Cursor Down */
		       if (wp->cur_verpos < (wp->cur_verlen - 1))
			  wp->cur_verpos++;
		       break;

		  case 'C':  /* Cursor Forward */
		       if (wp->cur_horpos < (wp->cur_horlen - 1))
			  wp->cur_horpos++;
		       break;

		  case 'D':  /* Cursor Backward */
		       if (wp->cur_horpos > 0)
			  wp->cur_horpos--;
		       break;

		  case 'E':  /* Clear screen */
		       win_cls(win_idx);
		       break;

		  case 'H':  /* Cursor home */
		       wp->cur_horpos = wp->cur_verpos = 0;
		       break;

		  case 'I':  /* Reverse linefeed */
		       if (wp->cur_verpos > 0) wp->cur_verpos--;
		       else		       win_scrolldown(win_idx,1);
		       break;

		  case 'J':  /* Clear to end of screen */
		       win_clreos(win_idx);
		       break;

		  case 'K':  /* Clear to end of line */
		       win_clreol(win_idx);
		       break;

		  case 'L':  /* Insert line and do CR */
		       win_insline(win_idx,1);
		       wp->cur_horpos = 0;
		       break;

		  case 'M':  /* Delete line and do CR */
		       win_delline(win_idx,1);
		       wp->cur_horpos = 0;
		       break;

		  case 'Y':  /* Position Cursor */
		  case 'b':  /* Set foreground colour */
		  case 'c':  /* Set background colour */
		       return;	/* they want another few parameters */

		  case 'd':  /* Clear to beginning of screen */
		       win_clrbos(win_idx);
		       break;

		  case 'e':  /* Show cursor */
		       win_setcur(win_idx,CUR_NORMAL);
		       break;

		  case 'f':  /* Hide cursor */
		       win_setcur(win_idx,CUR_HIDDEN);
		       break;

		  case 'j':  /* Save Cursor Position */
		       wp->con_horsav = wp->cur_horpos;
		       wp->con_versav = wp->cur_verpos;
		       break;

		  case 'k':  /* Restore Cursor Position */
		       wp->cur_horpos = wp->con_horsav;
		       wp->cur_verpos = wp->con_versav;
		       break;

		  case 'l':  /* Clear current line and do CR */
		       wp->cur_horpos = 0;
		       win_clreol(win_idx);
		       break;

		  case 'o':  /* Clear to beginning of line */
		       win_clrbol(win_idx);
		       break;

		  case 'p': /* Reverse video on */
		       win_setattrib(win_idx,CHR_INVERSE);
		       break;

		  case 'q': /* Reverse video off */
		       win_setattrib(win_idx,CHR_NORMAL);
		       break;

		  case 'v':  /* Set wrap on */
		       win_setconemu(win_idx,win_getconemu(win_idx) | CON_WRAP);
		       break;

		  case 'w':  /* Set wrap off */
		       win_setconemu(win_idx,win_getconemu(win_idx) & ~CON_WRAP);
		       break;

		  default:  /* Invalid or unsupported sequence char */
		       goto reset;
	   }

	   wp->con_seqlen = 0;
	   return;
	}
	else {	/* sequences with parameters */
	   wp->con_seqbuf[wp->con_seqlen++] = c;
	   switch (wp->con_seqbuf[1]) {
		  case 'Y':  /* Position Cursor */
		       if (wp->con_seqlen < 4)	/* still need more */
			  return;
		       wp->cur_verpos = wp->con_seqbuf[2] - ' ';
		       if (wp->cur_verpos >= wp->cur_verlen)
			  wp->cur_verpos = wp->cur_verlen - 1;
		       wp->cur_horpos = wp->con_seqbuf[3] - ' ';
		       if (wp->cur_horpos >= wp->cur_horlen)
			   wp->cur_horpos = wp->cur_horlen - 1;
		       break;

		  case 'b':  /* Set foreground colour */
		  case 'c':  /* Set background colour */
		       /* we need a Atari ST colour index for this one.... */
		       break;

		  default:   /* Can't happen, but just in case... */
		       goto reset;
	   }
	   wp->con_seqlen = 0;
	   return;
	}

reset:	wp->con_seqlen = 0;

norm:	if (c == 27) /* VT-52 escape character */
	   wp->con_seqbuf[wp->con_seqlen++] = c;
	else
	   con_putcooked(c);
}/*con_putvt52()*/


static void near _win_fast con_putavatar(c)		 /* Avatar emulation */
byte c;
{
	if (!wp->con_seqlen)
	   goto norm;

	if (wp->con_seqbuf[0] == 22) {	  /* Avatar control sequence <^V>... */
	   if (wp->con_seqlen != 2 || wp->con_seqbuf[1] != 13)
	      c &= 0x7f;
	   wp->con_seqbuf[wp->con_seqlen++] = c;

	   switch (wp->con_seqbuf[1]) {
		  case	1:  /* set attribute <^V><^A><attr> */
		       if (wp->con_seqlen < 3) return;
		       win_setattrib(win_idx,wp->con_seqbuf[2]);
		       break;

		  case	2:  /* turn blink on <^V><^B> */
		       win_setattrib(win_idx,win_getattrib(win_idx) | CHR_BLINK);
		       break;

		  case	3:  /* cursor up <^V><^V> */
		       if (wp->cur_verpos > 0)
			  wp->cur_verpos--;
		       break;

		  case	4:  /* cursor down <^V><^D> */
		       if (wp->cur_verpos < (wp->cur_verlen - 1))
			  wp->cur_verpos++;
		       break;

		  case	5:  /* cursor left <^V><^E> */
		       if (wp->cur_horpos > 0)
			  wp->cur_horpos--;
		       break;

		  case	6:  /* cursor right <^V><^F> */
		       if (wp->cur_horpos < (wp->cur_horlen - 1))
			  wp->cur_horpos++;
		       break;

		  case	7:  /* clear to end of line <^V><^G> */
		       win_clreol(win_idx);
		       break;

		  case	8:  /* set position <^V><^H><ver><hor> */
		       if (wp->con_seqlen < 4) return;
		       win_setpos(win_idx,wp->con_seqbuf[3],wp->con_seqbuf[2]);
		       break;

		  case	9:  /* turn insert mode on <^V><^I> */
		       wp->con_emu |= CON_INSERT;
		       goto done;

		  case 10:  /* scroll area up */
			    /* <^V><^J><numlines><top><left><bottom><right> */
		       { byte savleft, savtop, savright, savbottom;
			 byte savhor, savver;
			 byte numlines, left, top, right, bottom;

			 if (wp->con_seqlen < 7) return;

			 win_getrange(win_idx,&savleft,&savtop,&savright,&savbottom);
			 win_getpos(win_idx,&savhor,&savver);

			 if ((left = wp->con_seqbuf[4]) < 1) left = 1;
			 else if (left > wp->cur_horlen) left = wp->cur_horlen;
			 if ((top = wp->con_seqbuf[3]) < 1) top = 1;
			 else if (top > wp->cur_verlen) top = wp->cur_verlen;
			 if ((right = wp->con_seqbuf[6]) < 1) right = 1;
			 else if (right > wp->cur_horlen) right = wp->cur_horlen;
			 if ((bottom = wp->con_seqbuf[5]) < 1) bottom = 1;
			 else if (bottom > wp->cur_verlen) bottom = wp->cur_verlen;

			 left--; top--; right--; bottom--;
			 if (win_setrange(win_idx,savleft+left,savtop+top,savleft+right,savtop+bottom)) {
			    if ((numlines = wp->con_seqbuf[2]) < 1) numlines = 1;
			    win_scrollup(win_idx,numlines);
			 }

			 win_setrange(win_idx,savleft,savtop,savright,savbottom);
			 win_setpos(win_idx,savhor,savver);
		       }
		       break;

		  case 11:  /* scroll area down */
			    /* <^V><^K><numlines><top><left><bottom><right> */
		       if (wp->con_seqlen < 7) return;

		       { byte savleft, savtop, savright, savbottom;
			 byte savhor, savver;
			 byte numlines, left, top, right, bottom;

			 win_getrange(win_idx,&savleft,&savtop,&savright,&savbottom);
			 win_getpos(win_idx,&savhor,&savver);

			 if ((left = wp->con_seqbuf[4]) < 1) left = 1;
			 else if (left > wp->cur_horlen) left = wp->cur_horlen;
			 if ((top = wp->con_seqbuf[3]) < 1) top = 1;
			 else if (top > wp->cur_verlen) top = wp->cur_verlen;
			 if ((right = wp->con_seqbuf[6]) < 1) right = 1;
			 else if (right > wp->cur_horlen) right = wp->cur_horlen;
			 if ((bottom = wp->con_seqbuf[5]) < 1) bottom = 1;
			 else if (bottom > wp->cur_verlen) bottom = wp->cur_verlen;

			 left--; top--; right--; bottom--;
			 if (win_setrange(win_idx,savleft+left,savtop+top,savleft+right,savtop+bottom)) {
			    if ((numlines = wp->con_seqbuf[2]) < 1) numlines = 1;
			    win_scrolldown(win_idx,numlines);
			 }

			 win_setrange(win_idx,savleft,savtop,savright,savbottom);
			 win_setpos(win_idx,savhor,savver);
		       }
		       break;

		  case 12:  /* clear area, set attrib */
			    /* <^V><^L><attr><lines><columns> */
		       if (wp->con_seqlen < 5) return;

		       { byte savleft, savtop, savright, savbottom;
			 byte savhor, savver;
			 byte left, top, right, bottom;

			 win_setattrib(win_idx,wp->con_seqbuf[2]);

			 win_getrange(win_idx,&savleft,&savtop,&savright,&savbottom);
			 win_getpos(win_idx,&savhor,&savver);

			 left = savleft;
			 top  = savtop;
			 right = (left + wp->con_seqbuf[4]) - 1;
			 if (right > wp->cur_horlen) right = wp->cur_horlen;
			 bottom = (top + wp->con_seqbuf[3]) - 1;
			 if (bottom > wp->cur_verlen) bottom = wp->cur_verlen;

			 if (win_setrange(win_idx,left,top,right,bottom))
			    win_cls(win_idx);

			 win_setrange(win_idx,savleft,savtop,savright,savbottom);
			 win_setpos(win_idx,savhor,savver);
		       }
		       break;

		  case 13:  /* initialize area, set attrib */
			    /* <^V><^M><attr><char><lines><columns> */
		       if (wp->con_seqlen < 6) return;

		       { byte savleft, savtop, savright, savbottom;
			 byte savhor, savver;
			 byte left, top, right, bottom;

			 win_setattrib(win_idx,wp->con_seqbuf[2]);

			 win_getrange(win_idx,&savleft,&savtop,&savright,&savbottom);
			 win_getpos(win_idx,&savhor,&savver);

			 left = savleft;
			 top  = savtop;
			 right = (left + wp->con_seqbuf[5]) - 1;
			 if (right > wp->cur_horlen) right = wp->cur_horlen;
			 bottom = (top + wp->con_seqbuf[4]) - 1;
			 if (bottom > wp->cur_verlen) bottom = wp->cur_verlen;

			 if (win_setrange(win_idx,left,top,right,bottom))
			    win_fill(win_idx,wp->con_seqbuf[3]);

			 win_setrange(win_idx,savleft,savtop,savright,savbottom);
			 win_setpos(win_idx,savhor,savver);
		       }
		       break;

		  case 14:  /* del char, scroll rest of line left <^V><^N> */
		       win_delchar(win_idx);
		       break;

		  case 25:  /* repeat pattern */
			    /* <^V><^Y><numchars><char>[...]<count> */
		       if (wp->con_seqlen < 3 ||
			   wp->con_seqlen < (wp->con_seqbuf[2] + 4))
			  return;
		       { int numchars, count;
			 char buf[81];

			 if ((numchars = wp->con_seqbuf[2]) > 80) numchars = 80;
			 count = wp->con_seqbuf[3 + numchars];
			 strncpy(buf,(char *) &wp->con_seqbuf[3],numchars);
			 buf[numchars] = '\0';
			 wp->con_seqlen = 0;
			 for ( ; count > 0; count--) con_puts(buf);
		       }
		       goto done;

		  default:  /* Invalid or unsupported sequence char */
		       goto reset;
	   }
	   wp->con_emu &= ~CON_INSERT;
	   goto done;
	}

	if (wp->con_seqbuf[0] == 25) {	  /* Avatar rep.chr <^Y><chr8><num8> */
	   int i;

	   wp->con_seqbuf[wp->con_seqlen++] = c;
	   if (wp->con_seqlen < 3) return;

	   wp->con_seqlen = 0;
	   c = wp->con_seqbuf[1];
	   for (i = wp->con_seqbuf[2]; i > 0; i--)
	       con_putavatar(c);
	   goto done;
	}

	goto norm;  /* in a sequence of ANSI or something... */

done:	wp->con_seqlen = 0;
	return;

reset:	wp->con_seqlen = 0;

norm:	switch (c) {
	       case 25:  /* Avatar repeat char sequence <^Y><chr8><num8> */
	       case 22:  /* Avatar control sequence	<^V> ...	 */
		    wp->con_seqbuf[wp->con_seqlen++] = c;
		    break;

	       case 9:	/* TAB, every eighth char move to next !(horpos % 8) */
		    /* different from ANSI: don't move if already on right */
		    if (wp->cur_horpos < (wp->cur_horlen - 1)) {
		       do con_putraw((byte) ' ');
		       while (++wp->cur_horpos < wp->cur_horlen &&
			      wp->cur_horpos % 8);
		    }
		    break;

	       case 10: /* linefeed, move down one line, bottom = scroll */
		    /* Avatar specs say no CR, but practise says otherwise.. */
		    wp->cur_verpos++;
		    wp->cur_horpos = 0;
		    break;

	       case 12: /* FORM FEED, clear scr and move cur to win top left */
		    /* different from normal: reset insert, set default atr */
		    wp->con_emu &= ~CON_INSERT;
		    win_setattrib(win_idx,CHR_F_CYAN);
		    win_cls(win_idx);
		    break;

	       default:
		    con_putansi(c);
		    break;
	}
}/*con_putavatar()*/


void _win_exp _win_fast con_puts(s)    /* print string in specified window win_idx/wp */
char *s;
{
#ifdef __PROTO__
	static void near _win_fast (*con_emuputc[])(byte c) =
		       { con_putraw, con_putcooked, con_putansi, con_putvt52, con_putavatar };
	static void near _win_fast (*con_emufunc)(byte c);
#else
	static void near _win_fast (*con_emuputc[])() =
		       { con_putraw, con_putcooked, con_putansi, con_putvt52, con_putavatar };
	static void near _win_fast (*con_emufunc)();
#endif

	if (wp->con_emu & CON_UNBLANK)
	   scr_unblank();

	con_emufunc = con_emuputc[wp->con_emu & CON_EMUMASK];

	while (*s) {
	      (*con_emufunc)((byte) *s++);

	      if (wp->cur_horpos >= wp->cur_horlen) {
		 if (wp->con_emu & CON_WRAP) {
		    wp->cur_horpos = 0;
		    wp->cur_verpos++;
		 }
		 else
		    wp->cur_horpos = wp->cur_horlen - 1;
	      }

	      if (wp->cur_verpos >= wp->cur_verlen) {
		 wp->cur_verpos = wp->cur_verlen - 1;
		 if (wp->con_emu & CON_SCROLL)
		    win_scrollup(win_idx,1);
	      }
	}

	if (win_gettop() == win_idx)
	   scr_setpos();
}/*con_puts()*/


/* end of con_emu.c */
