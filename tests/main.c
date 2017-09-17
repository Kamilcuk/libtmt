/*
 * main.c
 *
 *  Created on: 15 wrz 2017
 *      Author: Kamil Cukrowski <kamilcukrowski __at__ gmail.com>
 *     License: jointly under MIT License and the Beerware License.
 */

#include <stdio.h>
#include <stdlib.h>
#include "../tmt.h"
#include "callback.h"

char longstr[] = "qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjklzxcvbnm\n";
const int longstr_len = sizeof(longstr)/sizeof(*longstr);


int main() {
	{
		TMT *vt = tmt_open(2, 10, callback, NULL, NULL);
		tmt_write(vt, "aaa", 3);
		tmt_write(vt, longstr, longstr_len);
		tmt_close(vt);
	}
	{
		TMT_DECLARE(vt, vt_dirty, vt_chars, vt_tabs, 5, 10);
		tmt_init(&vt, vt_dirty, vt_chars, vt_tabs, 2, 10, callback, NULL, NULL);
		tmt_write(&vt, longstr, longstr_len);
		tmt_resize_static(&vt, 5, 10);
		tmt_write(&vt, longstr, longstr_len);
		tmt_deinit(&vt);
	}
}
