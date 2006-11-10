/*
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * The contents of this file were shamelessly borrowed from libjpeg v6b
 * with optimizations from jpeg-mmx (http://mjpeg.sourceforge.net/)
 * It contains only the routines necessary to implement Huffman encoding and
 * decoding for the Sun mediaLib JPEG codec for VirtualGL.
 */

#include <string.h>


static const int jpeg_natural_order[64+16] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63,
	63, 63, 63, 63, 63, 63, 63, 63, /* extra entries for safety in decoder */
	63, 63, 63, 63, 63, 63, 63, 63
};

static unsigned char lookup_bits_needed[256];
static int bits_needed_init = 0;

#define bits_needed(val) ((val&(~0xFF))? 8+lookup_bits_needed[val>>8]:lookup_bits_needed[val])


/*
 * Compute the derived values for a Huffman table.
 * This routine also performs some validation checks on the table.
 *
 * Note this is also used by jcphuff.c.
 */

static int jpeg_make_c_derived_tbl(const mlib_u8 *bits, const mlib_u8 *vals, int isDC, c_derived_tbl *dtbl)
{
	int p, i, l, lastp, si, maxsymbol;
	char huffsize[257];
	unsigned int huffcode[257];
	unsigned int code;

	if( !bits_needed_init )
	{
		int i;
		int temp;
		unsigned int nbits;
		for(i = 0; i < 256; ++i )
		{
			nbits = 1;
			temp = i;
			while ((temp >>= 1)) ++nbits;
			lookup_bits_needed[i] = (unsigned char)nbits;
		}
		bits_needed_init = 1;
	}

	/* Figure C.1: make table of Huffman code length for each symbol */
	p = 0;
	for (l = 1; l <= 16; l++)
	{
		i = (int) bits[l-1];
		if (i < 0 || p + i > 255)	/* protect against table overrun */
			_throw("Bad Huffman table");
		while (i--) huffsize[p++] = (char) l;
	}
	huffsize[p] = 0;
	lastp = p;

	/* Figure C.2: generate the codes themselves */
	/* We also validate that the counts represent a legal Huffman code tree. */
	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p])
	{
		while (((int) huffsize[p]) == si)
		{
			huffcode[p++] = code;
			code++;
		}
		/* code is now 1 more than the last code used for codelength si; but
		 * it must still fit in si bits, since no code is allowed to be all ones.
		 */
		if (((int) code) >= (((int) 1) << si))
			_throw("Bad Huffman table");
		code <<= 1;
		si++;
	}

	/* Figure C.3: generate encoding tables */
	/* These are code and size indexed by symbol value */

	/* Set all codeless symbols to have code length 0;
	 * this lets us detect duplicate VAL entries here, and later
	 * allows emit_bits to detect any attempt to emit such symbols.
	 */
	memset(dtbl->ehufsi, 0, sizeof(dtbl->ehufsi));

	/* This is also a convenient place to check for out-of-range
	 * and duplicated VAL entries.  We allow 0..255 for AC symbols
	 * but only 0..15 for DC.  (We could constrain them further
	 * based on data depth and mode, but this seems enough.)
	 */
	maxsymbol = isDC ? 15 : 255;

	for (p = 0; p < lastp; p++)
	{
		i = vals[p];
		if (i < 0 || i > maxsymbol || dtbl->ehufsi[i])
			_throw("Bad Huffman table");
		dtbl->ehufco[i] = huffcode[p];
		dtbl->ehufsi[i] = huffsize[p];
	}
	return 0;

	bailout:
	return -1;
}


/* Outputting bits to the file */

/* Only the right 24 bits of put_buffer are used; the valid bits are
 * left-justified in this part.  At most 16 bits can be passed to emit_bits
 * in one call, and we never retain more than 7 bits in put_buffer
 * between calls, so 24 bits are sufficient.
 */

#define emit_bits(code, size) {  \
	register int put_buffer = (int) code;  \
	put_bits += size;  /* new number of bits in buffer */  \
	put_buffer <<= 24 - put_bits;  /* align incoming bits */  \
	put_buffer |= huffbuf;  /* and merge with old buffer contents */  \
	while (put_bits >= 8)  {  \
		*jpgptr = put_buffer >> 16;  bytes++;  \
		if (*(jpgptr++) == 0xFF) {  /* need to stuff a zero byte? */  \
			*(jpgptr++)=0;  bytes++;  \
		}  \
		put_buffer <<= 8;  \
		put_bits -= 8;  \
	}  \
	huffbuf = put_buffer;  \
}

/* Encode a single block's worth of coefficients */

static int encode_one_block (jpgstruct *jpg, mlib_s16 *block,
	int *last_dc_val, c_derived_tbl *dctbl, c_derived_tbl *actbl)
{
	register int temp, temp2;
	register int nbits;
	register int k, r, i, index;
	register int huffbuf = jpg->huffbuf;
	register int put_bits = jpg->huffbits;
	register mlib_u8 *jpgptr=jpg->jpgptr;
	register long bytes=0;

	/* Encode the DC coefficient difference per section F.1.2.1 */
	temp = temp2 = block[0] - *last_dc_val;

	if (temp < 0)
	{
		temp = -temp;		/* temp is abs value of input */
		/* For a negative input, want temp2 = bitwise complement of abs(input) */
		/* This code assumes we are on a two's complement machine */
		temp2--;
	}

	/* Find the number of bits needed for the magnitude of the coefficient */
	nbits = bits_needed(temp);
	nbits = !temp ? 0 : nbits;

	/* Check for out-of-range coefficient values.
	 * Since we're encoding a difference, the range limit is twice as much.
	 */
	if (nbits > 10+1) _throw("Bad DCT coefficient");

	/* Emit the Huffman-coded symbol for the number of bits */
	emit_bits(dctbl->ehufco[nbits], dctbl->ehufsi[nbits]);

	/* Emit that number of bits of the value, if positive, */
	/* or the complement of its magnitude, if negative. */
	temp2 &= (((int) 1)<<nbits) - 1;  /* mask off any extra bits in code */
 	if (nbits)			/* emit_bits rejects calls with size 0 */
		emit_bits((unsigned int) temp2, nbits);

	/* Encode the AC coefficients per section F.1.2.2 */
	r = 0;			/* r = run length of zeros */
	k = 1;
	index = jpeg_natural_order[1];

	while( k < 64 )
	{
		temp = block[index];  ++k;
		if (temp == 0) ++r;
		else 
		{
			/* if run length > 15, must emit special run-length-16 codes (0xF0) */
			while (r > 15)
			{
				emit_bits(actbl->ehufco[0xF0], actbl->ehufsi[0xF0]);
				r -= 16;
			}
			temp2 = (temp < 0) ? temp-1 : temp;
			temp  = (temp < 0) ? -temp : temp;

			/* Find the number of bits needed for the magnitude of the coefficient */
			nbits = bits_needed(temp);

			/* Emit Huffman symbol for run length / number of bits */
			i = (r << 4) + nbits;
			emit_bits(actbl->ehufco[i], actbl->ehufsi[i]);

			/* Emit that number of bits of the value, if positive, */
			/* or the complement of its magnitude, if negative. */
			temp2 &= (((int) 1)<<nbits) - 1;  /* mask off any extra bits in code */
			emit_bits((unsigned int) temp2, nbits);

			r = 0;
		}
		index = jpeg_natural_order[k];
	}

	/* If the last coef(s) were zero, emit an end-of-block code */
	if (r > 0)
		emit_bits(actbl->ehufco[0], actbl->ehufsi[0]);

	jpg->huffbuf = huffbuf; /* update state variables */
	jpg->huffbits = put_bits;
	jpg->jpgptr = jpgptr;
	jpg->bytesprocessed += bytes;  jpg->bytesleft -= bytes;
	*last_dc_val=block[0];
	return 0;

	bailout:
	return -1;
}


static int flush_bits (jpgstruct *jpg)
{
	register int huffbuf = jpg->huffbuf;
	register int put_bits = jpg->huffbits;
	register mlib_u8 *jpgptr = jpg->jpgptr;
	register long bytes = 0;
	emit_bits(0x7F, 7); /* fill any partial byte with ones */
	jpg->huffbuf = 0;	/* and reset bit-buffer to empty */
	jpg->huffbits = 0;
	jpg->jpgptr = jpgptr;
	jpg->bytesprocessed += bytes;  jpg->bytesleft -= bytes;
	return 0;
}


/*
 * Compute the derived values for a Huffman table.
 * This routine also performs some validation checks on the table.
 *
 */
static int jpeg_make_d_derived_tbl(mlib_u8 *bits, mlib_u8 *vals, int isDC, d_derived_tbl *dtbl)
{
	int p, i, l, si, numsymbols;
	int lookbits, ctr;
	char huffsize[257];
	unsigned int huffcode[257];
	unsigned int code;

	/* Figure C.1: make table of Huffman code length for each symbol */
	p = 0;
	for (l = 1; l <= 16; l++)
	{
		i = (int) bits[l-1];
		if (i < 0 || p + i > 255)	/* protect against table overrun */
			_throw("Bad Huffman table");
		while (i--) huffsize[p++] = (char) l;
	}
	huffsize[p] = 0;
	numsymbols = p;

	/* Figure C.2: generate the codes themselves */
	/* We also validate that the counts represent a legal Huffman code tree. */

	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p])
	{
		while (((int) huffsize[p]) == si)
		{
			huffcode[p++] = code;
			code++;
		}
		/* code is now 1 more than the last code used for codelength si; but
		 * it must still fit in si bits, since no code is allowed to be all ones.
		 */
		if (((int) code) >= (((int) 1) << si)) _throw("Bad Huffman table");
		code <<= 1;
		si++;
	}

	/* Figure F.15: generate decoding tables for bit-sequential decoding */
	p = 0;
	for (l = 1; l <= 16; l++)
	{
		if(bits[l-1])
		{
			/* valoffset[l] = huffval[] index of 1st symbol of code length l,
			 * minus the minimum code of length l
			 */
			dtbl->valoffset[l] = (int) p - (int) huffcode[p];
			p += bits[l-1];
			dtbl->maxcode[l] = huffcode[p-1]; /* maximum code of length l */
		}
		else
		{
			dtbl->maxcode[l] = -1;	/* -1 if no codes of this length */
		}
  }
	dtbl->maxcode[17] = 0xFFFFFL; /* ensures jpeg_huff_decode terminates */

	/* Compute lookahead tables to speed up decoding.
	 * First we set all the table entries to 0, indicating "too long";
	 * then we iterate through the Huffman codes that are short enough and
	 * fill in all the entries that correspond to bit sequences starting
	 * with that code.
	 */
	memset(dtbl->look_nbits, 0, sizeof(dtbl->look_nbits));

	p = 0;
	for (l = 1; l <= HUFF_LOOKAHEAD; l++)
	{
		for (i = 1; i <= (int) bits[l-1]; i++, p++)
		{
			/* l = current code's length, p = its index in huffcode[] & huffval[]. */
			/* Generate left-justified code followed by all possible bit sequences */
			lookbits = huffcode[p] << (HUFF_LOOKAHEAD-l);
			for (ctr = 1 << (HUFF_LOOKAHEAD-l); ctr > 0; ctr--)
			{
				dtbl->look_nbits[lookbits] = l;
				dtbl->look_sym[lookbits] = vals[p];
				lookbits++;
			}
		}
	}

	/* Validate symbols as being reasonable.
	 * For AC tables, we make no check, but accept all byte values 0..255.
	 * For DC tables, we require the symbols to be in range 0..15.
	 * (Tighter bounds could be applied depending on the data depth and mode,
	 * but this is sufficient to ensure safe decoding.)
	 */
	if (isDC)
	{
		for (i = 0; i < numsymbols; i++)
		{
			int sym = vals[i];
			if (sym < 0 || sym > 15) _throw("Bad Huffman table");
		}
	}

	memset(dtbl->huffval, 0, 256);
	memcpy(dtbl->huffval, vals, numsymbols);

	return 0;

	bailout:
	return -1;
}


typedef struct
{
	/* Bitreading working state within an MCU */
	/* Current data source location */
	/* We need a copy, rather than munging the original, in case of suspension */
	mlib_u8 * next_input_byte; /* => next byte to read from source */
	unsigned long bytes_in_buffer;	/* # of bytes remaining in source buffer */
	/* Bit input buffer --- note these values are kept in register variables,
	 * not in this struct, inside the inner loops.
	 */
	int get_buffer;	/* current bit-extraction buffer */
	int bits_left;		/* # of unused bits in it */
	/* Pointer needed by jpeg_fill_bit_buffer. */
	jpgstruct *jpg;	/* back link to decompress master record */
} bitread_working_state;


/*
 * Out-of-line code for bit fetching (shared with jdphuff.c).
 * See jdhuff.h for info about usage.
 * Note: current values of get_buffer and bits_left are passed as parameters,
 * but are returned in the corresponding fields of the state struct.
 *
 * On most machines MIN_GET_BITS should be 25 to allow the full 32-bit width
 * of get_buffer to be used.  (On machines with wider words, an even larger
 * buffer could be used.)  However, on some machines 32-bit shifts are
 * quite slow and take time proportional to the number of places shifted.
 * (This is true with most PC compilers, for instance.)  In this case it may
 * be a win to set MIN_GET_BITS to the minimum value of 15.  This reduces the
 * average shift distance at the cost of more calls to jpeg_fill_bit_buffer.
 */

#define BIT_BUF_SIZE  32
#define MIN_GET_BITS  (BIT_BUF_SIZE-7)

/* Load up the bit buffer to a depth of at least nbits */
static int jpeg_fill_bit_buffer (bitread_working_state * state, register int get_buffer,
	register int bits_left, int nbits)
{
	register mlib_u8 * next_input_byte = state->next_input_byte;
	register unsigned long bytes_in_buffer = state->bytes_in_buffer;
	jpgstruct *jpg = state->jpg;

	/* Attempt to load at least MIN_GET_BITS bits into get_buffer. */
	/* (It is assumed that no request will be for more than that many bits.) */
	/* We fail to do so only if we hit a marker or are forced to suspend. */
	if (jpg->unread_marker == 0)
	{	/* cannot advance past a marker */
		while (bits_left < MIN_GET_BITS)
		{
			register int c;

			/* Attempt to read a byte */
			if (bytes_in_buffer == 0) _throw("Unexpected end of image");
			bytes_in_buffer--;
			c = (int)(*next_input_byte++);

			/* If it's 0xFF, check and discard stuffed zero byte */
			if (c == 0xFF)
			{
				/* Loop here to discard any padding FF's on terminating marker,
				 * so that we can save a valid unread_marker value.  NOTE: we will
				 * accept multiple FF's followed by a 0 as meaning a single FF data
				 * byte.  This data pattern is not valid according to the standard.
				 */
				do
				{
					if (bytes_in_buffer == 0) _throw("Unexpected end of image");
					bytes_in_buffer--;
					c = (int)(*next_input_byte++);
				} while (c == 0xFF);

				if (c == 0)
				{
					/* Found FF/00, which represents an FF data byte */
					c = 0xFF;
				}
				else
				{
					/* Oops, it's actually a marker indicating end of compressed data.
					 * Save the marker code for later use.
					 * Fine point: it might appear that we should save the marker into
					 * bitread working state, not straight into permanent state.  But
					 * once we have hit a marker, we cannot need to suspend within the
					 * current MCU, because we will read no more bytes from the data
					 * source.  So it is OK to update permanent state right away.
					 */
					jpg->unread_marker = c;
					/* See if we need to insert some fake zero bits. */
					goto no_more_bytes;
				}
			}

			/* OK, load c into get_buffer */
			get_buffer = (get_buffer << 8) | c;
			bits_left += 8;
		} /* end while */
	}
	else
	{
		no_more_bytes:
		/* We get here if we've read the marker that terminates the compressed
		 * data segment.  There should be enough bits in the buffer register
		 * to satisfy the request; if so, no problem.
		 */
		if (nbits > bits_left)
		{
      /* Uh-oh.  Report corrupted data to user and stuff zeroes into
       * the data stream, so that we can produce some kind of image.
       * We use a nonvolatile flag to ensure that only one warning message
       * appears per data segment.
       */
			if(!jpg->insufficient_data)
			{
				jpg->insufficient_data = 1;
			}
			/* Fill the buffer with zero bits */
			get_buffer <<= MIN_GET_BITS - bits_left;
			bits_left = MIN_GET_BITS;
    }
  }

	/* Unload the local registers */
	state->next_input_byte = next_input_byte;
	state->bytes_in_buffer = bytes_in_buffer;
	state->get_buffer = get_buffer;
	state->bits_left = bits_left;

  return 0;

	bailout:
	return -1;
}


/*
 * These macros provide the in-line portion of bit fetching.
 * Use CHECK_BIT_BUFFER to ensure there are N bits in get_buffer
 * before using GET_BITS, PEEK_BITS, or DROP_BITS.
 * The variables get_buffer and bits_left are assumed to be locals,
 * but the state struct might not be (jpeg_huff_decode needs this).
 *	CHECK_BIT_BUFFER(state,n,action);
 *		Ensure there are N bits in get_buffer; if suspend, take action.
 *      val = GET_BITS(n);
 *		Fetch next N bits.
 *      val = PEEK_BITS(n);
 *		Fetch next N bits without removing them from the buffer.
 *	DROP_BITS(n);
 *		Discard next N bits.
 * The value N should be a simple variable, not an expression, because it
 * is evaluated multiple times.
 */

#define CHECK_BIT_BUFFER(state, nbits) { \
	if (bits_left < (nbits)) {  \
		_catch( jpeg_fill_bit_buffer(&(state), get_buffer, bits_left, nbits) ); \
		get_buffer = (state).get_buffer;  bits_left = (state).bits_left; \
	}}

#define GET_BITS(nbits) \
	(((int) (get_buffer >> (bits_left -= (nbits)))) & ((1<<(nbits))-1))

#define PEEK_BITS(nbits) \
	(((int) (get_buffer >> (bits_left -  (nbits)))) & ((1<<(nbits))-1))

#define DROP_BITS(nbits) \
	(bits_left -= (nbits))


/*
 * Out-of-line code for Huffman code decoding.
 * See jdhuff.h for info about usage.
 */

static int jpeg_huff_decode (bitread_working_state * state, register int get_buffer,
	register int bits_left, d_derived_tbl *htbl, int min_bits)
{
	register int l = min_bits;
	register int code;

	/* HUFF_DECODE has determined that the code is at least min_bits */
	/* bits long, so fetch that many bits in one swoop. */
	CHECK_BIT_BUFFER(*state, l);
	code = GET_BITS(l);

	/* Collect the rest of the Huffman code one bit at a time. */
	/* This is per Figure F.16 in the JPEG spec. */
	while (code > htbl->maxcode[l])
	{
		code <<= 1;
		CHECK_BIT_BUFFER(*state, 1);
		code |= GET_BITS(1);
		l++;
	}

	/* Unload the local registers */
	state->get_buffer = get_buffer;
	state->bits_left = bits_left;

	/* With garbage input we may reach the sentinel value l = 17. */
	if (l > 16)
	{
//	WARNMS(state->cinfo, JWRN_HUFF_BAD_CODE);
		return 0;			/* fake a zero as the safest result */
	}

	return htbl->huffval[ (int) (code + htbl->valoffset[l]) ];

	bailout:
	return -1;
}


/*
 * Code for extracting next Huffman-coded symbol from input bit stream.
 * Again, this is time-critical and we make the main paths be macros.
 *
 * We use a lookahead table to process codes of up to HUFF_LOOKAHEAD bits
 * without looping.  Usually, more than 95% of the Huffman codes will be 8
 * or fewer bits long.  The few overlength codes are handled with a loop,
 * which need not be inline code.
 *
 * Notes about the HUFF_DECODE macro:
 * 1. Near the end of the data segment, we may fail to get enough bits
 *    for a lookahead.  In that case, we do it the hard way.
 * 2. If the lookahead table contains no entry, the next code must be
 *    more than HUFF_LOOKAHEAD bits long.
 * 3. jpeg_huff_decode returns -1 if forced to suspend.
 */

#define HUFF_DECODE(result, state, htbl, slowlabel) { \
	register int nb, look; \
	if (bits_left < HUFF_LOOKAHEAD) { \
		_catch( jpeg_fill_bit_buffer(&state, get_buffer, bits_left, 0) ); \
		get_buffer = state.get_buffer;  bits_left = state.bits_left; \
		if (bits_left < HUFF_LOOKAHEAD) { \
			nb = 1;  goto slowlabel; \
		} \
	} \
	look = PEEK_BITS(HUFF_LOOKAHEAD); \
	if ((nb = htbl->look_nbits[look]) != 0) { \
		DROP_BITS(nb); \
		result = htbl->look_sym[look]; \
	} \
	else { \
		nb = HUFF_LOOKAHEAD+1; \
	slowlabel: \
		_catch( result=jpeg_huff_decode(&state, get_buffer, bits_left, htbl, nb) ); \
	get_buffer = state.get_buffer;  bits_left = state.bits_left; \
	} \
}


/*
 * Figure F.12: extend sign bit.
 * On some machines, a shift and add will be faster than a table lookup.
 */

#ifdef AVOID_TABLES

#define HUFF_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))

#else

#define HUFF_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

static const int extend_test[16] =   /* entry n is 2**(n-1) */
{
	0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
	0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000
};

static const int extend_offset[16] = /* entry n is (-1 << n) + 1 */
{
	0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
	((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
	((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
	((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1
};

#endif /* AVOID_TABLES */


/* Decode a single block's worth of coefficients */

static int decode_one_block (jpgstruct *jpg, mlib_s16 *block,
	int *last_dc_val, d_derived_tbl *dctbl, d_derived_tbl *actbl)
{
	register int get_buffer, bits_left, s, k, r;
	bitread_working_state br_state;  int lastdc=*last_dc_val;

	/* If we've run out of data, just leave the MCU set to zeroes.
	 * This way, we return uniform gray for the remainder of the segment.
	 */
	if(jpg->insufficient_data) return 0;

	/* Load up working state */
	br_state.jpg = jpg;
	br_state.next_input_byte = jpg->jpgptr;
	br_state.bytes_in_buffer = jpg->bytesleft;
	get_buffer = jpg->huffbuf;
	bits_left = jpg->huffbits;

	memset(block, 0, 64*sizeof(mlib_s16));

	/* Section F.2.2.1: decode the DC coefficient difference */
	HUFF_DECODE(s, br_state, dctbl, label1);
	if (s)
	{
		CHECK_BIT_BUFFER(br_state, s);
		r = GET_BITS(s);
		s = HUFF_EXTEND(r, s);
	}

	/* Convert DC difference to actual value, update last_dc_val */
	s += lastdc;
	lastdc = s;
	/* Output the DC coefficient (assumes jpeg_natural_order[0] = 0) */
	block[0] = (mlib_s16) s;

	/* Section F.2.2.2: decode the AC coefficients */
	/* Since zeroes are skipped, output area must be cleared beforehand */
	for (k = 1; k < 64; k++)
	{
		HUFF_DECODE(s, br_state, actbl, label2);
		r = s >> 4;
		s &= 15;
		if (s)
		{
			k += r;
			CHECK_BIT_BUFFER(br_state, s);
			r = GET_BITS(s);
			s = HUFF_EXTEND(r, s);
			/* Output coefficient in natural (dezigzagged) order.
			 * Note: the extra entries in jpeg_natural_order[] will save us
			 * if k >= DCTSIZE2, which could happen if the data is corrupted.
			 */
			block[jpeg_natural_order[k]] = (mlib_s16) s;
	  }
		else
		{
			if (r != 15) break;
			k += 15;
		}
	}

	/* Completed MCU, so update state */
	jpg->jpgptr = br_state.next_input_byte;
	jpg->bytesleft = br_state.bytes_in_buffer;
	jpg->huffbuf = get_buffer;
	jpg->huffbits = bits_left;

	*last_dc_val = lastdc;

	return 0;

	bailout:
	return -1;
}
