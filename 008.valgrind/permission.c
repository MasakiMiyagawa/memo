#include <stdio.h>

int main(int argc, char *argv)
{
	FILE *fp = fopen("/root/test", "w");
	if (!fp)
		perror("fopen");

	return 0;
}
