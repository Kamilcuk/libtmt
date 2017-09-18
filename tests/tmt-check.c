#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "../tmt.c"
#include "callback.h"

char longstr[] = "qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjkl;zxcvbnm,qwertyuiopasdfghjklzxcvbnm\n";
const int longstr_len = sizeof(longstr)/sizeof(*longstr);

#define _ck_assert_char(X, OP, Y) do { \
  intmax_t _ck_x = (X); \
  intmax_t _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '%s' failed: %s == '%c', %s == '%c'", #X" "#OP" "#Y, #X, _ck_x, #Y, _ck_y); \
} while (0)

static void tmt_printf(TMT *vt)
{
	printf("- tmt_printf -\n");
	for(uint8_t j=0;j<vt->screen.nline;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			printf("%c", TMT_CHAR(vt,j,i).c);
		}
		printf("\n");
	}
}

START_TEST(test_scrdn)
{
	TMT *vt = tmt_open(5, 2, NULL, NULL, NULL);
	for(uint8_t j=0;j<vt->screen.nline;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			TMT_CHAR(vt,j,i).c = '0' + i + j;
		}
	}

	const int move = 2;
	scrdn(vt, 0, move);

	for(uint8_t j=0;j<move;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			_ck_assert_char(TMT_CHAR(vt,j,i).c, ==, ' ');
		}
	}
	for(uint8_t j=move;j<vt->screen.nline;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			_ck_assert_char(TMT_CHAR(vt,j,i).c, ==, '0' + i + j - move);
		}
	}

	tmt_close(vt);
}
END_TEST

START_TEST(test_scrup)
{
	TMT *vt = tmt_open(5, 2, NULL, NULL, NULL);
	for(uint8_t j=0;j<vt->screen.nline;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			TMT_CHAR(vt,j,i).c = '0' + i + j;
		}
	}

	const int move = 2;
	scrup(vt, 0, move);

	for(uint8_t j=0;j<vt->screen.nline-move;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			_ck_assert_char(TMT_CHAR(vt,j,i).c, ==, '0' + i + j + move);
		}
	}
	for(uint8_t j=vt->screen.nline-move;j<vt->screen.nline;++j) {
		for(uint8_t i=0;i<vt->screen.ncol;++i) {
			_ck_assert_char(TMT_CHAR(vt,j,i).c, ==, ' ');
		}
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
