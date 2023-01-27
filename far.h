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


#ifndef __FAR_DEF_
#define __FAR_DEF_

#ifndef NIL
#define NIL	0L
#endif

#ifdef USEFAR
#include <stdio.h>	/* for FILE struct */
#define FAR	far
#ifdef __cplusplus
extern "C" {
#endif
void far *   _win_fast myfaralloc   (unsigned int nbytes);
void	     _win_fast myfarfree    (void far *block);
void	     _win_fast myfarmemset  (void far *s, char ch, unsigned int n);
void	     _win_fast myfarmemmove (void far *dest, void far *src, unsigned int n);
unsigned far _win_fast myfarfread   (void far *ptr,unsigned size,unsigned n,FILE *stream);
unsigned far _win_fast myfarfwrite  (void far *ptr,unsigned size,unsigned n,FILE *stream);
#ifdef	__cplusplus
}
#endif
#else
#define FAR
#define myfaralloc   malloc
#define myfarfree    free
#define myfarmemset  memset
#define myfarmemmove memmove
#define myfarfread   fread
#define myfarfwrite  fwrite
#endif

#endif


/* end of far.h */
