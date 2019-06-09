/*
 * Generate a FCS table for the HDLC FCS (RFC1171).
 *
 * Drew D. Perkins at Carnegie Mellon University.
 *
 * Code liberally borrowed from Mohsen Banan and D. Hugh Redelmeier.
 */

#include <stdio.h>

/* The HDLC polynomail: X**0 + X**5 + X**12 + X**16 (0x8408) */

#define Polynomial	0x8408

int
main (int argc, char *argv[])
{
	unsigned int	count, octet;
	int		bit;

	printf("%s = {\n", argv[1]);
	for (count = 0; count < 256; count ++) {
		octet = count;
		for (bit = 8; bit-- > 0; )
			octet = (octet & 0x01) ? ((octet >> 1) ^ Polynomial) : (octet >> 1);
		printf("%c0x%04x,", count % 8 ? ' ' : '\t', octet & 0xffff);
		if (count % 8 == 7)
			putchar('\n');
		}
	fputs("\t};\n", stdout);
	return 0;
	}
