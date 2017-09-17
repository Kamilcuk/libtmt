#ifndef CALLBACK_H_
#define CALLBACK_H_

#include "../tmt.h"

struct callback_data_s {
	tmt_msg_t lastm;
	bool dirty[20];
	uint8_t screen[100][100];
};

void callback(tmt_msg_t m, TMT *vt, const void *a, void *p);

#endif // CALLBACK_H_
