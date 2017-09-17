#include <stdio.h>
#include <stdlib.h>
#include "../tmt.h"
#include "callback.h"

char longstr[] = "qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjklzxcvbnm\n";
const int longstr_len = sizeof(longstr)/sizeof(*longstr);

#test init_deinit
{
	TMT_DECLARE(vt, vt_lines, vt_buffer, vt_tabs, 5, 10);
	tmt_init(&vt, vt_lines, vt_buffer, vt_tabs, 2, 10, callback, NULL, NULL);
	tmt_write(&vt, longstr, longstr_len);
	tmt_deinit(&vt);
}

#test open_close
{
	TMT *vt = tmt_open(2, 10, callback, NULL, NULL);
	ck_assert_ptr_nonnull(vt);
	tmt_write(vt, longstr, longstr_len);
	tmt_close(vt);
}

#test more
{
	TMT *vt = tmt_open(2, 10, callback, NULL, NULL);
	ck_assert_ptr_nonnull(vt);
	tmt_write(vt, longstr, longstr_len);
	tmt_resize(vt, 5, 10);
	tmt_write(vt, longstr, longstr_len);
	tmt_close(vt);
}

#test more_static
{
	TMT_DECLARE(vt, vt_lines, vt_buffer, vt_tabs, 5, 10);
	tmt_init(&vt, vt_lines, vt_buffer, vt_tabs, 2, 10, callback, NULL, NULL);
	tmt_write(&vt, longstr, longstr_len);
	tmt_resize_static(&vt, 5, 10);
	tmt_write(&vt, longstr, longstr_len);
	tmt_deinit(&vt);
}

