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
	uint8_t data[64];
	struct block *next;
} Block;

// merge four uint8_t variables to a uint32_t variable in big-endian order.
uint32_t merge_uint8_t(uint8_t *p)
{
	uint8_t v0 = *p;
	uint8_t v1 = *(p + 1);
	uint8_t v2 = *(p + 2);
	uint8_t v3 = *(p + 3);

	return v3 | v2 << 8 | v1 << 16 | v0 << 24;
}

void reader(FILE *fp, Block *b)
{
	uint8_t t;
	uint64_t i = 0;
	uint64_t l = 0;

	for (t = fgetc(fp); !feof(fp); t = fgetc(fp)) {
		b->data[i] = t;

		if (i < 63) {
			i++;
			l += 8;	// count the number of bits
		} else {
			i = 0;

			b->next = malloc(sizeof(Block));
			b = b->next;
			b->next = NULL;
		}
	}

	b->data[i] = 0x80;	// mark the end of message.

	if (i > 55) {
		for (i += 1; i < 64; i++) {
			b->data[i] = 0;
		}

		b->next = malloc(sizeof(Block));
		b = b->next;
		b->next = NULL;

		for (i = 0; i < 56; i++) {
			b->data[i] = 0;
		}
	} else {
		for (i += 1; i < 56; i++) {
			b->data[i] = 0;
		}
	}

	for (i = 0; i < 8; i++) {
		b->data[i + 56] = (l >> ((8 - i - 1) * 8)) & 0xff;
	}
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
			for (j = 0; j < 64; j += 4) {
				printf("%08x\n", merge_uint8_t(&current->data[j]));
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
