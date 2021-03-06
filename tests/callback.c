#include <stdio.h>
#include <stdlib.h>
#include "callback.h"

struct callback_data_s cd_global;

void
callback(tmt_msg_t m, TMT *vt, const void *a, void *p)
{
    /* grab a pointer to the virtual screen */
    const TMTSCREEN *s = tmt_screen(vt);
    (void)s;
    const TMTPOINT *c = tmt_cursor(vt);
    (void)c;
    struct callback_data_s *cd = p;
    if ( cd == NULL ) {
    	cd = &cd_global;
    }
    cd->lastm = m;

    switch (m){
        case TMT_MSG_BELL:
            /* the terminal is requesting that we ring the bell/flash the
             * screen/do whatever ^G is supposed to do; a is NULL
             */
#if 0
            printf("BELL\n");
#endif
            break;

        case TMT_MSG_UPDATE:
            /* the screen image changed; a is a pointer to the TMTSCREEN */
            for (size_t r = 0; r < s->nline; r++){
                cd->dirty[r] = s->lineattrs[r].dirty;
                for (size_t c = 0; c < s->ncol; c++){
                    cd->screen[r][c] = tmt_char(vt,r,c)->c;
                }
            }
#if 0
        	printf("TMT_MSG_UPDATE\n");
            for (size_t r = 0; r < s->nline; r++){
                if (s->lineattrs[r].dirty) {
                	printf("line: %lu\n\"", r);
                	const TMTCHAR *line = tmt_line(vt, r);
                    for (size_t c = 0; c < s->ncol; c++){
                    	putchar(line[c].c);
                    }
                    printf("\"\n");
                }
            }
#endif
            /* let tmt know we've redrawn the screen */
            tmt_updated(vt);
            break;

        case TMT_MSG_ANSWER:
            /* the terminal has a response to give to the program; a is a
             * pointer to a string */
#if 0
            printf("TMT_MSG_ANSWER %s\n", (const char *)a);
#endif
            break;

        case TMT_MSG_MOVED:
            /* the cursor moved; a is a pointer to the cursor's TMTPOINT */
#if 0
            printf("TMT_MSG_MOVED %zu,%zu\n", c->r, c->c);
#endif
            break;
        default:
        	break;
    }
}
