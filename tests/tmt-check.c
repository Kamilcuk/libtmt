#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "../tmt.c"
#include "callback.h"

char longstr[] = "qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjklzxcvbnm\n";
const int longstr_len = sizeof(longstr)/sizeof(*longstr);

START_TEST(test_scrdn)
{
	TMT *vt = tmt_open(2, 10, NULL, NULL, NULL);
	for(uint8_t i=0;i<10;++i) {
		vt->screen.chars[i].c = '1';
		vt->screen.chars[10+i].c = '2';
	}

	scrdn(vt, 0, 1);

	for(uint8_t i=0;i<10;++i) {
		ck_assert_int_eq(vt->screen.chars[i].c, ' ');
		ck_assert_int_eq(vt->screen.chars[10+i].c, '1');
	}
	tmt_close(vt);
}
END_TEST

START_TEST(test_scrup)
{
	TMT *vt = tmt_open(2, 10, NULL, NULL, NULL);
	for(uint8_t i=0;i<10;++i) {
		vt->screen.chars[i].c = '1';
		vt->screen.chars[10+i].c = '2';
	}

	scrup(vt, 0, 1);

	for(uint8_t i=0;i<10;++i) {
		ck_assert_int_eq(vt->screen.chars[i].c, '2');
		ck_assert_int_eq(vt->screen.chars[10+i].c, ' ');
	}
	tmt_close(vt);
}
END_TEST

START_TEST(init_deinit)
{
	TMT_DECLARE(vt, vt_dirty, vt_chars, vt_tabs, 5, 10);
	tmt_init(&vt, vt_dirty, vt_chars, vt_tabs, 2, 10, callback, NULL, NULL);
	tmt_deinit(&vt);
}
END_TEST

START_TEST(open_close)
{
	TMT *vt = tmt_open(2, 10, callback, NULL, NULL);
	ck_assert_ptr_nonnull(vt);
	tmt_close(vt);
}
END_TEST

START_TEST(more)
{
	TMT *vt = tmt_open(2, 10, callback, NULL, NULL);
	ck_assert_ptr_nonnull(vt);
	tmt_write(vt, longstr, longstr_len);
	tmt_resize(vt, 5, 10);
	tmt_write(vt, longstr, longstr_len);
	tmt_close(vt);
}
END_TEST

START_TEST(more_static)
{
	TMT_DECLARE(vt, vt_dirty, vt_chars, vt_tabs, 5, 10);
	tmt_init(&vt, vt_dirty, vt_chars, vt_tabs, 2, 10, callback, NULL, NULL);
	tmt_write(&vt, longstr, longstr_len);
	tmt_resize_static(&vt, 5, 10);
	tmt_write(&vt, longstr, longstr_len);
	tmt_deinit(&vt);
}
END_TEST

int main(void)
{
    Suite *s1 = suite_create("Core");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_test(tc1_1, test_scrdn);
    tcase_add_test(tc1_1, test_scrup);
    tcase_add_test(tc1_1, init_deinit);
    tcase_add_test(tc1_1, open_close);
    tcase_add_test(tc1_1, more);
    tcase_add_test(tc1_1, more_static);

    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? 0 : 1;
}
