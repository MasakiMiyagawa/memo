#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void UnSetLdPreload(void)
{
	unsetenv("LD_PRELOAD");
}

int32_t ExecWithLdPreload(int argc, char *argv[])
{
	char *mock_lib = NULL;
	char *ld_preload = NULL;
	int32_t err = 0;

	ld_preload = getenv("LD_PRELOAD");
	if (ld_preload) {
		printf("LD_PRELOAD is already set as %s\n", ld_preload);
		return 0;
	}

	mock_lib = getenv("MOCK_LIBRARY");
	if (!mock_lib)
		return 0;

	if (setenv("LD_PRELOAD", mock_lib, 1))
		return -1;

	execv(argv[0], argv);
	return -1;
}
