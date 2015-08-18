#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "input.h"

/* ------------------------------------------------------------------ */

static void list_devices(void)
{
	int i,fd;

	for (i = 0; i < 32; i++) {
		/* try to open */
		fd = device_open(i,1);
		if (-1 == fd)
			return;
		device_info(fd);
		close(fd);
	}
	return;
}

int main(int argc, char *argv[])
{
	list_devices();
	exit(0);
}

/* ---------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
