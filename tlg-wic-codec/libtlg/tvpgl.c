/*

	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000-2009 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"


*/

/* core C routines for graphics operations */
/* this file is always generated by gengl.pl rev. 0.1 */

/* #include "tjsCommHead.h" */
#include "tvpgl.h"

#include <memory.h>
#include <math.h>

/*export*/
TVP_GL_FUNC_DECL(void, TVPFillARGB, (tjs_uint32 *dest, tjs_int len, tjs_uint32 value))
{
	{
		int ___index = 0;
		len -= (8-1);

		while(___index < len)
		{
{
	dest[(___index+0)] = value;
}
{
	dest[(___index+1)] = value;
}
{
	dest[(___index+2)] = value;
}
{
	dest[(___index+3)] = value;
}
{
	dest[(___index+4)] = value;
}
{
	dest[(___index+5)] = value;
}
{
	dest[(___index+6)] = value;
}
{
	dest[(___index+7)] = value;
}
			___index += 8;
		}

		len += (8-1);

		while(___index < len)
		{
{
	dest[___index] = value;
}
			___index ++;
		}
	}
}

/*-----------------------------------------------------------------*/

#define TVP_TLG6_GOLOMB_HALF_THRESHOLD 8


#define TVP_TLG6_GOLOMB_N_COUNT  4
#define TVP_TLG6_LeadingZeroTable_BITS 12
#define TVP_TLG6_LeadingZeroTable_SIZE  (1<<TVP_TLG6_LeadingZeroTable_BITS)
tjs_uint8 TVPTLG6LeadingZeroTable[TVP_TLG6_LeadingZeroTable_SIZE];
short int TVPTLG6GolombCompressed[TVP_TLG6_GOLOMB_N_COUNT][9] = {
		{3,7,15,27,63,108,223,448,130,},
		{3,5,13,24,51,95,192,384,257,},
		{2,5,12,21,39,86,155,320,384,},
		{2,3,9,18,33,61,129,258,511,},
	/* Tuned by W.Dee, 2004/03/25 */
};
char TVPTLG6GolombBitLengthTable
	[TVP_TLG6_GOLOMB_N_COUNT*2*128][TVP_TLG6_GOLOMB_N_COUNT] =
	{ { 0 } };



void TVPTLG6InitLeadingZeroTable(void)
{
	/* table which indicates first set bit position + 1. */
	/* this may be replaced by BSF (IA32 instrcution). */

	int i;
	for(i = 0; i < TVP_TLG6_LeadingZeroTable_SIZE; i++)
	{
		int cnt = 0;
		int j;
		for(j = 1; j != TVP_TLG6_LeadingZeroTable_SIZE && !(i & j);
			j <<= 1, cnt++);
		cnt ++;
		if(j == TVP_TLG6_LeadingZeroTable_SIZE) cnt = 0;
		TVPTLG6LeadingZeroTable[i] = cnt;
	}
}

void TVPTLG6InitGolombTable(void)
{
	int n, i, j;
	for(n = 0; n < TVP_TLG6_GOLOMB_N_COUNT; n++)
	{
		int a = 0;
		for(i = 0; i < 9; i++)
		{
			for(j = 0; j < TVPTLG6GolombCompressed[n][i]; j++)
				TVPTLG6GolombBitLengthTable[a++][n] = (char)i;
		}
		if(a != TVP_TLG6_GOLOMB_N_COUNT*2*128)
			*(char*)0 = 0;   /* THIS MUST NOT BE EXECUETED! */
				/* (this is for compressed table data check) */
	}
}

void TVPCreateTable(void)
{
	TVPTLG6InitLeadingZeroTable();
	TVPTLG6InitGolombTable();
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG5ComposeColors3To4, (tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const * buf, tjs_int width))
{
	tjs_int x;
	tjs_uint8 pc[3];
	tjs_uint8 c[3];
	pc[0] = pc[1] = pc[2] = 0;
	for(x = 0; x < width; x++)
	{
		c[0] = buf[0][x];
		c[1] = buf[1][x];
		c[2] = buf[2][x];
		c[0] += c[1]; c[2] += c[1];
		*(tjs_uint32 *)outp =
								((((pc[0] += c[0]) + upper[0]) & 0xff)      ) +
								((((pc[1] += c[1]) + upper[1]) & 0xff) <<  8) +
								((((pc[2] += c[2]) + upper[2]) & 0xff) << 16) +
								0xff000000;
		outp += 4;
		upper += 4;
	}
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG5ComposeColors4To4, (tjs_uint8 *outp, const tjs_uint8 *upper, tjs_uint8 * const* buf, tjs_int width))
{
	tjs_int x;
	tjs_uint8 pc[4];
	tjs_uint8 c[4];
	pc[0] = pc[1] = pc[2] = pc[3] = 0;
	for(x = 0; x < width; x++)
	{
		c[0] = buf[0][x];
		c[1] = buf[1][x];
		c[2] = buf[2][x];
		c[3] = buf[3][x];
		c[0] += c[1]; c[2] += c[1];
		*(tjs_uint32 *)outp =
								((((pc[0] += c[0]) + upper[0]) & 0xff)      ) +
								((((pc[1] += c[1]) + upper[1]) & 0xff) <<  8) +
								((((pc[2] += c[2]) + upper[2]) & 0xff) << 16) +
								((((pc[3] += c[3]) + upper[3]) & 0xff) << 24);
		outp += 4;
		upper += 4;
	}
}

/*export*/
TVP_GL_FUNC_DECL(tjs_int, TVPTLG5DecompressSlide, (tjs_uint8 *out, const tjs_uint8 *in, tjs_int insize, tjs_uint8 *text, tjs_int initialr))
{
	tjs_int r = initialr;
	tjs_uint flags = 0;
	const tjs_uint8 *inlim = in + insize;
	while(in < inlim)
	{
		if(((flags >>= 1) & 256) == 0)
		{
			flags = 0[in++] | 0xff00;
		}
		if(flags & 1)
		{
			tjs_int mpos = in[0] | ((in[1] & 0xf) << 8);
			tjs_int mlen = (in[1] & 0xf0) >> 4;
			in += 2;
			mlen += 3;
			if(mlen == 18) mlen += 0[in++];

			while(mlen--)
			{
				0[out++] = text[r++] = text[mpos++];
				mpos &= (4096 - 1);
				r &= (4096 - 1);
			}
		}
		else
		{
			unsigned char c = 0[in++];
			0[out++] = c;
			text[r++] = c;
/*			0[out++] = text[r++] = 0[in++];*/
			r &= (4096 - 1);
		}
	}
	return r;
}


#if TJS_HOST_IS_BIG_ENDIAN
	#define TVP_TLG6_BYTEOF(a, x) (((tjs_uint8*)(a))[(x)])

	#define TVP_TLG6_FETCH_32BITS(addr) ((tjs_uint32)TVP_TLG6_BYTEOF((addr), 0) +  \
									((tjs_uint32)TVP_TLG6_BYTEOF((addr), 1) << 8) + \
									((tjs_uint32)TVP_TLG6_BYTEOF((addr), 2) << 16) + \
									((tjs_uint32)TVP_TLG6_BYTEOF((addr), 3) << 24) )
#else
	#define TVP_TLG6_FETCH_32BITS(addr) (*(tjs_uint32*)addr)
#endif



/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeGolombValuesForFirst, (tjs_int8 *pixelbuf, tjs_int pixel_count, tjs_uint8 *bit_pool))
{
	/*
		decode values packed in "bit_pool".
		values are coded using golomb code.

		"ForFirst" function do dword access to pixelbuf,
		clearing with zero except for blue (least siginificant byte).
	*/

	int n = TVP_TLG6_GOLOMB_N_COUNT - 1; /* output counter */
	int a = 0; /* summary of absolute values of errors */

	tjs_int bit_pos = 1;
	tjs_uint8 zero = (*bit_pool & 1)?0:1;

	tjs_int8 * limit = pixelbuf + pixel_count*4;

	while(pixelbuf < limit)
	{
		/* get running count */
		int count;

		{
			tjs_uint32 t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
			tjs_int b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
			int bit_count = b;
			while(!b)
			{
				bit_count += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;
				t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
				b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
				bit_count += b;
			}


			bit_pos += b;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

			bit_count --;
			count = 1 << bit_count;
			count += ((TVP_TLG6_FETCH_32BITS(bit_pool) >> (bit_pos)) & (count-1));

			bit_pos += bit_count;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

		}

		if(zero)
		{
			/* zero values */

			/* fill distination with zero */
			do { *(tjs_uint32*)pixelbuf = 0; pixelbuf+=4; } while(--count);

			zero ^= 1;
		}
		else
		{
			/* non-zero values */

			/* fill distination with glomb code */

			do
			{
				int k = TVPTLG6GolombBitLengthTable[a][n], v, sign;

				tjs_uint32 t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
				tjs_int bit_count;
				tjs_int b;
				if(t)
				{
					b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
					bit_count = b;
					while(!b)
					{
						bit_count += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pool += bit_pos >> 3;
						bit_pos &= 7;
						t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
						b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
						bit_count += b;
					}
					bit_count --;
				}
				else
				{
					bit_pool += 5;
					bit_count = bit_pool[-1];
					bit_pos = 0;
					t = TVP_TLG6_FETCH_32BITS(bit_pool);
					b = 0;
				}


				v = (bit_count << k) + ((t >> b) & ((1<<k)-1));
				sign = (v & 1) - 1;
				v >>= 1;
				a += v;
				*(tjs_uint32*)pixelbuf = (unsigned char) ((v ^ sign) + sign + 1);
				pixelbuf += 4;

				bit_pos += b;
				bit_pos += k;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;

				if (--n < 0) {
					a >>= 1;  n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}
			} while(--count);
			zero ^= 1;
		}
	}
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeGolombValues, (tjs_int8 *pixelbuf, tjs_int pixel_count, tjs_uint8 *bit_pool))
{
	/*
		decode values packed in "bit_pool".
		values are coded using golomb code.
	*/

	int n = TVP_TLG6_GOLOMB_N_COUNT - 1; /* output counter */
	int a = 0; /* summary of absolute values of errors */

	tjs_int bit_pos = 1;
	tjs_uint8 zero = (*bit_pool & 1)?0:1;

	tjs_int8 * limit = pixelbuf + pixel_count*4;

	while(pixelbuf < limit)
	{
		/* get running count */
		int count;

		{
			tjs_uint32 t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
			tjs_int b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
			int bit_count = b;
			while(!b)
			{
				bit_count += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;
				t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
				b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
				bit_count += b;
			}


			bit_pos += b;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

			bit_count --;
			count = 1 << bit_count;
			count += ((TVP_TLG6_FETCH_32BITS(bit_pool) >> (bit_pos)) & (count-1));

			bit_pos += bit_count;
			bit_pool += bit_pos >> 3;
			bit_pos &= 7;

		}

		if(zero)
		{
			/* zero values */

			/* fill distination with zero */
			do { *pixelbuf = 0; pixelbuf+=4; } while(--count);

			zero ^= 1;
		}
		else
		{
			/* non-zero values */

			/* fill distination with glomb code */

			do
			{
				int k = TVPTLG6GolombBitLengthTable[a][n], v, sign;

				tjs_uint32 t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
				tjs_int bit_count;
				tjs_int b;
				if(t)
				{
					b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
					bit_count = b;
					while(!b)
					{
						bit_count += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pos += TVP_TLG6_LeadingZeroTable_BITS;
						bit_pool += bit_pos >> 3;
						bit_pos &= 7;
						t = TVP_TLG6_FETCH_32BITS(bit_pool) >> bit_pos;
						b = TVPTLG6LeadingZeroTable[t&(TVP_TLG6_LeadingZeroTable_SIZE-1)];
						bit_count += b;
					}
					bit_count --;
				}
				else
				{
					bit_pool += 5;
					bit_count = bit_pool[-1];
					bit_pos = 0;
					t = TVP_TLG6_FETCH_32BITS(bit_pool);
					b = 0;
				}


				v = (bit_count << k) + ((t >> b) & ((1<<k)-1));
				sign = (v & 1) - 1;
				v >>= 1;
				a += v;
				*pixelbuf = (char) ((v ^ sign) + sign + 1);
				pixelbuf += 4;

				bit_pos += b;
				bit_pos += k;
				bit_pool += bit_pos >> 3;
				bit_pos &= 7;

				if (--n < 0) {
					a >>= 1;  n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}
			} while(--count);
			zero ^= 1;
		}
	}
}


static TVP_INLINE_FUNC tjs_uint32 make_gt_mask(tjs_uint32 a, tjs_uint32 b){
	tjs_uint32 tmp2 = ~b;
	tjs_uint32 tmp = ((a & tmp2) + (((a ^ tmp2) >> 1) & 0x7f7f7f7f) ) & 0x80808080;
	tmp = ((tmp >> 7) + 0x7f7f7f7f) ^ 0x7f7f7f7f;
	return tmp;
}
static TVP_INLINE_FUNC tjs_uint32 packed_bytes_add(tjs_uint32 a, tjs_uint32 b)
{
	tjs_uint32 tmp = (((a & b)<<1) + ((a ^ b) & 0xfefefefe) ) & 0x01010100;
	return a+b-tmp;
}
static TVP_INLINE_FUNC tjs_uint32 med2(tjs_uint32 a, tjs_uint32 b, tjs_uint32 c){
	/* do Median Edge Detector   thx, Mr. sugi  at    kirikiri.info */
	tjs_uint32 aa_gt_bb = make_gt_mask(a, b);
	tjs_uint32 a_xor_b_and_aa_gt_bb = ((a ^ b) & aa_gt_bb);
	tjs_uint32 aa = a_xor_b_and_aa_gt_bb ^ a;
	tjs_uint32 bb = a_xor_b_and_aa_gt_bb ^ b;
	tjs_uint32 n = make_gt_mask(c, bb);
	tjs_uint32 nn = make_gt_mask(aa, c);
	tjs_uint32 m = ~(n | nn);
	return (n & aa) | (nn & bb) | ((bb & m) - (c & m) + (aa & m));
}
static TVP_INLINE_FUNC tjs_uint32 med(tjs_uint32 a, tjs_uint32 b, tjs_uint32 c, tjs_uint32 v){
	return packed_bytes_add(med2(a, b, c), v);
}

#define TLG6_AVG_PACKED(x, y) ((((x) & (y)) + ((((x) ^ (y)) & 0xfefefefe) >> 1)) +\
			(((x)^(y))&0x01010101))

static TVP_INLINE_FUNC tjs_uint32 avg(tjs_uint32 a, tjs_uint32 b, tjs_uint32 c, tjs_uint32 v){
	return packed_bytes_add(TLG6_AVG_PACKED(a, b), v);
}

#define TVP_TLG6_DO_CHROMA_DECODE_PROTO(B, G, R, A, POST_INCREMENT) do \
			{ \
				tjs_uint32 u = *prevline; \
				p = med(p, u, up, \
					(0xff0000 & ((R)<<16)) + (0xff00 & ((G)<<8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*curline = p; \
				curline ++; \
				prevline ++; \
				POST_INCREMENT \
			} while(--w);
#define TVP_TLG6_DO_CHROMA_DECODE_PROTO2(B, G, R, A, POST_INCREMENT) do \
			{ \
				tjs_uint32 u = *prevline; \
				p = avg(p, u, up, \
					(0xff0000 & ((R)<<16)) + (0xff00 & ((G)<<8)) + (0xff & (B)) + ((A) << 24) ); \
				up = u; \
				*curline = p; \
				curline ++; \
				prevline ++; \
				POST_INCREMENT \
			} while(--w);
#define TVP_TLG6_DO_CHROMA_DECODE(N, R, G, B) case (N<<1): \
	TVP_TLG6_DO_CHROMA_DECODE_PROTO(R, G, B, IA, {in+=step;}) break; \
	case (N<<1)+1: \
	TVP_TLG6_DO_CHROMA_DECODE_PROTO2(R, G, B, IA, {in+=step;}) break;

/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeLineGeneric, (tjs_uint32 *prevline, tjs_uint32 *curline, tjs_int width, tjs_int start_block, tjs_int block_limit, tjs_uint8 *filtertypes, tjs_int skipblockbytes, tjs_uint32 *in, tjs_uint32 initialp, tjs_int oddskip, tjs_int dir))
{
	/*
		chroma/luminosity decoding
		(this does reordering, color correlation filter, MED/AVG  at a time)
	*/
	tjs_uint32 p, up;
	int step, i;

	if(start_block)
	{
		prevline += start_block * TVP_TLG6_W_BLOCK_SIZE;
		curline  += start_block * TVP_TLG6_W_BLOCK_SIZE;
		p  = curline[-1];
		up = prevline[-1];
	}
	else
	{
		p = up = initialp;
	}

	in += skipblockbytes * start_block;
	step = (dir&1)?1:-1;

	for(i = start_block; i < block_limit; i ++)
	{
		int w = width - i*TVP_TLG6_W_BLOCK_SIZE, ww;
		if(w > TVP_TLG6_W_BLOCK_SIZE) w = TVP_TLG6_W_BLOCK_SIZE;
		ww = w;
		if(step==-1) in += ww-1;
		if(i&1) in += oddskip * ww;
		switch(filtertypes[i])
		{
#define IA	(char)((*in>>24)&0xff)
#define IR	(char)((*in>>16)&0xff)
#define IG  (char)((*in>>8 )&0xff)
#define IB  (char)((*in    )&0xff)
		TVP_TLG6_DO_CHROMA_DECODE( 0, IB, IG, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 1, IB+IG, IG, IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 2, IB, IG+IB, IR+IB+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 3, IB+IR+IG, IG+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 4, IB+IR, IG+IB+IR, IR+IB+IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 5, IB+IR, IG+IB+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 6, IB+IG, IG, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 7, IB, IG+IB, IR); 
		TVP_TLG6_DO_CHROMA_DECODE( 8, IB, IG, IR+IG); 
		TVP_TLG6_DO_CHROMA_DECODE( 9, IB+IG+IR+IB, IG+IR+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(10, IB+IR, IG+IR, IR); 
		TVP_TLG6_DO_CHROMA_DECODE(11, IB, IG+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(12, IB, IG+IR+IB, IR+IB); 
		TVP_TLG6_DO_CHROMA_DECODE(13, IB+IG, IG+IR+IB+IG, IR+IB+IG); 
		TVP_TLG6_DO_CHROMA_DECODE(14, IB+IG+IR, IG+IR, IR+IB+IG+IR); 
		TVP_TLG6_DO_CHROMA_DECODE(15, IB, IG+(IB<<1), IR+(IB<<1));

		default: return;
		}
		if(step == 1)
			in += skipblockbytes - ww;
		else
			in += skipblockbytes + 1;
		if(i&1) in -= oddskip * ww;
#undef IR
#undef IG
#undef IB
	}
}

/*export*/
TVP_GL_FUNC_DECL(void, TVPTLG6DecodeLine, (tjs_uint32 *prevline, tjs_uint32 *curline, tjs_int width, tjs_int block_count, tjs_uint8 *filtertypes, tjs_int skipblockbytes, tjs_uint32 *in, tjs_uint32 initialp, tjs_int oddskip, tjs_int dir))
{
	TVPTLG6DecodeLineGeneric(prevline, curline, width, 0, block_count,
		filtertypes, skipblockbytes, in, initialp, oddskip, dir);
}

/*end of the file*/