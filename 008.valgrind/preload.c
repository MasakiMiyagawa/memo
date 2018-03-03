#include <stdio.h>
#include <unistd.h>

int rand(void)
{
	printf("Busy loop is started.\n");
	while (1) {
		usleep(10);
	}
	return 1;
}
