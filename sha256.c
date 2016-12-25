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

typedef struct block {
	unsigned char data[64];
	struct block *next;
} Block;

int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned char t;
	int i;
	int j;
	long l;
	unsigned char g;
	unsigned char s;

	if (argc < 2) {
		printf("Usage: %s [file] ...\n", argv[0]);
		return 0;
	}

	for (i = 1; i < argc; i++) {

		Block *head = malloc(sizeof(Block));
		head->next = NULL;

		Block *current = head;

		fp = fopen(argv[i], "rb");

		if (!fp) {
			fprintf(stderr,
				"Unable to open %s: %s\n",
				argv[i], strerror(errno));

			return errno;
		}

		j = 0;
		l = 0;

		for (t = fgetc(fp); !feof(fp); t = fgetc(fp)) {
			current->data[j] = t;

			if (j < 63) {
				j++;
				l += 8;	// count the number of bits
			} else {
				j = 0;

				current->next = malloc(sizeof(Block));
				current = current->next;
				current->next = NULL;
			}
		}

		current->data[j] = 0x80;	// mark the end of message.

		if (j > 55) {
			for (j += 1; j < 64; j++) {
				current->data[j] = 0;
			}

			current->next = malloc(sizeof(Block));
			current = current->next;
			current->next = NULL;

			for (j = 0; j < 56; j++) {
				current->data[j] = 0;
			}
		} else {
			for (j += 1; j < 56; j++) {
				current->data[j] = 0;
			}
		}

		for (j = 0; j < 8; j++) {
			current->data[j + 56] = (l >> ((8 - j - 1) * 8)) & 0xff;
		}

		current = head;

		while (current->next != NULL) {
			for (j = 0; j < 64; j++) {
				printf("%02x\n", current->data[j]);
			}
			current = current->next;
		}

		for (j = 0; j < 64; j++) {
			printf("%02x\n", current->data[j]);
		}

	}

	return 0;
}
