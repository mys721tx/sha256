/*
 * sha256.c: an implementation of SHA-256 in C
 *
 * Copyright (c) 2016 Yishen Miao
 *
 * This file is part of sha256
 *
 * sha256 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * sha256 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sha256.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned char t;
	int i;

	if (argc < 2) {
		printf("Usage: %s [file] ...\n", argv[0]);
		return 0;
	}

	for (i = 1; i < argc; i++) {
		fp = fopen(argv[i], "rb");

		if (!fp) {
			fprintf(stderr,
				"Unable to open %s: %s\n",
				argv[i], strerror(errno));

			return errno;
		}

		for (t = fgetc(fp); !feof(fp); t = fgetc(fp)) {
			printf("0x%02x\n", t);
		}
	}

	return 0;
}
