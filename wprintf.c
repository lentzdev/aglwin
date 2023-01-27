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


#include <stdio.h>
#include <stdarg.h>
#include "window.h"


int _win_exp cdecl win_printf(WIN_IDX win, char *fmt, ...)   /* printf formatted str to win */
{
	char buf[256];
	va_list arg_ptr;

	win_setwp(win);
	if (wp->win_status == WIN_FREE) 		     /* win not open */
	   return (0);

	va_start(arg_ptr,fmt);
	vsprintf(buf,fmt,arg_ptr);
	va_end(arg_ptr);

	con_puts(buf);

	return (1);
}/*win_printf()*/


int _win_exp cdecl win_xyprintf(WIN_IDX win, byte horpos, byte verpos, char *fmt, ...)
{	   /* printf formatted string to window at specified cursor position */
	char buf[256];
	va_list arg_ptr;

	if (!win_setpos(win,horpos,verpos))
	   return (0);

	va_start(arg_ptr,fmt);
	vsprintf(buf,fmt,arg_ptr);
	va_end(arg_ptr);

	con_puts(buf);

	return (1);
}/*win_printf()*/


/* end of wprintf.c */
