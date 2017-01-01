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

const uint32_t hvi[8] = {
	0x6a09e667,
	0xbb67ae85,
	0x3c6ef372,
	0xa54ff53a,
	0x510e527f,
	0x9b05688c,
	0x1f83d9ab,
	0x5be0cd19
};

const uint32_t k[64] = {
	0x428a2f98,
	0x71374491,
	0xb5c0fbcf,
	0xe9b5dba5,
	0x3956c25b,
	0x59f111f1,
	0x923f82a4,
	0xab1c5ed5,
	0xd807aa98,
	0x12835b01,
	0x243185be,
	0x550c7dc3,
	0x72be5d74,
	0x80deb1fe,
	0x9bdc06a7,
	0xc19bf174,
	0xe49b69c1,
	0xefbe4786,
	0x0fc19dc6,
	0x240ca1cc,
	0x2de92c6f,
	0x4a7484aa,
	0x5cb0a9dc,
	0x76f988da,
	0x983e5152,
	0xa831c66d,
	0xb00327c8,
	0xbf597fc7,
	0xc6e00bf3,
	0xd5a79147,
	0x06ca6351,
	0x14292967,
	0x27b70a85,
	0x2e1b2138,
	0x4d2c6dfc,
	0x53380d13,
	0x650a7354,
	0x766a0abb,
	0x81c2c92e,
	0x92722c85,
	0xa2bfe8a1,
	0xa81a664b,
	0xc24b8b70,
	0xc76c51a3,
	0xd192e819,
	0xd6990624,
	0xf40e3585,
	0x106aa070,
	0x19a4c116,
	0x1e376c08,
	0x2748774c,
	0x34b0bcb5,
	0x391c0cb3,
	0x4ed8aa4a,
	0x5b9cca4f,
	0x682e6ff3,
	0x748f82ee,
	0x78a5636f,
	0x84c87814,
	0x8cc70208,
	0x90befffa,
	0xa4506ceb,
	0xbef9a3f7,
	0xc67178f2
};

uint32_t flip_uint32_t(uint32_t x)
{
	return ((x & 0x000000ff) << 24) |
		((x & 0x0000ff00) << 8) |
		((x & 0x00ff0000) >> 8) |
		((x & 0xff000000) >> 24);
}

/*
 * reader() takes a file pointer and a head pointer to a singly linked list.
 * reader() extends the singly linked list in place.
 */

void reader(FILE * fp, Block * b)
{
	uint64_t l = 0;
	uint_least8_t i;

	for (;;) {
		l += fread(b->data, sizeof(uint8_t), 64, fp);

		for (i = 0; i < 16; i++) {
			b->data[i] = flip_uint32_t(b->data[i]);
		}

		/*
		 * If the message fits precisely a block, a new block is
		 * created and eof will be encountered in the next loop.
		 * This guarantees 0x80 will be placed in the right place.
		 */

		if (feof(fp)) {
			break;
		} else {
			b->next = calloc(1, sizeof(Block));
			b = b->next;
			b->next = NULL;
		}
	}

	b->data[l % 64 / 4] |= 0x80 << (4 - l % 4 - 1) * 8;

	// extend b if there are no room for for l.
	if (l % 64 / 4 > 13) {
		b->next = calloc(1, sizeof(Block));
		b = b->next;
		b->next = NULL;
	}

	l *= 8;

	b->data[14] = (uint32_t) (l >> 32);

	b->data[15] = (uint32_t) (l & 0xffffffff);
}

// Ch function defined in SHA-256 white paper
uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (~x & z);
}

// Maj function defined in SHA-256 white paper
uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
	return (x & y) ^ (x & z) ^ (y & z);
}

// right rotation 
uint32_t s(uint32_t x, uint_least8_t n)
{
	return x >> n | x << (8 * sizeof(uint32_t) - n);
}

// Sigma_0 function defined in SHA-256 white paper
uint32_t s_0(uint32_t x)
{
	return s(x, 2) ^ s(x, 13) ^ s(x, 22);
}

// Sigma_1 function defined in SHA-256 white paper
uint32_t s_1(uint32_t x)
{
	return s(x, 6) ^ s(x, 11) ^ s(x, 25);
}

// sigma_0 function defined in SHA-256 white paper
uint32_t s_3(uint32_t x)
{
	return s(x, 7) ^ s(x, 18) ^ (x >> 3);
}

// sigma_1 function defined in SHA-256 white paper
uint32_t s_4(uint32_t x)
{
	return s(x, 17) ^ s(x, 19) ^ (x >> 10);
}

uint32_t *extend(Block * b)
{
	uint_least8_t i;

	uint32_t *eb = malloc(sizeof(uint32_t) * 64);

	memcpy(eb, b->data, sizeof(b->data));

	for (i = 16; i < 64; i++) {
		eb[i] = s_4(eb[i - 2]);
		eb[i] += eb[i - 7];
		eb[i] += s_3(eb[i - 15]);
		eb[i] += eb[i - 16];
	}

	return eb;
}

uint32_t *hash(Block * bl)
{
	uint32_t a, b, c, d, e, f, g, h, t1, t2;
	uint_least8_t i;
	uint32_t *eb;
	Block *current = bl;
	Block *recycle;
	uint32_t *ha = malloc(sizeof(hvi));

	memcpy(ha, hvi, sizeof(hvi));

	for (;;) {

		a = ha[0];
		b = ha[1];
		c = ha[2];
		d = ha[3];
		e = ha[4];
		f = ha[5];
		g = ha[6];
		h = ha[7];

		eb = extend(current);

		for (i = 0; i < 64; i++) {
			t1 = h + s_1(e) + ch(e, f, g) + k[i] + eb[i];
			t2 = s_0(a) + maj(a, b, c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		free(eb);

		ha[0] += a;
		ha[1] += b;
		ha[2] += c;
		ha[3] += d;
		ha[4] += e;
		ha[5] += f;
		ha[6] += g;
		ha[7] += h;

		if (current->next == NULL) {
			free(current);
			break;
		} else {
			recycle = current;
			current = current->next;
			free(recycle);
		}
	}

	return ha;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	uint_least8_t i;
	uint_least8_t j;
	uint32_t *hv;

	if (argc < 2) {
		printf("Usage: %s [file] ...\n", argv[0]);
		return 0;
	}

	for (i = 1; i < argc; i++) {

		Block *head = calloc(1, sizeof(Block));
		head->next = NULL;

		fp = fopen(argv[i], "rb");

		if (!fp) {
			fprintf(stderr,
				"Unable to open %s: %s\n",
				argv[i], strerror(errno));

			return errno;
		}

		reader(fp, head);

		hv = hash(head);

		for (j = 0; j < 8; j++) {
			printf("%08x", hv[j]);
		}

		printf("\t%s\n", argv[i]);

		free(hv);
	}

	return 0;
}
