#include <stdio.h>
#include <string.h>

#include "umr.h"

int main(int argc, char *argv[])
{
	int i, j, c;
	char filename[128];
	const char *n, *t;
	struct upkg *pkg;
	int32_t ofs0, ofs1;
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
		printf("%s:\n", argv[c]);
		pkg = upkg_open(argv[c]);
		if (pkg != NULL) {
			j++;

			for (i = 1; i < upkg_ocount(pkg) + 1; i++) {
				t = upkg_otype(pkg, i);
				n = upkg_oname(pkg, i);
				l = upkg_object_size(pkg, i);

				if (t != NULL && n != NULL && l > 0) {
					strcpy(filename, n);
					strcat(filename, ".");
					strcat(filename, t);

					ofs0 = upkg_export_offset(pkg, i);
					ofs1 = upkg_object_offset(pkg, i);
					n = upkg_oclassname(pkg, i);
					printf("%s\t(%s, %ld bytes, export @ 0x%x, object @ 0x%x)\n",
						filename, n, l, ofs0, ofs1);

					upkg_object_dump(pkg, filename, i);
				}
			}

			upkg_close(pkg);
			printf("\n");
		}
		else {
			printf("Failed opening or not an upkg\n\n");
		}
	}

	printf("%d files processed\n\n", j);

	return 0;
}
