#include <stdio.h>
#include <string.h>

#include "umr.h"

int main(int argc, char *argv[])
{
	int i, j, c;
	char filename[128];
	const char *n, *t;
	long l;

	if (argc < 2) {
		printf("%s filename [filename ...]\n", argv[0]);
		return 1;
	}

	printf("%d file", argc - 1);
	if (argc > 2)
		printf("s\n\n");
	else	printf("\n\n");
	j = 0;

	for (c = 1; c < argc; c++) {
		if (upkg_open(argv[c]) == 0) {
			j++;
			printf("%s\n", argv[c]);

			for (i = 1; i < upkg_ocount() + 1; i++) {
				t = upkg_otype(i);
				n = upkg_oname(i);
				l = upkg_object_size(i);

				if (t != NULL && l > 0) {
					strcpy(filename, n);
					strcat(filename, ".");
					strcat(filename, t);

					n = upkg_oclassname(i);
					printf("%s\t(%s, %ld bytes)\n",
						filename, n, l);

					upkg_object_dump(filename, i);
				}
			}

			upkg_close();
			printf("\n");
		}
	}

	printf("%d files processed\n\n", j);

	return 0;
}
