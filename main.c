#include <stdio.h>
#include <string.h>

#include "umr.h"

int main(int argc, char *argv[])
{
	int i, j, c;
	char filename[128];
	const char *n, *t;
	struct upkg *pkg;
	long l;

	if (argc < 2) {
		printf("%s filename [filename ...]\n", argv[0]);
		return 1;
	}

	printf("%d file", argc - 1);
	if (argc > 2)
		printf("s\n\n");
	else	printf("\n\n");

	for (j = 0, c = 1; c < argc; c++) {
		pkg = upkg_open(argv[c]);
		if (pkg != NULL) {
			j++;
			printf("%s\n", argv[c]);

			for (i = 1; i < upkg_ocount(pkg) + 1; i++) {
				t = upkg_otype(pkg, i);
				n = upkg_oname(pkg, i);
				l = upkg_object_size(pkg, i);

				if (t != NULL && l > 0) {
					strcpy(filename, n);
					strcat(filename, ".");
					strcat(filename, t);

					n = upkg_oclassname(pkg, i);
					printf("%s\t(%s, %ld bytes)\n",
						filename, n, l);

					upkg_object_dump(pkg, filename, i);
				}
			}

			upkg_close(pkg);
			printf("\n");
		}
	}

	printf("%d files processed\n\n", j);

	return 0;
}
