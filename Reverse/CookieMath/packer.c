#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define FLAG       "flag{alg3bra_1z_sti11_fun_y0!}"
#define DECOY_FLAG "flag{h4h4h4_n1c3_7ry_h0tsh0t!}"
//                  0123456789abcdef0123456789abcd
//                  a   b   c   d   e   f

// Indices to pull out chunks from input
#define A 0x05
#define B 0x09
#define C 0x0D
#define D 0x11
#define E 0x15
#define F 0x19
//TODO: Make a packer state dependent
//TODO: Make 'red herrings' that invalidate the result
//TODO: Draw the unpack/pack graph
//TODO: Finish the simple verify
//TODO: Scramble order of functions AND CALLs
//TODO: Add more decoys
//TODO: Generate the final binary
#define FLAG_LEN sizeof(FLAG)

char *input_buf = NULL;
unsigned int cookie = 0;

#define COOKIE_TARGET 0xdeadbea7
#define FORMAT_COOKIE 0x35e4eebf

#define COOKIE_1 0xa47e904c
#define COOKIE_2 0xcbff9345
#define COOKIE_3 0xc30b088b
#define COOKIE_4 0x7e3c14cd
#define COOKIE_5 0x372f660e
#define COOKIE_6 0x5f3ceee9
#define COOKIE_7 0xb93e4867
#define COOKIE_8 0xe8d28fd7

#define COOKIE_DECOY_TARGET 0x0101f001
#define COOKIE_BAD1 0xb2c58526
#define COOKIE_BAD2 0x65aafd58
#define COOKIE_BAD3 0xbd87bd77
#define COOKIE_BAD4 0x21ddc691
#define COOKIE_BAD5 0x453057e3
#define COOKIE_BAD6 0xce71dd4a
#define COOKIE_BAD7 0x4d663570
#define COOKIE_BAD8 0xb9f7a2ff

#define MAKE_FUNC(num, x, y, sum, hash) static void v##num()\
{\
	unsigned int a = *(int*)&input_buf[x];\
	unsigned int b = *(int*)&input_buf[y];\
	if ((a + b) == sum)\
	{\
		cookie ^= hash;\
	}\
}
void fail()
{
	printf("No bueno\n");
	exit(EXIT_FAILURE);
}
void win()
{
	printf("You win! You should know the flag by now\n");
	exit(EXIT_SUCCESS);
}

void flag_format_check()
{
	if (input_buf[4] == '{')
	if (input_buf[2] == 'a')
	if (input_buf[1] == 'l')
	if (input_buf[0] == 'f')
	if (input_buf[3] == 'g')
	if (input_buf[FLAG_LEN-2] == '}')
		cookie ^= FORMAT_COOKIE;
}

MAKE_FUNC(6, A, B, 0x92c8dec3, COOKIE_6)
MAKE_FUNC(d1, A, C, 0x939b9799, COOKIE_BAD1)
MAKE_FUNC(d4, A, D, 0x93e1a69f, COOKIE_BAD4)
MAKE_FUNC(1, A, E, 0xa1dcd2c0, COOKIE_1)
MAKE_FUNC(3, A, F, 0x5497e5c0, COOKIE_3)
MAKE_FUNC(d5, A, F, 0x55dc64d0, COOKIE_BAD5)
MAKE_FUNC(d2, B, C, 0xcd929799, COOKIE_BAD2)
MAKE_FUNC(d6, B, D, 0xcdd8a69f, COOKIE_BAD6)
MAKE_FUNC(4, B, E, 0xcdd6d8c1, COOKIE_4)
MAKE_FUNC(d7, B, F, 0x8fd364d0, COOKIE_BAD7)
MAKE_FUNC(2, C, D, 0xa490e3a5, COOKIE_2)
MAKE_FUNC(7, C, E, 0xe1d4e090, COOKIE_7)
MAKE_FUNC(d3, C, F, 0x80a79399, COOKIE_BAD3)
MAKE_FUNC(5, D, E, 0x9fa6cfd3, COOKIE_5)
MAKE_FUNC(8, D, F, 0x5261e2d3, COOKIE_8)
MAKE_FUNC(d8, E, F, 0x94e860d0, COOKIE_BAD8)
/*
 * abcdef
 * ab ac ad ae af
 *    bc bd be bf
 *       cd ce cf
 *          de df
 *             ef
 */

int main(int argc, char* argv[])
{
	unsigned int cookie_key = 0xdfac4ea6;
	if (argc != 2) {
		printf("usage: %s flag\n", argv[0]);
		return -1;
	}
	input_buf = (char*)malloc(FLAG_LEN);

	strncpy(input_buf, argv[1], FLAG_LEN);
	input_buf[FLAG_LEN - 1] = '\0';

	v7();
	vd8();
	v4();
	vd4();
	vd2();
	v5();
	v2();
	flag_format_check();
	vd5();
	v8();
	vd6();
	v3();
	vd1();
	v1();
	vd7();
	v6();
	vd3();

	if ((cookie ^ cookie_key) == COOKIE_DECOY_TARGET)
	{
		win();
	} else {
		fail();
	}

	return 0;
}
