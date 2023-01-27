#include <stdio.h>
#define __PROTO__
#include "window.h"


void main (void)
{
	word c;

	win_init(10,CUR_NORMAL,CON_AVATAR | CON_SCROLL,CHR_NORMAL,KEY_RAW);

	while ((c = win_keygetc()) != Esc)
	      win_printf(0,"%d\n",(int) c);

	win_deinit();
}


