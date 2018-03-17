#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

extern int32_t ValgrindMe(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	ValgrindMe(argc, argv);
	printf("RAND %d\n", rand());
	{
		FILE *fp = fopen("/root/test", "w");
		if (!fp)
			perror("fopen");
	}
}
