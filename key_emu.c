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


/* key_emu.c - keyboard emulation functions */
#include <string.h>
#include "window.h"


#ifdef __PROTO__
static int near pascal key_putraw  (word keycode);
static int near pascal key_putansi (word keycode);
static int near pascal key_putvt52 (word keycode);
#endif


#define KEY_BUFLEN	128		    /* length of keyboard ringbuffer */
static word	key_buf[KEY_BUFLEN];
static byte	key_next, key_fill;


static int near pascal key_putraw(keycode)
word keycode;
{
	key_buf[((key_next + key_fill) % KEY_BUFLEN)] = keycode;
	key_fill++;

	return (1);
}/*key_putraw()*/


static int near pascal key_putansi(keycode)
word keycode;
{
	switch (keycode) {
	       case Left:  case MKEY_LEFTMOVE:	return (win_keyputs("\033[D"));
	       case Up:    case MKEY_UPMOVE:	return (win_keyputs("\033[A"));
	       case Right: case MKEY_RIGHTMOVE: return (win_keyputs("\033[C"));
	       case Down:  case MKEY_DOWNMOVE:	return (win_keyputs("\033[B"));
	       case MKEY_LEFTBUTTON:		return (key_putraw(13));
	       case MKEY_RIGHTBUTTON:		return (key_putraw(27));
	       default: 			return (key_putraw(keycode));
	}
}/*key_putansi()*/


static int near pascal key_putvt52(keycode)
word keycode;
{
	switch (keycode) {
	       case Left:  case MKEY_LEFTMOVE:	return (win_keyputs("\033D"));
	       case Up:    case MKEY_UPMOVE:	return (win_keyputs("\033A"));
	       case Right: case MKEY_RIGHTMOVE: return (win_keyputs("\033C"));
	       case Down:  case MKEY_DOWNMOVE:	return (win_keyputs("\033B"));
	       case MKEY_LEFTBUTTON:		return (key_putraw(13));
	       case MKEY_RIGHTBUTTON:		return (key_putraw(27));
	       default: 			return (key_putraw(keycode));
	}
}/*key_putvt52()*/


						/* win_keyfull NOT exported! */
int _win_fast win_keyfull(/*void*/)		/* return 1 if buffer full   */
{
	return ((key_fill >= (KEY_BUFLEN - 1)) ? 1 : 0);
}/*win_keyfull()*/


int _win_exp _win_fast win_keyputc(keycode)  /* place keycode in keyboard ring buffer */
word keycode;
{
#ifdef __PROTO__
	static int near pascal (*key_emuputc[])(word keycode) =
				      { key_putraw, key_putansi, key_putvt52 };
#else
	static int near pascal (*key_emuputc[])() =
				      { key_putraw, key_putansi, key_putvt52 };
#endif

	if (windows == NULL) return (0);
	if (!keycode) return (1);
	if (key_fill >= (KEY_BUFLEN - 1)) return (0);

	return ((*key_emuputc[(win_getwp(win_gettop())->key_emu)])(keycode));
}/*win_keyputc()*/


int _win_exp _win_fast win_keyputs(s)	      /* place string in keyboard ring buffer */
char *s;
{
	if (windows == NULL || ((key_fill + strlen(s)) > (KEY_BUFLEN - 1)))
	   return (0);

	while (*s) {
	      if (!win_keyputc(((word) (*(s++))) & 0x0ff))
		 return (0);
	}

	return (1);
}/*win_keyputs()*/


int _win_exp _win_fast win_keypush(keycode)   /* push keycode in keyboard ring buffer */
word keycode;
{
	if (windows == NULL) return (0);
	if (!keycode) return (1);
	if (key_fill >= (KEY_BUFLEN - 1)) return (0);

	if (key_next) key_next--;
	else	      key_next = KEY_BUFLEN - 1;
	key_buf[key_next] = keycode;
	key_fill++;

	return (1);
}/*win_keypush()*/


void _win_exp _win_fast win_keyreset(/*void*/)
{
	if (windows == NULL)
	   return;

	key_next = key_fill = 0;
}/*win_keyreset()*/


void _win_exp _win_fast win_keypurge(/*void*/)
{
	if (windows == NULL)
	   return;

	win_keyreset();
	key_fillbuf();
	win_keyreset();
}/*win_keypurge()*/


word _win_exp _win_fast win_keyscan(/*void*/)
{
	if (windows == NULL)
	   return (0);

	key_fillbuf();
	return (key_fill ? key_buf[key_next] : 0);
}/*win_keyscan()*/


int _win_exp _win_fast win_keycheck(word keycode)
{
	int  found = 0;
	byte cur, next, left;

	if (windows == NULL)
	   return (0);

	key_fillbuf();

	if (key_fill) {
	   cur = next = key_next;
	   for (left = key_fill; left; left--) {
	       if (key_buf[next] == keycode) {
		  found = 1;
		  key_fill--;
	       }
	       else {
		  key_buf[cur++] = key_buf[next];
		  cur %= KEY_BUFLEN;
	       }
	       next++;
	       next %= KEY_BUFLEN;
	   }
	}

	return (found);
}/*win_keycheck()*/


word _win_exp _win_fast win_keygetc(/*void*/)
{
	word keycode;

	if (windows == NULL)
	   return (0);

	while ((keycode = win_keyscan()) == 0)
	      win_idle();

	key_next++;
	key_next %= KEY_BUFLEN;
	key_fill--;

	return (keycode);
}/*win_keygetc()*/


/* end of key_emu.c */
