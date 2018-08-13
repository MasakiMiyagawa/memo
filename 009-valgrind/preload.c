#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int rand(void)
{
	malloc(0x100);
	return 9999;
}
