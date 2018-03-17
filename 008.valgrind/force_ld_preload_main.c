#include <stdio.h>
#include <stdlib.h>

extern int32_t ExecWithLdPreload(int argc, char *argv[]);
extern void UnSetLdPreload(void);

int main(int argc, char *argv[])
{
	atexit(UnSetLdPreload);
	printf("RAND %d\n", rand());
	ExecWithLdPreload(argc, argv);
	return 0;
}
