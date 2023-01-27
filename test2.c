#include <stdio.h>
#define __PROTO__
#include "window.h"


void main (void)
{
	byte hor, ver, oldhor, oldver;
	byte line = LINE_NONE;

	win_init(10,CUR_NORMAL,CON_AVATAR,CHR_NORMAL,KEY_RAW);
	win_cls(0);
        win_printf(0,"win_maxhor=%d win_maxver=%d\n",(int)win_maxhor,(int)win_maxver);
	win_printf(0,"0-4=None,SingleIBM,DoubleIBM,SingleASCII,DoubleASCII\n");

	win_getpos(0,&hor,&ver);
	oldhor = hor;
	oldver = ver;
	for (;;) {
	    win_setpos(0,hor,ver);
	    switch (win_keygetc()) {
		   case '0': line = LINE_NONE;	  break;
		   case '1': line = LINE_SINGLE;  break;
		   case '2': line = LINE_DOUBLE;  break;
		   case '3': line = LINE_ASINGLE; break;
		   case '4': line = LINE_ADOUBLE; break;

		   case Alt_X:
		   case Esc:
		   case MKEY_RIGHTBUTTON:
			goto fini;

		   case Left:
		   case MKEY_LEFTMOVE:
			if (hor > 1) {
			   hor--;
			   if (line)
			      win_line(0,oldhor,oldver,hor,ver,line);
			   oldhor = hor;
			}
			break;

		   case Up:
		   case MKEY_UPMOVE:
			if (ver > 1) {
			   ver--;
			   if (line)
			      win_line(0,oldhor,oldver,hor,ver,line);
			   oldver = ver;
			}
			break;

		   case Right:
		   case MKEY_RIGHTMOVE:
			if (hor < win_maxhor) {
			   hor++;
			   if (line)
			      win_line(0,oldhor,oldver,hor,ver,line);
			   oldhor = hor;
			}
			break;

		   case Down:
		   case MKEY_DOWNMOVE:
			if (ver < win_maxver) {
			   ver++;
			   if (line)
			      win_line(0,oldhor,oldver,hor,ver,line);
			   oldver = ver;
			}
			break;
	    }
	}

fini:	win_cls(0);
	win_deinit();
}
