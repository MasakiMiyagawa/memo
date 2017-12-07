#include <stdio.h>
#include <unistd.h>
#define TESTFILE "TESTFILE"
int main(int argc, char **argv)
{
	FILE *fp;
	char filepath[256] = {0};
	int err = 0;
	char buf[256] = {0};
	
	if (argc < 2) {
		printf("You need to specify test dir path.");
		return -1;
	}

	sprintf(filepath, "%s/%s", argv[1], TESTFILE);

	printf("Test 1 : Open test file write mode\n");
	fp = fopen((const char *)filepath, "w");
	if (!fp) {
		perror("fopen(w):");
		return -1;
	}
	
	printf("Test 2 : write test\n");
	err = fprintf(fp, "Hello! this statement is written by %s", __FILE__);
	if (err < 0) {
		perror("fprintf(w):");
		return -1;
	}
	
	fclose(fp);
	fp = NULL;
	
	printf("Test 3 : Open test file read mode\n");
	fp = fopen((const char *)filepath, "r");
	if (!fp) {
		perror("fopen(r):");
		return -1;
	}

	printf("Test 4 : read test\n");
	err = fread(buf, 1, sizeof(buf), fp);
	if (err < 0) {
		perror("fread:");
		return -1;
	}
	printf("Test 5 : checking file \"%s\"\n", buf);
	
	fclose(fp);
	fp = NULL;
	
	printf("Test 6 : unlink test\n");
	err = unlink((const char *)filepath);
	if (err < 0) {
		perror("unlink:");
		return -1;
	}

	return 0;
}
