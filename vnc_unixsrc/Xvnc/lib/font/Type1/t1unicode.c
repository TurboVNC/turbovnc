/*
Copyright (c) 1998 by Juliusz Chroboczek

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/* $XFree86$ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "t1unicode.h"

static char* table_32[] = 
{ "space", "exclam", "quotedbl", "numbersign", "dollar", "percent",
  "ampersand", "quotesingle", "parenleft", "parenright", "asterisk",
  "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two",
  "three", "four", "five", "six", "seven", "eight", "nine", "colon",
  "semicolon", "less", "equal", "greater", "question", "at", "A", "B",
  "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P",
  "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
  "backslash", "bracketright", "asciicircum", "underscore", "grave",
  "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n",
  "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
  "braceleft", "bar", "braceright", "asciitilde", 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, "space", "exclamdown", "cent", "sterling", "currency", "yen",
  "brokenbar", "section", "dieresis", "copyright", "ordfeminine",
  "guillemotleft", "logicalnot", "hyphen", "registered", "macron",
  "degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu",
  "paragraph", "periodcentered", "cedilla", "onesuperior",
  "ordmasculine", "guillemotright", "onequarter", "onehalf",
  "threequarters", "questiondown", "Agrave", "Aacute", "Acircumflex",
  "Atilde", "Adieresis", "Aring", "AE", "Ccedilla", "Egrave", "Eacute",
  "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex",
  "Idieresis", "Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex",
  "Otilde", "Odieresis", "multiply", "Oslash", "Ugrave", "Uacute",
  "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls", "agrave",
  "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae",
  "ccedilla", "egrave", "eacute", "ecircumflex", "edieresis", "igrave",
  "iacute", "icircumflex", "idieresis", "eth", "ntilde", "ograve",
  "oacute", "ocircumflex", "otilde", "odieresis", "divide", "oslash",
  "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn",
  "ydieresis", "Amacron", "amacron", "Abreve", "abreve", "Aogonek",
  "aogonek", "Cacute", "cacute", "Ccircumflex", "ccircumflex",
  "Cdotaccent", "cdotaccent", "Ccaron", "ccaron", "Dcaron", "dcaron",
  "Dcroat", "dcroat", "Emacron", "emacron", "Ebreve", "ebreve",
  "Edotaccent", "edotaccent", "Eogonek", "eogonek", "Ecaron", "ecaron",
  "Gcircumflex", "gcircumflex", "Gbreve", "gbreve", "Gdotaccent",
  "gdotaccent", "Gcommaaccent", "gcommaaccent", "Hcircumflex",
  "hcircumflex", "Hbar", "hbar", "Itilde", "itilde", "Imacron",
  "imacron", "Ibreve", "ibreve", "Iogonek", "iogonek", "Idotaccent",
  "dotlessi", "IJ", "ij", "Jcircumflex", "jcircumflex", "Kcommaaccent",
  "kcommaaccent", "kgreenlandic", "Lacute", "lacute", "Lcommaaccent",
  "lcommaaccent", "Lcaron", "lcaron", "Ldot", "ldot", "Lslash",
  "lslash", "Nacute", "nacute", "Ncommaaccent", "ncommaaccent",
  "Ncaron", "ncaron", "napostrophe", "Eng", "eng", "Omacron", "omacron",
  "Obreve", "obreve", "Ohungarumlaut", "ohungarumlaut", "OE", "oe",
  "Racute", "racute", "Rcommaaccent", "rcommaaccent", "Rcaron",
  "rcaron", "Sacute", "sacute", "Scircumflex", "scircumflex",
  "Scommaaccent", "scommaaccent", "Scaron", "scaron", "Tcommaaccent",
  "tcommaaccent", "Tcaron", "tcaron", "Tbar", "tbar", "Utilde",
  "utilde", "Umacron", "umacron", "Ubreve", "ubreve", "Uring", "uring",
  "Uhungarumlaut", "uhungarumlaut", "Uogonek", "uogonek", "Wcircumflex",
  "wcircumflex", "Ycircumflex", "ycircumflex", "Ydieresis", "Zacute",
  "zacute", "Zdotaccent", "zdotaccent", "Zcaron", "zcaron", "longs", 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "florin", 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Ohorn", "ohorn", 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, "Uhorn", "uhorn", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Gcaron",
  "gcaron", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "Aringacute", "aringacute", "AEacute", "aeacute", "Oslashacute",
  "oslashacute", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57929", "afii64937", 0, 0, 0, 0, 0, 0,
  0, 0, "circumflex", "caron", 0, "macron", 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, "breve", "dotaccent", "ring", "ogonek", "tilde",
  "hungarumlaut", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "gravecomb",
  "acutecomb", 0, "tildecomb", 0, 0, 0, 0, 0, "hookabovecomb", 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "dotbelowcomb", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, "tonos", "dieresistonos", "Alphatonos",
  "anoteleia", "Epsilontonos", "Etatonos", "Iotatonos", 0,
  "Omicrontonos", 0, "Upsilontonos", "Omegatonos", "iotadieresistonos",
  "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta",
  "Iota", "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho",
  0, "Sigma", "Tau", "Upsilon", "Phi", "Chi", "Psi", "Omega",
  "Iotadieresis", "Upsilondieresis", "alphatonos", "epsilontonos",
  "etatonos", "iotatonos", "upsilondieresistonos", "alpha", "beta",
  "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa",
  "lambda", "mu", "nu", "xi", "omicron", "pi", "rho", "sigma1", "sigma",
  "tau", "upsilon", "phi", "chi", "psi", "omega", "iotadieresis",
  "upsilondieresis", "omicrontonos", "upsilontonos", "omegatonos", 0, 0,
  "theta1", "Upsilon1", 0, 0, "phi1", "omega1", 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii10023", "afii10051",
  "afii10052", "afii10053", "afii10054", "afii10055", "afii10056",
  "afii10057", "afii10058", "afii10059", "afii10060", "afii10061", 0,
  "afii10062", "afii10145", "afii10017", "afii10018", "afii10019",
  "afii10020", "afii10021", "afii10022", "afii10024", "afii10025",
  "afii10026", "afii10027", "afii10028", "afii10029", "afii10030",
  "afii10031", "afii10032", "afii10033", "afii10034", "afii10035",
  "afii10036", "afii10037", "afii10038", "afii10039", "afii10040",
  "afii10041", "afii10042", "afii10043", "afii10044", "afii10045",
  "afii10046", "afii10047", "afii10048", "afii10049", "afii10065",
  "afii10066", "afii10067", "afii10068", "afii10069", "afii10070",
  "afii10072", "afii10073", "afii10074", "afii10075", "afii10076",
  "afii10077", "afii10078", "afii10079", "afii10080", "afii10081",
  "afii10082", "afii10083", "afii10084", "afii10085", "afii10086",
  "afii10087", "afii10088", "afii10089", "afii10090", "afii10091",
  "afii10092", "afii10093", "afii10094", "afii10095", "afii10096",
  "afii10097", 0, "afii10071", "afii10099", "afii10100", "afii10101",
  "afii10102", "afii10103", "afii10104", "afii10105", "afii10106",
  "afii10107", "afii10108", "afii10109", 0, "afii10110", "afii10193", 0,
  0, "afii10146", "afii10194", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "afii10147", "afii10195", "afii10148", "afii10196", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "afii10050", "afii10098", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii10846", 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "afii57799", "afii57801", "afii57800", "afii57802", "afii57793",
  "afii57794", "afii57795", "afii57798", "afii57797", "afii57806", 0,
  "afii57796", "afii57807", "afii57839", "afii57645", "afii57841",
  "afii57842", "afii57804", "afii57803", "afii57658", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, "afii57664", "afii57665", "afii57666", "afii57667",
  "afii57668", "afii57669", "afii57670", "afii57671", "afii57672",
  "afii57673", "afii57674", "afii57675", "afii57676", "afii57677",
  "afii57678", "afii57679", "afii57680", "afii57681", "afii57682",
  "afii57683", "afii57684", "afii57685", "afii57686", "afii57687",
  "afii57688", "afii57689", "afii57690", 0, 0, 0, 0, 0, "afii57716",
  "afii57717", "afii57718", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57388", 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, "afii57403", 0, 0, 0, "afii57407", 0, "afii57409",
  "afii57410", "afii57411", "afii57412", "afii57413", "afii57414",
  "afii57415", "afii57416", "afii57417", "afii57418", "afii57419",
  "afii57420", "afii57421", "afii57422", "afii57423", "afii57424",
  "afii57425", "afii57426", "afii57427", "afii57428", "afii57429",
  "afii57430", "afii57431", "afii57432", "afii57433", "afii57434", 0, 0,
  0, 0, 0, "afii57440", "afii57441", "afii57442", "afii57443",
  "afii57444", "afii57445", "afii57446", "afii57470", "afii57448",
  "afii57449", "afii57450", "afii57451", "afii57452", "afii57453",
  "afii57454", "afii57455", "afii57456", "afii57457", "afii57458", 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57392", "afii57393",
  "afii57394", "afii57395", "afii57396", "afii57397", "afii57398",
  "afii57399", "afii57400", "afii57401", "afii57381", 0, 0, "afii63167",
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57511", 0, 0, 0, 0, "afii57506",
  0, 0, 0, 0, 0, 0, 0, "afii57507", 0, "afii57512", 0, 0, 0, 0, 0, 0, 0,
  0, "afii57513", 0, 0, 0, 0, 0, 0, "afii57508", 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, "afii57505", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57509", 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, "afii57514", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57519", 0, 0, "afii57534", 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char* table_2000[] =     /* general punctuation, s*scripts, currency */
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii61664", "afii301", "afii299",
  "afii300", 0, 0, "figuredash", "endash", "emdash", "afii00208", 0,
  "underscoredbl", "quoteleft", "quoteright", "quotesinglbase",
  "quotereversed", "quotedblleft", "quotedblright", "quotedblbase", 0,
  "dagger", "daggerdbl", "bullet", 0, "onedotenleader",
  "twodotenleader", "ellipsis", 0, 0, 0, 0, 0, "afii61573", "afii61574",
  "afii61575", 0, "perthousand", 0, "minute", "second", 0, 0, 0, 0, 0,
  "guilsinglleft", "guilsinglright", 0, "exclamdbl", 0, 0, 0, 0, 0, 0,
  0, "fraction", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, "zerosuperior", 0, 0, 0, "foursuperior", "fivesuperior",
  "sixsuperior", "sevensuperior", "eightsuperior", "ninesuperior", 0, 0,
  0, "parenleftsuperior", "parenrightsuperior", "nsuperior",
  "zeroinferior", "oneinferior", "twoinferior", "threeinferior",
  "fourinferior", "fiveinferior", "sixinferior", "seveninferior",
  "eightinferior", "nineinferior", 0, 0, 0, "parenleftinferior",
  "parenrightinferior", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, "colonmonetary", 0, "franc", "lira", 0, 0, "peseta", 0, 0,
  "afii57636", "dong", "Euro", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char* table_2500[]=      /* line and box drawing */
{ "SF100000", 0, "SF110000", 0, 0, 0, 0, 0, 0, 0, 0, 0, "SF010000", 0,
  0, 0, "SF030000", 0, 0, 0, "SF020000", 0, 0, 0, "SF040000", 0, 0, 0,
  "SF080000", 0, 0, 0, 0, 0, 0, 0, "SF090000", 0, 0, 0, 0, 0, 0, 0,
  "SF060000", 0, 0, 0, 0, 0, 0, 0, "SF070000", 0, 0, 0, 0, 0, 0, 0,
  "SF050000", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "SF430000", "SF240000", "SF510000", "SF520000", "SF390000",
  "SF220000", "SF210000", "SF250000", "SF500000", "SF490000",
  "SF380000", "SF280000", "SF270000", "SF260000", "SF360000",
  "SF370000", "SF420000", "SF190000", "SF200000", "SF230000",
  "SF470000", "SF480000", "SF410000", "SF450000", "SF460000",
  "SF400000", "SF540000", "SF530000", "SF440000", 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "upblock", 0, 0, 0, "dnblock", 0,
  0, 0, "block", 0, 0, 0, "lfblock", 0, 0, 0, "rtblock", "ltshade",
  "shade", "dkshade", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char* table_FB00[] =     /* alphabetic presentation forms */
{ "ff", "fi", "fl", "ffi", "ffl", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "afii57705", 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, "afii57694", "afii57695", 0, 0, 0, 0, 0, 0, 0, 0, 0,
  "afii57723", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, "afii57700", 0, 0, 0, 0 };

char*
unicodetoPSname(unsigned short code)
{
  if(code<32) return 0;
  else if(code<0x6FF) return table_32[code-32];
  else if(code<0x2000) return 0;
  else if(code<0x20D0) return table_2000[code-0x2000];
  else if(code==0x2116) return "afii61352"; /* numero sign, for Koi */
  else if(code==0x2122) return "trademark";
  else if(code<0x2500) return 0;
  else if(code<0x25A0) return table_2500[code-0x2500];
  else if(code<0xFB00) return 0;
  else if(code<0xFB50) return table_FB00[code-0xFB00];
  else return 0;
}
