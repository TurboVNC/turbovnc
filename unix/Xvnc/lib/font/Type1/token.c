/* $Xorg: token.c,v 1.4 2000/08/17 19:46:34 cpqbld Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/token.c,v 1.6 2003/05/27 22:26:47 tsi Exp $ */
/* Authors: Sig Nin & Carol Thompson IBM Almaden Research Laboratory */
#include "t1stdio.h"
#include "util.h"
#include "digit.h"
#include "token.h"
#include "tokst.h"
#include "hdigit.h"
 
/*
 * -------------------------------------------------------------------
 * Globals
 * -------------------------------------------------------------------
 */
 
/* These variables are set by the caller */
char           *tokenStartP;   /* Pointer to token buffer in VM */
char           *tokenMaxP;     /* Pointer to last byte in buffer + 1 */
 
/* These variables are set by TOKEN */
int             tokenLength;   /* Characters in token */
boolean         tokenTooLong;  /* Token too long for buffer */
int             tokenType;     /* Type of token identified */
psvalue         tokenValue;    /* Token value */
 
/*
 * -------------------------------------------------------------------
 * Private variables
 * -------------------------------------------------------------------
 */
 
static FILE    *inputFileP;    /* Current input file */
 
 
/* Token */
static char    *tokenCharP;    /* Pointer to next character in token */
 
/*
 * -------------------------------------------------------------------
 * Private routines for manipulating numbers
 * -------------------------------------------------------------------
 */
 
#define Exp10(e) \
((e) == 0\
 ? (double)(1.0)\
 : (-64 <= (e) && (e) <= 63\
    ? Exp10T[(e)+64]\
    : P10(e)\
   )\
)
 
static double Exp10T[128] = {
  1e-64, 1e-63, 1e-62, 1e-61, 1e-60, 1e-59, 1e-58, 1e-57,
  1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49,
  1e-48, 1e-47, 1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41,
  1e-40, 1e-39, 1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
  1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25,
  1e-24, 1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17,
  1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9,
  1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1,
  1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
  1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
  1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23,
  1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31,
  1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38, 1e39,
  1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47,
  1e48, 1e49, 1e50, 1e51, 1e52, 1e53, 1e54, 1e55,
  1e56, 1e57, 1e58, 1e59, 1e60, 1e61, 1e62, 1e63
};
 
static double 
P10(long exponent)
{
  double value, power;
 
  if (exponent < 0) {
    power = 0.1;
    value = (exponent & 1 ? power : 1.0);
    exponent++;
    exponent = -(exponent >> 1); /* portable C for -(exponent/2) */
  }
  else {
    power = 10.0;
    value = (exponent & 1 ? power : 1.0);
    exponent = exponent >> 1;
  }
 
  while(exponent > 0) {
    power *= power;
    if (exponent & 1)
      value *= power;
    exponent >>= 1;
  }
 
  return(value);
}
 
/*
 * -------------------------------------------------------------------
 * Private routines and macros for manipulating the input
 * -------------------------------------------------------------------
 */
 
/* Get next character from the input --
 *
 */
#define next_ch()    (_XT1getc(inputFileP))
 
/* Push a character back into the input --
 *
 * Ungetc of EOF will fail, but that's ok: the next getc will
 * return EOF.
 *
 * NOTE:  These macros are presently written to return the character
 * pushed, or EOF if none was pushed.  However, they are not
 * required to return anything in particular, and callers should
 * not rely on the returned value.
 */
#define back_ch(ch)   (T1Ungetc(ch, inputFileP))
 
/* Push a character back into the input if it was not white space.
 * If it is a carriage return (\r) then check next char for
 * linefeed and consume them both, otherwise put next char back.
 *
 */
#define back_ch_not_white(ch) \
(\
isWHITE_SPACE(ch)\
 ? ((ch == '\r')\
   ? (((ch = next_ch()) == '\n')\
     ? EOF\
     : back_ch(ch)\
     )\
   : EOF\
   )\
 : back_ch(ch)\
)
 
/*
 * -------------------------------------------------------------------
 * Private routines and macros for manipulating the token buffer
 * -------------------------------------------------------------------
 */
 
/* Add a character to the token
 * ---- use ONLY when you KNOW that this character will
 *      be stored within the token buffer.
 */
#define save_unsafe_ch(ch) (*tokenCharP++ = ch)
 
/* Add a character to the token, if not too long to fit */
#define save_ch(ch) \
((tokenCharP < tokenMaxP)\
 ? save_unsafe_ch(ch)\
 : (tokenTooLong = TRUE)\
)

#define save_ch_no_inc(ch) \
if (tokenCharP < tokenMaxP) *tokenCharP = ch
 
/*
 * -------------------------------------------------------------------
 * Action Routines
 *
 *  These routines all
 *    -- take int ch as a parameter
 *    -- return int ch if no token was recognized, DONE otherwise
 *    -- leave the next character in the input, if returning DONE
 * -------------------------------------------------------------------
 */
 
#define DONE  (256)
 
/* Get the next input character */
static int 
next_char(int ch)
{
  return(next_ch());
}
 
/* Add character to token */
static int 
add_char(int ch)
{
  save_ch(ch);
  return(next_ch());
}
 
 
/* -------------------------------------------------------------------
 * Skip white space and comments
 */
 
/* Skip white space */
static int 
skip_space(int ch)
{
  do {
    ch = next_ch();
  } while(isWHITE_SPACE(ch));
  return(ch);
}
 
/* Skip comments */
static int 
skip_comment(int ch)
{
  do {
    ch = next_ch();
  } while(isCOMMENT(ch));
  return(ch);
}
 
/* -------------------------------------------------------------------
 * Collect value elements for a number
 */
 
/* decimal integer or real number mantissa */
static int m_sign;
static long m_value;
static long m_scale;
 
/* real number exponent */
static int e_sign;
static long e_value;
 
/* radix number */
static long r_base;
static long r_value;
static long r_scale;
 
static int 
add_sign(int ch)
{
  m_sign = ch;
  save_unsafe_ch(ch);
  return(next_ch());
}
 
static int 
add_1st_digits(int ch)
{
  m_sign = '+';
  return(add_digits(ch));
}
 
static int 
add_digits(int ch)
{
  long value, p_value, scale;
  int digit;
 
  /* On entry, expect m_sign to be set to '+' or '-';
   *  ch is a decimal digit.
   * Expect at most one character saved at this point,
   *  a sign.  This routine will save up to 10 more
   *  characters without checking the buffer boundary.
   */
 
  value = ch - '0';
  save_unsafe_ch(ch);
  ch = next_ch();
 
  while(isDECIMAL_DIGIT(ch) && value < (MAX_INTEGER/10)) {
    value = (value << 3) + (value << 1) + (ch - '0');
    save_unsafe_ch(ch);
    ch = next_ch();
  }
 
  /* Quick exit for small integers --
   *    |x| <= 10*((MAX_INTEGER/10)-1)+9
   *    |x| <= 2,147,483,639 for 32 bit integers
   */
  if (isNUMBER_ENDER(ch)) {
    back_ch_not_white(ch);
    tokenValue.integer = (m_sign == '-' ? -value : value);
    tokenType = TOKEN_INTEGER;
    return(DONE);
  }
 
  /* Handle additional digits.  Beyond the boundary case,
   *   10*(MAX_INTEGER/10) <= |number| <= MAX_INTEGER
   * just count the digits: the number is too large to
   * represent as an integer and will be returned as a real.
   * The mantissa of a real holds fewer bits than an integer.
   */
  p_value = value;
  value = (m_sign == '-' ? -value : value);
  scale = 0;
 
  if (isDECIMAL_DIGIT(ch)) {
 
    /* Handle the boundary case */
    if (p_value == (MAX_INTEGER/10)) {
      digit = ch - '0';
 
      /* Must handle positive and negative values separately  */
      /* for 2's complement arithmetic */
      if (value > 0) {
        if (digit <= MAX_INTEGER%10)
          value = (value << 3) + (value << 1) + digit;
        else
          ++scale;  /* Too big, just count it */
      }
      else {
        /* Use positive % operands for portability */
        if (digit <= -(MIN_INTEGER+10)%10)
          value = (value << 3) + (value << 1) - digit;
        else
          ++scale;  /* Too big, just count it */
      }
    }
    else
      ++scale;  /* Not boundary case, just count digit */
 
    save_unsafe_ch(ch);
    ch = next_ch();
 
    /* Continue scanning digits, but can't store them */
    while(isDECIMAL_DIGIT(ch)) {
      ++scale;
      save_ch(ch);
      ch = next_ch();
    }
  }
 
  /* Continue from here scanning radix integer or real */
  m_value = value;
  m_scale = scale;
 
  /* Initialize for possible real */
  e_sign = '+';
  e_value = 0;
 
  return(ch);
}
 
static int 
add_1st_decpt(int ch)
{
  m_sign = '+';
  return(add_decpt(ch));
}
 
static int 
add_decpt(int ch)
{
  /* On entry, expect m_sign to be set to '+' or '-' */
  m_value = 0;
  m_scale = 0;
  save_unsafe_ch(ch);
  return(next_ch());
}
 
static int 
add_fraction(int ch)
{
  long value, scale;
  int digit;
 
  /* On entry, expect m_value and m_scale to be initialized,
   * and m_sign to be set to '+' or '-'.  Expect m_value and m_sign
   * to be consistent (this is not checked).
   */
  value = m_value;
  scale = m_scale;
 
  /* Scan leading zeroes */
  if (value == 0) {
    while(ch == '0') {
      --scale;
      save_ch(ch);
      ch = next_ch();
    }
 
    /* Scan first significant digit */
    if (isDECIMAL_DIGIT(ch)) {
      --scale;
      value = ch - '0';
      value = (m_sign == '-' ? -value : value);
      save_ch(ch);
      ch = next_ch();
    }
    else
      /* no significant digits -- number is zero */
      scale = 0;
  }
  /* value != 0 || value == 0 && !isDECIMAL_DIGIT(ch) */
 
  /* Scan additional significant digits */
  if (isDECIMAL_DIGIT(ch)) {
    if (value > 0) {
      while(isDECIMAL_DIGIT(ch) && value < (MAX_INTEGER/10)) {
        --scale;
        value = (value << 3) + (value << 1) + (ch - '0');
        save_ch(ch);
        ch = next_ch();
      }
      /* Check boundary case */
      if (isDECIMAL_DIGIT(ch) && value == (MAX_INTEGER/10)) {
        digit = ch - '0';
        if (digit <= MAX_INTEGER%10) {
          --scale;
          value = (value << 3) + (value << 1) + digit;
          save_ch(ch);
          ch = next_ch();
        }
      }
    }
    else {
      /* value < 0 */
      while(isDECIMAL_DIGIT(ch) && value > -(-(MIN_INTEGER+10)/10+1)) {
        /* Use positive / operands for portability */
        --scale;
        value = (value << 3) + (value << 1) - (ch - '0');
        save_ch(ch);
        ch = next_ch();
      }
      /* Check boundary case */
      if (isDECIMAL_DIGIT(ch)
          && value == -(-(MIN_INTEGER+10)/10+1)) {
        digit = ch - '0';
        if (digit <= -(MIN_INTEGER+10)%10) {
        /* Use positive % operands for portability */
          --scale;
          value = (value << 3) + (value << 1) - digit;
          save_ch(ch);
          ch = next_ch();
        }
      }
    }
 
    /* Additional digits can be discarded */
    while(isDECIMAL_DIGIT(ch)) {
      save_ch(ch);
      ch = next_ch();
    }
  }
 
  /* Store results */
  m_value = value;
  m_scale = scale;
 
  /* Initialize for possible real */
  e_sign = '+';
  e_value = 0;
 
  return(ch);
}
 
static int 
add_e_sign(int ch)
{
  e_sign = ch;
  save_ch(ch);
  return(next_ch());
}
 
static int 
add_exponent(int ch)
{
  long value, p_value;
  long scale = 0;
  int digit;
 
  /* On entry, expect e_sign to be set to '+' or '-' */
 
  value = ch - '0';
  save_ch(ch);
  ch = next_ch();
 
  while(isDECIMAL_DIGIT(ch) && value < (MAX_INTEGER/10)) {
    value = (value << 3) + (value << 1) + (ch - '0');
    save_ch(ch);
    ch = next_ch();
  }
 
  p_value = value;
  value = (e_sign == '-' ? -value : value);
 
  /* Handle additional digits.  Beyond the boundary case,
   *   10*(MAX_INTEGER/10) <= |number| <= MAX_INTEGER
   * just count the digits: the number is too large to
   * represent as an integer.
   */
  if (isDECIMAL_DIGIT(ch)) {
 
    /* Examine boundary case */
    if (p_value == (MAX_INTEGER/10)) {
      digit = ch - '0';
 
      /* Must handle positive and negative values separately */
      /*  for 2's complement arithmetic */
      if (value > 0) {
        if (digit <= MAX_INTEGER%10)
          value = (value << 3) + (value << 1) + digit;
        else
          ++scale; /* Too big, just count it */
      }
      else {
        /* Use positive % operands for portability */
        if (digit <= -(MIN_INTEGER+10)%10)
          value = (value << 3) + (value << 1) - digit;
        else
          ++scale; /* Too big, just count it */
      }
    }
    else
      ++scale;  /* Not boundary case, just count digit */
 
    save_ch(ch);
    ch = next_ch();
 
    /* Continue scanning digits, but can't store any more */
    while(isDECIMAL_DIGIT(ch)) {
      ++scale;
      save_ch(ch);
      ch = next_ch();
    }
  }
 
  /* Store results */
  e_value = value;
 
  return(ch);
}
 
static int 
add_radix(int ch)
{
  if (2 <= m_value && m_value <= 36 && m_scale == 0) {
    r_base = m_value;
    save_ch(ch);
    return(next_ch());
  }
  else {
    /* Radix invalid, complete a name token */
    return(AAH_NAME(ch));
  }
}
 
static int 
add_r_digits(int ch)
{
  unsigned long value;
  long radix, scale;
  int digit;
 
  /* NOTE:  The syntax of a radix number allows only for
   * values of zero or more.  The value will be stored as
   * a 32 bit integer, which PostScript then interprets
   * as signed.  This means, for example, that the numbers:
   *
   *     8#37777777777
   *    10#4294967295
   *    16#FFFFFFFF
   *    36#1Z141Z3
   *
   * are all interpreted as -1.  This routine implements this
   * idea explicitly:  it accumulates the number's value
   * as unsigned, then casts it to signed when done.
   */
 
  /* Expect r_base to be initialized */
  radix = r_base;
  value = 0;
  scale = 0;
 
  /* Scan leading zeroes */
  while(ch == '0') {
    save_ch(ch);
    ch = next_ch();
  }
 
  /* Handle first non-zero digit */
  if ((digit=digit_value[ch]) < radix) {
    value = digit;
    save_ch(ch);
    ch = next_ch();
 
    /* Add digits until boundary case reached */
    while((digit=digit_value[ch]) < radix
            && value < (MAX_ULONG / radix)) {
      value = value * radix + digit;
      save_ch(ch);
      ch = next_ch();
    };
 
    /* Scan remaining digits */
    if ((digit=digit_value[ch]) < radix) {
 
      /* Examine boundary case ---
       *   radix*(MAX_ULONG/radix) <= number <= MAX_ULONG
       */
      if (value == (MAX_ULONG/radix) && digit <= MAX_ULONG%radix)
        value = value * radix + digit;
      else
        ++scale;
 
      /* Continue scanning digits, but can't store them */
      save_ch(ch);
      ch = next_ch();
      while(digit_value[ch] < radix) {
        ++scale;
        save_ch(ch);
        ch = next_ch();
      }
    }
  }
 
  /* Store result */
  r_value = (long) value; /* result is signed */
  r_scale = scale;
 
  return(ch);
}
 
/* -------------------------------------------------------------------
 * Complete a number; set token type and done flag.
 * Put current input character back, if it is not white space.
 */
 
/* Done: Radix Number */
static int 
RADIX_NUMBER(int ch)
{
  back_ch_not_white(ch);
  if (r_scale == 0) {
    tokenValue.integer = r_value;
    tokenType = TOKEN_INTEGER;
  }
  else {
    tokenType = TOKEN_NAME;
  }
  return(DONE);
}
 
/* Done: Integer */
static int 
INTEGER(int ch)
{
  back_ch_not_white(ch);
  if (m_scale == 0) {
    tokenValue.integer = m_value;
    tokenType = TOKEN_INTEGER;
  }
  else {
    tokenValue.real = (double)(m_value) * Exp10(m_scale);
    tokenType = TOKEN_REAL;
  }
  return(DONE);
}
 
/* Done: Real */
static int 
REAL(int ch)
{
  double temp;
 
  back_ch_not_white(ch);
 
  /* HAZARD: exponent overflow of intermediate result
   * (e.g., in 370 floating point); this should not be a problem
   * with IEEE floating point.  Reduce exponent overflow hazard by
   * combining m_scale and e_value first, if they have different signs,
   * or multiplying m_value and one of the other factors, if both
   * m_scale and e_value are negative.
   */
  if ((m_scale >= 0 && e_value <= 0)
      || (m_scale <= 0 && e_value >= 0)) {
    tokenValue.real = (double)(m_value) * Exp10(m_scale + e_value);
  }
  else {
    temp = (double)(m_value) * Exp10(m_scale);
    tokenValue.real = temp * Exp10(e_value);
  }
 
  tokenType = TOKEN_REAL;
  return(DONE);
}
 
 
/* -------------------------------------------------------------------
 * Assemble a hex string; set token type and done flag.
 */
 
/* Done: Hex String */
static int 
HEX_STRING(int ch)
{
  int value;
 
  while(TRUE) {
 
    /* Process odd digit */
    ch = next_ch();
    if (!isHEX_DIGIT(ch)) {
 
      /* Skip white space */
      while(isWHITE_SPACE(ch))
        ch = next_ch();
 
      /* Check for terminator */
      if (!isHEX_DIGIT(ch)) {
        break;
      }
    }
    value = digit_value[ch] << 4;
 
    /* Process even digit */
    ch = next_ch();
    if (!isHEX_DIGIT(ch)) {
 
      /* Skip white space */
      while(isWHITE_SPACE(ch))
        ch = next_ch();
 
      /* Check for terminator */
      if (!isHEX_DIGIT(ch)) {
        save_ch(value);
        break;
      }
    }
    save_ch(value + digit_value[ch]);
  }
 
  /* Classify result, based on why loop ended */
  if (ch == '>')
    tokenType = TOKEN_HEX_STRING;
  else {
    /* save the invalid character for error reporting */
    save_ch(ch);
    tokenType = TOKEN_INVALID;
  }
 
  return(DONE);
}
 
/* -------------------------------------------------------------------
 * Assemble a string; set token type and done flag
 */
 
/* Save a backslash-coded character in a string --
 *
 *   Store the proper character for special cases
 *   "\b", "\f", "\n", "\r", and "\t".
 *
 *   Decode and store octal-coded character, up to
 *   three octal digits, "\o", "\oo", and "\ooo".
 *
 *   The sequence "\<newline>" is a line continuation,
 *   so consume both without storing anything.
 *
 *   The sequence "\<EOF>" is an error; exit without
 *   storing anything and let the caller handle it.
 *
 *   For other characters, including the sequences
 *   "\\", "\(", and "\)", simply store the second
 *   character.
 */
static void 
save_digraph(int ch)
{
  int value;
 
  switch (ch) {
 
    case 'b':   /* backspace */
      ch = '\b';
      break;
 
    case 'f':   /* formfeed */
      ch = '\f';
      break;
 
    case 'n':   /* newline */
      ch = '\n';
      break;
 
    case 'r':   /* carriage return */
      ch = '\r';
      break;
 
    case 't':   /* horizontal tab */
      ch = '\t';
      break;
 
    case '\n':  /* line continuation -- consume it */
      return;
 
    case '\r':  /* carriage return   -- consume it */
      ch = next_ch();   /* look at next character, is it \n?  */
      if (ch == '\n')  return;
      back_ch(ch);      /* if not a line feed, then return it */
      return;
 
    case EOF:   /* end of file -- forget it */
      return;
 
  default:
    /* scan up to three octal digits to get value */
    if (isOCTAL_DIGIT(ch)) {
      value = digit_value[ch];
      ch = next_ch();
      if (isOCTAL_DIGIT(ch)) {
        value = (value << 3) + digit_value[ch];
        ch = next_ch();
        if (isOCTAL_DIGIT(ch))
          value = (value << 3) + digit_value[ch];
        else
          back_ch(ch);
      }
      else
        back_ch(ch);
      ch = value;
    }
  }
 
  /* Found a character to save */
  save_ch(ch);
}
 
/* Done: String */
static int 
STRING(int ch)
{
  int nest_level = 1;
 
  tokenType = TOKEN_STRING;
 
  do {
 
    ch = next_ch();
    while(!isSTRING_SPECIAL(ch)) {
      save_ch(ch);
      ch = next_ch();
    };
 
    switch (ch) {
 
     case '(':
       ++nest_level;
       save_ch(ch);
       break;
 
     case ')':
       if (--nest_level > 0)
         save_ch(ch);
       break;
 
     case '\\':
          save_digraph(next_ch());
        break;
 
     case '\r':
        /* All carriage returns (\r) are turned into linefeeds (\n)*/
          ch = next_ch();       /* get the next one, is it \n? */
          if (ch != '\n') {     /* if not, then put it back.   */
            back_ch(ch);
          }
          save_ch('\n');        /* in either case, save a linefeed */
        break;
 
 
     case EOF:
       tokenType = TOKEN_INVALID;  /* Unterminated string */
       nest_level = 0;
       break;
    }
 
  } while(nest_level > 0);
 
  /* If there's room, add a 0-byte termination without increasing string
     length.  This fixes certain dependencies on 0-terminated strings */
  save_ch_no_inc(0);

  return(DONE);
}
 
 
/* -------------------------------------------------------------------
 * Assemble a name; set token type and done flag.
 * Put current input character back, if it is not white space.
 */
 
/* Done: Name
 *  (Safe version used to complete name tokens that
 *   start out looking like something else).
 */
 
static int 
AAH_NAME(int ch)
{
  do {
    save_ch(ch);
    ch = next_ch();
  } while(isNAME(ch));
 
  back_ch_not_white(ch);
  tokenType = TOKEN_NAME;
  return(DONE);
}
 
/* Done: Name */
static int 
NAME(int ch)
{
  save_unsafe_ch(ch);
  ch = next_ch();
  if (isNAME(ch)) {
    save_unsafe_ch(ch);
    ch = next_ch();
    if (isNAME(ch)) {
      save_unsafe_ch(ch);
      ch = next_ch();
      if (isNAME(ch)) {
        save_unsafe_ch(ch);
        ch = next_ch();
        if (isNAME(ch)) {
          save_unsafe_ch(ch);
          ch = next_ch();
          if (isNAME(ch)) {
            save_unsafe_ch(ch);
            ch = next_ch();
            if (isNAME(ch)) {
              save_unsafe_ch(ch);
              ch = next_ch();
              while(isNAME(ch)) {
                save_ch(ch);
                ch = next_ch();
              }
            }
          }
        }
      }
    }
  }
 
  back_ch_not_white(ch);
  tokenType = TOKEN_NAME;
  return(DONE);
}
 
/* Done: Literal Name */
static int 
LITERAL_NAME(int ch)
{
  if (isNAME(ch)) {
    save_unsafe_ch(ch);
    ch = next_ch();
    if (isNAME(ch)) {
      save_unsafe_ch(ch);
      ch = next_ch();
      if (isNAME(ch)) {
        save_unsafe_ch(ch);
        ch = next_ch();
        if (isNAME(ch)) {
          save_unsafe_ch(ch);
          ch = next_ch();
          if (isNAME(ch)) {
            save_unsafe_ch(ch);
            ch = next_ch();
            if (isNAME(ch)) {
              save_unsafe_ch(ch);
              ch = next_ch();
              while(isNAME(ch)) {
                save_ch(ch);
                ch = next_ch();
              }
            }
          }
        }
      }
    }
  }
 
  back_ch_not_white(ch);
  tokenType = TOKEN_LITERAL_NAME;
  return(DONE);
}
 
/* Done: immediate Name */
static int 
IMMED_NAME(int ch)
{
  ch = next_ch();
  if (isNAME(ch)) {
    save_unsafe_ch(ch);
    ch = next_ch();
    if (isNAME(ch)) {
      save_unsafe_ch(ch);
      ch = next_ch();
      if (isNAME(ch)) {
        save_unsafe_ch(ch);
        ch = next_ch();
        if (isNAME(ch)) {
          save_unsafe_ch(ch);
          ch = next_ch();
          if (isNAME(ch)) {
            save_unsafe_ch(ch);
            ch = next_ch();
            if (isNAME(ch)) {
              save_unsafe_ch(ch);
              ch = next_ch();
              while(isNAME(ch)) {
                save_ch(ch);
                ch = next_ch();
              }
            }
          }
        }
      }
    }
  }
 
  back_ch_not_white(ch);
  tokenType = TOKEN_IMMED_NAME;
  return(DONE);
}
 
/* Done: Name found while looking for something else */
static int 
OOPS_NAME(int ch)
{
  back_ch_not_white(ch);
  tokenType = TOKEN_NAME;
  return(DONE);
}
 
 
/* -------------------------------------------------------------------
 * Complete a miscellaneous token; set token type and done flag.
 */
 
/* Done: Unmatched Right Angle-Bracket */
static int 
RIGHT_ANGLE(int ch)
{
  tokenType = TOKEN_RIGHT_ANGLE;
  return(DONE);
}
 
/* Done: Unmatched Right Parenthesis */
static int 
RIGHT_PAREN(int ch)
{
  tokenType = TOKEN_RIGHT_PAREN;
  return(DONE);
}
 
/* Done: Left Brace */
static int 
LEFT_BRACE(int ch)
{
  tokenType = TOKEN_LEFT_BRACE;
  return(DONE);
}
 
/* Done: Right Brace */
static int 
RIGHT_BRACE(int ch)
{
  tokenType = TOKEN_RIGHT_BRACE;
  return(DONE);
}
 
/* Done: Left Bracket */
static int 
LEFT_BRACKET(int ch)
{
  save_unsafe_ch(ch);
  tokenType = TOKEN_LEFT_BRACKET;
  return(DONE);
}
 
/* Done: Right Bracket */
static int 
RIGHT_BRACKET(int ch)
{
  save_unsafe_ch(ch);
  tokenType = TOKEN_RIGHT_BRACKET;
  return(DONE);
}
 
/* Done: Break */
static int 
BREAK_SIGNAL(int ch)
{
  tokenType = TOKEN_BREAK;
  return(DONE);
}
 
/* Done: No Token Found */
static int 
NO_TOKEN(int ch)
{
  tokenType = TOKEN_EOF;
  return(DONE);
}
 
 
/*
 * -------------------------------------------------------------------
 *  scan_token -- scan one token from the input.  It uses a simple
 *    finite state machine to recognize token classes.
 *
 *  The input is from a file.
 *
 *  On entry --
 *
 *    inputP -> input PostScript object, a file.
 *    tokenStartP -> buffer in VM for accumulating the token.
 *    tokenMaxP -> last character in the token buffer
 *
 *  On exit --
 *
 *    tokenLength = number of characters in the token
 *    tokenTooLong = TRUE if the token did not fit in the buffer
 *    tokenType = code for the type of token parsed.
 *    tokenValue = converted value of a numeric token.
 *
 *
 * -------------------------------------------------------------------
 */
void 
scan_token(psobj *inputP)
{
  int ch;
  unsigned char *stateP = s0;
  unsigned char entry;
  int (*actionP)(int);
 
  /* Define input source */
  inputFileP = inputP->data.fileP;
  if (inputFileP == NULL)  {
    tokenType = TOKEN_EOF;
    return;
  }
 
  /* Ensure enough space for most cases
   * (so we don't have to keep checking)
   * The length needs to cover the maximum number
   * of save_unsafe_ch() calls that might be executed.
   * That number is 11 (a sign and 10 decimal digits, e.g.,
   * when scanning -2147483648), but use MAX_NAME_LEN
   * in case someone changes that without checking.
   */
  tokenStartP = vm_next_byte();
  tokenMaxP = tokenStartP + MIN(vm_free_bytes(), MAX_STRING_LEN);
 
  if ((tokenMaxP-tokenStartP) < (MAX_NAME_LEN)) {
    tokenLength = 0;
    tokenTooLong = TRUE;
    tokenType = TOKEN_NONE;
    tokenValue.integer = 0;
    return;
  }
 
  /* Reset token */
  tokenCharP = tokenStartP;
  tokenTooLong = FALSE;
 
  /* Scan one token */
  ch = next_ch();
  do {
    entry = stateP[ch];
    stateP = classActionTable[entry].nextStateP;
    actionP = classActionTable[entry].actionRoutineP;
    ch = (*actionP)(ch);
  } while(ch != DONE);
 
 
  /* Return results */
  tokenLength = tokenCharP - tokenStartP;
}
