#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	printf ("You opened a PROGRAM!\n");
	int i = 0;
	while (argv[i] != NULL)
	{
	printf("%s\n", argv[i]);
	i++;
	}
	return 0;
}
