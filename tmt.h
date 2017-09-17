/* Copyright (c) 2017 Rob King
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS,
 * COPYRIGHT HOLDERS, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TMT_H
#define TMT_H

#include <stdbool.h>
#include <stddef.h>
#include <wchar.h>

#include "tmt-config.h"

/**** Internal buffer size */
#ifndef TMT_BUF_MAX
#define TMT_BUF_MAX 100
#endif

/**** */
#ifndef TMT_PAR_MAX
#define TMT_PAR_MAX 8
#endif

/**** TAB SIZE */
#ifndef TMT_TAB
#define TMT_TAB 8
#endif

/**** INVALID WIDE CHARACTER */
#ifndef TMT_INVALID_CHAR
#define TMT_INVALID_CHAR ((wchar_t)0xfffd)
#endif

/**** INPUT SEQUENCES */
#define TMT_KEY_UP             "\033[A"
#define TMT_KEY_DOWN           "\033[B"
#define TMT_KEY_RIGHT          "\033[C"
#define TMT_KEY_LEFT           "\033[D"
#define TMT_KEY_HOME           "\033[H"
#define TMT_KEY_END            "\033[Y"
#define TMT_KEY_INSERT         "\033[L"
#define TMT_KEY_BACKSPACE      "\x08"
#define TMT_KEY_ESCAPE         "\x1b"
#define TMT_KEY_BACK_TAB       "\033[Z"
#define TMT_KEY_PAGE_UP        "\033[V"
#define TMT_KEY_PAGE_DOWN      "\033[U"
#define TMT_KEY_F1             "\033OP"
#define TMT_KEY_F2             "\033OQ"
#define TMT_KEY_F3             "\033OR"
#define TMT_KEY_F4             "\033OS"
#define TMT_KEY_F5             "\033OT"
#define TMT_KEY_F6             "\033OU"
#define TMT_KEY_F7             "\033OV"
#define TMT_KEY_F8             "\033OW"
#define TMT_KEY_F9             "\033OX"
#define TMT_KEY_F10            "\033OY"

/* accesing line in a screen */
#define TMT_LINE_IDX(vt,r)         ((vt)->screen.ncol*(r))
#define TMT_LINE(vt,r)             (&(vt)->screen.chars[TMT_LINE_IDX(vt,r)])

/**** BASIC DATA STRUCTURES */
typedef struct TMT TMT;

typedef enum{
    TMT_COLOR_DEFAULT = -1,
    TMT_COLOR_BLACK = 1,
    TMT_COLOR_RED,
    TMT_COLOR_GREEN,
    TMT_COLOR_YELLOW,
    TMT_COLOR_BLUE,
    TMT_COLOR_MAGENTA,
    TMT_COLOR_CYAN,
    TMT_COLOR_WHITE,
    TMT_COLOR_MAX
} tmt_color_t;

typedef struct TMTATTRS TMTATTRS;
struct TMTATTRS{
    bool bold;
    bool dim;
    bool underline;
    bool blink;
    bool reverse;
    bool invisible;
    tmt_color_t fg;
    tmt_color_t bg;
};

typedef struct TMTCHAR TMTCHAR;
struct TMTCHAR{
    wchar_t c;
    TMTATTRS a;
};

typedef struct TMTPOINT TMTPOINT;
struct TMTPOINT{
    size_t r;
    size_t c;
};

typedef struct TMTSCREEN TMTSCREEN;
struct TMTSCREEN{
    size_t nline;
    size_t ncol;
    bool *dirty; //! array of nline elements. True means that this line is "dirty" and should be redraw.
    TMTCHAR *chars; //! array of nline*ncol elemnents. Represents all currently written on the screen.
};

/**** CALLBACK SUPPORT */
typedef enum{
    TMT_MSG_MOVED,
    TMT_MSG_UPDATE,
    TMT_MSG_ANSWER,
    TMT_MSG_BELL,
    TMT_MSG_CURSOR
} tmt_msg_t;

typedef void (*TMTCALLBACK)(tmt_msg_t m, struct TMT *v, const void *r, void *p);

struct TMT{
    TMTPOINT curs, oldcurs;
    TMTATTRS attrs, oldattrs;

    bool dirty, acs, ignored;
    TMTSCREEN screen;
    TMTCHAR *tabs; //! ncol elements

    TMTCALLBACK cb;
    void *p;
    const wchar_t *acschars;

    mbstate_t ms;
    size_t nmb;
    char mb[TMT_BUF_MAX + 1];

    size_t pars[TMT_PAR_MAX];
    size_t npar;
    size_t arg;
    enum {S_NUL, S_ESC, S_ARG} state;
};

/**** PUBLIC FUNCTIONS */
TMT *tmt_open(size_t nline, size_t ncol, TMTCALLBACK cb, void *p,
              const wchar_t *acs);
void tmt_close(TMT *vt);
bool tmt_resize(TMT *vt, size_t nline, size_t ncol);

#define TMT_DECLARE(var_vt, var_dirty, var_chars, var_tabs, nline, ncol) \
		TMTCHAR (var_chars) [(nline)*(ncol)]; \
		bool (var_dirty) [(nline)]; \
		TMTCHAR (var_tabs) [(ncol)]; \
		TMT (var_vt)

bool tmt_init(TMT *vt, bool *dirty, TMTCHAR *chars, TMTCHAR *tabs,
		size_t nline, size_t ncol, TMTCALLBACK cb, void *p,
         const wchar_t *acs);
void tmt_deinit(TMT *vt);
bool tmt_resize_static(TMT *vt, size_t nline, size_t ncol);

void tmt_write(TMT *vt, const char *s, size_t n);
void tmt_reset(TMT *vt);

void tmt_dirty_clean(TMT *vt);

const TMTSCREEN *tmt_screen(const TMT *vt);
const TMTPOINT *tmt_cursor(const TMT *vt);
static inline TMTCHAR *
tmt_line(TMT *vt, size_t line)
{
	return TMT_LINE(vt, line);
}

#endif
