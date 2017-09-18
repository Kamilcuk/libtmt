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
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tmt.h"

#ifndef __GNUC__
# define MAX(x, y) (((size_t)(x) > (size_t)(y)) ? (size_t)(x) : (size_t)(y))
# define MIN(x, y) (((size_t)(x) < (size_t)(y)) ? (size_t)(x) : (size_t)(y))
#else
# define MAX(x, y) \
    ({ __typeof__ (x) _x = (x); \
        __typeof__ (y) _y = (y); \
        _x > _y ? _x : _y; })
# define MIN(x, y) \
    ({ __typeof__ (x) _x = (x); \
        __typeof__ (y) _y = (y); \
        _x < _y ? _x : _y; })
#endif

#define CLINE_IDX(vt)          TMT_LINE_IDX((vt),(vt)->curs.r)
#define CLINE(vt)              (&(vt)->screen.chars[CLINE_IDX(vt)])

#define P0(x) (vt->pars[x])
#define P1(x) (vt->pars[x]? vt->pars[x] : 1)
#define CB(vt, m, a) ((vt)->cb? (vt)->cb(m, vt, a, (vt)->p) : (void)0)
#define INESC ((vt)->state)

#define UNUSED(x) ((void)x) //! remove unused warning from a variable

#define COMMON_VARS             \
    TMTSCREEN *s = &vt->screen; UNUSED(s); \
    TMTPOINT *c = &vt->curs;    UNUSED(c); \
    TMTCHAR *l = CLINE(vt);     UNUSED(l); \
    char *t = vt->tabs;         UNUSED(t)

#define HANDLER(name) static void name (TMT *vt) { COMMON_VARS; 

#define DEFATTRS (TMTATTRS){ .fg = TMT_COLOR_DEFAULT, .bg = TMT_COLOR_DEFAULT }
#define DEFCHAR  (TMTCHAR){ .a = DEFATTRS, .c = L' ', }

static void writecharatcurs(TMT *vt, wchar_t w);

static wchar_t
tacs(const TMT *vt, unsigned char c)
{
    /* The terminfo alternate character set for ANSI. */
    static unsigned char map[] = {0020U, 0021U, 0030U, 0031U, 0333U, 0004U,
                                  0261U, 0370U, 0361U, 0260U, 0331U, 0277U,
                                  0332U, 0300U, 0305U, 0176U, 0304U, 0304U,
                                  0304U, 0137U, 0303U, 0264U, 0301U, 0302U,
                                  0263U, 0363U, 0362U, 0343U, 0330U, 0234U,
                                  0376U};
    for (size_t i = 0; i < sizeof(map); i++) if (map[i] == c)
        return vt->acschars[i];
    return (wchar_t)c;
}

static void
dirtylines(TMT *vt, size_t s, size_t e)
{
	assert(s < e && e < vt->screen.ncol);
    vt->dirty = true;
    for (size_t i = s; i < e; i++)
       vt->screen.lineattrs[i].dirty = true;
}

static void
clearchars(TMTCHAR *base, size_t n)
{
	assert(n);
    while(n--){
    	*base++ = DEFCHAR;
    }
}

static void
clearline(TMT *vt, size_t l, size_t s, size_t e)
{
	assert(s < e && e < vt->screen.ncol);
	assert(l < vt->screen.nline);
	vt->dirty = vt->screen.lineattrs[l].dirty = true;
	clearchars(&TMT_CHAR(vt,l,s), e - s);
}

static void
clearlines(TMT *vt, size_t r, size_t n)
{
    for (size_t i = r; i < n; i++)
        clearline(vt, i, 0, vt->screen.ncol);
}

/**
 * screen up
 */
static void
scrup(TMT *vt, size_t r, size_t n)
{
	assert(n != 0 && r+n < vt->screen.nline);
    memmove(TMT_LINE(vt,r), TMT_LINE(vt,r+n), (vt->screen.nline - n) * vt->screen.ncol * sizeof(TMTCHAR));
    clearlines(vt, vt->screen.nline - n, vt->screen.nline);
    dirtylines(vt, r, vt->screen.nline);
}

/**
 * screen down
 */
static void
scrdn(TMT *vt, size_t r, size_t n)
{
	assert(n != 0 && r+n < vt->screen.nline);
    memmove(TMT_LINE(vt,r+n), TMT_LINE(vt,r), (vt->screen.nline - n) * vt->screen.ncol * sizeof(TMTCHAR));
    clearlines(vt, r, n);
    dirtylines(vt, r, vt->screen.nline);
}

HANDLER(ed)
    size_t b = 0;
    size_t e = s->nline;

    switch (P0(0)){
        case 0: b = c->r + 1; clearline(vt, c->r, c->c, vt->screen.ncol); break;
        case 1: e = c->r - 1; clearline(vt, c->r, 0, c->c);               break;
        case 2:  /* use defaults */                                       break;
        default: /* do nothing   */                                       return;
    }

    clearlines(vt, b, e - b);
}

HANDLER(ich)
    size_t n = P1(0); /* XXX use MAX */
    if (n > s->ncol - c->c - 1) n = s->ncol - c->c - 1;

    memmove(l + c->c + n, l + c->c,
            MIN(s->ncol - 1 - c->c,
            (s->ncol - c->c - n - 1)) * sizeof(TMTCHAR));
    clearline(vt, c->r, c->c, n);
}

HANDLER(dch)
    size_t n = P1(0); /* XXX use MAX */
    if (n > s->ncol - c->c) n = s->ncol - c->c;

    memmove(l + c->c, l + c->c + n,
            (s->ncol - c->c - n) * sizeof(TMTCHAR));

    clearline(vt, c->r, s->ncol - c->c - n, s->ncol);
}

HANDLER(el)
    switch (P0(0)){
        case 0: clearline(vt, c->r, c->c, vt->screen.ncol);         break;
        case 1: clearline(vt, c->r, 0, MIN(c->c + 1, s->ncol - 1)); break;
        case 2: clearline(vt, c->r, 0, vt->screen.ncol);            break;
    }
}

HANDLER(sgr)
    #define FGBG(c) *(P0(i) < 40? &vt->attrs.fg : &vt->attrs.bg) = c
    for (size_t i = 0; i < vt->npar; i++) switch (P0(i)){
        case  0: vt->attrs                    = DEFATTRS;   break;
        case  1: case 22: vt->attrs.bold      = P0(0) < 20; break;
        case  2: case 23: vt->attrs.dim       = P0(0) < 20; break;
        case  4: case 24: vt->attrs.underline = P0(0) < 20; break;
        case  5: case 25: vt->attrs.blink     = P0(0) < 20; break;
        case  7: case 27: vt->attrs.reverse   = P0(0) < 20; break;
        case  8: case 28: vt->attrs.invisible = P0(0) < 20; break;
        case 10: case 11: vt->acs             = P0(0) > 10; break;
        case 30: case 40: FGBG(TMT_COLOR_BLACK);            break;
        case 31: case 41: FGBG(TMT_COLOR_RED);              break;
        case 32: case 42: FGBG(TMT_COLOR_GREEN);            break;
        case 33: case 43: FGBG(TMT_COLOR_YELLOW);           break;
        case 34: case 44: FGBG(TMT_COLOR_BLUE);             break;
        case 35: case 45: FGBG(TMT_COLOR_MAGENTA);          break;
        case 36: case 46: FGBG(TMT_COLOR_CYAN);             break;
        case 37: case 47: FGBG(TMT_COLOR_WHITE);            break;
        case 39: case 49: FGBG(TMT_COLOR_DEFAULT);          break;
    }
}

HANDLER(rep)
    if (!c->c) return;
    wchar_t r = l[c->c - 1].c;
    for (size_t i = 0; i < P1(0); i++)
        writecharatcurs(vt, r);
}

HANDLER(dsr)
    char r[TMT_BUF_MAX + 1] = {0};
    snprintf(r, TMT_BUF_MAX, "\033[%zd;%zdR", c->r, c->c);
    CB(vt, TMT_MSG_ANSWER, (const char *)r);
}

HANDLER(resetparser)
    memset(vt->pars, 0, sizeof(vt->pars));
    vt->state = vt->npar = vt->arg = vt->ignored = (bool)0;
}

HANDLER(consumearg)
    if (vt->npar < TMT_PAR_MAX)
        vt->pars[vt->npar++] = vt->arg;
    vt->arg = 0;
}

HANDLER(fixcursor)
    c->r = MIN(c->r, s->nline - 1);
    c->c = MIN(c->c, s->ncol - 1);
}

static bool
handlechar(TMT *vt, char i)
{
    COMMON_VARS;

    char cs[] = {i, 0};
    #define ON(S, C, A) if (vt->state == (S) && strchr(C, i)){ A; return true;}
    #define DO(S, C, A) ON(S, C, consumearg(vt); if (!vt->ignored) {A;} \
                                 fixcursor(vt); resetparser(vt););

    DO(S_NUL, "\x07",       CB(vt, TMT_MSG_BELL, NULL))
    DO(S_NUL, "\x08",       if (c->c) c->c--)
    DO(S_NUL, "\x09",       while (++c->c < s->ncol - 1 && t[c->c] != '*'))
    DO(S_NUL, "\x0a",       c->r < s->nline - 1? (void)c->r++ : scrup(vt, 0, 1))
    DO(S_NUL, "\x0d",       c->c = 0)
    ON(S_NUL, "\x1b",       vt->state = S_ESC)
    ON(S_ESC, "\x1b",       vt->state = S_ESC)
    DO(S_ESC, "H",          t[c->c] = '*')
    DO(S_ESC, "7",          vt->oldcurs = vt->curs; vt->oldattrs = vt->attrs)
    DO(S_ESC, "8",          vt->curs = vt->oldcurs; vt->attrs = vt->oldattrs)
    ON(S_ESC, "+*()",       vt->ignored = true; vt->state = S_ARG)
    DO(S_ESC, "c",          tmt_reset(vt))
    ON(S_ESC, "[",          vt->state = S_ARG)
    ON(S_ARG, "\x1b",       vt->state = S_ESC)
    ON(S_ARG, ";",          consumearg(vt))
    ON(S_ARG, "?",          (void)0)
    ON(S_ARG, "0123456789", vt->arg = vt->arg * 10 + atoi(cs))
    DO(S_ARG, "A",          c->r = MAX(c->r - P1(0), 0))
    DO(S_ARG, "B",          c->r = MIN(c->r + P1(0), s->nline - 1))
    DO(S_ARG, "C",          c->c = MIN(c->c + P1(0), s->ncol - 1))
    DO(S_ARG, "D",          c->c = MIN(c->c - P1(0), c->c))
    DO(S_ARG, "E",          c->c = 0; c->r = MIN(c->r + P1(0), s->nline - 1))
    DO(S_ARG, "F",          c->c = 0; c->r = MAX(c->r - P1(0), 0))
    DO(S_ARG, "G",          c->c = MIN(P1(0) - 1, s->ncol - 1))
    DO(S_ARG, "d",          c->r = MIN(P1(0) - 1, s->nline - 1))
    DO(S_ARG, "Hf",         c->r = P1(0) - 1; c->c = P1(1) - 1)
    DO(S_ARG, "I",          while (++c->c < s->ncol - 1 && t[c->c] != '*'))
    DO(S_ARG, "J",          ed(vt))
    DO(S_ARG, "K",          el(vt))
    DO(S_ARG, "L",          scrdn(vt, c->r, P1(0)))
    DO(S_ARG, "M",          scrup(vt, c->r, P1(0)))
    DO(S_ARG, "P",          dch(vt))
    DO(S_ARG, "S",          scrup(vt, 0, P1(0)))
    DO(S_ARG, "T",          scrdn(vt, 0, P1(0)))
    DO(S_ARG, "X",          clearline(vt, c->r, c->c, P1(0)))
    DO(S_ARG, "Z",          while (c->c && t[--c->c] != '*'))
    DO(S_ARG, "b",          rep(vt));
    DO(S_ARG, "c",          CB(vt, TMT_MSG_ANSWER, "\033[?6c"))
    DO(S_ARG, "g",          if (P0(0) == 3) memset(vt->tabs, ' ', s->ncol))
    DO(S_ARG, "m",          sgr(vt))
    DO(S_ARG, "n",          if (P0(0) == 6) dsr(vt))
    DO(S_ARG, "h",          if (P0(0) == 25) CB(vt, TMT_MSG_CURSOR, "t"))
    DO(S_ARG, "i",          (void)0)
    DO(S_ARG, "l",          if (P0(0) == 25) CB(vt, TMT_MSG_CURSOR, "f"))
    DO(S_ARG, "s",          vt->oldcurs = vt->curs; vt->oldattrs = vt->attrs)
    DO(S_ARG, "u",          vt->curs = vt->oldcurs; vt->attrs = vt->oldattrs)
    DO(S_ARG, "@",          ich(vt))

    return resetparser(vt), false;
}

static void
notify(TMT *vt, bool update, bool moved)
{
    if (update) CB(vt, TMT_MSG_UPDATE, &vt->screen);
    if (moved) CB(vt, TMT_MSG_MOVED, &vt->curs);
}

static bool
tmt_resize_realloc(TMT *vt, size_t nline, size_t ncol)
{
    if (nline < 2 || ncol < 2) return false;

    TMTCHAR *l = realloc(vt->screen.chars, nline * ncol * sizeof(TMTCHAR));
    if (!l) return false;
    vt->screen.chars = l;

    TMTLINEATTRS *d = realloc(vt->screen.lineattrs, nline * sizeof(TMTLINEATTRS));
    if (!d) return false;
    vt->screen.lineattrs = d;

    char *t = realloc(vt->tabs, ncol * sizeof(char));
    if (!t) return false;
    vt->tabs = t;

    return true;
}

static bool
_tmt_init(TMT *vt, size_t nline, size_t ncol, TMTCALLBACK cb, void *p,
        const wchar_t *acs)
{
    /* ASCII-safe defaults for box-drawing characters. */
    vt->acschars = acs? acs : L"><^v#+:o##+++++~---_++++|<>*!fo";
    vt->cb = cb;
    vt->p = p;

    tmt_resize_static(vt, nline, ncol);

    memset(vt->tabs, ' ', ncol * sizeof(char));
    for(size_t i = 0; i < vt->screen.nline; i+=TMT_TAB) {
    	vt->tabs[i] = '*';
    }

    return true;
}

TMT *
tmt_open(size_t nline, size_t ncol, TMTCALLBACK cb, void *p,
         const wchar_t *acs)
{
    TMT *vt = calloc(1, sizeof(TMT));
    if (!vt) return NULL;

    if (!tmt_resize_realloc(vt, nline, ncol)) return tmt_close(vt), NULL;

    if (!_tmt_init(vt, nline, ncol, cb, p, acs)) return tmt_close(vt), NULL;

    return vt;
}

void
tmt_close(TMT *vt)
{
    free(vt->screen.chars);
    free(vt->screen.lineattrs);
	free(vt->tabs);
	free(vt);
}

bool
tmt_resize(TMT *vt, size_t nline, size_t ncol)
{
	if (!tmt_resize_realloc(vt, nline, ncol)) return false;
	tmt_resize_static(vt, nline, ncol);
	return true;
}

bool
tmt_init(TMT *vt, TMTLINEATTRS *lineattrs, TMTCHAR *chars, char *tabs,
		size_t nline, size_t ncol, TMTCALLBACK cb, void *p,
        const wchar_t *acs)
{
    if (!vt || !lineattrs || !chars || !tabs) return false;

    memset(vt, 0, sizeof(TMT));

	vt->screen.chars = chars;
	vt->screen.lineattrs = lineattrs;

	return _tmt_init(vt, nline, ncol, cb, p, acs);
}

void
tmt_deinit(TMT *vt)
{
    UNUSED(vt);
}

static void
tmt_resize_screen(TMTCHAR *chars,
		size_t new_nline, size_t new_ncol,
		size_t old_nline, size_t old_ncol)
{
	if ( new_ncol < old_ncol ) {
		for(size_t l = 1, lmax = MIN(new_nline,old_nline); l < lmax; l++) {
			memcpy(&chars[new_ncol*l], &chars[old_ncol*l], new_ncol);
		}
	} else if ( new_ncol > old_ncol ) {
		for(size_t l = MIN(new_nline,old_nline); l; --l) {
			memcpy(&chars[new_ncol*(l-1)], &chars[old_ncol*(l-1)], old_ncol);
			clearchars(&chars[new_ncol * (l-1) + old_ncol], new_ncol - old_ncol);
		}
	}

	// initialize new lines
	for(size_t l = old_nline; l < new_nline; ++l) {
		clearchars(&chars[new_ncol * l], new_ncol);
	}
}


void
tmt_resize_static(TMT *vt, size_t nline, size_t ncol)
{
    if (nline < 2 || ncol < 2) return;

    // let user know, he needs to update the whole display
    dirtylines(vt, 0, nline);

    tmt_resize_screen(vt->screen.chars, nline, ncol, vt->screen.nline, vt->screen.ncol);
    vt->screen.ncol = ncol;
    vt->screen.nline = nline;

    fixcursor(vt);
    dirtylines(vt, 0, nline);
    notify(vt, true, true);
}

static void
writecharatcurs(TMT *vt, wchar_t w)
{
    COMMON_VARS;

    #ifdef TMT_HAS_WCWIDTH
    extern int wcwidth(wchar_t c);
    if (wcwidth(w) > 1)  w = TMT_INVALID_CHAR;
    if (wcwidth(w) < 0) return;
    #endif

    CLINE(vt)->c = w;
    CLINE(vt)->a = vt->attrs;
    vt->screen.lineattrs[vt->curs.r].dirty = true;

    if (c->c < s->ncol - 1)
        c->c++;
    else{
        c->c = 0;
        c->r++;
    }

    if (c->r >= s->nline){
        c->r = s->nline - 1;
        scrup(vt, 0, 1);
    }
}

static inline size_t
testmbchar(TMT *vt)
{
    mbstate_t ts = vt->ms;
    return vt->nmb? mbrtowc(NULL, vt->mb, vt->nmb, &ts) : (size_t)-2;
}

static inline wchar_t
getmbchar(TMT *vt)
{
    wchar_t c = 0;
    size_t n = mbrtowc(&c, vt->mb, vt->nmb, &vt->ms);
    vt->nmb = 0;
    return (n == (size_t)-1 || n == (size_t)-2) ? TMT_INVALID_CHAR : c;
}

void
tmt_write(TMT *vt, const char *s, size_t n)
{
    TMTPOINT oc = vt->curs;
    n = n? n : strlen(s);

    for (size_t p = 0; p < n; p++){
        if (handlechar(vt, s[p]))
            continue;
        else if (vt->acs)
            writecharatcurs(vt, tacs(vt, (unsigned char)s[p]));
        else if (vt->nmb >= TMT_BUF_MAX)
            writecharatcurs(vt, getmbchar(vt));
        else{
            switch (testmbchar(vt)){
                case (size_t)-1: writecharatcurs(vt, getmbchar(vt)); break;
                case (size_t)-2: vt->mb[vt->nmb++] = s[p];           break;
            }

            if (testmbchar(vt) <= MB_LEN_MAX)
                writecharatcurs(vt, getmbchar(vt));
        }
    }

    notify(vt, vt->dirty, memcmp(&oc, &vt->curs, sizeof(oc)) != 0);
}

void
tmt_updated(TMT *vt)
{
	vt->dirty = false;
    for (size_t i = 0; i < vt->screen.nline; i++)
         vt->screen.lineattrs[i].dirty = false;
}

void
tmt_reset(TMT *vt)
{
    vt->curs.r = vt->curs.c = vt->oldcurs.r = vt->oldcurs.c = vt->acs = (bool)0;
    resetparser(vt);
    vt->attrs = vt->oldattrs = DEFATTRS;
    memset(&vt->ms, 0, sizeof(vt->ms));
    clearlines(vt, 0, vt->screen.nline);
    CB(vt, TMT_MSG_CURSOR, "t");
    notify(vt, true, true);
}
