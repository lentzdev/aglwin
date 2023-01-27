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
#include <dos.h>
#include <alloc.h>
#include <mem.h>
#include "window.h"


void far * _win_fast myfaralloc(unsigned int nbytes)
{
	return (farmalloc((unsigned long) nbytes));
}


void _win_fast myfarfree(void far *block)
{
	farfree(block);
}


void _win_fast myfarmemset(register void far *s,register char ch,register unsigned n)
{
	while (n--)
	      *((char far *)s+n)=ch;
}


void _win_fast myfarmemmove (void far *dest, void far *src, unsigned int n)
{
	movedata(FP_SEG(src),FP_OFF(src),FP_SEG(dest),FP_OFF(dest),n);
}


unsigned far _win_fast myfarfread(void far *ptr,unsigned size,unsigned n,FILE *stream)
{
	union REGS regs;
	struct SREGS sregs;

	regs.h.ah = 0x3f;	/* read from file with handle */
	regs.x.bx = fileno(stream);
	regs.x.cx = n * size;
	regs.x.dx = FP_OFF(ptr);
	sregs.ds  = FP_SEG(ptr);
	intdosx(&regs, &regs, &sregs);
	if (regs.x.cflag) {
	   _doserrno = regs.x.ax;
	   return (0);
	}
	return (regs.x.ax / size);
}


unsigned far _win_fast myfarfwrite(void far *ptr,unsigned size,unsigned n,FILE *stream)
{
	union REGS regs;
	struct SREGS sregs;

	regs.h.ah = 0x40;	/* write to file with handle */
	regs.x.bx = fileno(stream);
	regs.x.cx = n * size;
	regs.x.dx = FP_OFF(ptr);
	sregs.ds  = FP_SEG(ptr);
	intdosx(&regs, &regs, &sregs);
	if (regs.x.cflag) {
	   _doserrno = regs.x.ax;
	   return (0);
	}
	return (regs.x.ax / size);
}


/* end of far.c */
