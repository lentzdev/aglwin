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


/* input.c - input functions */
#include <string.h>
#ifdef __PROTO__
#include <stdlib.h>
#endif
#ifdef __MSDOS__
#include <mem.h>
#endif
#include "window.h"

#ifdef __PROTO__
static word near _win_fast findkey(word keyarray[], word keycode);
#endif


static word near _win_fast findkey(keyarray, keycode)
word keyarray[], keycode;
{
	for (; *keyarray && *keyarray != keycode; keyarray++);

	return (*keyarray);
}/*findkey()*/


word _win_exp cdecl win_input(horpos, verpos, width, str,
			      abortkeys, endkeys, endaction,
			      checkfunc)
byte horpos, verpos, width;
char str[];
word abortkeys[], endkeys[];
byte endaction;
#ifdef __PROTO__
int cdecl (*checkfunc)(char str[]);
#else
int cdecl (*checkfunc)();
#endif
{							 /* topwin line-edit */
	register byte i, j;
	register word c;
	byte oldkeyemu;
	int insert;
	int firstkey;
	char *s;
	word attrib;
	SCR_PTR win_ptr;
	byte mhorpos, mverpos;

	if (windows == NULL)
	   return (0);

	if (!width)				      /* zero length string? */
	   return (0);

	win_setwp(win_gettop());
	scr_unblank();

	horpos--;  /* recalc to 0 based */
	verpos--;
	if (wp->win_status != WIN_ACTIVE)	  /* win not open, or hidden */
	   return (0);
	if (horpos + (width - 1) >= wp->cur_horlen ||	 /* out of cur range */
	    verpos >= wp->cur_verlen)
	   return (0);
	if ((s = (char *) malloc(width + 1)) == NULL)
	   return (0);

	wp->cur_horpos = horpos;	   /* set cursor pos at start of str */
	wp->cur_verpos = verpos;
	attrib = wp->chr_attrib << 8;

	win_ptr = wp->win_buf +
		  (wp->cur_top + wp->cur_verpos) * wp->win_horlen +
		  wp->cur_left + wp->cur_horpos;
	s[width] = '\0';		    /* terminate definate end of str */
	firstkey = insert = 1;
	win_setcur(win_idx,CUR_INSERT);

	oldkeyemu = win_getkeyemu(win_idx);
	win_setkeyemu(win_idx,KEY_RAW);

	for (i = 0; str[i]; i++) {		  /* find end of current str */
	    s[i] = str[i];
	    win_ptr[i] = attrib | (byte) str[i];      /* and print all chars */
	}

	memset(&s[i],' ',width - i);		    /* fill rest with spaces */
	win_setbuf(&win_ptr[i],attrib | (byte) ' ',width - i);
	scr_update(wp,wp->cur_left + horpos,wp->cur_top + verpos,width);
	mcur_reset();

	for (i = 0; ; firstkey = 0) {			     /* endless loop */
top:	    wp->cur_horpos = horpos + i;      /* set cur pos at right s char */
	    scr_setpos();		       /* show cursor in right place */

	    while (!win_keyscan()) {
		  if ((mcur_buttons() & MCUR_LEFTBUTTON) ||
		      (mcur_status() & MCUR_LEFTCLICK)) {
		     if (mcur_getwin() == win_idx &&
			 mcur_getcurpos(&mhorpos,&mverpos) &&
			 (mverpos == (verpos + 1)) &&
			 (mhorpos >= (horpos + 1)) &&
			 (mhorpos <  (horpos + width + 1)))
			goto top;

		     c = MKEY_CURBUTTON;
		     goto end;
		  }

		  win_idle();
	    }

	    c = win_keygetc();

	    if (endkeys != NULL && findkey(endkeys,c)) {  /* entry completed */
	       if (c != Esc || firstkey) {
		  if (c == Esc) c = endkeys[0];
		  goto end;
	       }
	    }

	    if (abortkeys != NULL && findkey(abortkeys,c)) {  /* abort entry */
	       for (i = 0; i < width; i++)		 /* restore original */
		   win_ptr[i] = attrib | (byte) s[i];
	       scr_update(wp,wp->cur_left + horpos,wp->cur_top + verpos, width);
	       break;
	    }

	    switch (c) {
		   case Left:		     /* go one char left if possible */
		   case MKEY_LEFTMOVE:
			if (i)
			   i--;
			break;

		   case Right:		    /* go one char right if possible */
		   case MKEY_RIGHTMOVE:
			if (i < (width - 1))
			   i++;
			break;

		   case Home:			     /* go to beginning of s */
			i = 0;
			break;

		   case End:		   /* go to pos after last char in s */
			for (i = width; s[i-1] == ' ' && --i > 0; );
			if (i == width)
			   i--;
			break;

		   case Ins:			/* toggle insert mode on/off */
			insert = !insert;
			win_setcur(win_idx,insert ? CUR_INSERT : CUR_NORMAL);
			break;

		   case BS:		/* del prev char and move s/cur back */
			if (i)
			   i--;
			/* fallthrough to Del_Key */

		   case Del:		 /* del char and move rest of s back */
		   case Ctrl_BS:
			if (i < (width - 1)) {	 /* move back rest of string */
			   memmove(&s[i],&s[i+1],width - (i + 1));
			   win_movebuf(&win_ptr[i],&win_ptr[i+1],
				       width - (i + 1));
			   scr_update(wp,wp->cur_left + horpos + i,
				      wp->cur_top + verpos,width - (i + 1));
			}
			s[width - 1] = ' ';	/* insert space at last char */
			win_ptr[width - 1] = attrib | (byte) ' ';
			scr_update(wp,wp->cur_left + horpos + (width - 1),
				   wp->cur_top + verpos,1);
			break;

		   case Ctrl_Left:	 /* go to start of prev word in line */
			if (i) {
			   if (--i > 0) {
			      while (s[i] == ' ' && --i > 0);
			      if (i)
				 while (s[i-1] != ' ' && --i > 0);
			   }
			}
			break;

		   case Ctrl_Right:	 /* go to start of next word in line */
			for (j = width; s[j-1] == ' ' && --j > 0; );
			if (j == width)
			   j--;

			if (i >= j)		      /* limit i to end of s */
			   i = j;
			else {
			   while (s[i] != ' ' && ++i < j);
			   if (i < j)
			      while (s[i] == ' ' && ++i < j);
			}
			break;

		   case Ctrl_End:		    /* delete to end of line */
			memset(&s[i],' ',width - i);
			win_setbuf(win_ptr + i,attrib | (byte) ' ',width - i);
			scr_update(wp,wp->cur_left + horpos + i,
				   wp->cur_top + verpos,width - i);
			break;

		   case Ctrl_Y: 		       /* delete entire line */
			memset(s,' ',width);
			win_setbuf(win_ptr,attrib | (byte) ' ',width);
			scr_update(wp,wp->cur_left + horpos,
				   wp->cur_top + verpos,width);
			i = 0;
			break;

		   default:	  /* all other (hopefully normal) characters */
			if (c < ' ' || c >= 0x100)
			   break;

			if (firstkey) { 	  /* clear entire line first */
			   memset(s,' ',width);
			   win_setbuf(win_ptr,attrib | (byte) ' ',width);
			   scr_update(wp,wp->cur_left + horpos,
				      wp->cur_top + verpos,width);
			   i = 0;
			}

			if (insert && i < (width - 1)) {
			   memmove(&s[i+1],&s[i],width - (i + 1));
			   win_movebuf(&win_ptr[i+1],&win_ptr[i],
				       width - (i + 1));
			   scr_update(wp,wp->cur_left + horpos + i + 1,
				      wp->cur_top + verpos,width - (i + 1));
			}

			s[i] = c;
			win_ptr[i] = attrib | c;
			scr_update(wp,wp->cur_left + horpos + i,
				   wp->cur_top + verpos,1);

			if (i < (width - 1))
			   i++;
			else {
			   if (endaction & END_BELL)
			      scr_bell();
			   if (endaction & END_EXIT) {
end:			      for (j = width; j > i && s[--j] == ' '; );
			      i =  j + (s[j] != ' ');
			      s[i] = '\0';
			      if (checkfunc != NULL) {	   /* checkroutine ? */
				 if ((*checkfunc)(s)) {      /* yes, check s */
				    strcpy(str,s);	    /* check went ok */
				    goto exit;
				 }
				 else { 	   /* not okay, resume entry */
				    if (i < width) s[i] = ' ';
				    else	   i--;
				 }
			      }
			      else {		    /* no check, just return */
				 strcpy(str,s);
				 goto exit;
			      }
			   }
			}
			break;
	    }/*switch*/
	}/*for*/

exit:	free(s);
	win_setkeyemu(win_idx,oldkeyemu);
	return (c);			     /* return last entered key code */
}/*win_input()*/


/* end of input.c */
