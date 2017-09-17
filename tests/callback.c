#include <stdio.h>
#include <stdlib.h>
#include "callback.h"

struct callback_data_s cd_global;

void
callback(tmt_msg_t m, TMT *vt, const void *a, void *p)
{
    /* grab a pointer to the virtual screen */
    const TMTSCREEN *s = tmt_screen(vt);
    const TMTPOINT *c = tmt_cursor(vt);
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
            printf("BELL\n");
            break;

        case TMT_MSG_UPDATE:
            /* the screen image changed; a is a pointer to the TMTSCREEN */
        	printf("TMT_MSG_UPDATE\n");

            for (size_t r = 0; r < s->nline; r++){
                if (s->lines[r]->dirty) {
                	cd->dirty[r] = true;
                }
                for (size_t c = 0; c < s->ncol; c++){
                    cd->screen[r][c] = s->lines[r]->chars[c].c;
                }
            }

            for (size_t r = 0; r < s->nline; r++){
                if (s->lines[r]->dirty) {
                	printf("line: %lu\n\"", r);
                    for (size_t c = 0; c < s->ncol; c++){
                        printf("%lc", s->lines[r]->chars[c].c);
                    }
                    printf("\"\n");
                }
            }
            /* let tmt know we've redrawn the screen */
            tmt_clean(vt);
            break;

        case TMT_MSG_ANSWER:
            /* the terminal has a response to give to the program; a is a
             * pointer to a string */
            printf("TMT_MSG_ANSWER %s\n", (const char *)a);
            break;

        case TMT_MSG_MOVED:
            /* the cursor moved; a is a pointer to the cursor's TMTPOINT */
            printf("TMT_MSG_MOVED %zu,%zu\n", c->r, c->c);
            break;
        default:
        	break;
    }
}
