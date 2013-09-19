#include <stdio.h>
#include <string.h>

#include "umr.h"

static char filename[128];


static void usage(const char *myname)
{
	printf("%s filename [filename ...]\n", myname);
}

static void get_filename(char *out, int upkg_num)
{
	strcpy(out, upkg_oname(upkg_num));
	strcat(out, ".");
	strcat(out, upkg_otype(upkg_num));
}

int main(int argc, char *argv[])
{
	int i, j, c;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	printf("%d file", argc - 1);
	if (argc > 2)
		printf("s\n\n");
	else
		printf("\n\n");

	j = 0;

	for (c = 1; c < argc; c++) {

		if (upkg_open(argv[c]) == 0) {
			j++;

			printf("%s\n", argv[c]);

			for (i = 1; i < upkg_ocount() + 1; i++) {
				if (upkg_otype(i) != NULL) {
					get_filename(filename, i);

					printf("%s\t(%s, %d bytes)\n",
					       filename,
					       upkg_oclassname(i),
					       upkg_object_size(i));

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
