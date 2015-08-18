#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>

#include "input.h"

/* ------------------------------------------------------------------ */

char *EV_NAME[EV_CNT] = {
#include "EV.h"
};

static char *REL_NAME[REL_CNT] = {
#include "REL.h"
};

static char *ABS_NAME[ABS_CNT] = {
#include "ABS.h"
};

static char *MSC_NAME[MSC_CNT] = {
#include "MSC.h"
};

static char *LED_NAME[LED_CNT] = {
#include "LED.h"
};

static char *SND_NAME[SND_CNT] = {
#include "SND.h"
};

#ifndef REP_CNT
#define REP_CNT (REP_MAX + 1)
#endif

static char *REP_NAME[REP_CNT] = {
#include "REP.h"
};

static char *KEY_NAME[KEY_CNT] = {
#include "KEY.h"
#include "BTN.h"
};

static char *SW_NAME[SW_CNT] = {
#include "SW.h"
};

int EV_TYPE_MAX[EV_CNT] = {
	[EV_REL] = REL_MAX,
	[EV_ABS] = ABS_MAX,
	[EV_MSC] = MSC_MAX,
	[EV_LED] = LED_MAX,
	[EV_SND] = SND_MAX,
	[EV_REP] = REP_MAX,
	[EV_KEY] = KEY_MAX,
	[EV_SW] = SW_MAX,
};

char **EV_TYPE_NAME[EV_CNT] = {
	[EV_REL] = REL_NAME,
	[EV_ABS] = ABS_NAME,
	[EV_MSC] = MSC_NAME,
	[EV_LED] = LED_NAME,
	[EV_SND] = SND_NAME,
	[EV_REP] = REP_NAME,
	[EV_KEY] = KEY_NAME,
	[EV_SW] = SW_NAME,
};

char *BUS_NAME[] = {
#include "BUS.h"
};

/* ------------------------------------------------------------------ */

int device_open(int nr, int verbose)
{
	char filename[32];
	int fd;

	snprintf(filename,sizeof(filename),
		 "/dev/input/event%d",nr);
	fd = open(filename,O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr,"open %s: %s\n",
			filename,strerror(errno));
		return -1;
	}
	if (verbose)
		fprintf(stderr,"%s\n",filename);

	return fd;
}

void device_info(int fd)
{
	struct input_id id;
	BITFIELD bits[32];
	char buf[32];
	int rc,bit;

	rc = ioctl(fd,EVIOCGID,&id);
	if (rc >= 0)
		fprintf(stderr,
			"   bustype : %s\n"
			"   vendor  : 0x%x\n"
			"   product : 0x%x\n"
			"   version : %d\n",
			BUS_NAME[id.bustype],
			id.vendor, id.product, id.version);
	rc = ioctl(fd,EVIOCGNAME(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   name    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGPHYS(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   phys    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGUNIQ(sizeof(buf)),buf);
	if (rc >= 0)
		fprintf(stderr,"   uniq    : \"%.*s\"\n",rc,buf);
	rc = ioctl(fd,EVIOCGBIT(0,sizeof(bits)),bits);
	if (rc >= 0) {
		fprintf(stderr,"   bits ev :");
		for (bit = 0; bit < rc*8 && bit < EV_MAX; bit++) {
			if (test_bit(bit,bits))
				fprintf(stderr," %s", EV_NAME[bit]);
		}
		fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n");
}

/* ------------------------------------------------------------------ */

void print_event(struct input_event *event)
{
	char ts[32];
	time_t t;

	t = event->time.tv_sec;
	strftime(ts,sizeof(ts),"%H:%M:%S",localtime(&t));
	fprintf(stderr,"%s.%06ld: %s",ts,event->time.tv_usec,
		EV_NAME[event->type]);
	switch (event->type) {
	case EV_KEY:
		fprintf(stderr," %s %s",
			ev_type_name(EV_KEY, event->code),
			event->value ? "pressed" : "released");
		break;
	case EV_REL:
	case EV_ABS:
	case EV_MSC:
	case EV_LED:
	case EV_SND:
	case EV_REP:
	case EV_SW:
		fprintf(stderr," %s %d",
			ev_type_name(event->type, event->code), event->value);
		break;
	default:
		fprintf(stderr," code=%u value=%d",
			(unsigned int)event->code, event->value);
	}
	fprintf(stderr,"\n");
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
