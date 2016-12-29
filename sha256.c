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
#include <stdint.h>
#include <errno.h>

typedef struct block {
	uint32_t data[16];
	struct block *next;
} Block;

// reader() takes a file pointer and a head pointer to a singly linked list.
// reader() extends the singly linked list in place.

void reader(FILE *fp, Block *b)
{
	uint32_t t;
	uint64_t i = 0;
	uint64_t j = 3;
	uint64_t l = 0;

	for (t = fgetc(fp); !feof(fp); t = fgetc(fp)) {

		l += 8;	// count the number of bits

		b->data[i] |= t << j * 8;

		if (i > 15) {
			i = 0;
			b->next = malloc(sizeof(Block));
			b = b->next;
			b->next = NULL;
			memset(b->data, 0, sizeof(b->data));
		}

		if (j == 0) {
			i++;
			j = 3;
		} else {
			j--;
		}
	}

	b->data[i] |= 0x80 << j * 8;	// mark the end of message.

	if (i > 13) {
		b->next = malloc(sizeof(Block));
		b = b->next;
		b->next = NULL;
		memset(b->data, 0, sizeof(b->data));
	}

	b->data[14] = (uint32_t) (l >> 32);

	b->data[15] = (uint32_t) (l & 0xffffffff);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	uint64_t i;
	uint64_t j;

	if (argc < 2) {
		printf("Usage: %s [file] ...\n", argv[0]);
		return 0;
	}

	for (i = 1; i < argc; i++) {

		Block *head = malloc(sizeof(Block));
		head->next = NULL;
		memset(head->data, 0, sizeof(head->data));

		Block *current = head;

		fp = fopen(argv[i], "rb");

		if (!fp) {
			fprintf(stderr,
				"Unable to open %s: %s\n",
				argv[i], strerror(errno));

			return errno;
		}

		reader(fp, head);

		current = head;

		for (;;) {
			for (j = 0; j < 16; j ++) {
				printf("%08x\n", current->data[j]);
			}

			if (current->next == NULL) {
				break;
			} else {
				current = current->next;
			}
		}
	}

	return 0;
}
