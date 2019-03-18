/*
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
   SHA-1 in C
   By Steve Reid <sreid@sea-to-sky.net>
   100% Public Domain

   -----------------
   Modified 7/98
   By James H. Brown <jbrown@burgoyne.com>
   Still 100% Public Domain

   Corrected a problem which generated improper hash values on 16 bit machines
   Routine SHA1Update changed from
        void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int
   len)
   to
        void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned
   long len)

   The 'len' parameter was declared an int which works fine on 32 bit machines.
   However, on 16 bit machines an int is too small for the shifts being done
   against
   it.  This caused the hash function to generate incorrect values if len was
   greater than 8191 (8K - 1) due to the 'len << 3' on line 3 of SHA1Update().

   Since the file IO in main() reads 16K at a time, any file 8K or larger would
   be guaranteed to generate the wrong hash (e.g. Test Vector #3, a million
   "a"s).

   I also changed the declaration of variables i & j in SHA1Update to
   ulong from unsigned int for the same reason.

   These changes should make no difference to any 32 bit implementations since
   an
   int and a long are the same size in those environments.

   --
   I also corrected a few compiler warnings generated by Borland C.
   1. Added #include <process.h> for exit() prototype
   2. Removed unused variable 'j' in SHA1Final
   3. Changed exit(0) to return (0) at end of main.

   ALL changes I made can be located by searching for comments containing 'JHB'
   -----------------
   Modified 8/98
   By Steve Reid <sreid@sea-to-sky.net>
   Still 100% public domain

   1- Removed #include <process.h> and used return () instead of exit()
   2- Fixed overwriting of finalcount in SHA1Final() (discovered by Chris Hall)
   3- Changed email address from steve@edmweb.com to sreid@sea-to-sky.net

   -----------------
   Modified 4/01
   By Saul Kravitz <Saul.Kravitz@celera.com>
   Still 100% PD
   Modified to run on Compaq Alpha hardware.

 */

/*
   Test Vectors (from FIPS PUB 180-1)
   "abc"
   A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
   "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
   84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
   A million repetitions of "a"
   34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 */

#define SHA1HANDSOFF

/* #include <process.h> */	/* prototype for exit() - JHB */
/* Using return () instead of exit() - SWR */

#define rol(value, bits) (((value)<<(bits))|((value)>>(32-(bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define blk0(i) is_bigendian ? block->l[i] : (block->l[i] = (rol(block->l[i], 24)&0xFF00FF00)|(rol(block->l[i], 8)&0x00FF00FF))
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] ^ block->l[(i+2)&15]^block->l[i&15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define SHA1_R0(v, w, x, y, z, i) z += ((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v, 5); w = rol(w, 30);
#define SHA1_R1(v, w, x, y, z, i) z += ((w&(x^y))^y)+blk(i)+0x5A827999+rol(v, 5); w = rol(w, 30);
#define SHA1_R2(v, w, x, y, z, i) z += (w^x^y)+blk(i)+0x6ED9EBA1+rol(v, 5); w = rol(w, 30);
#define SHA1_R3(v, w, x, y, z, i) z += (((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v, 5); w = rol(w, 30);
#define SHA1_R4(v, w, x, y, z, i) z += (w^x^y)+blk(i)+0xCA62C1D6+rol(v, 5); w = rol(w, 30);

#ifdef VERBOSE  /* SAK */
static void __db_SHAPrintContext __P((SHA1_CTX*, char *));

static void __db_SHAPrintContext(SHA1_CTX * context, char * msg)
{
	printf("%s (%d,%d) %x %x %x %x %x\n", msg,
		context->count[0], context->count[1], context->state[0],
		context->state[1], context->state[2], context->state[3], context->state[4]);
}
#endif

/* Hash a single 512-bit block. This is the core of the algorithm. */

/*
 * __db_SHA1Transform --
 *
 * PUBLIC: void __db_SHA1Transform __P((uint32 *, unsigned char *));
 */
void __db_SHA1Transform(uint32 * state, unsigned char * buffer)
{
	uint32 a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		uint32 l[16];
	} CHAR64LONG16;
	CHAR64LONG16 * block;
	int is_bigendian;
#ifdef SHA1HANDSOFF
	unsigned char workspace[64];
	block = (CHAR64LONG16 *)workspace;
	memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16 *)buffer;
#endif
	is_bigendian = __db_isbigendian();

	// Copy context->state[] to working vars 
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	// 4 rounds of 20 operations each. Loop unrolled. 
	SHA1_R0(a, b, c, d, e, 0);  SHA1_R0(e, a, b, c, d, 1);  SHA1_R0(d, e, a, b, c, 2);  SHA1_R0(c, d, e, a, b, 3);
	SHA1_R0(b, c, d, e, a, 4);  SHA1_R0(a, b, c, d, e, 5);  SHA1_R0(e, a, b, c, d, 6);  SHA1_R0(d, e, a, b, c, 7);
	SHA1_R0(c, d, e, a, b, 8);  SHA1_R0(b, c, d, e, a, 9);  SHA1_R0(a, b, c, d, e, 10); SHA1_R0(e, a, b, c, d, 11);
	SHA1_R0(d, e, a, b, c, 12); SHA1_R0(c, d, e, a, b, 13); SHA1_R0(b, c, d, e, a, 14); SHA1_R0(a, b, c, d, e, 15);
	SHA1_R1(e, a, b, c, d, 16); SHA1_R1(d, e, a, b, c, 17); SHA1_R1(c, d, e, a, b, 18); SHA1_R1(b, c, d, e, a, 19);
	SHA1_R2(a, b, c, d, e, 20); SHA1_R2(e, a, b, c, d, 21); SHA1_R2(d, e, a, b, c, 22); SHA1_R2(c, d, e, a, b, 23);
	SHA1_R2(b, c, d, e, a, 24); SHA1_R2(a, b, c, d, e, 25); SHA1_R2(e, a, b, c, d, 26); SHA1_R2(d, e, a, b, c, 27);
	SHA1_R2(c, d, e, a, b, 28); SHA1_R2(b, c, d, e, a, 29); SHA1_R2(a, b, c, d, e, 30); SHA1_R2(e, a, b, c, d, 31);
	SHA1_R2(d, e, a, b, c, 32); SHA1_R2(c, d, e, a, b, 33); SHA1_R2(b, c, d, e, a, 34); SHA1_R2(a, b, c, d, e, 35);
	SHA1_R2(e, a, b, c, d, 36); SHA1_R2(d, e, a, b, c, 37); SHA1_R2(c, d, e, a, b, 38); SHA1_R2(b, c, d, e, a, 39);
	SHA1_R3(a, b, c, d, e, 40); SHA1_R3(e, a, b, c, d, 41); SHA1_R3(d, e, a, b, c, 42); SHA1_R3(c, d, e, a, b, 43);
	SHA1_R3(b, c, d, e, a, 44); SHA1_R3(a, b, c, d, e, 45); SHA1_R3(e, a, b, c, d, 46); SHA1_R3(d, e, a, b, c, 47);
	SHA1_R3(c, d, e, a, b, 48); SHA1_R3(b, c, d, e, a, 49); SHA1_R3(a, b, c, d, e, 50); SHA1_R3(e, a, b, c, d, 51);
	SHA1_R3(d, e, a, b, c, 52); SHA1_R3(c, d, e, a, b, 53); SHA1_R3(b, c, d, e, a, 54); SHA1_R3(a, b, c, d, e, 55);
	SHA1_R3(e, a, b, c, d, 56); SHA1_R3(d, e, a, b, c, 57); SHA1_R3(c, d, e, a, b, 58); SHA1_R3(b, c, d, e, a, 59);
	SHA1_R4(a, b, c, d, e, 60); SHA1_R4(e, a, b, c, d, 61); SHA1_R4(d, e, a, b, c, 62); SHA1_R4(c, d, e, a, b, 63);
	SHA1_R4(b, c, d, e, a, 64); SHA1_R4(a, b, c, d, e, 65); SHA1_R4(e, a, b, c, d, 66); SHA1_R4(d, e, a, b, c, 67);
	SHA1_R4(c, d, e, a, b, 68); SHA1_R4(b, c, d, e, a, 69); SHA1_R4(a, b, c, d, e, 70); SHA1_R4(e, a, b, c, d, 71);
	SHA1_R4(d, e, a, b, c, 72); SHA1_R4(c, d, e, a, b, 73); SHA1_R4(b, c, d, e, a, 74); SHA1_R4(a, b, c, d, e, 75);
	SHA1_R4(e, a, b, c, d, 76); SHA1_R4(d, e, a, b, c, 77); SHA1_R4(c, d, e, a, b, 78); SHA1_R4(b, c, d, e, a, 79);
	// Add the working vars back into context.state[] 
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
}

/* SHA1Init - Initialize new context */

/*
 * __db_SHA1Init --
 *    Initialize new context
 *
 * PUBLIC: void __db_SHA1Init __P((SHA1_CTX *));
 */
void __db_SHA1Init(SHA1_CTX * context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */

/*
 * __db_SHA1Update --
 *    Run your data through this.
 *
 * PUBLIC: void __db_SHA1Update __P((SHA1_CTX *, unsigned char *,
 * PUBLIC:     size_t));
 */
void __db_SHA1Update(SHA1_CTX * context, unsigned char * data, size_t len)
{
	uint32 i, j; /* JHB */
#ifdef VERBOSE
	__db_SHAPrintContext(context, DB_STR_P("before"));
#endif
	j = (context->count[0]>>3)&63;
	if((context->count[0] += (uint32)len<<3) < (len<<3))
		context->count[1]++;
	context->count[1] += (uint32)(len>>29);
	if((j+len) > 63) {
		memcpy(&context->buffer[j], data, (i = 64-j));
		__db_SHA1Transform(context->state, context->buffer);
		for(; i+63 < len; i += 64) {
			__db_SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	}
	else
		i = 0;
	memcpy(&context->buffer[j], &data[i], len-i);
#ifdef VERBOSE
	__db_SHAPrintContext(context, DB_STR_P("after "));
#endif
}

/* Add padding and return the message digest. */

/*
 * __db_SHA1Final --
 *    Add padding and return the message digest.
 *
 * PUBLIC: void __db_SHA1Final __P((unsigned char *, SHA1_CTX *));
 */
void __db_SHA1Final(unsigned char * digest, SHA1_CTX * context)
{
	uint32 i; /* JHB */
	unsigned char finalcount[8];
	for(i = 0; i < 8; i++) {
		finalcount[i] = (uchar)((context->count[(i >= 4 ? 0 : 1)]>>((3-(i&3))*8) )&255); /* Endian independent */
	}
	__db_SHA1Update(context, (unsigned char *)"\200", 1);
	while((context->count[0]&504) != 448) {
		__db_SHA1Update(context, (unsigned char *)"\0", 1);
	}
	__db_SHA1Update(context, finalcount, 8); /* Should cause a SHA1Transform() */
	for(i = 0; i < 20; i++) {
		digest[i] = (uchar)((context->state[i>>2]>>((3-(i&3))*8) )&255);
	}
	/* Wipe variables */
	i = 0;  /* JHB */
	memzero(context->buffer, 64);
	memzero(context->state, 20);
	memzero(context->count, 8);
	memzero(finalcount, 8); /* SWR */
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
	__db_SHA1Transform(context->state, context->buffer);
#endif
}

/*************************************************************/
