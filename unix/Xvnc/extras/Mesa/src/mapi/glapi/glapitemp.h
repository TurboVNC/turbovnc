/* DO NOT EDIT - This file generated automatically by gl_apitemp.py (from Mesa) script */

/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL, IBM,
 * AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#  if defined(__GNUC__) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#    define HIDDEN  __attribute__((visibility("hidden")))
#  else
#    define HIDDEN
#  endif

/*
 * This file is a template which generates the OpenGL API entry point
 * functions.  It should be included by a .c file which first defines
 * the following macros:
 *   KEYWORD1 - usually nothing, but might be __declspec(dllexport) on Win32
 *   KEYWORD2 - usually nothing, but might be __stdcall on Win32
 *   NAME(n)  - builds the final function name (usually add "gl" prefix)
 *   DISPATCH(func, args, msg) - code to do dispatch of named function.
 *                               msg is a printf-style debug message.
 *   RETURN_DISPATCH(func, args, msg) - code to do dispatch with a return value
 *
 * Here is an example which generates the usual OpenGL functions:
 *   #define KEYWORD1
 *   #define KEYWORD2
 *   #define NAME(func)  gl##func
 *   #define DISPATCH(func, args, msg)                           \
 *          struct _glapi_table *dispatch = CurrentDispatch;     \
 *          (*dispatch->func) args
 *   #define RETURN DISPATCH(func, args, msg)                    \
 *          struct _glapi_table *dispatch = CurrentDispatch;     \
 *          return (*dispatch->func) args
 *
 */


#if defined( NAME )
#ifndef KEYWORD1
#define KEYWORD1
#endif

#ifndef KEYWORD1_ALT
#define KEYWORD1_ALT HIDDEN
#endif

#ifndef KEYWORD2
#define KEYWORD2
#endif

#ifndef DISPATCH
#error DISPATCH must be defined
#endif

#ifndef RETURN_DISPATCH
#error RETURN_DISPATCH must be defined
#endif


#ifndef _GLAPI_SKIP_NORMAL_ENTRY_POINTS

KEYWORD1 void KEYWORD2 NAME(NewList)(GLuint list, GLenum mode)
{
    (void) list; (void) mode;
   DISPATCH(NewList, (list, mode), (F, "glNewList(%d, 0x%x);\n", list, mode));
}

KEYWORD1 void KEYWORD2 NAME(EndList)(void)
{
   DISPATCH(EndList, (), (F, "glEndList();\n"));
}

KEYWORD1 void KEYWORD2 NAME(CallList)(GLuint list)
{
    (void) list;
   DISPATCH(CallList, (list), (F, "glCallList(%d);\n", list));
}

KEYWORD1 void KEYWORD2 NAME(CallLists)(GLsizei n, GLenum type, const GLvoid * lists)
{
    (void) n; (void) type; (void) lists;
   DISPATCH(CallLists, (n, type, lists), (F, "glCallLists(%d, 0x%x, %p);\n", n, type, (const void *) lists));
}

KEYWORD1 void KEYWORD2 NAME(DeleteLists)(GLuint list, GLsizei range)
{
    (void) list; (void) range;
   DISPATCH(DeleteLists, (list, range), (F, "glDeleteLists(%d, %d);\n", list, range));
}

KEYWORD1 GLuint KEYWORD2 NAME(GenLists)(GLsizei range)
{
    (void) range;
   RETURN_DISPATCH(GenLists, (range), (F, "glGenLists(%d);\n", range));
}

KEYWORD1 void KEYWORD2 NAME(ListBase)(GLuint base)
{
    (void) base;
   DISPATCH(ListBase, (base), (F, "glListBase(%d);\n", base));
}

KEYWORD1 void KEYWORD2 NAME(Begin)(GLenum mode)
{
    (void) mode;
   DISPATCH(Begin, (mode), (F, "glBegin(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap)
{
    (void) width; (void) height; (void) xorig; (void) yorig; (void) xmove; (void) ymove; (void) bitmap;
   DISPATCH(Bitmap, (width, height, xorig, yorig, xmove, ymove, bitmap), (F, "glBitmap(%d, %d, %f, %f, %f, %f, %p);\n", width, height, xorig, yorig, xmove, ymove, (const void *) bitmap));
}

KEYWORD1 void KEYWORD2 NAME(Color3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3b, (red, green, blue), (F, "glColor3b(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3bv)(const GLbyte * v)
{
    (void) v;
   DISPATCH(Color3bv, (v), (F, "glColor3bv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3d, (red, green, blue), (F, "glColor3d(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Color3dv, (v), (F, "glColor3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3f, (red, green, blue), (F, "glColor3f(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Color3fv, (v), (F, "glColor3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3i)(GLint red, GLint green, GLint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3i, (red, green, blue), (F, "glColor3i(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Color3iv, (v), (F, "glColor3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3s)(GLshort red, GLshort green, GLshort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3s, (red, green, blue), (F, "glColor3s(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Color3sv, (v), (F, "glColor3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3ub, (red, green, blue), (F, "glColor3ub(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3ubv)(const GLubyte * v)
{
    (void) v;
   DISPATCH(Color3ubv, (v), (F, "glColor3ubv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3ui)(GLuint red, GLuint green, GLuint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3ui, (red, green, blue), (F, "glColor3ui(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3uiv)(const GLuint * v)
{
    (void) v;
   DISPATCH(Color3uiv, (v), (F, "glColor3uiv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color3us)(GLushort red, GLushort green, GLushort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(Color3us, (red, green, blue), (F, "glColor3us(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(Color3usv)(const GLushort * v)
{
    (void) v;
   DISPATCH(Color3usv, (v), (F, "glColor3usv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4b, (red, green, blue, alpha), (F, "glColor4b(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4bv)(const GLbyte * v)
{
    (void) v;
   DISPATCH(Color4bv, (v), (F, "glColor4bv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4d, (red, green, blue, alpha), (F, "glColor4d(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Color4dv, (v), (F, "glColor4dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4f, (red, green, blue, alpha), (F, "glColor4f(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Color4fv, (v), (F, "glColor4fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4i)(GLint red, GLint green, GLint blue, GLint alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4i, (red, green, blue, alpha), (F, "glColor4i(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Color4iv, (v), (F, "glColor4iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4s, (red, green, blue, alpha), (F, "glColor4s(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Color4sv, (v), (F, "glColor4sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4ub, (red, green, blue, alpha), (F, "glColor4ub(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4ubv)(const GLubyte * v)
{
    (void) v;
   DISPATCH(Color4ubv, (v), (F, "glColor4ubv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4ui, (red, green, blue, alpha), (F, "glColor4ui(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4uiv)(const GLuint * v)
{
    (void) v;
   DISPATCH(Color4uiv, (v), (F, "glColor4uiv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4us, (red, green, blue, alpha), (F, "glColor4us(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(Color4usv)(const GLushort * v)
{
    (void) v;
   DISPATCH(Color4usv, (v), (F, "glColor4usv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(EdgeFlag)(GLboolean flag)
{
    (void) flag;
   DISPATCH(EdgeFlag, (flag), (F, "glEdgeFlag(%d);\n", flag));
}

KEYWORD1 void KEYWORD2 NAME(EdgeFlagv)(const GLboolean * flag)
{
    (void) flag;
   DISPATCH(EdgeFlagv, (flag), (F, "glEdgeFlagv(%p);\n", (const void *) flag));
}

KEYWORD1 void KEYWORD2 NAME(End)(void)
{
   DISPATCH(End, (), (F, "glEnd();\n"));
}

KEYWORD1 void KEYWORD2 NAME(Indexd)(GLdouble c)
{
    (void) c;
   DISPATCH(Indexd, (c), (F, "glIndexd(%f);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(Indexdv)(const GLdouble * c)
{
    (void) c;
   DISPATCH(Indexdv, (c), (F, "glIndexdv(%p);\n", (const void *) c));
}

KEYWORD1 void KEYWORD2 NAME(Indexf)(GLfloat c)
{
    (void) c;
   DISPATCH(Indexf, (c), (F, "glIndexf(%f);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(Indexfv)(const GLfloat * c)
{
    (void) c;
   DISPATCH(Indexfv, (c), (F, "glIndexfv(%p);\n", (const void *) c));
}

KEYWORD1 void KEYWORD2 NAME(Indexi)(GLint c)
{
    (void) c;
   DISPATCH(Indexi, (c), (F, "glIndexi(%d);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(Indexiv)(const GLint * c)
{
    (void) c;
   DISPATCH(Indexiv, (c), (F, "glIndexiv(%p);\n", (const void *) c));
}

KEYWORD1 void KEYWORD2 NAME(Indexs)(GLshort c)
{
    (void) c;
   DISPATCH(Indexs, (c), (F, "glIndexs(%d);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(Indexsv)(const GLshort * c)
{
    (void) c;
   DISPATCH(Indexsv, (c), (F, "glIndexsv(%p);\n", (const void *) c));
}

KEYWORD1 void KEYWORD2 NAME(Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3b, (nx, ny, nz), (F, "glNormal3b(%d, %d, %d);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Normal3bv)(const GLbyte * v)
{
    (void) v;
   DISPATCH(Normal3bv, (v), (F, "glNormal3bv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3d, (nx, ny, nz), (F, "glNormal3d(%f, %f, %f);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Normal3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Normal3dv, (v), (F, "glNormal3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3f, (nx, ny, nz), (F, "glNormal3f(%f, %f, %f);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Normal3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Normal3fv, (v), (F, "glNormal3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Normal3i)(GLint nx, GLint ny, GLint nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3i, (nx, ny, nz), (F, "glNormal3i(%d, %d, %d);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Normal3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Normal3iv, (v), (F, "glNormal3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Normal3s)(GLshort nx, GLshort ny, GLshort nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3s, (nx, ny, nz), (F, "glNormal3s(%d, %d, %d);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Normal3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Normal3sv, (v), (F, "glNormal3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2d)(GLdouble x, GLdouble y)
{
    (void) x; (void) y;
   DISPATCH(RasterPos2d, (x, y), (F, "glRasterPos2d(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(RasterPos2dv, (v), (F, "glRasterPos2dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2f)(GLfloat x, GLfloat y)
{
    (void) x; (void) y;
   DISPATCH(RasterPos2f, (x, y), (F, "glRasterPos2f(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(RasterPos2fv, (v), (F, "glRasterPos2fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2i)(GLint x, GLint y)
{
    (void) x; (void) y;
   DISPATCH(RasterPos2i, (x, y), (F, "glRasterPos2i(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2iv)(const GLint * v)
{
    (void) v;
   DISPATCH(RasterPos2iv, (v), (F, "glRasterPos2iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2s)(GLshort x, GLshort y)
{
    (void) x; (void) y;
   DISPATCH(RasterPos2s, (x, y), (F, "glRasterPos2s(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos2sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(RasterPos2sv, (v), (F, "glRasterPos2sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(RasterPos3d, (x, y, z), (F, "glRasterPos3d(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(RasterPos3dv, (v), (F, "glRasterPos3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(RasterPos3f, (x, y, z), (F, "glRasterPos3f(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(RasterPos3fv, (v), (F, "glRasterPos3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3i)(GLint x, GLint y, GLint z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(RasterPos3i, (x, y, z), (F, "glRasterPos3i(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(RasterPos3iv, (v), (F, "glRasterPos3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3s)(GLshort x, GLshort y, GLshort z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(RasterPos3s, (x, y, z), (F, "glRasterPos3s(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(RasterPos3sv, (v), (F, "glRasterPos3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(RasterPos4d, (x, y, z, w), (F, "glRasterPos4d(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(RasterPos4dv, (v), (F, "glRasterPos4dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(RasterPos4f, (x, y, z, w), (F, "glRasterPos4f(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(RasterPos4fv, (v), (F, "glRasterPos4fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4i)(GLint x, GLint y, GLint z, GLint w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(RasterPos4i, (x, y, z, w), (F, "glRasterPos4i(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4iv)(const GLint * v)
{
    (void) v;
   DISPATCH(RasterPos4iv, (v), (F, "glRasterPos4iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(RasterPos4s, (x, y, z, w), (F, "glRasterPos4s(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(RasterPos4sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(RasterPos4sv, (v), (F, "glRasterPos4sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    (void) x1; (void) y1; (void) x2; (void) y2;
   DISPATCH(Rectd, (x1, y1, x2, y2), (F, "glRectd(%f, %f, %f, %f);\n", x1, y1, x2, y2));
}

KEYWORD1 void KEYWORD2 NAME(Rectdv)(const GLdouble * v1, const GLdouble * v2)
{
    (void) v1; (void) v2;
   DISPATCH(Rectdv, (v1, v2), (F, "glRectdv(%p, %p);\n", (const void *) v1, (const void *) v2));
}

KEYWORD1 void KEYWORD2 NAME(Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    (void) x1; (void) y1; (void) x2; (void) y2;
   DISPATCH(Rectf, (x1, y1, x2, y2), (F, "glRectf(%f, %f, %f, %f);\n", x1, y1, x2, y2));
}

KEYWORD1 void KEYWORD2 NAME(Rectfv)(const GLfloat * v1, const GLfloat * v2)
{
    (void) v1; (void) v2;
   DISPATCH(Rectfv, (v1, v2), (F, "glRectfv(%p, %p);\n", (const void *) v1, (const void *) v2));
}

KEYWORD1 void KEYWORD2 NAME(Recti)(GLint x1, GLint y1, GLint x2, GLint y2)
{
    (void) x1; (void) y1; (void) x2; (void) y2;
   DISPATCH(Recti, (x1, y1, x2, y2), (F, "glRecti(%d, %d, %d, %d);\n", x1, y1, x2, y2));
}

KEYWORD1 void KEYWORD2 NAME(Rectiv)(const GLint * v1, const GLint * v2)
{
    (void) v1; (void) v2;
   DISPATCH(Rectiv, (v1, v2), (F, "glRectiv(%p, %p);\n", (const void *) v1, (const void *) v2));
}

KEYWORD1 void KEYWORD2 NAME(Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    (void) x1; (void) y1; (void) x2; (void) y2;
   DISPATCH(Rects, (x1, y1, x2, y2), (F, "glRects(%d, %d, %d, %d);\n", x1, y1, x2, y2));
}

KEYWORD1 void KEYWORD2 NAME(Rectsv)(const GLshort * v1, const GLshort * v2)
{
    (void) v1; (void) v2;
   DISPATCH(Rectsv, (v1, v2), (F, "glRectsv(%p, %p);\n", (const void *) v1, (const void *) v2));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1d)(GLdouble s)
{
    (void) s;
   DISPATCH(TexCoord1d, (s), (F, "glTexCoord1d(%f);\n", s));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(TexCoord1dv, (v), (F, "glTexCoord1dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1f)(GLfloat s)
{
    (void) s;
   DISPATCH(TexCoord1f, (s), (F, "glTexCoord1f(%f);\n", s));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(TexCoord1fv, (v), (F, "glTexCoord1fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1i)(GLint s)
{
    (void) s;
   DISPATCH(TexCoord1i, (s), (F, "glTexCoord1i(%d);\n", s));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1iv)(const GLint * v)
{
    (void) v;
   DISPATCH(TexCoord1iv, (v), (F, "glTexCoord1iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1s)(GLshort s)
{
    (void) s;
   DISPATCH(TexCoord1s, (s), (F, "glTexCoord1s(%d);\n", s));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord1sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(TexCoord1sv, (v), (F, "glTexCoord1sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2d)(GLdouble s, GLdouble t)
{
    (void) s; (void) t;
   DISPATCH(TexCoord2d, (s, t), (F, "glTexCoord2d(%f, %f);\n", s, t));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(TexCoord2dv, (v), (F, "glTexCoord2dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2f)(GLfloat s, GLfloat t)
{
    (void) s; (void) t;
   DISPATCH(TexCoord2f, (s, t), (F, "glTexCoord2f(%f, %f);\n", s, t));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(TexCoord2fv, (v), (F, "glTexCoord2fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2i)(GLint s, GLint t)
{
    (void) s; (void) t;
   DISPATCH(TexCoord2i, (s, t), (F, "glTexCoord2i(%d, %d);\n", s, t));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2iv)(const GLint * v)
{
    (void) v;
   DISPATCH(TexCoord2iv, (v), (F, "glTexCoord2iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2s)(GLshort s, GLshort t)
{
    (void) s; (void) t;
   DISPATCH(TexCoord2s, (s, t), (F, "glTexCoord2s(%d, %d);\n", s, t));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord2sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(TexCoord2sv, (v), (F, "glTexCoord2sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3d)(GLdouble s, GLdouble t, GLdouble r)
{
    (void) s; (void) t; (void) r;
   DISPATCH(TexCoord3d, (s, t, r), (F, "glTexCoord3d(%f, %f, %f);\n", s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(TexCoord3dv, (v), (F, "glTexCoord3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3f)(GLfloat s, GLfloat t, GLfloat r)
{
    (void) s; (void) t; (void) r;
   DISPATCH(TexCoord3f, (s, t, r), (F, "glTexCoord3f(%f, %f, %f);\n", s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(TexCoord3fv, (v), (F, "glTexCoord3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3i)(GLint s, GLint t, GLint r)
{
    (void) s; (void) t; (void) r;
   DISPATCH(TexCoord3i, (s, t, r), (F, "glTexCoord3i(%d, %d, %d);\n", s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(TexCoord3iv, (v), (F, "glTexCoord3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3s)(GLshort s, GLshort t, GLshort r)
{
    (void) s; (void) t; (void) r;
   DISPATCH(TexCoord3s, (s, t, r), (F, "glTexCoord3s(%d, %d, %d);\n", s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(TexCoord3sv, (v), (F, "glTexCoord3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    (void) s; (void) t; (void) r; (void) q;
   DISPATCH(TexCoord4d, (s, t, r, q), (F, "glTexCoord4d(%f, %f, %f, %f);\n", s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(TexCoord4dv, (v), (F, "glTexCoord4dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    (void) s; (void) t; (void) r; (void) q;
   DISPATCH(TexCoord4f, (s, t, r, q), (F, "glTexCoord4f(%f, %f, %f, %f);\n", s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(TexCoord4fv, (v), (F, "glTexCoord4fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4i)(GLint s, GLint t, GLint r, GLint q)
{
    (void) s; (void) t; (void) r; (void) q;
   DISPATCH(TexCoord4i, (s, t, r, q), (F, "glTexCoord4i(%d, %d, %d, %d);\n", s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4iv)(const GLint * v)
{
    (void) v;
   DISPATCH(TexCoord4iv, (v), (F, "glTexCoord4iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q)
{
    (void) s; (void) t; (void) r; (void) q;
   DISPATCH(TexCoord4s, (s, t, r, q), (F, "glTexCoord4s(%d, %d, %d, %d);\n", s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(TexCoord4sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(TexCoord4sv, (v), (F, "glTexCoord4sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2d)(GLdouble x, GLdouble y)
{
    (void) x; (void) y;
   DISPATCH(Vertex2d, (x, y), (F, "glVertex2d(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Vertex2dv, (v), (F, "glVertex2dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2f)(GLfloat x, GLfloat y)
{
    (void) x; (void) y;
   DISPATCH(Vertex2f, (x, y), (F, "glVertex2f(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Vertex2fv, (v), (F, "glVertex2fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2i)(GLint x, GLint y)
{
    (void) x; (void) y;
   DISPATCH(Vertex2i, (x, y), (F, "glVertex2i(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Vertex2iv, (v), (F, "glVertex2iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2s)(GLshort x, GLshort y)
{
    (void) x; (void) y;
   DISPATCH(Vertex2s, (x, y), (F, "glVertex2s(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(Vertex2sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Vertex2sv, (v), (F, "glVertex2sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3d)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Vertex3d, (x, y, z), (F, "glVertex3d(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Vertex3dv, (v), (F, "glVertex3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3f)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Vertex3f, (x, y, z), (F, "glVertex3f(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Vertex3fv, (v), (F, "glVertex3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3i)(GLint x, GLint y, GLint z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Vertex3i, (x, y, z), (F, "glVertex3i(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Vertex3iv, (v), (F, "glVertex3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3s)(GLshort x, GLshort y, GLshort z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Vertex3s, (x, y, z), (F, "glVertex3s(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Vertex3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Vertex3sv, (v), (F, "glVertex3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Vertex4d, (x, y, z, w), (F, "glVertex4d(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(Vertex4dv, (v), (F, "glVertex4dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Vertex4f, (x, y, z, w), (F, "glVertex4f(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(Vertex4fv, (v), (F, "glVertex4fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4i)(GLint x, GLint y, GLint z, GLint w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Vertex4i, (x, y, z, w), (F, "glVertex4i(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4iv)(const GLint * v)
{
    (void) v;
   DISPATCH(Vertex4iv, (v), (F, "glVertex4iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Vertex4s, (x, y, z, w), (F, "glVertex4s(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Vertex4sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(Vertex4sv, (v), (F, "glVertex4sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(ClipPlane)(GLenum plane, const GLdouble * equation)
{
    (void) plane; (void) equation;
   DISPATCH(ClipPlane, (plane, equation), (F, "glClipPlane(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(ColorMaterial)(GLenum face, GLenum mode)
{
    (void) face; (void) mode;
   DISPATCH(ColorMaterial, (face, mode), (F, "glColorMaterial(0x%x, 0x%x);\n", face, mode));
}

KEYWORD1 void KEYWORD2 NAME(CullFace)(GLenum mode)
{
    (void) mode;
   DISPATCH(CullFace, (mode), (F, "glCullFace(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(Fogf)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(Fogf, (pname, param), (F, "glFogf(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Fogfv)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(Fogfv, (pname, params), (F, "glFogfv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Fogi)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(Fogi, (pname, param), (F, "glFogi(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Fogiv)(GLenum pname, const GLint * params)
{
    (void) pname; (void) params;
   DISPATCH(Fogiv, (pname, params), (F, "glFogiv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(FrontFace)(GLenum mode)
{
    (void) mode;
   DISPATCH(FrontFace, (mode), (F, "glFrontFace(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(Hint)(GLenum target, GLenum mode)
{
    (void) target; (void) mode;
   DISPATCH(Hint, (target, mode), (F, "glHint(0x%x, 0x%x);\n", target, mode));
}

KEYWORD1 void KEYWORD2 NAME(Lightf)(GLenum light, GLenum pname, GLfloat param)
{
    (void) light; (void) pname; (void) param;
   DISPATCH(Lightf, (light, pname, param), (F, "glLightf(0x%x, 0x%x, %f);\n", light, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Lightfv)(GLenum light, GLenum pname, const GLfloat * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(Lightfv, (light, pname, params), (F, "glLightfv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Lighti)(GLenum light, GLenum pname, GLint param)
{
    (void) light; (void) pname; (void) param;
   DISPATCH(Lighti, (light, pname, param), (F, "glLighti(0x%x, 0x%x, %d);\n", light, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Lightiv)(GLenum light, GLenum pname, const GLint * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(Lightiv, (light, pname, params), (F, "glLightiv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(LightModelf)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(LightModelf, (pname, param), (F, "glLightModelf(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(LightModelfv)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(LightModelfv, (pname, params), (F, "glLightModelfv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(LightModeli)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(LightModeli, (pname, param), (F, "glLightModeli(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(LightModeliv)(GLenum pname, const GLint * params)
{
    (void) pname; (void) params;
   DISPATCH(LightModeliv, (pname, params), (F, "glLightModeliv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(LineStipple)(GLint factor, GLushort pattern)
{
    (void) factor; (void) pattern;
   DISPATCH(LineStipple, (factor, pattern), (F, "glLineStipple(%d, %d);\n", factor, pattern));
}

KEYWORD1 void KEYWORD2 NAME(LineWidth)(GLfloat width)
{
    (void) width;
   DISPATCH(LineWidth, (width), (F, "glLineWidth(%f);\n", width));
}

KEYWORD1 void KEYWORD2 NAME(Materialf)(GLenum face, GLenum pname, GLfloat param)
{
    (void) face; (void) pname; (void) param;
   DISPATCH(Materialf, (face, pname, param), (F, "glMaterialf(0x%x, 0x%x, %f);\n", face, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Materialfv)(GLenum face, GLenum pname, const GLfloat * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(Materialfv, (face, pname, params), (F, "glMaterialfv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Materiali)(GLenum face, GLenum pname, GLint param)
{
    (void) face; (void) pname; (void) param;
   DISPATCH(Materiali, (face, pname, param), (F, "glMateriali(0x%x, 0x%x, %d);\n", face, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Materialiv)(GLenum face, GLenum pname, const GLint * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(Materialiv, (face, pname, params), (F, "glMaterialiv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(PointSize)(GLfloat size)
{
    (void) size;
   DISPATCH(PointSize, (size), (F, "glPointSize(%f);\n", size));
}

KEYWORD1 void KEYWORD2 NAME(PolygonMode)(GLenum face, GLenum mode)
{
    (void) face; (void) mode;
   DISPATCH(PolygonMode, (face, mode), (F, "glPolygonMode(0x%x, 0x%x);\n", face, mode));
}

KEYWORD1 void KEYWORD2 NAME(PolygonStipple)(const GLubyte * mask)
{
    (void) mask;
   DISPATCH(PolygonStipple, (mask), (F, "glPolygonStipple(%p);\n", (const void *) mask));
}

KEYWORD1 void KEYWORD2 NAME(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) x; (void) y; (void) width; (void) height;
   DISPATCH(Scissor, (x, y, width, height), (F, "glScissor(%d, %d, %d, %d);\n", x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(ShadeModel)(GLenum mode)
{
    (void) mode;
   DISPATCH(ShadeModel, (mode), (F, "glShadeModel(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexParameterf, (target, pname, param), (F, "glTexParameterf(0x%x, 0x%x, %f);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterfv, (target, pname, params), (F, "glTexParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexParameteri, (target, pname, param), (F, "glTexParameteri(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameteriv, (target, pname, params), (F, "glTexParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) border; (void) format; (void) type; (void) pixels;
   DISPATCH(TexImage1D, (target, level, internalformat, width, border, format, type, pixels), (F, "glTexImage1D(0x%x, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, internalformat, width, border, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) border; (void) format; (void) type; (void) pixels;
   DISPATCH(TexImage2D, (target, level, internalformat, width, height, border, format, type, pixels), (F, "glTexImage2D(0x%x, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, internalformat, width, height, border, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexEnvf)(GLenum target, GLenum pname, GLfloat param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexEnvf, (target, pname, param), (F, "glTexEnvf(0x%x, 0x%x, %f);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexEnvfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexEnvfv, (target, pname, params), (F, "glTexEnvfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexEnvi)(GLenum target, GLenum pname, GLint param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexEnvi, (target, pname, param), (F, "glTexEnvi(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexEnviv)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexEnviv, (target, pname, params), (F, "glTexEnviv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexGend)(GLenum coord, GLenum pname, GLdouble param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGend, (coord, pname, param), (F, "glTexGend(0x%x, 0x%x, %f);\n", coord, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexGendv)(GLenum coord, GLenum pname, const GLdouble * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGendv, (coord, pname, params), (F, "glTexGendv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexGenf)(GLenum coord, GLenum pname, GLfloat param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGenf, (coord, pname, param), (F, "glTexGenf(0x%x, 0x%x, %f);\n", coord, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_190)(GLenum coord, GLenum pname, GLfloat param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_190)(GLenum coord, GLenum pname, GLfloat param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGenf, (coord, pname, param), (F, "glTexGenfOES(0x%x, 0x%x, %f);\n", coord, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexGenfv)(GLenum coord, GLenum pname, const GLfloat * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGenfv, (coord, pname, params), (F, "glTexGenfv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_191)(GLenum coord, GLenum pname, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_191)(GLenum coord, GLenum pname, const GLfloat * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGenfv, (coord, pname, params), (F, "glTexGenfvOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexGeni)(GLenum coord, GLenum pname, GLint param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGeni, (coord, pname, param), (F, "glTexGeni(0x%x, 0x%x, %d);\n", coord, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_192)(GLenum coord, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_192)(GLenum coord, GLenum pname, GLint param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGeni, (coord, pname, param), (F, "glTexGeniOES(0x%x, 0x%x, %d);\n", coord, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexGeniv)(GLenum coord, GLenum pname, const GLint * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGeniv, (coord, pname, params), (F, "glTexGeniv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_193)(GLenum coord, GLenum pname, const GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_193)(GLenum coord, GLenum pname, const GLint * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGeniv, (coord, pname, params), (F, "glTexGenivOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(FeedbackBuffer)(GLsizei size, GLenum type, GLfloat * buffer)
{
    (void) size; (void) type; (void) buffer;
   DISPATCH(FeedbackBuffer, (size, type, buffer), (F, "glFeedbackBuffer(%d, 0x%x, %p);\n", size, type, (const void *) buffer));
}

KEYWORD1 void KEYWORD2 NAME(SelectBuffer)(GLsizei size, GLuint * buffer)
{
    (void) size; (void) buffer;
   DISPATCH(SelectBuffer, (size, buffer), (F, "glSelectBuffer(%d, %p);\n", size, (const void *) buffer));
}

KEYWORD1 GLint KEYWORD2 NAME(RenderMode)(GLenum mode)
{
    (void) mode;
   RETURN_DISPATCH(RenderMode, (mode), (F, "glRenderMode(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(InitNames)(void)
{
   DISPATCH(InitNames, (), (F, "glInitNames();\n"));
}

KEYWORD1 void KEYWORD2 NAME(LoadName)(GLuint name)
{
    (void) name;
   DISPATCH(LoadName, (name), (F, "glLoadName(%d);\n", name));
}

KEYWORD1 void KEYWORD2 NAME(PassThrough)(GLfloat token)
{
    (void) token;
   DISPATCH(PassThrough, (token), (F, "glPassThrough(%f);\n", token));
}

KEYWORD1 void KEYWORD2 NAME(PopName)(void)
{
   DISPATCH(PopName, (), (F, "glPopName();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PushName)(GLuint name)
{
    (void) name;
   DISPATCH(PushName, (name), (F, "glPushName(%d);\n", name));
}

KEYWORD1 void KEYWORD2 NAME(DrawBuffer)(GLenum mode)
{
    (void) mode;
   DISPATCH(DrawBuffer, (mode), (F, "glDrawBuffer(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(Clear)(GLbitfield mask)
{
    (void) mask;
   DISPATCH(Clear, (mask), (F, "glClear(%d);\n", mask));
}

KEYWORD1 void KEYWORD2 NAME(ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(ClearAccum, (red, green, blue, alpha), (F, "glClearAccum(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(ClearIndex)(GLfloat c)
{
    (void) c;
   DISPATCH(ClearIndex, (c), (F, "glClearIndex(%f);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(ClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(ClearColor, (red, green, blue, alpha), (F, "glClearColor(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(ClearStencil)(GLint s)
{
    (void) s;
   DISPATCH(ClearStencil, (s), (F, "glClearStencil(%d);\n", s));
}

KEYWORD1 void KEYWORD2 NAME(ClearDepth)(GLclampd depth)
{
    (void) depth;
   DISPATCH(ClearDepth, (depth), (F, "glClearDepth(%f);\n", depth));
}

KEYWORD1 void KEYWORD2 NAME(StencilMask)(GLuint mask)
{
    (void) mask;
   DISPATCH(StencilMask, (mask), (F, "glStencilMask(%d);\n", mask));
}

KEYWORD1 void KEYWORD2 NAME(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(ColorMask, (red, green, blue, alpha), (F, "glColorMask(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(DepthMask)(GLboolean flag)
{
    (void) flag;
   DISPATCH(DepthMask, (flag), (F, "glDepthMask(%d);\n", flag));
}

KEYWORD1 void KEYWORD2 NAME(IndexMask)(GLuint mask)
{
    (void) mask;
   DISPATCH(IndexMask, (mask), (F, "glIndexMask(%d);\n", mask));
}

KEYWORD1 void KEYWORD2 NAME(Accum)(GLenum op, GLfloat value)
{
    (void) op; (void) value;
   DISPATCH(Accum, (op, value), (F, "glAccum(0x%x, %f);\n", op, value));
}

KEYWORD1 void KEYWORD2 NAME(Disable)(GLenum cap)
{
    (void) cap;
   DISPATCH(Disable, (cap), (F, "glDisable(0x%x);\n", cap));
}

KEYWORD1 void KEYWORD2 NAME(Enable)(GLenum cap)
{
    (void) cap;
   DISPATCH(Enable, (cap), (F, "glEnable(0x%x);\n", cap));
}

KEYWORD1 void KEYWORD2 NAME(Finish)(void)
{
   DISPATCH(Finish, (), (F, "glFinish();\n"));
}

KEYWORD1 void KEYWORD2 NAME(Flush)(void)
{
   DISPATCH(Flush, (), (F, "glFlush();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PopAttrib)(void)
{
   DISPATCH(PopAttrib, (), (F, "glPopAttrib();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PushAttrib)(GLbitfield mask)
{
    (void) mask;
   DISPATCH(PushAttrib, (mask), (F, "glPushAttrib(%d);\n", mask));
}

KEYWORD1 void KEYWORD2 NAME(Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points)
{
    (void) target; (void) u1; (void) u2; (void) stride; (void) order; (void) points;
   DISPATCH(Map1d, (target, u1, u2, stride, order, points), (F, "glMap1d(0x%x, %f, %f, %d, %d, %p);\n", target, u1, u2, stride, order, (const void *) points));
}

KEYWORD1 void KEYWORD2 NAME(Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points)
{
    (void) target; (void) u1; (void) u2; (void) stride; (void) order; (void) points;
   DISPATCH(Map1f, (target, u1, u2, stride, order, points), (F, "glMap1f(0x%x, %f, %f, %d, %d, %p);\n", target, u1, u2, stride, order, (const void *) points));
}

KEYWORD1 void KEYWORD2 NAME(Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points)
{
    (void) target; (void) u1; (void) u2; (void) ustride; (void) uorder; (void) v1; (void) v2; (void) vstride; (void) vorder; (void) points;
   DISPATCH(Map2d, (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points), (F, "glMap2d(0x%x, %f, %f, %d, %d, %f, %f, %d, %d, %p);\n", target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, (const void *) points));
}

KEYWORD1 void KEYWORD2 NAME(Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points)
{
    (void) target; (void) u1; (void) u2; (void) ustride; (void) uorder; (void) v1; (void) v2; (void) vstride; (void) vorder; (void) points;
   DISPATCH(Map2f, (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points), (F, "glMap2f(0x%x, %f, %f, %d, %d, %f, %f, %d, %d, %p);\n", target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, (const void *) points));
}

KEYWORD1 void KEYWORD2 NAME(MapGrid1d)(GLint un, GLdouble u1, GLdouble u2)
{
    (void) un; (void) u1; (void) u2;
   DISPATCH(MapGrid1d, (un, u1, u2), (F, "glMapGrid1d(%d, %f, %f);\n", un, u1, u2));
}

KEYWORD1 void KEYWORD2 NAME(MapGrid1f)(GLint un, GLfloat u1, GLfloat u2)
{
    (void) un; (void) u1; (void) u2;
   DISPATCH(MapGrid1f, (un, u1, u2), (F, "glMapGrid1f(%d, %f, %f);\n", un, u1, u2));
}

KEYWORD1 void KEYWORD2 NAME(MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    (void) un; (void) u1; (void) u2; (void) vn; (void) v1; (void) v2;
   DISPATCH(MapGrid2d, (un, u1, u2, vn, v1, v2), (F, "glMapGrid2d(%d, %f, %f, %d, %f, %f);\n", un, u1, u2, vn, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    (void) un; (void) u1; (void) u2; (void) vn; (void) v1; (void) v2;
   DISPATCH(MapGrid2f, (un, u1, u2, vn, v1, v2), (F, "glMapGrid2f(%d, %f, %f, %d, %f, %f);\n", un, u1, u2, vn, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord1d)(GLdouble u)
{
    (void) u;
   DISPATCH(EvalCoord1d, (u), (F, "glEvalCoord1d(%f);\n", u));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord1dv)(const GLdouble * u)
{
    (void) u;
   DISPATCH(EvalCoord1dv, (u), (F, "glEvalCoord1dv(%p);\n", (const void *) u));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord1f)(GLfloat u)
{
    (void) u;
   DISPATCH(EvalCoord1f, (u), (F, "glEvalCoord1f(%f);\n", u));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord1fv)(const GLfloat * u)
{
    (void) u;
   DISPATCH(EvalCoord1fv, (u), (F, "glEvalCoord1fv(%p);\n", (const void *) u));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord2d)(GLdouble u, GLdouble v)
{
    (void) u; (void) v;
   DISPATCH(EvalCoord2d, (u, v), (F, "glEvalCoord2d(%f, %f);\n", u, v));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord2dv)(const GLdouble * u)
{
    (void) u;
   DISPATCH(EvalCoord2dv, (u), (F, "glEvalCoord2dv(%p);\n", (const void *) u));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord2f)(GLfloat u, GLfloat v)
{
    (void) u; (void) v;
   DISPATCH(EvalCoord2f, (u, v), (F, "glEvalCoord2f(%f, %f);\n", u, v));
}

KEYWORD1 void KEYWORD2 NAME(EvalCoord2fv)(const GLfloat * u)
{
    (void) u;
   DISPATCH(EvalCoord2fv, (u), (F, "glEvalCoord2fv(%p);\n", (const void *) u));
}

KEYWORD1 void KEYWORD2 NAME(EvalMesh1)(GLenum mode, GLint i1, GLint i2)
{
    (void) mode; (void) i1; (void) i2;
   DISPATCH(EvalMesh1, (mode, i1, i2), (F, "glEvalMesh1(0x%x, %d, %d);\n", mode, i1, i2));
}

KEYWORD1 void KEYWORD2 NAME(EvalPoint1)(GLint i)
{
    (void) i;
   DISPATCH(EvalPoint1, (i), (F, "glEvalPoint1(%d);\n", i));
}

KEYWORD1 void KEYWORD2 NAME(EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    (void) mode; (void) i1; (void) i2; (void) j1; (void) j2;
   DISPATCH(EvalMesh2, (mode, i1, i2, j1, j2), (F, "glEvalMesh2(0x%x, %d, %d, %d, %d);\n", mode, i1, i2, j1, j2));
}

KEYWORD1 void KEYWORD2 NAME(EvalPoint2)(GLint i, GLint j)
{
    (void) i; (void) j;
   DISPATCH(EvalPoint2, (i, j), (F, "glEvalPoint2(%d, %d);\n", i, j));
}

KEYWORD1 void KEYWORD2 NAME(AlphaFunc)(GLenum func, GLclampf ref)
{
    (void) func; (void) ref;
   DISPATCH(AlphaFunc, (func, ref), (F, "glAlphaFunc(0x%x, %f);\n", func, ref));
}

KEYWORD1 void KEYWORD2 NAME(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
    (void) sfactor; (void) dfactor;
   DISPATCH(BlendFunc, (sfactor, dfactor), (F, "glBlendFunc(0x%x, 0x%x);\n", sfactor, dfactor));
}

KEYWORD1 void KEYWORD2 NAME(LogicOp)(GLenum opcode)
{
    (void) opcode;
   DISPATCH(LogicOp, (opcode), (F, "glLogicOp(0x%x);\n", opcode));
}

KEYWORD1 void KEYWORD2 NAME(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    (void) func; (void) ref; (void) mask;
   DISPATCH(StencilFunc, (func, ref, mask), (F, "glStencilFunc(0x%x, %d, %d);\n", func, ref, mask));
}

KEYWORD1 void KEYWORD2 NAME(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    (void) fail; (void) zfail; (void) zpass;
   DISPATCH(StencilOp, (fail, zfail, zpass), (F, "glStencilOp(0x%x, 0x%x, 0x%x);\n", fail, zfail, zpass));
}

KEYWORD1 void KEYWORD2 NAME(DepthFunc)(GLenum func)
{
    (void) func;
   DISPATCH(DepthFunc, (func), (F, "glDepthFunc(0x%x);\n", func));
}

KEYWORD1 void KEYWORD2 NAME(PixelZoom)(GLfloat xfactor, GLfloat yfactor)
{
    (void) xfactor; (void) yfactor;
   DISPATCH(PixelZoom, (xfactor, yfactor), (F, "glPixelZoom(%f, %f);\n", xfactor, yfactor));
}

KEYWORD1 void KEYWORD2 NAME(PixelTransferf)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PixelTransferf, (pname, param), (F, "glPixelTransferf(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PixelTransferi)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(PixelTransferi, (pname, param), (F, "glPixelTransferi(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PixelStoref)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PixelStoref, (pname, param), (F, "glPixelStoref(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PixelStorei)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(PixelStorei, (pname, param), (F, "glPixelStorei(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat * values)
{
    (void) map; (void) mapsize; (void) values;
   DISPATCH(PixelMapfv, (map, mapsize, values), (F, "glPixelMapfv(0x%x, %d, %p);\n", map, mapsize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint * values)
{
    (void) map; (void) mapsize; (void) values;
   DISPATCH(PixelMapuiv, (map, mapsize, values), (F, "glPixelMapuiv(0x%x, %d, %p);\n", map, mapsize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort * values)
{
    (void) map; (void) mapsize; (void) values;
   DISPATCH(PixelMapusv, (map, mapsize, values), (F, "glPixelMapusv(0x%x, %d, %p);\n", map, mapsize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(ReadBuffer)(GLenum mode)
{
    (void) mode;
   DISPATCH(ReadBuffer, (mode), (F, "glReadBuffer(0x%x);\n", mode));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_254)(GLenum mode);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_254)(GLenum mode)
{
    (void) mode;
   DISPATCH(ReadBuffer, (mode), (F, "glReadBufferNV(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    (void) x; (void) y; (void) width; (void) height; (void) type;
   DISPATCH(CopyPixels, (x, y, width, height, type), (F, "glCopyPixels(%d, %d, %d, %d, 0x%x);\n", x, y, width, height, type));
}

KEYWORD1 void KEYWORD2 NAME(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels)
{
    (void) x; (void) y; (void) width; (void) height; (void) format; (void) type; (void) pixels;
   DISPATCH(ReadPixels, (x, y, width, height, format, type, pixels), (F, "glReadPixels(%d, %d, %d, %d, 0x%x, 0x%x, %p);\n", x, y, width, height, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) width; (void) height; (void) format; (void) type; (void) pixels;
   DISPATCH(DrawPixels, (width, height, format, type, pixels), (F, "glDrawPixels(%d, %d, 0x%x, 0x%x, %p);\n", width, height, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(GetBooleanv)(GLenum pname, GLboolean * params)
{
    (void) pname; (void) params;
   DISPATCH(GetBooleanv, (pname, params), (F, "glGetBooleanv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetClipPlane)(GLenum plane, GLdouble * equation)
{
    (void) plane; (void) equation;
   DISPATCH(GetClipPlane, (plane, equation), (F, "glGetClipPlane(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(GetDoublev)(GLenum pname, GLdouble * params)
{
    (void) pname; (void) params;
   DISPATCH(GetDoublev, (pname, params), (F, "glGetDoublev(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 GLenum KEYWORD2 NAME(GetError)(void)
{
   RETURN_DISPATCH(GetError, (), (F, "glGetError();\n"));
}

KEYWORD1 void KEYWORD2 NAME(GetFloatv)(GLenum pname, GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(GetFloatv, (pname, params), (F, "glGetFloatv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetIntegerv)(GLenum pname, GLint * params)
{
    (void) pname; (void) params;
   DISPATCH(GetIntegerv, (pname, params), (F, "glGetIntegerv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetLightfv)(GLenum light, GLenum pname, GLfloat * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(GetLightfv, (light, pname, params), (F, "glGetLightfv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetLightiv)(GLenum light, GLenum pname, GLint * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(GetLightiv, (light, pname, params), (F, "glGetLightiv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetMapdv)(GLenum target, GLenum query, GLdouble * v)
{
    (void) target; (void) query; (void) v;
   DISPATCH(GetMapdv, (target, query, v), (F, "glGetMapdv(0x%x, 0x%x, %p);\n", target, query, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetMapfv)(GLenum target, GLenum query, GLfloat * v)
{
    (void) target; (void) query; (void) v;
   DISPATCH(GetMapfv, (target, query, v), (F, "glGetMapfv(0x%x, 0x%x, %p);\n", target, query, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetMapiv)(GLenum target, GLenum query, GLint * v)
{
    (void) target; (void) query; (void) v;
   DISPATCH(GetMapiv, (target, query, v), (F, "glGetMapiv(0x%x, 0x%x, %p);\n", target, query, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetMaterialfv)(GLenum face, GLenum pname, GLfloat * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(GetMaterialfv, (face, pname, params), (F, "glGetMaterialfv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetMaterialiv)(GLenum face, GLenum pname, GLint * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(GetMaterialiv, (face, pname, params), (F, "glGetMaterialiv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetPixelMapfv)(GLenum map, GLfloat * values)
{
    (void) map; (void) values;
   DISPATCH(GetPixelMapfv, (map, values), (F, "glGetPixelMapfv(0x%x, %p);\n", map, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetPixelMapuiv)(GLenum map, GLuint * values)
{
    (void) map; (void) values;
   DISPATCH(GetPixelMapuiv, (map, values), (F, "glGetPixelMapuiv(0x%x, %p);\n", map, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetPixelMapusv)(GLenum map, GLushort * values)
{
    (void) map; (void) values;
   DISPATCH(GetPixelMapusv, (map, values), (F, "glGetPixelMapusv(0x%x, %p);\n", map, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetPolygonStipple)(GLubyte * mask)
{
    (void) mask;
   DISPATCH(GetPolygonStipple, (mask), (F, "glGetPolygonStipple(%p);\n", (const void *) mask));
}

KEYWORD1 const GLubyte * KEYWORD2 NAME(GetString)(GLenum name)
{
    (void) name;
   RETURN_DISPATCH(GetString, (name), (F, "glGetString(0x%x);\n", name));
}

KEYWORD1 void KEYWORD2 NAME(GetTexEnvfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexEnvfv, (target, pname, params), (F, "glGetTexEnvfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexEnviv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexEnviv, (target, pname, params), (F, "glGetTexEnviv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexGendv)(GLenum coord, GLenum pname, GLdouble * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGendv, (coord, pname, params), (F, "glGetTexGendv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexGenfv)(GLenum coord, GLenum pname, GLfloat * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGenfv, (coord, pname, params), (F, "glGetTexGenfv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_279)(GLenum coord, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_279)(GLenum coord, GLenum pname, GLfloat * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGenfv, (coord, pname, params), (F, "glGetTexGenfvOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexGeniv)(GLenum coord, GLenum pname, GLint * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGeniv, (coord, pname, params), (F, "glGetTexGeniv(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_280)(GLenum coord, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_280)(GLenum coord, GLenum pname, GLint * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGeniv, (coord, pname, params), (F, "glGetTexGenivOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels)
{
    (void) target; (void) level; (void) format; (void) type; (void) pixels;
   DISPATCH(GetTexImage, (target, level, format, type, pixels), (F, "glGetTexImage(0x%x, %d, 0x%x, 0x%x, %p);\n", target, level, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterfv, (target, pname, params), (F, "glGetTexParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameteriv, (target, pname, params), (F, "glGetTexParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params)
{
    (void) target; (void) level; (void) pname; (void) params;
   DISPATCH(GetTexLevelParameterfv, (target, level, pname, params), (F, "glGetTexLevelParameterfv(0x%x, %d, 0x%x, %p);\n", target, level, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params)
{
    (void) target; (void) level; (void) pname; (void) params;
   DISPATCH(GetTexLevelParameteriv, (target, level, pname, params), (F, "glGetTexLevelParameteriv(0x%x, %d, 0x%x, %p);\n", target, level, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsEnabled)(GLenum cap)
{
    (void) cap;
   RETURN_DISPATCH(IsEnabled, (cap), (F, "glIsEnabled(0x%x);\n", cap));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsList)(GLuint list)
{
    (void) list;
   RETURN_DISPATCH(IsList, (list), (F, "glIsList(%d);\n", list));
}

KEYWORD1 void KEYWORD2 NAME(DepthRange)(GLclampd zNear, GLclampd zFar)
{
    (void) zNear; (void) zFar;
   DISPATCH(DepthRange, (zNear, zFar), (F, "glDepthRange(%f, %f);\n", zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Frustum, (left, right, bottom, top, zNear, zFar), (F, "glFrustum(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(LoadIdentity)(void)
{
   DISPATCH(LoadIdentity, (), (F, "glLoadIdentity();\n"));
}

KEYWORD1 void KEYWORD2 NAME(LoadMatrixf)(const GLfloat * m)
{
    (void) m;
   DISPATCH(LoadMatrixf, (m), (F, "glLoadMatrixf(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(LoadMatrixd)(const GLdouble * m)
{
    (void) m;
   DISPATCH(LoadMatrixd, (m), (F, "glLoadMatrixd(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MatrixMode)(GLenum mode)
{
    (void) mode;
   DISPATCH(MatrixMode, (mode), (F, "glMatrixMode(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(MultMatrixf)(const GLfloat * m)
{
    (void) m;
   DISPATCH(MultMatrixf, (m), (F, "glMultMatrixf(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultMatrixd)(const GLdouble * m)
{
    (void) m;
   DISPATCH(MultMatrixd, (m), (F, "glMultMatrixd(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Ortho, (left, right, bottom, top, zNear, zFar), (F, "glOrtho(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(PopMatrix)(void)
{
   DISPATCH(PopMatrix, (), (F, "glPopMatrix();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PushMatrix)(void)
{
   DISPATCH(PushMatrix, (), (F, "glPushMatrix();\n"));
}

KEYWORD1 void KEYWORD2 NAME(Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    (void) angle; (void) x; (void) y; (void) z;
   DISPATCH(Rotated, (angle, x, y, z), (F, "glRotated(%f, %f, %f, %f);\n", angle, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    (void) angle; (void) x; (void) y; (void) z;
   DISPATCH(Rotatef, (angle, x, y, z), (F, "glRotatef(%f, %f, %f, %f);\n", angle, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Scaled)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Scaled, (x, y, z), (F, "glScaled(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Scalef)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Scalef, (x, y, z), (F, "glScalef(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Translated)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Translated, (x, y, z), (F, "glTranslated(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Translatef)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Translatef, (x, y, z), (F, "glTranslatef(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) x; (void) y; (void) width; (void) height;
   DISPATCH(Viewport, (x, y, width, height), (F, "glViewport(%d, %d, %d, %d);\n", x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(ArrayElement)(GLint i)
{
    (void) i;
   DISPATCH(ArrayElement, (i), (F, "glArrayElement(%d);\n", i));
}

KEYWORD1 void KEYWORD2 NAME(ArrayElementEXT)(GLint i)
{
    (void) i;
   DISPATCH(ArrayElement, (i), (F, "glArrayElementEXT(%d);\n", i));
}

KEYWORD1 void KEYWORD2 NAME(BindTexture)(GLenum target, GLuint texture)
{
    (void) target; (void) texture;
   DISPATCH(BindTexture, (target, texture), (F, "glBindTexture(0x%x, %d);\n", target, texture));
}

KEYWORD1 void KEYWORD2 NAME(BindTextureEXT)(GLenum target, GLuint texture)
{
    (void) target; (void) texture;
   DISPATCH(BindTexture, (target, texture), (F, "glBindTextureEXT(0x%x, %d);\n", target, texture));
}

KEYWORD1 void KEYWORD2 NAME(ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(ColorPointer, (size, type, stride, pointer), (F, "glColorPointer(%d, 0x%x, %d, %p);\n", size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(DisableClientState)(GLenum array)
{
    (void) array;
   DISPATCH(DisableClientState, (array), (F, "glDisableClientState(0x%x);\n", array));
}

KEYWORD1 void KEYWORD2 NAME(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    (void) mode; (void) first; (void) count;
   DISPATCH(DrawArrays, (mode, first, count), (F, "glDrawArrays(0x%x, %d, %d);\n", mode, first, count));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysEXT)(GLenum mode, GLint first, GLsizei count)
{
    (void) mode; (void) first; (void) count;
   DISPATCH(DrawArrays, (mode, first, count), (F, "glDrawArraysEXT(0x%x, %d, %d);\n", mode, first, count));
}

KEYWORD1 void KEYWORD2 NAME(DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    (void) mode; (void) count; (void) type; (void) indices;
   DISPATCH(DrawElements, (mode, count, type, indices), (F, "glDrawElements(0x%x, %d, 0x%x, %p);\n", mode, count, type, (const void *) indices));
}

KEYWORD1 void KEYWORD2 NAME(EdgeFlagPointer)(GLsizei stride, const GLvoid * pointer)
{
    (void) stride; (void) pointer;
   DISPATCH(EdgeFlagPointer, (stride, pointer), (F, "glEdgeFlagPointer(%d, %p);\n", stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(EnableClientState)(GLenum array)
{
    (void) array;
   DISPATCH(EnableClientState, (array), (F, "glEnableClientState(0x%x);\n", array));
}

KEYWORD1 void KEYWORD2 NAME(IndexPointer)(GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) pointer;
   DISPATCH(IndexPointer, (type, stride, pointer), (F, "glIndexPointer(0x%x, %d, %p);\n", type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(Indexub)(GLubyte c)
{
    (void) c;
   DISPATCH(Indexub, (c), (F, "glIndexub(%d);\n", c));
}

KEYWORD1 void KEYWORD2 NAME(Indexubv)(const GLubyte * c)
{
    (void) c;
   DISPATCH(Indexubv, (c), (F, "glIndexubv(%p);\n", (const void *) c));
}

KEYWORD1 void KEYWORD2 NAME(InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid * pointer)
{
    (void) format; (void) stride; (void) pointer;
   DISPATCH(InterleavedArrays, (format, stride, pointer), (F, "glInterleavedArrays(0x%x, %d, %p);\n", format, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(NormalPointer)(GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) pointer;
   DISPATCH(NormalPointer, (type, stride, pointer), (F, "glNormalPointer(0x%x, %d, %p);\n", type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(PolygonOffset)(GLfloat factor, GLfloat units)
{
    (void) factor; (void) units;
   DISPATCH(PolygonOffset, (factor, units), (F, "glPolygonOffset(%f, %f);\n", factor, units));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(TexCoordPointer, (size, type, stride, pointer), (F, "glTexCoordPointer(%d, 0x%x, %d, %p);\n", size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(VertexPointer, (size, type, stride, pointer), (F, "glVertexPointer(%d, 0x%x, %d, %p);\n", size, type, stride, (const void *) pointer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(AreTexturesResident)(GLsizei n, const GLuint * textures, GLboolean * residences)
{
    (void) n; (void) textures; (void) residences;
   RETURN_DISPATCH(AreTexturesResident, (n, textures, residences), (F, "glAreTexturesResident(%d, %p, %p);\n", n, (const void *) textures, (const void *) residences));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    (void) target; (void) level; (void) internalformat; (void) x; (void) y; (void) width; (void) border;
   DISPATCH(CopyTexImage1D, (target, level, internalformat, x, y, width, border), (F, "glCopyTexImage1D(0x%x, %d, 0x%x, %d, %d, %d, %d);\n", target, level, internalformat, x, y, width, border));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_323)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_323)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    (void) target; (void) level; (void) internalformat; (void) x; (void) y; (void) width; (void) border;
   DISPATCH(CopyTexImage1D, (target, level, internalformat, x, y, width, border), (F, "glCopyTexImage1DEXT(0x%x, %d, 0x%x, %d, %d, %d, %d);\n", target, level, internalformat, x, y, width, border));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    (void) target; (void) level; (void) internalformat; (void) x; (void) y; (void) width; (void) height; (void) border;
   DISPATCH(CopyTexImage2D, (target, level, internalformat, x, y, width, height, border), (F, "glCopyTexImage2D(0x%x, %d, 0x%x, %d, %d, %d, %d, %d);\n", target, level, internalformat, x, y, width, height, border));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_324)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_324)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    (void) target; (void) level; (void) internalformat; (void) x; (void) y; (void) width; (void) height; (void) border;
   DISPATCH(CopyTexImage2D, (target, level, internalformat, x, y, width, height, border), (F, "glCopyTexImage2DEXT(0x%x, %d, 0x%x, %d, %d, %d, %d, %d);\n", target, level, internalformat, x, y, width, height, border));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) level; (void) xoffset; (void) x; (void) y; (void) width;
   DISPATCH(CopyTexSubImage1D, (target, level, xoffset, x, y, width), (F, "glCopyTexSubImage1D(0x%x, %d, %d, %d, %d, %d);\n", target, level, xoffset, x, y, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_325)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_325)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) level; (void) xoffset; (void) x; (void) y; (void) width;
   DISPATCH(CopyTexSubImage1D, (target, level, xoffset, x, y, width), (F, "glCopyTexSubImage1DEXT(0x%x, %d, %d, %d, %d, %d);\n", target, level, xoffset, x, y, width));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTexSubImage2D, (target, level, xoffset, yoffset, x, y, width, height), (F, "glCopyTexSubImage2D(0x%x, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, x, y, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_326)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_326)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTexSubImage2D, (target, level, xoffset, yoffset, x, y, width, height), (F, "glCopyTexSubImage2DEXT(0x%x, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(DeleteTextures)(GLsizei n, const GLuint * textures)
{
    (void) n; (void) textures;
   DISPATCH(DeleteTextures, (n, textures), (F, "glDeleteTextures(%d, %p);\n", n, (const void *) textures));
}

KEYWORD1 void KEYWORD2 NAME(GenTextures)(GLsizei n, GLuint * textures)
{
    (void) n; (void) textures;
   DISPATCH(GenTextures, (n, textures), (F, "glGenTextures(%d, %p);\n", n, (const void *) textures));
}

KEYWORD1 void KEYWORD2 NAME(GetPointerv)(GLenum pname, GLvoid ** params)
{
    (void) pname; (void) params;
   DISPATCH(GetPointerv, (pname, params), (F, "glGetPointerv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetPointervEXT)(GLenum pname, GLvoid ** params)
{
    (void) pname; (void) params;
   DISPATCH(GetPointerv, (pname, params), (F, "glGetPointervEXT(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_329)(GLenum pname, GLvoid ** params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_329)(GLenum pname, GLvoid ** params)
{
    (void) pname; (void) params;
   DISPATCH(GetPointerv, (pname, params), (F, "glGetPointervKHR(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsTexture)(GLuint texture)
{
    (void) texture;
   RETURN_DISPATCH(IsTexture, (texture), (F, "glIsTexture(%d);\n", texture));
}

KEYWORD1 void KEYWORD2 NAME(PrioritizeTextures)(GLsizei n, const GLuint * textures, const GLclampf * priorities)
{
    (void) n; (void) textures; (void) priorities;
   DISPATCH(PrioritizeTextures, (n, textures, priorities), (F, "glPrioritizeTextures(%d, %p, %p);\n", n, (const void *) textures, (const void *) priorities));
}

KEYWORD1 void KEYWORD2 NAME(PrioritizeTexturesEXT)(GLsizei n, const GLuint * textures, const GLclampf * priorities)
{
    (void) n; (void) textures; (void) priorities;
   DISPATCH(PrioritizeTextures, (n, textures, priorities), (F, "glPrioritizeTexturesEXT(%d, %p, %p);\n", n, (const void *) textures, (const void *) priorities));
}

KEYWORD1 void KEYWORD2 NAME(TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) width; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage1D, (target, level, xoffset, width, format, type, pixels), (F, "glTexSubImage1D(0x%x, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, width, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_332)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_332)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) width; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage1D, (target, level, xoffset, width, format, type, pixels), (F, "glTexSubImage1DEXT(0x%x, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, width, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage2D, (target, level, xoffset, yoffset, width, height, format, type, pixels), (F, "glTexSubImage2D(0x%x, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, yoffset, width, height, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_333)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_333)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage2D, (target, level, xoffset, yoffset, width, height, format, type, pixels), (F, "glTexSubImage2DEXT(0x%x, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, yoffset, width, height, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(PopClientAttrib)(void)
{
   DISPATCH(PopClientAttrib, (), (F, "glPopClientAttrib();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PushClientAttrib)(GLbitfield mask)
{
    (void) mask;
   DISPATCH(PushClientAttrib, (mask), (F, "glPushClientAttrib(%d);\n", mask));
}

KEYWORD1 void KEYWORD2 NAME(BlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(BlendColor, (red, green, blue, alpha), (F, "glBlendColor(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(BlendColorEXT)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(BlendColor, (red, green, blue, alpha), (F, "glBlendColorEXT(%f, %f, %f, %f);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(BlendEquation)(GLenum mode)
{
    (void) mode;
   DISPATCH(BlendEquation, (mode), (F, "glBlendEquation(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(BlendEquationEXT)(GLenum mode)
{
    (void) mode;
   DISPATCH(BlendEquation, (mode), (F, "glBlendEquationEXT(0x%x);\n", mode));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_337)(GLenum mode);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_337)(GLenum mode)
{
    (void) mode;
   DISPATCH(BlendEquation, (mode), (F, "glBlendEquationOES(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
    (void) mode; (void) start; (void) end; (void) count; (void) type; (void) indices;
   DISPATCH(DrawRangeElements, (mode, start, end, count, type, indices), (F, "glDrawRangeElements(0x%x, %d, %d, %d, 0x%x, %p);\n", mode, start, end, count, type, (const void *) indices));
}

KEYWORD1 void KEYWORD2 NAME(DrawRangeElementsEXT)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
    (void) mode; (void) start; (void) end; (void) count; (void) type; (void) indices;
   DISPATCH(DrawRangeElements, (mode, start, end, count, type, indices), (F, "glDrawRangeElementsEXT(0x%x, %d, %d, %d, 0x%x, %p);\n", mode, start, end, count, type, (const void *) indices));
}

KEYWORD1 void KEYWORD2 NAME(ColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table)
{
    (void) target; (void) internalformat; (void) width; (void) format; (void) type; (void) table;
   DISPATCH(ColorTable, (target, internalformat, width, format, type, table), (F, "glColorTable(0x%x, 0x%x, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, format, type, (const void *) table));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_339)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_339)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table)
{
    (void) target; (void) internalformat; (void) width; (void) format; (void) type; (void) table;
   DISPATCH(ColorTable, (target, internalformat, width, format, type, table), (F, "glColorTableSGI(0x%x, 0x%x, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, format, type, (const void *) table));
}

KEYWORD1 void KEYWORD2 NAME(ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ColorTableParameterfv, (target, pname, params), (F, "glColorTableParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_340)(GLenum target, GLenum pname, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_340)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ColorTableParameterfv, (target, pname, params), (F, "glColorTableParameterfvSGI(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ColorTableParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ColorTableParameteriv, (target, pname, params), (F, "glColorTableParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_341)(GLenum target, GLenum pname, const GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_341)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ColorTableParameteriv, (target, pname, params), (F, "glColorTableParameterivSGI(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(CopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width;
   DISPATCH(CopyColorTable, (target, internalformat, x, y, width), (F, "glCopyColorTable(0x%x, 0x%x, %d, %d, %d);\n", target, internalformat, x, y, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_342)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_342)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width;
   DISPATCH(CopyColorTable, (target, internalformat, x, y, width), (F, "glCopyColorTableSGI(0x%x, 0x%x, %d, %d, %d);\n", target, internalformat, x, y, width));
}

KEYWORD1 void KEYWORD2 NAME(GetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid * table)
{
    (void) target; (void) format; (void) type; (void) table;
   DISPATCH(GetColorTable, (target, format, type, table), (F, "glGetColorTable(0x%x, 0x%x, 0x%x, %p);\n", target, format, type, (const void *) table));
}

KEYWORD1 void KEYWORD2 NAME(GetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetColorTableParameterfv, (target, pname, params), (F, "glGetColorTableParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetColorTableParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetColorTableParameteriv, (target, pname, params), (F, "glGetColorTableParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data)
{
    (void) target; (void) start; (void) count; (void) format; (void) type; (void) data;
   DISPATCH(ColorSubTable, (target, start, count, format, type, data), (F, "glColorSubTable(0x%x, %d, %d, 0x%x, 0x%x, %p);\n", target, start, count, format, type, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_346)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_346)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data)
{
    (void) target; (void) start; (void) count; (void) format; (void) type; (void) data;
   DISPATCH(ColorSubTable, (target, start, count, format, type, data), (F, "glColorSubTableEXT(0x%x, %d, %d, 0x%x, 0x%x, %p);\n", target, start, count, format, type, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) start; (void) x; (void) y; (void) width;
   DISPATCH(CopyColorSubTable, (target, start, x, y, width), (F, "glCopyColorSubTable(0x%x, %d, %d, %d, %d);\n", target, start, x, y, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_347)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_347)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) start; (void) x; (void) y; (void) width;
   DISPATCH(CopyColorSubTable, (target, start, x, y, width), (F, "glCopyColorSubTableEXT(0x%x, %d, %d, %d, %d);\n", target, start, x, y, width));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image)
{
    (void) target; (void) internalformat; (void) width; (void) format; (void) type; (void) image;
   DISPATCH(ConvolutionFilter1D, (target, internalformat, width, format, type, image), (F, "glConvolutionFilter1D(0x%x, 0x%x, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, format, type, (const void *) image));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_348)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_348)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image)
{
    (void) target; (void) internalformat; (void) width; (void) format; (void) type; (void) image;
   DISPATCH(ConvolutionFilter1D, (target, internalformat, width, format, type, image), (F, "glConvolutionFilter1DEXT(0x%x, 0x%x, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, format, type, (const void *) image));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image)
{
    (void) target; (void) internalformat; (void) width; (void) height; (void) format; (void) type; (void) image;
   DISPATCH(ConvolutionFilter2D, (target, internalformat, width, height, format, type, image), (F, "glConvolutionFilter2D(0x%x, 0x%x, %d, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, height, format, type, (const void *) image));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_349)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_349)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image)
{
    (void) target; (void) internalformat; (void) width; (void) height; (void) format; (void) type; (void) image;
   DISPATCH(ConvolutionFilter2D, (target, internalformat, width, height, format, type, image), (F, "glConvolutionFilter2DEXT(0x%x, 0x%x, %d, %d, 0x%x, 0x%x, %p);\n", target, internalformat, width, height, format, type, (const void *) image));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionParameterf)(GLenum target, GLenum pname, GLfloat params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameterf, (target, pname, params), (F, "glConvolutionParameterf(0x%x, 0x%x, %f);\n", target, pname, params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_350)(GLenum target, GLenum pname, GLfloat params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_350)(GLenum target, GLenum pname, GLfloat params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameterf, (target, pname, params), (F, "glConvolutionParameterfEXT(0x%x, 0x%x, %f);\n", target, pname, params));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameterfv, (target, pname, params), (F, "glConvolutionParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_351)(GLenum target, GLenum pname, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_351)(GLenum target, GLenum pname, const GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameterfv, (target, pname, params), (F, "glConvolutionParameterfvEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionParameteri)(GLenum target, GLenum pname, GLint params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameteri, (target, pname, params), (F, "glConvolutionParameteri(0x%x, 0x%x, %d);\n", target, pname, params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_352)(GLenum target, GLenum pname, GLint params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_352)(GLenum target, GLenum pname, GLint params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameteri, (target, pname, params), (F, "glConvolutionParameteriEXT(0x%x, 0x%x, %d);\n", target, pname, params));
}

KEYWORD1 void KEYWORD2 NAME(ConvolutionParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameteriv, (target, pname, params), (F, "glConvolutionParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_353)(GLenum target, GLenum pname, const GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_353)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(ConvolutionParameteriv, (target, pname, params), (F, "glConvolutionParameterivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(CopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width;
   DISPATCH(CopyConvolutionFilter1D, (target, internalformat, x, y, width), (F, "glCopyConvolutionFilter1D(0x%x, 0x%x, %d, %d, %d);\n", target, internalformat, x, y, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_354)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_354)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width;
   DISPATCH(CopyConvolutionFilter1D, (target, internalformat, x, y, width), (F, "glCopyConvolutionFilter1DEXT(0x%x, 0x%x, %d, %d, %d);\n", target, internalformat, x, y, width));
}

KEYWORD1 void KEYWORD2 NAME(CopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyConvolutionFilter2D, (target, internalformat, x, y, width, height), (F, "glCopyConvolutionFilter2D(0x%x, 0x%x, %d, %d, %d, %d);\n", target, internalformat, x, y, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_355)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_355)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) internalformat; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyConvolutionFilter2D, (target, internalformat, x, y, width, height), (F, "glCopyConvolutionFilter2DEXT(0x%x, 0x%x, %d, %d, %d, %d);\n", target, internalformat, x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(GetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid * image)
{
    (void) target; (void) format; (void) type; (void) image;
   DISPATCH(GetConvolutionFilter, (target, format, type, image), (F, "glGetConvolutionFilter(0x%x, 0x%x, 0x%x, %p);\n", target, format, type, (const void *) image));
}

KEYWORD1 void KEYWORD2 NAME(GetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetConvolutionParameterfv, (target, pname, params), (F, "glGetConvolutionParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetConvolutionParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetConvolutionParameteriv, (target, pname, params), (F, "glGetConvolutionParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span)
{
    (void) target; (void) format; (void) type; (void) row; (void) column; (void) span;
   DISPATCH(GetSeparableFilter, (target, format, type, row, column, span), (F, "glGetSeparableFilter(0x%x, 0x%x, 0x%x, %p, %p, %p);\n", target, format, type, (const void *) row, (const void *) column, (const void *) span));
}

KEYWORD1 void KEYWORD2 NAME(SeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column)
{
    (void) target; (void) internalformat; (void) width; (void) height; (void) format; (void) type; (void) row; (void) column;
   DISPATCH(SeparableFilter2D, (target, internalformat, width, height, format, type, row, column), (F, "glSeparableFilter2D(0x%x, 0x%x, %d, %d, 0x%x, 0x%x, %p, %p);\n", target, internalformat, width, height, format, type, (const void *) row, (const void *) column));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_360)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_360)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column)
{
    (void) target; (void) internalformat; (void) width; (void) height; (void) format; (void) type; (void) row; (void) column;
   DISPATCH(SeparableFilter2D, (target, internalformat, width, height, format, type, row, column), (F, "glSeparableFilter2DEXT(0x%x, 0x%x, %d, %d, 0x%x, 0x%x, %p, %p);\n", target, internalformat, width, height, format, type, (const void *) row, (const void *) column));
}

KEYWORD1 void KEYWORD2 NAME(GetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) values;
   DISPATCH(GetHistogram, (target, reset, format, type, values), (F, "glGetHistogram(0x%x, %d, 0x%x, 0x%x, %p);\n", target, reset, format, type, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetHistogramParameterfv, (target, pname, params), (F, "glGetHistogramParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetHistogramParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetHistogramParameteriv, (target, pname, params), (F, "glGetHistogramParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) values;
   DISPATCH(GetMinmax, (target, reset, format, type, values), (F, "glGetMinmax(0x%x, %d, 0x%x, 0x%x, %p);\n", target, reset, format, type, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetMinmaxParameterfv, (target, pname, params), (F, "glGetMinmaxParameterfv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetMinmaxParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetMinmaxParameteriv, (target, pname, params), (F, "glGetMinmaxParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Histogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    (void) target; (void) width; (void) internalformat; (void) sink;
   DISPATCH(Histogram, (target, width, internalformat, sink), (F, "glHistogram(0x%x, %d, 0x%x, %d);\n", target, width, internalformat, sink));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_367)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_367)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    (void) target; (void) width; (void) internalformat; (void) sink;
   DISPATCH(Histogram, (target, width, internalformat, sink), (F, "glHistogramEXT(0x%x, %d, 0x%x, %d);\n", target, width, internalformat, sink));
}

KEYWORD1 void KEYWORD2 NAME(Minmax)(GLenum target, GLenum internalformat, GLboolean sink)
{
    (void) target; (void) internalformat; (void) sink;
   DISPATCH(Minmax, (target, internalformat, sink), (F, "glMinmax(0x%x, 0x%x, %d);\n", target, internalformat, sink));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_368)(GLenum target, GLenum internalformat, GLboolean sink);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_368)(GLenum target, GLenum internalformat, GLboolean sink)
{
    (void) target; (void) internalformat; (void) sink;
   DISPATCH(Minmax, (target, internalformat, sink), (F, "glMinmaxEXT(0x%x, 0x%x, %d);\n", target, internalformat, sink));
}

KEYWORD1 void KEYWORD2 NAME(ResetHistogram)(GLenum target)
{
    (void) target;
   DISPATCH(ResetHistogram, (target), (F, "glResetHistogram(0x%x);\n", target));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_369)(GLenum target);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_369)(GLenum target)
{
    (void) target;
   DISPATCH(ResetHistogram, (target), (F, "glResetHistogramEXT(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(ResetMinmax)(GLenum target)
{
    (void) target;
   DISPATCH(ResetMinmax, (target), (F, "glResetMinmax(0x%x);\n", target));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_370)(GLenum target);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_370)(GLenum target)
{
    (void) target;
   DISPATCH(ResetMinmax, (target), (F, "glResetMinmaxEXT(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) format; (void) type; (void) pixels;
   DISPATCH(TexImage3D, (target, level, internalformat, width, height, depth, border, format, type, pixels), (F, "glTexImage3D(0x%x, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, internalformat, width, height, depth, border, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexImage3DEXT)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) format; (void) type; (void) pixels;
   DISPATCH(TexImage3D, (target, level, internalformat, width, height, depth, border, format, type, pixels), (F, "glTexImage3DEXT(0x%x, %d, 0x%x, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, internalformat, width, height, depth, border, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_371)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_371)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) format; (void) type; (void) pixels;
   DISPATCH(TexImage3D, (target, level, internalformat, width, height, depth, border, format, type, pixels), (F, "glTexImage3DOES(0x%x, %d, 0x%x, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, internalformat, width, height, depth, border, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels), (F, "glTexSubImage3D(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(TexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels), (F, "glTexSubImage3DEXT(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_372)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_372)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) pixels;
   DISPATCH(TexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels), (F, "glTexSubImage3DOES(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTexSubImage3D, (target, level, xoffset, yoffset, zoffset, x, y, width, height), (F, "glCopyTexSubImage3D(0x%x, %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, zoffset, x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(CopyTexSubImage3DEXT)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTexSubImage3D, (target, level, xoffset, yoffset, zoffset, x, y, width, height), (F, "glCopyTexSubImage3DEXT(0x%x, %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, zoffset, x, y, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_373)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_373)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTexSubImage3D, (target, level, xoffset, yoffset, zoffset, x, y, width, height), (F, "glCopyTexSubImage3DOES(0x%x, %d, %d, %d, %d, %d, %d, %d, %d);\n", target, level, xoffset, yoffset, zoffset, x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(ActiveTexture)(GLenum texture)
{
    (void) texture;
   DISPATCH(ActiveTexture, (texture), (F, "glActiveTexture(0x%x);\n", texture));
}

KEYWORD1 void KEYWORD2 NAME(ActiveTextureARB)(GLenum texture)
{
    (void) texture;
   DISPATCH(ActiveTexture, (texture), (F, "glActiveTextureARB(0x%x);\n", texture));
}

KEYWORD1 void KEYWORD2 NAME(ClientActiveTexture)(GLenum texture)
{
    (void) texture;
   DISPATCH(ClientActiveTexture, (texture), (F, "glClientActiveTexture(0x%x);\n", texture));
}

KEYWORD1 void KEYWORD2 NAME(ClientActiveTextureARB)(GLenum texture)
{
    (void) texture;
   DISPATCH(ClientActiveTexture, (texture), (F, "glClientActiveTextureARB(0x%x);\n", texture));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1d)(GLenum target, GLdouble s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1d, (target, s), (F, "glMultiTexCoord1d(0x%x, %f);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1dARB)(GLenum target, GLdouble s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1d, (target, s), (F, "glMultiTexCoord1dARB(0x%x, %f);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1dv)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1dv, (target, v), (F, "glMultiTexCoord1dv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1dvARB)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1dv, (target, v), (F, "glMultiTexCoord1dvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1f)(GLenum target, GLfloat s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1fARB, (target, s), (F, "glMultiTexCoord1f(0x%x, %f);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1fARB)(GLenum target, GLfloat s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1fARB, (target, s), (F, "glMultiTexCoord1fARB(0x%x, %f);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1fv)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1fvARB, (target, v), (F, "glMultiTexCoord1fv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1fvARB)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1fvARB, (target, v), (F, "glMultiTexCoord1fvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1i)(GLenum target, GLint s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1i, (target, s), (F, "glMultiTexCoord1i(0x%x, %d);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1iARB)(GLenum target, GLint s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1i, (target, s), (F, "glMultiTexCoord1iARB(0x%x, %d);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1iv)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1iv, (target, v), (F, "glMultiTexCoord1iv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1ivARB)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1iv, (target, v), (F, "glMultiTexCoord1ivARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1s)(GLenum target, GLshort s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1s, (target, s), (F, "glMultiTexCoord1s(0x%x, %d);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1sARB)(GLenum target, GLshort s)
{
    (void) target; (void) s;
   DISPATCH(MultiTexCoord1s, (target, s), (F, "glMultiTexCoord1sARB(0x%x, %d);\n", target, s));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1sv)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1sv, (target, v), (F, "glMultiTexCoord1sv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord1svARB)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord1sv, (target, v), (F, "glMultiTexCoord1svARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2d, (target, s, t), (F, "glMultiTexCoord2d(0x%x, %f, %f);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2d, (target, s, t), (F, "glMultiTexCoord2dARB(0x%x, %f, %f);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2dv)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2dv, (target, v), (F, "glMultiTexCoord2dv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2dvARB)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2dv, (target, v), (F, "glMultiTexCoord2dvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2fARB, (target, s, t), (F, "glMultiTexCoord2f(0x%x, %f, %f);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2fARB, (target, s, t), (F, "glMultiTexCoord2fARB(0x%x, %f, %f);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2fv)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2fvARB, (target, v), (F, "glMultiTexCoord2fv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2fvARB)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2fvARB, (target, v), (F, "glMultiTexCoord2fvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2i)(GLenum target, GLint s, GLint t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2i, (target, s, t), (F, "glMultiTexCoord2i(0x%x, %d, %d);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2iARB)(GLenum target, GLint s, GLint t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2i, (target, s, t), (F, "glMultiTexCoord2iARB(0x%x, %d, %d);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2iv)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2iv, (target, v), (F, "glMultiTexCoord2iv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2ivARB)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2iv, (target, v), (F, "glMultiTexCoord2ivARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2s)(GLenum target, GLshort s, GLshort t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2s, (target, s, t), (F, "glMultiTexCoord2s(0x%x, %d, %d);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t)
{
    (void) target; (void) s; (void) t;
   DISPATCH(MultiTexCoord2s, (target, s, t), (F, "glMultiTexCoord2sARB(0x%x, %d, %d);\n", target, s, t));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2sv)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2sv, (target, v), (F, "glMultiTexCoord2sv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord2svARB)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord2sv, (target, v), (F, "glMultiTexCoord2svARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3d, (target, s, t, r), (F, "glMultiTexCoord3d(0x%x, %f, %f, %f);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3d, (target, s, t, r), (F, "glMultiTexCoord3dARB(0x%x, %f, %f, %f);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3dv)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3dv, (target, v), (F, "glMultiTexCoord3dv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3dvARB)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3dv, (target, v), (F, "glMultiTexCoord3dvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3fARB, (target, s, t, r), (F, "glMultiTexCoord3f(0x%x, %f, %f, %f);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3fARB, (target, s, t, r), (F, "glMultiTexCoord3fARB(0x%x, %f, %f, %f);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3fv)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3fvARB, (target, v), (F, "glMultiTexCoord3fv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3fvARB)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3fvARB, (target, v), (F, "glMultiTexCoord3fvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3i, (target, s, t, r), (F, "glMultiTexCoord3i(0x%x, %d, %d, %d);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3i, (target, s, t, r), (F, "glMultiTexCoord3iARB(0x%x, %d, %d, %d);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3iv)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3iv, (target, v), (F, "glMultiTexCoord3iv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3ivARB)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3iv, (target, v), (F, "glMultiTexCoord3ivARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3s, (target, s, t, r), (F, "glMultiTexCoord3s(0x%x, %d, %d, %d);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    (void) target; (void) s; (void) t; (void) r;
   DISPATCH(MultiTexCoord3s, (target, s, t, r), (F, "glMultiTexCoord3sARB(0x%x, %d, %d, %d);\n", target, s, t, r));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3sv)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3sv, (target, v), (F, "glMultiTexCoord3sv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord3svARB)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord3sv, (target, v), (F, "glMultiTexCoord3svARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4d, (target, s, t, r, q), (F, "glMultiTexCoord4d(0x%x, %f, %f, %f, %f);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4d, (target, s, t, r, q), (F, "glMultiTexCoord4dARB(0x%x, %f, %f, %f, %f);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4dv)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4dv, (target, v), (F, "glMultiTexCoord4dv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4dvARB)(GLenum target, const GLdouble * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4dv, (target, v), (F, "glMultiTexCoord4dvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4fARB, (target, s, t, r, q), (F, "glMultiTexCoord4f(0x%x, %f, %f, %f, %f);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4fARB, (target, s, t, r, q), (F, "glMultiTexCoord4fARB(0x%x, %f, %f, %f, %f);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4fv)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4fvARB, (target, v), (F, "glMultiTexCoord4fv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4fvARB)(GLenum target, const GLfloat * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4fvARB, (target, v), (F, "glMultiTexCoord4fvARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4i, (target, s, t, r, q), (F, "glMultiTexCoord4i(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4i, (target, s, t, r, q), (F, "glMultiTexCoord4iARB(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4iv)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4iv, (target, v), (F, "glMultiTexCoord4iv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4ivARB)(GLenum target, const GLint * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4iv, (target, v), (F, "glMultiTexCoord4ivARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4s, (target, s, t, r, q), (F, "glMultiTexCoord4s(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4s, (target, s, t, r, q), (F, "glMultiTexCoord4sARB(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4sv)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4sv, (target, v), (F, "glMultiTexCoord4sv(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4svARB)(GLenum target, const GLshort * v)
{
    (void) target; (void) v;
   DISPATCH(MultiTexCoord4sv, (target, v), (F, "glMultiTexCoord4svARB(0x%x, %p);\n", target, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage1D, (target, level, internalformat, width, border, imageSize, data), (F, "glCompressedTexImage1D(0x%x, %d, 0x%x, %d, %d, %d, %p);\n", target, level, internalformat, width, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage1DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage1D, (target, level, internalformat, width, border, imageSize, data), (F, "glCompressedTexImage1DARB(0x%x, %d, 0x%x, %d, %d, %d, %p);\n", target, level, internalformat, width, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage2D, (target, level, internalformat, width, height, border, imageSize, data), (F, "glCompressedTexImage2D(0x%x, %d, 0x%x, %d, %d, %d, %d, %p);\n", target, level, internalformat, width, height, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage2DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage2D, (target, level, internalformat, width, height, border, imageSize, data), (F, "glCompressedTexImage2DARB(0x%x, %d, 0x%x, %d, %d, %d, %d, %p);\n", target, level, internalformat, width, height, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage3D, (target, level, internalformat, width, height, depth, border, imageSize, data), (F, "glCompressedTexImage3D(0x%x, %d, 0x%x, %d, %d, %d, %d, %d, %p);\n", target, level, internalformat, width, height, depth, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexImage3DARB)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage3D, (target, level, internalformat, width, height, depth, border, imageSize, data), (F, "glCompressedTexImage3DARB(0x%x, %d, 0x%x, %d, %d, %d, %d, %d, %p);\n", target, level, internalformat, width, height, depth, border, imageSize, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_410)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_410)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) internalformat; (void) width; (void) height; (void) depth; (void) border; (void) imageSize; (void) data;
   DISPATCH(CompressedTexImage3D, (target, level, internalformat, width, height, depth, border, imageSize, data), (F, "glCompressedTexImage3DOES(0x%x, %d, 0x%x, %d, %d, %d, %d, %d, %p);\n", target, level, internalformat, width, height, depth, border, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) width; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage1D, (target, level, xoffset, width, format, imageSize, data), (F, "glCompressedTexSubImage1D(0x%x, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, width, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage1DARB)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) width; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage1D, (target, level, xoffset, width, format, imageSize, data), (F, "glCompressedTexSubImage1DARB(0x%x, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, width, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage2D, (target, level, xoffset, yoffset, width, height, format, imageSize, data), (F, "glCompressedTexSubImage2D(0x%x, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, yoffset, width, height, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage2DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage2D, (target, level, xoffset, yoffset, width, height, format, imageSize, data), (F, "glCompressedTexSubImage2DARB(0x%x, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, yoffset, width, height, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data), (F, "glCompressedTexSubImage3D(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(CompressedTexSubImage3DARB)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data), (F, "glCompressedTexSubImage3DARB(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_413)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_413)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) target; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTexSubImage3D, (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data), (F, "glCompressedTexSubImage3DOES(0x%x, %d, %d, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetCompressedTexImage)(GLenum target, GLint level, GLvoid * img)
{
    (void) target; (void) level; (void) img;
   DISPATCH(GetCompressedTexImage, (target, level, img), (F, "glGetCompressedTexImage(0x%x, %d, %p);\n", target, level, (const void *) img));
}

KEYWORD1 void KEYWORD2 NAME(GetCompressedTexImageARB)(GLenum target, GLint level, GLvoid * img)
{
    (void) target; (void) level; (void) img;
   DISPATCH(GetCompressedTexImage, (target, level, img), (F, "glGetCompressedTexImageARB(0x%x, %d, %p);\n", target, level, (const void *) img));
}

KEYWORD1 void KEYWORD2 NAME(LoadTransposeMatrixd)(const GLdouble * m)
{
    (void) m;
   DISPATCH(LoadTransposeMatrixd, (m), (F, "glLoadTransposeMatrixd(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(LoadTransposeMatrixdARB)(const GLdouble * m)
{
    (void) m;
   DISPATCH(LoadTransposeMatrixd, (m), (F, "glLoadTransposeMatrixdARB(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(LoadTransposeMatrixf)(const GLfloat * m)
{
    (void) m;
   DISPATCH(LoadTransposeMatrixf, (m), (F, "glLoadTransposeMatrixf(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(LoadTransposeMatrixfARB)(const GLfloat * m)
{
    (void) m;
   DISPATCH(LoadTransposeMatrixf, (m), (F, "glLoadTransposeMatrixfARB(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultTransposeMatrixd)(const GLdouble * m)
{
    (void) m;
   DISPATCH(MultTransposeMatrixd, (m), (F, "glMultTransposeMatrixd(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultTransposeMatrixdARB)(const GLdouble * m)
{
    (void) m;
   DISPATCH(MultTransposeMatrixd, (m), (F, "glMultTransposeMatrixdARB(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultTransposeMatrixf)(const GLfloat * m)
{
    (void) m;
   DISPATCH(MultTransposeMatrixf, (m), (F, "glMultTransposeMatrixf(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultTransposeMatrixfARB)(const GLfloat * m)
{
    (void) m;
   DISPATCH(MultTransposeMatrixf, (m), (F, "glMultTransposeMatrixfARB(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(SampleCoverage)(GLclampf value, GLboolean invert)
{
    (void) value; (void) invert;
   DISPATCH(SampleCoverage, (value, invert), (F, "glSampleCoverage(%f, %d);\n", value, invert));
}

KEYWORD1 void KEYWORD2 NAME(SampleCoverageARB)(GLclampf value, GLboolean invert)
{
    (void) value; (void) invert;
   DISPATCH(SampleCoverage, (value, invert), (F, "glSampleCoverageARB(%f, %d);\n", value, invert));
}

KEYWORD1 void KEYWORD2 NAME(BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    (void) sfactorRGB; (void) dfactorRGB; (void) sfactorAlpha; (void) dfactorAlpha;
   DISPATCH(BlendFuncSeparate, (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha), (F, "glBlendFuncSeparate(0x%x, 0x%x, 0x%x, 0x%x);\n", sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha));
}

KEYWORD1 void KEYWORD2 NAME(BlendFuncSeparateEXT)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    (void) sfactorRGB; (void) dfactorRGB; (void) sfactorAlpha; (void) dfactorAlpha;
   DISPATCH(BlendFuncSeparate, (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha), (F, "glBlendFuncSeparateEXT(0x%x, 0x%x, 0x%x, 0x%x);\n", sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_420)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_420)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    (void) sfactorRGB; (void) dfactorRGB; (void) sfactorAlpha; (void) dfactorAlpha;
   DISPATCH(BlendFuncSeparate, (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha), (F, "glBlendFuncSeparateINGR(0x%x, 0x%x, 0x%x, 0x%x);\n", sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordPointer)(GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) pointer;
   DISPATCH(FogCoordPointer, (type, stride, pointer), (F, "glFogCoordPointer(0x%x, %d, %p);\n", type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordPointerEXT)(GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) pointer;
   DISPATCH(FogCoordPointer, (type, stride, pointer), (F, "glFogCoordPointerEXT(0x%x, %d, %p);\n", type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordd)(GLdouble coord)
{
    (void) coord;
   DISPATCH(FogCoordd, (coord), (F, "glFogCoordd(%f);\n", coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoorddEXT)(GLdouble coord)
{
    (void) coord;
   DISPATCH(FogCoordd, (coord), (F, "glFogCoorddEXT(%f);\n", coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoorddv)(const GLdouble * coord)
{
    (void) coord;
   DISPATCH(FogCoorddv, (coord), (F, "glFogCoorddv(%p);\n", (const void *) coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoorddvEXT)(const GLdouble * coord)
{
    (void) coord;
   DISPATCH(FogCoorddv, (coord), (F, "glFogCoorddvEXT(%p);\n", (const void *) coord));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount)
{
    (void) mode; (void) first; (void) count; (void) primcount;
   DISPATCH(MultiDrawArrays, (mode, first, count, primcount), (F, "glMultiDrawArrays(0x%x, %p, %p, %d);\n", mode, (const void *) first, (const void *) count, primcount));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawArraysEXT)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount)
{
    (void) mode; (void) first; (void) count; (void) primcount;
   DISPATCH(MultiDrawArrays, (mode, first, count, primcount), (F, "glMultiDrawArraysEXT(0x%x, %p, %p, %d);\n", mode, (const void *) first, (const void *) count, primcount));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterf)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterf, (pname, param), (F, "glPointParameterf(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterfARB)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterf, (pname, param), (F, "glPointParameterfARB(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterfEXT)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterf, (pname, param), (F, "glPointParameterfEXT(0x%x, %f);\n", pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_425)(GLenum pname, GLfloat param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_425)(GLenum pname, GLfloat param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterf, (pname, param), (F, "glPointParameterfSGIS(0x%x, %f);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterfv)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterfv, (pname, params), (F, "glPointParameterfv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterfvARB)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterfv, (pname, params), (F, "glPointParameterfvARB(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterfvEXT)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterfv, (pname, params), (F, "glPointParameterfvEXT(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_426)(GLenum pname, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_426)(GLenum pname, const GLfloat * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterfv, (pname, params), (F, "glPointParameterfvSGIS(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(PointParameteri)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameteri, (pname, param), (F, "glPointParameteri(0x%x, %d);\n", pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_427)(GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_427)(GLenum pname, GLint param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameteri, (pname, param), (F, "glPointParameteriNV(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PointParameteriv)(GLenum pname, const GLint * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameteriv, (pname, params), (F, "glPointParameteriv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_428)(GLenum pname, const GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_428)(GLenum pname, const GLint * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameteriv, (pname, params), (F, "glPointParameterivNV(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3b, (red, green, blue), (F, "glSecondaryColor3b(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3bEXT)(GLbyte red, GLbyte green, GLbyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3b, (red, green, blue), (F, "glSecondaryColor3bEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3bv)(const GLbyte * v)
{
    (void) v;
   DISPATCH(SecondaryColor3bv, (v), (F, "glSecondaryColor3bv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3bvEXT)(const GLbyte * v)
{
    (void) v;
   DISPATCH(SecondaryColor3bv, (v), (F, "glSecondaryColor3bvEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3d, (red, green, blue), (F, "glSecondaryColor3d(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3dEXT)(GLdouble red, GLdouble green, GLdouble blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3d, (red, green, blue), (F, "glSecondaryColor3dEXT(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(SecondaryColor3dv, (v), (F, "glSecondaryColor3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3dvEXT)(const GLdouble * v)
{
    (void) v;
   DISPATCH(SecondaryColor3dv, (v), (F, "glSecondaryColor3dvEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3i)(GLint red, GLint green, GLint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3i, (red, green, blue), (F, "glSecondaryColor3i(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3iEXT)(GLint red, GLint green, GLint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3i, (red, green, blue), (F, "glSecondaryColor3iEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(SecondaryColor3iv, (v), (F, "glSecondaryColor3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ivEXT)(const GLint * v)
{
    (void) v;
   DISPATCH(SecondaryColor3iv, (v), (F, "glSecondaryColor3ivEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3s)(GLshort red, GLshort green, GLshort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3s, (red, green, blue), (F, "glSecondaryColor3s(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3sEXT)(GLshort red, GLshort green, GLshort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3s, (red, green, blue), (F, "glSecondaryColor3sEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(SecondaryColor3sv, (v), (F, "glSecondaryColor3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3svEXT)(const GLshort * v)
{
    (void) v;
   DISPATCH(SecondaryColor3sv, (v), (F, "glSecondaryColor3svEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3ub, (red, green, blue), (F, "glSecondaryColor3ub(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ubEXT)(GLubyte red, GLubyte green, GLubyte blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3ub, (red, green, blue), (F, "glSecondaryColor3ubEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ubv)(const GLubyte * v)
{
    (void) v;
   DISPATCH(SecondaryColor3ubv, (v), (F, "glSecondaryColor3ubv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ubvEXT)(const GLubyte * v)
{
    (void) v;
   DISPATCH(SecondaryColor3ubv, (v), (F, "glSecondaryColor3ubvEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3ui, (red, green, blue), (F, "glSecondaryColor3ui(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3uiEXT)(GLuint red, GLuint green, GLuint blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3ui, (red, green, blue), (F, "glSecondaryColor3uiEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3uiv)(const GLuint * v)
{
    (void) v;
   DISPATCH(SecondaryColor3uiv, (v), (F, "glSecondaryColor3uiv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3uivEXT)(const GLuint * v)
{
    (void) v;
   DISPATCH(SecondaryColor3uiv, (v), (F, "glSecondaryColor3uivEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3us)(GLushort red, GLushort green, GLushort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3us, (red, green, blue), (F, "glSecondaryColor3us(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3usEXT)(GLushort red, GLushort green, GLushort blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3us, (red, green, blue), (F, "glSecondaryColor3usEXT(%d, %d, %d);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3usv)(const GLushort * v)
{
    (void) v;
   DISPATCH(SecondaryColor3usv, (v), (F, "glSecondaryColor3usv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3usvEXT)(const GLushort * v)
{
    (void) v;
   DISPATCH(SecondaryColor3usv, (v), (F, "glSecondaryColor3usvEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(SecondaryColorPointer, (size, type, stride, pointer), (F, "glSecondaryColorPointer(%d, 0x%x, %d, %p);\n", size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColorPointerEXT)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(SecondaryColorPointer, (size, type, stride, pointer), (F, "glSecondaryColorPointerEXT(%d, 0x%x, %d, %p);\n", size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2d)(GLdouble x, GLdouble y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2d, (x, y), (F, "glWindowPos2d(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2dARB)(GLdouble x, GLdouble y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2d, (x, y), (F, "glWindowPos2dARB(%f, %f);\n", x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_444)(GLdouble x, GLdouble y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_444)(GLdouble x, GLdouble y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2d, (x, y), (F, "glWindowPos2dMESA(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos2dv, (v), (F, "glWindowPos2dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2dvARB)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos2dv, (v), (F, "glWindowPos2dvARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_445)(const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_445)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos2dv, (v), (F, "glWindowPos2dvMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2f)(GLfloat x, GLfloat y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2f, (x, y), (F, "glWindowPos2f(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2fARB)(GLfloat x, GLfloat y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2f, (x, y), (F, "glWindowPos2fARB(%f, %f);\n", x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_446)(GLfloat x, GLfloat y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_446)(GLfloat x, GLfloat y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2f, (x, y), (F, "glWindowPos2fMESA(%f, %f);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos2fv, (v), (F, "glWindowPos2fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2fvARB)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos2fv, (v), (F, "glWindowPos2fvARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_447)(const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_447)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos2fv, (v), (F, "glWindowPos2fvMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2i)(GLint x, GLint y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2i, (x, y), (F, "glWindowPos2i(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2iARB)(GLint x, GLint y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2i, (x, y), (F, "glWindowPos2iARB(%d, %d);\n", x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_448)(GLint x, GLint y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_448)(GLint x, GLint y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2i, (x, y), (F, "glWindowPos2iMESA(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2iv)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos2iv, (v), (F, "glWindowPos2iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2ivARB)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos2iv, (v), (F, "glWindowPos2ivARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_449)(const GLint * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_449)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos2iv, (v), (F, "glWindowPos2ivMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2s)(GLshort x, GLshort y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2s, (x, y), (F, "glWindowPos2s(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2sARB)(GLshort x, GLshort y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2s, (x, y), (F, "glWindowPos2sARB(%d, %d);\n", x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_450)(GLshort x, GLshort y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_450)(GLshort x, GLshort y)
{
    (void) x; (void) y;
   DISPATCH(WindowPos2s, (x, y), (F, "glWindowPos2sMESA(%d, %d);\n", x, y));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos2sv, (v), (F, "glWindowPos2sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos2svARB)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos2sv, (v), (F, "glWindowPos2svARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_451)(const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_451)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos2sv, (v), (F, "glWindowPos2svMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3d, (x, y, z), (F, "glWindowPos3d(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3dARB)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3d, (x, y, z), (F, "glWindowPos3dARB(%f, %f, %f);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_452)(GLdouble x, GLdouble y, GLdouble z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_452)(GLdouble x, GLdouble y, GLdouble z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3d, (x, y, z), (F, "glWindowPos3dMESA(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3dv)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos3dv, (v), (F, "glWindowPos3dv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3dvARB)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos3dv, (v), (F, "glWindowPos3dvARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_453)(const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_453)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos3dv, (v), (F, "glWindowPos3dvMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3f, (x, y, z), (F, "glWindowPos3f(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3fARB)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3f, (x, y, z), (F, "glWindowPos3fARB(%f, %f, %f);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_454)(GLfloat x, GLfloat y, GLfloat z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_454)(GLfloat x, GLfloat y, GLfloat z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3f, (x, y, z), (F, "glWindowPos3fMESA(%f, %f, %f);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos3fv, (v), (F, "glWindowPos3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3fvARB)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos3fv, (v), (F, "glWindowPos3fvARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_455)(const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_455)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos3fv, (v), (F, "glWindowPos3fvMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3i)(GLint x, GLint y, GLint z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3i, (x, y, z), (F, "glWindowPos3i(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3iARB)(GLint x, GLint y, GLint z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3i, (x, y, z), (F, "glWindowPos3iARB(%d, %d, %d);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_456)(GLint x, GLint y, GLint z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_456)(GLint x, GLint y, GLint z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3i, (x, y, z), (F, "glWindowPos3iMESA(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3iv)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos3iv, (v), (F, "glWindowPos3iv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3ivARB)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos3iv, (v), (F, "glWindowPos3ivARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_457)(const GLint * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_457)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos3iv, (v), (F, "glWindowPos3ivMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3s)(GLshort x, GLshort y, GLshort z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3s, (x, y, z), (F, "glWindowPos3s(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3sARB)(GLshort x, GLshort y, GLshort z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3s, (x, y, z), (F, "glWindowPos3sARB(%d, %d, %d);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_458)(GLshort x, GLshort y, GLshort z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_458)(GLshort x, GLshort y, GLshort z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(WindowPos3s, (x, y, z), (F, "glWindowPos3sMESA(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3sv)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos3sv, (v), (F, "glWindowPos3sv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(WindowPos3svARB)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos3sv, (v), (F, "glWindowPos3svARB(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_459)(const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_459)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos3sv, (v), (F, "glWindowPos3svMESA(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(BeginQuery)(GLenum target, GLuint id)
{
    (void) target; (void) id;
   DISPATCH(BeginQuery, (target, id), (F, "glBeginQuery(0x%x, %d);\n", target, id));
}

KEYWORD1 void KEYWORD2 NAME(BeginQueryARB)(GLenum target, GLuint id)
{
    (void) target; (void) id;
   DISPATCH(BeginQuery, (target, id), (F, "glBeginQueryARB(0x%x, %d);\n", target, id));
}

KEYWORD1 void KEYWORD2 NAME(BindBuffer)(GLenum target, GLuint buffer)
{
    (void) target; (void) buffer;
   DISPATCH(BindBuffer, (target, buffer), (F, "glBindBuffer(0x%x, %d);\n", target, buffer));
}

KEYWORD1 void KEYWORD2 NAME(BindBufferARB)(GLenum target, GLuint buffer)
{
    (void) target; (void) buffer;
   DISPATCH(BindBuffer, (target, buffer), (F, "glBindBufferARB(0x%x, %d);\n", target, buffer));
}

KEYWORD1 void KEYWORD2 NAME(BufferData)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage)
{
    (void) target; (void) size; (void) data; (void) usage;
   DISPATCH(BufferData, (target, size, data, usage), (F, "glBufferData(0x%x, %d, %p, 0x%x);\n", target, size, (const void *) data, usage));
}

KEYWORD1 void KEYWORD2 NAME(BufferDataARB)(GLenum target, GLsizeiptrARB size, const GLvoid * data, GLenum usage)
{
    (void) target; (void) size; (void) data; (void) usage;
   DISPATCH(BufferData, (target, size, data, usage), (F, "glBufferDataARB(0x%x, %d, %p, 0x%x);\n", target, size, (const void *) data, usage));
}

KEYWORD1 void KEYWORD2 NAME(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
    (void) target; (void) offset; (void) size; (void) data;
   DISPATCH(BufferSubData, (target, offset, size, data), (F, "glBufferSubData(0x%x, %d, %d, %p);\n", target, offset, size, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(BufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data)
{
    (void) target; (void) offset; (void) size; (void) data;
   DISPATCH(BufferSubData, (target, offset, size, data), (F, "glBufferSubDataARB(0x%x, %d, %d, %p);\n", target, offset, size, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(DeleteBuffers)(GLsizei n, const GLuint * buffer)
{
    (void) n; (void) buffer;
   DISPATCH(DeleteBuffers, (n, buffer), (F, "glDeleteBuffers(%d, %p);\n", n, (const void *) buffer));
}

KEYWORD1 void KEYWORD2 NAME(DeleteBuffersARB)(GLsizei n, const GLuint * buffer)
{
    (void) n; (void) buffer;
   DISPATCH(DeleteBuffers, (n, buffer), (F, "glDeleteBuffersARB(%d, %p);\n", n, (const void *) buffer));
}

KEYWORD1 void KEYWORD2 NAME(DeleteQueries)(GLsizei n, const GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(DeleteQueries, (n, ids), (F, "glDeleteQueries(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 void KEYWORD2 NAME(DeleteQueriesARB)(GLsizei n, const GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(DeleteQueries, (n, ids), (F, "glDeleteQueriesARB(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 void KEYWORD2 NAME(EndQuery)(GLenum target)
{
    (void) target;
   DISPATCH(EndQuery, (target), (F, "glEndQuery(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(EndQueryARB)(GLenum target)
{
    (void) target;
   DISPATCH(EndQuery, (target), (F, "glEndQueryARB(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(GenBuffers)(GLsizei n, GLuint * buffer)
{
    (void) n; (void) buffer;
   DISPATCH(GenBuffers, (n, buffer), (F, "glGenBuffers(%d, %p);\n", n, (const void *) buffer));
}

KEYWORD1 void KEYWORD2 NAME(GenBuffersARB)(GLsizei n, GLuint * buffer)
{
    (void) n; (void) buffer;
   DISPATCH(GenBuffers, (n, buffer), (F, "glGenBuffersARB(%d, %p);\n", n, (const void *) buffer));
}

KEYWORD1 void KEYWORD2 NAME(GenQueries)(GLsizei n, GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(GenQueries, (n, ids), (F, "glGenQueries(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 void KEYWORD2 NAME(GenQueriesARB)(GLsizei n, GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(GenQueries, (n, ids), (F, "glGenQueriesARB(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferParameteriv, (target, pname, params), (F, "glGetBufferParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferParameterivARB)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferParameteriv, (target, pname, params), (F, "glGetBufferParameterivARB(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferPointerv)(GLenum target, GLenum pname, GLvoid ** params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferPointerv, (target, pname, params), (F, "glGetBufferPointerv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferPointervARB)(GLenum target, GLenum pname, GLvoid ** params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferPointerv, (target, pname, params), (F, "glGetBufferPointervARB(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_470)(GLenum target, GLenum pname, GLvoid ** params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_470)(GLenum target, GLenum pname, GLvoid ** params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferPointerv, (target, pname, params), (F, "glGetBufferPointervOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data)
{
    (void) target; (void) offset; (void) size; (void) data;
   DISPATCH(GetBufferSubData, (target, offset, size, data), (F, "glGetBufferSubData(0x%x, %d, %d, %p);\n", target, offset, size, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid * data)
{
    (void) target; (void) offset; (void) size; (void) data;
   DISPATCH(GetBufferSubData, (target, offset, size, data), (F, "glGetBufferSubDataARB(0x%x, %d, %d, %p);\n", target, offset, size, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryObjectiv)(GLuint id, GLenum pname, GLint * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjectiv, (id, pname, params), (F, "glGetQueryObjectiv(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryObjectivARB)(GLuint id, GLenum pname, GLint * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjectiv, (id, pname, params), (F, "glGetQueryObjectivARB(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjectuiv, (id, pname, params), (F, "glGetQueryObjectuiv(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryObjectuivARB)(GLuint id, GLenum pname, GLuint * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjectuiv, (id, pname, params), (F, "glGetQueryObjectuivARB(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryiv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetQueryiv, (target, pname, params), (F, "glGetQueryiv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryivARB)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetQueryiv, (target, pname, params), (F, "glGetQueryivARB(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsBuffer)(GLuint buffer)
{
    (void) buffer;
   RETURN_DISPATCH(IsBuffer, (buffer), (F, "glIsBuffer(%d);\n", buffer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsBufferARB)(GLuint buffer)
{
    (void) buffer;
   RETURN_DISPATCH(IsBuffer, (buffer), (F, "glIsBufferARB(%d);\n", buffer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsQuery)(GLuint id)
{
    (void) id;
   RETURN_DISPATCH(IsQuery, (id), (F, "glIsQuery(%d);\n", id));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsQueryARB)(GLuint id)
{
    (void) id;
   RETURN_DISPATCH(IsQuery, (id), (F, "glIsQueryARB(%d);\n", id));
}

KEYWORD1 GLvoid * KEYWORD2 NAME(MapBuffer)(GLenum target, GLenum access)
{
    (void) target; (void) access;
   RETURN_DISPATCH(MapBuffer, (target, access), (F, "glMapBuffer(0x%x, 0x%x);\n", target, access));
}

KEYWORD1 GLvoid * KEYWORD2 NAME(MapBufferARB)(GLenum target, GLenum access)
{
    (void) target; (void) access;
   RETURN_DISPATCH(MapBuffer, (target, access), (F, "glMapBufferARB(0x%x, 0x%x);\n", target, access));
}

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_477)(GLenum target, GLenum access);

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_477)(GLenum target, GLenum access)
{
    (void) target; (void) access;
   RETURN_DISPATCH(MapBuffer, (target, access), (F, "glMapBufferOES(0x%x, 0x%x);\n", target, access));
}

KEYWORD1 GLboolean KEYWORD2 NAME(UnmapBuffer)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(UnmapBuffer, (target), (F, "glUnmapBuffer(0x%x);\n", target));
}

KEYWORD1 GLboolean KEYWORD2 NAME(UnmapBufferARB)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(UnmapBuffer, (target), (F, "glUnmapBufferARB(0x%x);\n", target));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_478)(GLenum target);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_478)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(UnmapBuffer, (target), (F, "glUnmapBufferOES(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(AttachShader)(GLuint program, GLuint shader)
{
    (void) program; (void) shader;
   DISPATCH(AttachShader, (program, shader), (F, "glAttachShader(%d, %d);\n", program, shader));
}

KEYWORD1 void KEYWORD2 NAME(BindAttribLocation)(GLuint program, GLuint index, const GLchar * name)
{
    (void) program; (void) index; (void) name;
   DISPATCH(BindAttribLocation, (program, index, name), (F, "glBindAttribLocation(%d, %d, %p);\n", program, index, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(BindAttribLocationARB)(GLhandleARB program, GLuint index, const GLcharARB * name)
{
    (void) program; (void) index; (void) name;
   DISPATCH(BindAttribLocation, (program, index, name), (F, "glBindAttribLocationARB(%d, %d, %p);\n", program, index, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(BlendEquationSeparate)(GLenum modeRGB, GLenum modeA)
{
    (void) modeRGB; (void) modeA;
   DISPATCH(BlendEquationSeparate, (modeRGB, modeA), (F, "glBlendEquationSeparate(0x%x, 0x%x);\n", modeRGB, modeA));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_481)(GLenum modeRGB, GLenum modeA);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_481)(GLenum modeRGB, GLenum modeA)
{
    (void) modeRGB; (void) modeA;
   DISPATCH(BlendEquationSeparate, (modeRGB, modeA), (F, "glBlendEquationSeparateEXT(0x%x, 0x%x);\n", modeRGB, modeA));
}

KEYWORD1 void KEYWORD2 NAME(CompileShader)(GLuint shader)
{
    (void) shader;
   DISPATCH(CompileShader, (shader), (F, "glCompileShader(%d);\n", shader));
}

KEYWORD1 void KEYWORD2 NAME(CompileShaderARB)(GLhandleARB shader)
{
    (void) shader;
   DISPATCH(CompileShader, (shader), (F, "glCompileShaderARB(%d);\n", shader));
}

KEYWORD1 GLuint KEYWORD2 NAME(CreateProgram)(void)
{
   RETURN_DISPATCH(CreateProgram, (), (F, "glCreateProgram();\n"));
}

KEYWORD1 GLuint KEYWORD2 NAME(CreateShader)(GLenum type)
{
    (void) type;
   RETURN_DISPATCH(CreateShader, (type), (F, "glCreateShader(0x%x);\n", type));
}

KEYWORD1 void KEYWORD2 NAME(DeleteProgram)(GLuint program)
{
    (void) program;
   DISPATCH(DeleteProgram, (program), (F, "glDeleteProgram(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(DeleteShader)(GLuint program)
{
    (void) program;
   DISPATCH(DeleteShader, (program), (F, "glDeleteShader(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(DetachShader)(GLuint program, GLuint shader)
{
    (void) program; (void) shader;
   DISPATCH(DetachShader, (program, shader), (F, "glDetachShader(%d, %d);\n", program, shader));
}

KEYWORD1 void KEYWORD2 NAME(DisableVertexAttribArray)(GLuint index)
{
    (void) index;
   DISPATCH(DisableVertexAttribArray, (index), (F, "glDisableVertexAttribArray(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(DisableVertexAttribArrayARB)(GLuint index)
{
    (void) index;
   DISPATCH(DisableVertexAttribArray, (index), (F, "glDisableVertexAttribArrayARB(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(DrawBuffers)(GLsizei n, const GLenum * bufs)
{
    (void) n; (void) bufs;
   DISPATCH(DrawBuffers, (n, bufs), (F, "glDrawBuffers(%d, %p);\n", n, (const void *) bufs));
}

KEYWORD1 void KEYWORD2 NAME(DrawBuffersARB)(GLsizei n, const GLenum * bufs)
{
    (void) n; (void) bufs;
   DISPATCH(DrawBuffers, (n, bufs), (F, "glDrawBuffersARB(%d, %p);\n", n, (const void *) bufs));
}

KEYWORD1 void KEYWORD2 NAME(DrawBuffersATI)(GLsizei n, const GLenum * bufs)
{
    (void) n; (void) bufs;
   DISPATCH(DrawBuffers, (n, bufs), (F, "glDrawBuffersATI(%d, %p);\n", n, (const void *) bufs));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_489)(GLsizei n, const GLenum * bufs);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_489)(GLsizei n, const GLenum * bufs)
{
    (void) n; (void) bufs;
   DISPATCH(DrawBuffers, (n, bufs), (F, "glDrawBuffersNV(%d, %p);\n", n, (const void *) bufs));
}

KEYWORD1 void KEYWORD2 NAME(EnableVertexAttribArray)(GLuint index)
{
    (void) index;
   DISPATCH(EnableVertexAttribArray, (index), (F, "glEnableVertexAttribArray(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(EnableVertexAttribArrayARB)(GLuint index)
{
    (void) index;
   DISPATCH(EnableVertexAttribArray, (index), (F, "glEnableVertexAttribArrayARB(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveAttrib)(GLuint program, GLuint index, GLsizei  bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetActiveAttrib, (program, index, bufSize, length, size, type, name), (F, "glGetActiveAttrib(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveAttribARB)(GLhandleARB program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLcharARB * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetActiveAttrib, (program, index, bufSize, length, size, type, name), (F, "glGetActiveAttribARB(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetActiveUniform, (program, index, bufSize, length, size, type, name), (F, "glGetActiveUniform(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniformARB)(GLhandleARB program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLcharARB * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetActiveUniform, (program, index, bufSize, length, size, type, name), (F, "glGetActiveUniformARB(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * obj)
{
    (void) program; (void) maxCount; (void) count; (void) obj;
   DISPATCH(GetAttachedShaders, (program, maxCount, count, obj), (F, "glGetAttachedShaders(%d, %d, %p, %p);\n", program, maxCount, (const void *) count, (const void *) obj));
}

KEYWORD1 GLint KEYWORD2 NAME(GetAttribLocation)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetAttribLocation, (program, name), (F, "glGetAttribLocation(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 GLint KEYWORD2 NAME(GetAttribLocationARB)(GLhandleARB program, const GLcharARB * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetAttribLocation, (program, name), (F, "glGetAttribLocationARB(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    (void) program; (void) bufSize; (void) length; (void) infoLog;
   DISPATCH(GetProgramInfoLog, (program, bufSize, length, infoLog), (F, "glGetProgramInfoLog(%d, %d, %p, %p);\n", program, bufSize, (const void *) length, (const void *) infoLog));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramiv)(GLuint program, GLenum pname, GLint * params)
{
    (void) program; (void) pname; (void) params;
   DISPATCH(GetProgramiv, (program, pname, params), (F, "glGetProgramiv(%d, 0x%x, %p);\n", program, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    (void) shader; (void) bufSize; (void) length; (void) infoLog;
   DISPATCH(GetShaderInfoLog, (shader, bufSize, length, infoLog), (F, "glGetShaderInfoLog(%d, %d, %p, %p);\n", shader, bufSize, (const void *) length, (const void *) infoLog));
}

KEYWORD1 void KEYWORD2 NAME(GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source)
{
    (void) shader; (void) bufSize; (void) length; (void) source;
   DISPATCH(GetShaderSource, (shader, bufSize, length, source), (F, "glGetShaderSource(%d, %d, %p, %p);\n", shader, bufSize, (const void *) length, (const void *) source));
}

KEYWORD1 void KEYWORD2 NAME(GetShaderSourceARB)(GLhandleARB shader, GLsizei bufSize, GLsizei * length, GLcharARB * source)
{
    (void) shader; (void) bufSize; (void) length; (void) source;
   DISPATCH(GetShaderSource, (shader, bufSize, length, source), (F, "glGetShaderSourceARB(%d, %d, %p, %p);\n", shader, bufSize, (const void *) length, (const void *) source));
}

KEYWORD1 void KEYWORD2 NAME(GetShaderiv)(GLuint shader, GLenum pname, GLint * params)
{
    (void) shader; (void) pname; (void) params;
   DISPATCH(GetShaderiv, (shader, pname, params), (F, "glGetShaderiv(%d, 0x%x, %p);\n", shader, pname, (const void *) params));
}

KEYWORD1 GLint KEYWORD2 NAME(GetUniformLocation)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetUniformLocation, (program, name), (F, "glGetUniformLocation(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 GLint KEYWORD2 NAME(GetUniformLocationARB)(GLhandleARB program, const GLcharARB * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetUniformLocation, (program, name), (F, "glGetUniformLocationARB(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformfv)(GLuint program, GLint location, GLfloat * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformfv, (program, location, params), (F, "glGetUniformfv(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformfvARB)(GLhandleARB program, GLint location, GLfloat * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformfv, (program, location, params), (F, "glGetUniformfvARB(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformiv)(GLuint program, GLint location, GLint * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformiv, (program, location, params), (F, "glGetUniformiv(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformivARB)(GLhandleARB program, GLint location, GLint * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformiv, (program, location, params), (F, "glGetUniformivARB(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid ** pointer)
{
    (void) index; (void) pname; (void) pointer;
   DISPATCH(GetVertexAttribPointerv, (index, pname, pointer), (F, "glGetVertexAttribPointerv(%d, 0x%x, %p);\n", index, pname, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribPointervARB)(GLuint index, GLenum pname, GLvoid ** pointer)
{
    (void) index; (void) pname; (void) pointer;
   DISPATCH(GetVertexAttribPointerv, (index, pname, pointer), (F, "glGetVertexAttribPointervARB(%d, 0x%x, %p);\n", index, pname, (const void *) pointer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_503)(GLuint index, GLenum pname, GLvoid ** pointer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_503)(GLuint index, GLenum pname, GLvoid ** pointer)
{
    (void) index; (void) pname; (void) pointer;
   DISPATCH(GetVertexAttribPointerv, (index, pname, pointer), (F, "glGetVertexAttribPointervNV(%d, 0x%x, %p);\n", index, pname, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribdv, (index, pname, params), (F, "glGetVertexAttribdv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribdvARB)(GLuint index, GLenum pname, GLdouble * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribdv, (index, pname, params), (F, "glGetVertexAttribdvARB(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribfv, (index, pname, params), (F, "glGetVertexAttribfv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribfvARB)(GLuint index, GLenum pname, GLfloat * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribfv, (index, pname, params), (F, "glGetVertexAttribfvARB(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribiv)(GLuint index, GLenum pname, GLint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribiv, (index, pname, params), (F, "glGetVertexAttribiv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribivARB)(GLuint index, GLenum pname, GLint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribiv, (index, pname, params), (F, "glGetVertexAttribivARB(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsProgram)(GLuint program)
{
    (void) program;
   RETURN_DISPATCH(IsProgram, (program), (F, "glIsProgram(%d);\n", program));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsShader)(GLuint shader)
{
    (void) shader;
   RETURN_DISPATCH(IsShader, (shader), (F, "glIsShader(%d);\n", shader));
}

KEYWORD1 void KEYWORD2 NAME(LinkProgram)(GLuint program)
{
    (void) program;
   DISPATCH(LinkProgram, (program), (F, "glLinkProgram(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(LinkProgramARB)(GLhandleARB program)
{
    (void) program;
   DISPATCH(LinkProgram, (program), (F, "glLinkProgramARB(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(ShaderSource)(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length)
{
    (void) shader; (void) count; (void) string; (void) length;
   DISPATCH(ShaderSource, (shader, count, string, length), (F, "glShaderSource(%d, %d, %p, %p);\n", shader, count, (const void *) string, (const void *) length));
}

KEYWORD1 void KEYWORD2 NAME(ShaderSourceARB)(GLhandleARB shader, GLsizei count, const GLcharARB ** string, const GLint * length)
{
    (void) shader; (void) count; (void) string; (void) length;
   DISPATCH(ShaderSource, (shader, count, string, length), (F, "glShaderSourceARB(%d, %d, %p, %p);\n", shader, count, (const void *) string, (const void *) length));
}

KEYWORD1 void KEYWORD2 NAME(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    (void) face; (void) func; (void) ref; (void) mask;
   DISPATCH(StencilFuncSeparate, (face, func, ref, mask), (F, "glStencilFuncSeparate(0x%x, 0x%x, %d, %d);\n", face, func, ref, mask));
}

KEYWORD1 void KEYWORD2 NAME(StencilMaskSeparate)(GLenum face, GLuint mask)
{
    (void) face; (void) mask;
   DISPATCH(StencilMaskSeparate, (face, mask), (F, "glStencilMaskSeparate(0x%x, %d);\n", face, mask));
}

KEYWORD1 void KEYWORD2 NAME(StencilOpSeparate)(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass)
{
    (void) face; (void) sfail; (void) zfail; (void) zpass;
   DISPATCH(StencilOpSeparate, (face, sfail, zfail, zpass), (F, "glStencilOpSeparate(0x%x, 0x%x, 0x%x, 0x%x);\n", face, sfail, zfail, zpass));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_513)(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_513)(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass)
{
    (void) face; (void) sfail; (void) zfail; (void) zpass;
   DISPATCH(StencilOpSeparate, (face, sfail, zfail, zpass), (F, "glStencilOpSeparateATI(0x%x, 0x%x, 0x%x, 0x%x);\n", face, sfail, zfail, zpass));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1f)(GLint location, GLfloat v0)
{
    (void) location; (void) v0;
   DISPATCH(Uniform1f, (location, v0), (F, "glUniform1f(%d, %f);\n", location, v0));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1fARB)(GLint location, GLfloat v0)
{
    (void) location; (void) v0;
   DISPATCH(Uniform1f, (location, v0), (F, "glUniform1fARB(%d, %f);\n", location, v0));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1fv)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1fv, (location, count, value), (F, "glUniform1fv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1fvARB)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1fv, (location, count, value), (F, "glUniform1fvARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1i)(GLint location, GLint v0)
{
    (void) location; (void) v0;
   DISPATCH(Uniform1i, (location, v0), (F, "glUniform1i(%d, %d);\n", location, v0));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1iARB)(GLint location, GLint v0)
{
    (void) location; (void) v0;
   DISPATCH(Uniform1i, (location, v0), (F, "glUniform1iARB(%d, %d);\n", location, v0));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1iv)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1iv, (location, count, value), (F, "glUniform1iv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1ivARB)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1iv, (location, count, value), (F, "glUniform1ivARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2f)(GLint location, GLfloat v0, GLfloat v1)
{
    (void) location; (void) v0; (void) v1;
   DISPATCH(Uniform2f, (location, v0, v1), (F, "glUniform2f(%d, %f, %f);\n", location, v0, v1));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2fARB)(GLint location, GLfloat v0, GLfloat v1)
{
    (void) location; (void) v0; (void) v1;
   DISPATCH(Uniform2f, (location, v0, v1), (F, "glUniform2fARB(%d, %f, %f);\n", location, v0, v1));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2fv)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2fv, (location, count, value), (F, "glUniform2fv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2fvARB)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2fv, (location, count, value), (F, "glUniform2fvARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2i)(GLint location, GLint v0, GLint v1)
{
    (void) location; (void) v0; (void) v1;
   DISPATCH(Uniform2i, (location, v0, v1), (F, "glUniform2i(%d, %d, %d);\n", location, v0, v1));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2iARB)(GLint location, GLint v0, GLint v1)
{
    (void) location; (void) v0; (void) v1;
   DISPATCH(Uniform2i, (location, v0, v1), (F, "glUniform2iARB(%d, %d, %d);\n", location, v0, v1));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2iv)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2iv, (location, count, value), (F, "glUniform2iv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2ivARB)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2iv, (location, count, value), (F, "glUniform2ivARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    (void) location; (void) v0; (void) v1; (void) v2;
   DISPATCH(Uniform3f, (location, v0, v1, v2), (F, "glUniform3f(%d, %f, %f, %f);\n", location, v0, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    (void) location; (void) v0; (void) v1; (void) v2;
   DISPATCH(Uniform3f, (location, v0, v1, v2), (F, "glUniform3fARB(%d, %f, %f, %f);\n", location, v0, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3fv)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3fv, (location, count, value), (F, "glUniform3fv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3fvARB)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3fv, (location, count, value), (F, "glUniform3fvARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2)
{
    (void) location; (void) v0; (void) v1; (void) v2;
   DISPATCH(Uniform3i, (location, v0, v1, v2), (F, "glUniform3i(%d, %d, %d, %d);\n", location, v0, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3iARB)(GLint location, GLint v0, GLint v1, GLint v2)
{
    (void) location; (void) v0; (void) v1; (void) v2;
   DISPATCH(Uniform3i, (location, v0, v1, v2), (F, "glUniform3iARB(%d, %d, %d, %d);\n", location, v0, v1, v2));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3iv)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3iv, (location, count, value), (F, "glUniform3iv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3ivARB)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3iv, (location, count, value), (F, "glUniform3ivARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    (void) location; (void) v0; (void) v1; (void) v2; (void) v3;
   DISPATCH(Uniform4f, (location, v0, v1, v2, v3), (F, "glUniform4f(%d, %f, %f, %f, %f);\n", location, v0, v1, v2, v3));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4fARB)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    (void) location; (void) v0; (void) v1; (void) v2; (void) v3;
   DISPATCH(Uniform4f, (location, v0, v1, v2, v3), (F, "glUniform4fARB(%d, %f, %f, %f, %f);\n", location, v0, v1, v2, v3));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4fv)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4fv, (location, count, value), (F, "glUniform4fv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4fvARB)(GLint location, GLsizei count, const GLfloat * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4fv, (location, count, value), (F, "glUniform4fvARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    (void) location; (void) v0; (void) v1; (void) v2; (void) v3;
   DISPATCH(Uniform4i, (location, v0, v1, v2, v3), (F, "glUniform4i(%d, %d, %d, %d, %d);\n", location, v0, v1, v2, v3));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4iARB)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    (void) location; (void) v0; (void) v1; (void) v2; (void) v3;
   DISPATCH(Uniform4i, (location, v0, v1, v2, v3), (F, "glUniform4iARB(%d, %d, %d, %d, %d);\n", location, v0, v1, v2, v3));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4iv)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4iv, (location, count, value), (F, "glUniform4iv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4ivARB)(GLint location, GLsizei count, const GLint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4iv, (location, count, value), (F, "glUniform4ivARB(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2fv, (location, count, transpose, value), (F, "glUniformMatrix2fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix2fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2fv, (location, count, transpose, value), (F, "glUniformMatrix2fvARB(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3fv, (location, count, transpose, value), (F, "glUniformMatrix3fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix3fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3fv, (location, count, transpose, value), (F, "glUniformMatrix3fvARB(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4fv, (location, count, transpose, value), (F, "glUniformMatrix4fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix4fvARB)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4fv, (location, count, transpose, value), (F, "glUniformMatrix4fvARB(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UseProgram)(GLuint program)
{
    (void) program;
   DISPATCH(UseProgram, (program), (F, "glUseProgram(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(UseProgramObjectARB)(GLhandleARB program)
{
    (void) program;
   DISPATCH(UseProgram, (program), (F, "glUseProgramObjectARB(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(ValidateProgram)(GLuint program)
{
    (void) program;
   DISPATCH(ValidateProgram, (program), (F, "glValidateProgram(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(ValidateProgramARB)(GLhandleARB program)
{
    (void) program;
   DISPATCH(ValidateProgram, (program), (F, "glValidateProgramARB(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1d)(GLuint index, GLdouble x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1d, (index, x), (F, "glVertexAttrib1d(%d, %f);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1dARB)(GLuint index, GLdouble x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1d, (index, x), (F, "glVertexAttrib1dARB(%d, %f);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1dv)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1dv, (index, v), (F, "glVertexAttrib1dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1dvARB)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1dv, (index, v), (F, "glVertexAttrib1dvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1s)(GLuint index, GLshort x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1s, (index, x), (F, "glVertexAttrib1s(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1sARB)(GLuint index, GLshort x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1s, (index, x), (F, "glVertexAttrib1sARB(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1sv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1sv, (index, v), (F, "glVertexAttrib1sv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1svARB)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1sv, (index, v), (F, "glVertexAttrib1svARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2d, (index, x, y), (F, "glVertexAttrib2d(%d, %f, %f);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2dARB)(GLuint index, GLdouble x, GLdouble y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2d, (index, x, y), (F, "glVertexAttrib2dARB(%d, %f, %f);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2dv)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2dv, (index, v), (F, "glVertexAttrib2dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2dvARB)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2dv, (index, v), (F, "glVertexAttrib2dvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2s)(GLuint index, GLshort x, GLshort y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2s, (index, x, y), (F, "glVertexAttrib2s(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2sARB)(GLuint index, GLshort x, GLshort y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2s, (index, x, y), (F, "glVertexAttrib2sARB(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2sv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2sv, (index, v), (F, "glVertexAttrib2sv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2svARB)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2sv, (index, v), (F, "glVertexAttrib2svARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3d, (index, x, y, z), (F, "glVertexAttrib3d(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3d, (index, x, y, z), (F, "glVertexAttrib3dARB(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3dv)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3dv, (index, v), (F, "glVertexAttrib3dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3dvARB)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3dv, (index, v), (F, "glVertexAttrib3dvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3s, (index, x, y, z), (F, "glVertexAttrib3s(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3sARB)(GLuint index, GLshort x, GLshort y, GLshort z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3s, (index, x, y, z), (F, "glVertexAttrib3sARB(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3sv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3sv, (index, v), (F, "glVertexAttrib3sv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3svARB)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3sv, (index, v), (F, "glVertexAttrib3svARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nbv)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nbv, (index, v), (F, "glVertexAttrib4Nbv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NbvARB)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nbv, (index, v), (F, "glVertexAttrib4NbvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Niv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Niv, (index, v), (F, "glVertexAttrib4Niv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NivARB)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Niv, (index, v), (F, "glVertexAttrib4NivARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nsv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nsv, (index, v), (F, "glVertexAttrib4Nsv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NsvARB)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nsv, (index, v), (F, "glVertexAttrib4NsvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4Nub, (index, x, y, z, w), (F, "glVertexAttrib4Nub(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NubARB)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4Nub, (index, x, y, z, w), (F, "glVertexAttrib4NubARB(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nubv)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nubv, (index, v), (F, "glVertexAttrib4Nubv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NubvARB)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nubv, (index, v), (F, "glVertexAttrib4NubvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nuiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nuiv, (index, v), (F, "glVertexAttrib4Nuiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NuivARB)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nuiv, (index, v), (F, "glVertexAttrib4NuivARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4Nusv)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nusv, (index, v), (F, "glVertexAttrib4Nusv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4NusvARB)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4Nusv, (index, v), (F, "glVertexAttrib4NusvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4bv)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4bv, (index, v), (F, "glVertexAttrib4bv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4bvARB)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4bv, (index, v), (F, "glVertexAttrib4bvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4d, (index, x, y, z, w), (F, "glVertexAttrib4d(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4dARB)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4d, (index, x, y, z, w), (F, "glVertexAttrib4dARB(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4dv)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4dv, (index, v), (F, "glVertexAttrib4dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4dvARB)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4dv, (index, v), (F, "glVertexAttrib4dvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4iv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4iv, (index, v), (F, "glVertexAttrib4iv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4ivARB)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4iv, (index, v), (F, "glVertexAttrib4ivARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4s, (index, x, y, z, w), (F, "glVertexAttrib4s(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4sARB)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4s, (index, x, y, z, w), (F, "glVertexAttrib4sARB(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4sv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4sv, (index, v), (F, "glVertexAttrib4sv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4svARB)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4sv, (index, v), (F, "glVertexAttrib4svARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4ubv)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4ubv, (index, v), (F, "glVertexAttrib4ubv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4ubvARB)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4ubv, (index, v), (F, "glVertexAttrib4ubvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4uiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4uiv, (index, v), (F, "glVertexAttrib4uiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4uivARB)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4uiv, (index, v), (F, "glVertexAttrib4uivARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4usv)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4usv, (index, v), (F, "glVertexAttrib4usv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4usvARB)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4usv, (index, v), (F, "glVertexAttrib4usvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) normalized; (void) stride; (void) pointer;
   DISPATCH(VertexAttribPointer, (index, size, type, normalized, stride, pointer), (F, "glVertexAttribPointer(%d, %d, 0x%x, %d, %d, %p);\n", index, size, type, normalized, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribPointerARB)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) normalized; (void) stride; (void) pointer;
   DISPATCH(VertexAttribPointer, (index, size, type, normalized, stride, pointer), (F, "glVertexAttribPointerARB(%d, %d, 0x%x, %d, %d, %p);\n", index, size, type, normalized, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2x3fv, (location, count, transpose, value), (F, "glUniformMatrix2x3fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2x4fv, (location, count, transpose, value), (F, "glUniformMatrix2x4fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3x2fv, (location, count, transpose, value), (F, "glUniformMatrix3x2fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3x4fv, (location, count, transpose, value), (F, "glUniformMatrix3x4fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4x2fv, (location, count, transpose, value), (F, "glUniformMatrix4x2fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4x3fv, (location, count, transpose, value), (F, "glUniformMatrix4x3fv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(BeginConditionalRender)(GLuint query, GLenum mode)
{
    (void) query; (void) mode;
   DISPATCH(BeginConditionalRender, (query, mode), (F, "glBeginConditionalRender(%d, 0x%x);\n", query, mode));
}

KEYWORD1 void KEYWORD2 NAME(BeginConditionalRenderNV)(GLuint query, GLenum mode)
{
    (void) query; (void) mode;
   DISPATCH(BeginConditionalRender, (query, mode), (F, "glBeginConditionalRenderNV(%d, 0x%x);\n", query, mode));
}

KEYWORD1 void KEYWORD2 NAME(BeginTransformFeedback)(GLenum mode)
{
    (void) mode;
   DISPATCH(BeginTransformFeedback, (mode), (F, "glBeginTransformFeedback(0x%x);\n", mode));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_571)(GLenum mode);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_571)(GLenum mode)
{
    (void) mode;
   DISPATCH(BeginTransformFeedback, (mode), (F, "glBeginTransformFeedbackEXT(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(BindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
    (void) target; (void) index; (void) buffer;
   DISPATCH(BindBufferBase, (target, index, buffer), (F, "glBindBufferBase(0x%x, %d, %d);\n", target, index, buffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_572)(GLenum target, GLuint index, GLuint buffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_572)(GLenum target, GLuint index, GLuint buffer)
{
    (void) target; (void) index; (void) buffer;
   DISPATCH(BindBufferBase, (target, index, buffer), (F, "glBindBufferBaseEXT(0x%x, %d, %d);\n", target, index, buffer));
}

KEYWORD1 void KEYWORD2 NAME(BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    (void) target; (void) index; (void) buffer; (void) offset; (void) size;
   DISPATCH(BindBufferRange, (target, index, buffer, offset, size), (F, "glBindBufferRange(0x%x, %d, %d, %d, %d);\n", target, index, buffer, offset, size));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_573)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_573)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    (void) target; (void) index; (void) buffer; (void) offset; (void) size;
   DISPATCH(BindBufferRange, (target, index, buffer, offset, size), (F, "glBindBufferRangeEXT(0x%x, %d, %d, %d, %d);\n", target, index, buffer, offset, size));
}

KEYWORD1 void KEYWORD2 NAME(BindFragDataLocationEXT)(GLuint program, GLuint colorNumber, const GLchar * name)
{
    (void) program; (void) colorNumber; (void) name;
   DISPATCH(BindFragDataLocation, (program, colorNumber, name), (F, "glBindFragDataLocationEXT(%d, %d, %p);\n", program, colorNumber, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(BindFragDataLocation)(GLuint program, GLuint colorNumber, const GLchar * name)
{
    (void) program; (void) colorNumber; (void) name;
   DISPATCH(BindFragDataLocation, (program, colorNumber, name), (F, "glBindFragDataLocation(%d, %d, %p);\n", program, colorNumber, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(ClampColorARB)(GLenum target, GLenum clamp)
{
    (void) target; (void) clamp;
   DISPATCH(ClampColor, (target, clamp), (F, "glClampColorARB(0x%x, 0x%x);\n", target, clamp));
}

KEYWORD1 void KEYWORD2 NAME(ClampColor)(GLenum target, GLenum clamp)
{
    (void) target; (void) clamp;
   DISPATCH(ClampColor, (target, clamp), (F, "glClampColor(0x%x, 0x%x);\n", target, clamp));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    (void) buffer; (void) drawbuffer; (void) depth; (void) stencil;
   DISPATCH(ClearBufferfi, (buffer, drawbuffer, depth, stencil), (F, "glClearBufferfi(0x%x, %d, %f, %d);\n", buffer, drawbuffer, depth, stencil));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat * value)
{
    (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearBufferfv, (buffer, drawbuffer, value), (F, "glClearBufferfv(0x%x, %d, %p);\n", buffer, drawbuffer, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint * value)
{
    (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearBufferiv, (buffer, drawbuffer, value), (F, "glClearBufferiv(0x%x, %d, %p);\n", buffer, drawbuffer, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint * value)
{
    (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearBufferuiv, (buffer, drawbuffer, value), (F, "glClearBufferuiv(0x%x, %d, %p);\n", buffer, drawbuffer, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(ColorMaskIndexedEXT)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    (void) buf; (void) r; (void) g; (void) b; (void) a;
   DISPATCH(ColorMaski, (buf, r, g, b, a), (F, "glColorMaskIndexedEXT(%d, %d, %d, %d, %d);\n", buf, r, g, b, a));
}

KEYWORD1 void KEYWORD2 NAME(ColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    (void) buf; (void) r; (void) g; (void) b; (void) a;
   DISPATCH(ColorMaski, (buf, r, g, b, a), (F, "glColorMaski(%d, %d, %d, %d, %d);\n", buf, r, g, b, a));
}

KEYWORD1 void KEYWORD2 NAME(DisableIndexedEXT)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   DISPATCH(Disablei, (target, index), (F, "glDisableIndexedEXT(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(Disablei)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   DISPATCH(Disablei, (target, index), (F, "glDisablei(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(EnableIndexedEXT)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   DISPATCH(Enablei, (target, index), (F, "glEnableIndexedEXT(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(Enablei)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   DISPATCH(Enablei, (target, index), (F, "glEnablei(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(EndConditionalRender)(void)
{
   DISPATCH(EndConditionalRender, (), (F, "glEndConditionalRender();\n"));
}

KEYWORD1 void KEYWORD2 NAME(EndConditionalRenderNV)(void)
{
   DISPATCH(EndConditionalRender, (), (F, "glEndConditionalRenderNV();\n"));
}

KEYWORD1 void KEYWORD2 NAME(EndTransformFeedback)(void)
{
   DISPATCH(EndTransformFeedback, (), (F, "glEndTransformFeedback();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_584)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_584)(void)
{
   DISPATCH(EndTransformFeedback, (), (F, "glEndTransformFeedbackEXT();\n"));
}

KEYWORD1 void KEYWORD2 NAME(GetBooleanIndexedvEXT)(GLenum value, GLuint index, GLboolean * data)
{
    (void) value; (void) index; (void) data;
   DISPATCH(GetBooleani_v, (value, index, data), (F, "glGetBooleanIndexedvEXT(0x%x, %d, %p);\n", value, index, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetBooleani_v)(GLenum value, GLuint index, GLboolean * data)
{
    (void) value; (void) index; (void) data;
   DISPATCH(GetBooleani_v, (value, index, data), (F, "glGetBooleani_v(0x%x, %d, %p);\n", value, index, (const void *) data));
}

KEYWORD1 GLint KEYWORD2 NAME(GetFragDataLocationEXT)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetFragDataLocation, (program, name), (F, "glGetFragDataLocationEXT(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 GLint KEYWORD2 NAME(GetFragDataLocation)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetFragDataLocation, (program, name), (F, "glGetFragDataLocation(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetIntegerIndexedvEXT)(GLenum value, GLuint index, GLint * data)
{
    (void) value; (void) index; (void) data;
   DISPATCH(GetIntegeri_v, (value, index, data), (F, "glGetIntegerIndexedvEXT(0x%x, %d, %p);\n", value, index, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetIntegeri_v)(GLenum value, GLuint index, GLint * data)
{
    (void) value; (void) index; (void) data;
   DISPATCH(GetIntegeri_v, (value, index, data), (F, "glGetIntegeri_v(0x%x, %d, %p);\n", value, index, (const void *) data));
}

KEYWORD1 const GLubyte * KEYWORD2 NAME(GetStringi)(GLenum name, GLuint index)
{
    (void) name; (void) index;
   RETURN_DISPATCH(GetStringi, (name, index), (F, "glGetStringi(0x%x, %d);\n", name, index));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterIivEXT)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterIiv, (target, pname, params), (F, "glGetTexParameterIivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterIiv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterIiv, (target, pname, params), (F, "glGetTexParameterIiv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterIuivEXT)(GLenum target, GLenum pname, GLuint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterIuiv, (target, pname, params), (F, "glGetTexParameterIuivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterIuiv, (target, pname, params), (F, "glGetTexParameterIuiv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetTransformFeedbackVarying, (program, index, bufSize, length, size, type, name), (F, "glGetTransformFeedbackVarying(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_591)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_591)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name)
{
    (void) program; (void) index; (void) bufSize; (void) length; (void) size; (void) type; (void) name;
   DISPATCH(GetTransformFeedbackVarying, (program, index, bufSize, length, size, type, name), (F, "glGetTransformFeedbackVaryingEXT(%d, %d, %d, %p, %p, %p, %p);\n", program, index, bufSize, (const void *) length, (const void *) size, (const void *) type, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformuivEXT)(GLuint program, GLint location, GLuint * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformuiv, (program, location, params), (F, "glGetUniformuivEXT(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformuiv)(GLuint program, GLint location, GLuint * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformuiv, (program, location, params), (F, "glGetUniformuiv(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribIivEXT)(GLuint index, GLenum pname, GLint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribIiv, (index, pname, params), (F, "glGetVertexAttribIivEXT(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribIiv)(GLuint index, GLenum pname, GLint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribIiv, (index, pname, params), (F, "glGetVertexAttribIiv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribIuivEXT)(GLuint index, GLenum pname, GLuint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribIuiv, (index, pname, params), (F, "glGetVertexAttribIuivEXT(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribIuiv, (index, pname, params), (F, "glGetVertexAttribIuiv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsEnabledIndexedEXT)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   RETURN_DISPATCH(IsEnabledi, (target, index), (F, "glIsEnabledIndexedEXT(0x%x, %d);\n", target, index));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsEnabledi)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   RETURN_DISPATCH(IsEnabledi, (target, index), (F, "glIsEnabledi(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterIivEXT)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterIiv, (target, pname, params), (F, "glTexParameterIivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterIiv)(GLenum target, GLenum pname, const GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterIiv, (target, pname, params), (F, "glTexParameterIiv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterIuivEXT)(GLenum target, GLenum pname, const GLuint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterIuiv, (target, pname, params), (F, "glTexParameterIuivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterIuiv)(GLenum target, GLenum pname, const GLuint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterIuiv, (target, pname, params), (F, "glTexParameterIuiv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar * const * varyings, GLenum bufferMode)
{
    (void) program; (void) count; (void) varyings; (void) bufferMode;
   DISPATCH(TransformFeedbackVaryings, (program, count, varyings, bufferMode), (F, "glTransformFeedbackVaryings(%d, %d, %p, 0x%x);\n", program, count, (const void *) varyings, bufferMode));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_598)(GLuint program, GLsizei count, const GLchar * const * varyings, GLenum bufferMode);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_598)(GLuint program, GLsizei count, const GLchar * const * varyings, GLenum bufferMode)
{
    (void) program; (void) count; (void) varyings; (void) bufferMode;
   DISPATCH(TransformFeedbackVaryings, (program, count, varyings, bufferMode), (F, "glTransformFeedbackVaryingsEXT(%d, %d, %p, 0x%x);\n", program, count, (const void *) varyings, bufferMode));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1uiEXT)(GLint location, GLuint x)
{
    (void) location; (void) x;
   DISPATCH(Uniform1ui, (location, x), (F, "glUniform1uiEXT(%d, %d);\n", location, x));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1ui)(GLint location, GLuint x)
{
    (void) location; (void) x;
   DISPATCH(Uniform1ui, (location, x), (F, "glUniform1ui(%d, %d);\n", location, x));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1uivEXT)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1uiv, (location, count, value), (F, "glUniform1uivEXT(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform1uiv)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1uiv, (location, count, value), (F, "glUniform1uiv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2uiEXT)(GLint location, GLuint x, GLuint y)
{
    (void) location; (void) x; (void) y;
   DISPATCH(Uniform2ui, (location, x, y), (F, "glUniform2uiEXT(%d, %d, %d);\n", location, x, y));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2ui)(GLint location, GLuint x, GLuint y)
{
    (void) location; (void) x; (void) y;
   DISPATCH(Uniform2ui, (location, x, y), (F, "glUniform2ui(%d, %d, %d);\n", location, x, y));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2uivEXT)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2uiv, (location, count, value), (F, "glUniform2uivEXT(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform2uiv)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2uiv, (location, count, value), (F, "glUniform2uiv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3uiEXT)(GLint location, GLuint x, GLuint y, GLuint z)
{
    (void) location; (void) x; (void) y; (void) z;
   DISPATCH(Uniform3ui, (location, x, y, z), (F, "glUniform3uiEXT(%d, %d, %d, %d);\n", location, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3ui)(GLint location, GLuint x, GLuint y, GLuint z)
{
    (void) location; (void) x; (void) y; (void) z;
   DISPATCH(Uniform3ui, (location, x, y, z), (F, "glUniform3ui(%d, %d, %d, %d);\n", location, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3uivEXT)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3uiv, (location, count, value), (F, "glUniform3uivEXT(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform3uiv)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3uiv, (location, count, value), (F, "glUniform3uiv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4uiEXT)(GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
    (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Uniform4ui, (location, x, y, z, w), (F, "glUniform4uiEXT(%d, %d, %d, %d, %d);\n", location, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4ui)(GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
    (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Uniform4ui, (location, x, y, z, w), (F, "glUniform4ui(%d, %d, %d, %d, %d);\n", location, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4uivEXT)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4uiv, (location, count, value), (F, "glUniform4uivEXT(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(Uniform4uiv)(GLint location, GLsizei count, const GLuint * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4uiv, (location, count, value), (F, "glUniform4uiv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1ivEXT)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI1iv, (index, v), (F, "glVertexAttribI1ivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1iv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI1iv, (index, v), (F, "glVertexAttribI1iv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1uivEXT)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI1uiv, (index, v), (F, "glVertexAttribI1uivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1uiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI1uiv, (index, v), (F, "glVertexAttribI1uiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4bvEXT)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4bv, (index, v), (F, "glVertexAttribI4bvEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4bv)(GLuint index, const GLbyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4bv, (index, v), (F, "glVertexAttribI4bv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4svEXT)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4sv, (index, v), (F, "glVertexAttribI4svEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4sv)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4sv, (index, v), (F, "glVertexAttribI4sv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4ubvEXT)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4ubv, (index, v), (F, "glVertexAttribI4ubvEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4ubv)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4ubv, (index, v), (F, "glVertexAttribI4ubv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4usvEXT)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4usv, (index, v), (F, "glVertexAttribI4usvEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4usv)(GLuint index, const GLushort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4usv, (index, v), (F, "glVertexAttribI4usv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribIPointerEXT)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(VertexAttribIPointer, (index, size, type, stride, pointer), (F, "glVertexAttribIPointerEXT(%d, %d, 0x%x, %d, %p);\n", index, size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(VertexAttribIPointer, (index, size, type, stride, pointer), (F, "glVertexAttribIPointer(%d, %d, 0x%x, %d, %p);\n", index, size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(PrimitiveRestartIndex)(GLuint index)
{
    (void) index;
   DISPATCH(PrimitiveRestartIndex, (index), (F, "glPrimitiveRestartIndex(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(PrimitiveRestartIndexNV)(GLuint index)
{
    (void) index;
   DISPATCH(PrimitiveRestartIndex, (index), (F, "glPrimitiveRestartIndexNV(%d);\n", index));
}

KEYWORD1 void KEYWORD2 NAME(TexBufferARB)(GLenum target, GLenum internalFormat, GLuint buffer)
{
    (void) target; (void) internalFormat; (void) buffer;
   DISPATCH(TexBuffer, (target, internalFormat, buffer), (F, "glTexBufferARB(0x%x, 0x%x, %d);\n", target, internalFormat, buffer));
}

KEYWORD1 void KEYWORD2 NAME(TexBuffer)(GLenum target, GLenum internalFormat, GLuint buffer)
{
    (void) target; (void) internalFormat; (void) buffer;
   DISPATCH(TexBuffer, (target, internalFormat, buffer), (F, "glTexBuffer(0x%x, 0x%x, %d);\n", target, internalFormat, buffer));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) texture; (void) level;
   DISPATCH(FramebufferTexture, (target, attachment, texture, level), (F, "glFramebufferTexture(0x%x, 0x%x, %d, %d);\n", target, attachment, texture, level));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_616)(GLenum target, GLenum attachment, GLuint texture, GLint level);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_616)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) texture; (void) level;
   DISPATCH(FramebufferTexture, (target, attachment, texture, level), (F, "glFramebufferTextureOES(0x%x, 0x%x, %d, %d);\n", target, attachment, texture, level));
}

KEYWORD1 void KEYWORD2 NAME(GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetBufferParameteri64v, (target, pname, params), (F, "glGetBufferParameteri64v(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetInteger64i_v)(GLenum cap, GLuint index, GLint64 * data)
{
    (void) cap; (void) index; (void) data;
   DISPATCH(GetInteger64i_v, (cap, index, data), (F, "glGetInteger64i_v(0x%x, %d, %p);\n", cap, index, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribDivisorARB)(GLuint index, GLuint divisor)
{
    (void) index; (void) divisor;
   DISPATCH(VertexAttribDivisor, (index, divisor), (F, "glVertexAttribDivisorARB(%d, %d);\n", index, divisor));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribDivisor)(GLuint index, GLuint divisor)
{
    (void) index; (void) divisor;
   DISPATCH(VertexAttribDivisor, (index, divisor), (F, "glVertexAttribDivisor(%d, %d);\n", index, divisor));
}

KEYWORD1 void KEYWORD2 NAME(MinSampleShadingARB)(GLfloat value)
{
    (void) value;
   DISPATCH(MinSampleShading, (value), (F, "glMinSampleShadingARB(%f);\n", value));
}

KEYWORD1 void KEYWORD2 NAME(MinSampleShading)(GLfloat value)
{
    (void) value;
   DISPATCH(MinSampleShading, (value), (F, "glMinSampleShading(%f);\n", value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_621)(GLbitfield barriers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_621)(GLbitfield barriers)
{
    (void) barriers;
   DISPATCH(MemoryBarrierByRegion, (barriers), (F, "glMemoryBarrierByRegion(%d);\n", barriers));
}

KEYWORD1 void KEYWORD2 NAME(BindProgramARB)(GLenum target, GLuint program)
{
    (void) target; (void) program;
   DISPATCH(BindProgramARB, (target, program), (F, "glBindProgramARB(0x%x, %d);\n", target, program));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_622)(GLenum target, GLuint program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_622)(GLenum target, GLuint program)
{
    (void) target; (void) program;
   DISPATCH(BindProgramARB, (target, program), (F, "glBindProgramNV(0x%x, %d);\n", target, program));
}

KEYWORD1 void KEYWORD2 NAME(DeleteProgramsARB)(GLsizei n, const GLuint * programs)
{
    (void) n; (void) programs;
   DISPATCH(DeleteProgramsARB, (n, programs), (F, "glDeleteProgramsARB(%d, %p);\n", n, (const void *) programs));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_623)(GLsizei n, const GLuint * programs);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_623)(GLsizei n, const GLuint * programs)
{
    (void) n; (void) programs;
   DISPATCH(DeleteProgramsARB, (n, programs), (F, "glDeleteProgramsNV(%d, %p);\n", n, (const void *) programs));
}

KEYWORD1 void KEYWORD2 NAME(GenProgramsARB)(GLsizei n, GLuint * programs)
{
    (void) n; (void) programs;
   DISPATCH(GenProgramsARB, (n, programs), (F, "glGenProgramsARB(%d, %p);\n", n, (const void *) programs));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_624)(GLsizei n, GLuint * programs);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_624)(GLsizei n, GLuint * programs)
{
    (void) n; (void) programs;
   DISPATCH(GenProgramsARB, (n, programs), (F, "glGenProgramsNV(%d, %p);\n", n, (const void *) programs));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramEnvParameterdvARB)(GLenum target, GLuint index, GLdouble * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(GetProgramEnvParameterdvARB, (target, index, params), (F, "glGetProgramEnvParameterdvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramEnvParameterfvARB)(GLenum target, GLuint index, GLfloat * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(GetProgramEnvParameterfvARB, (target, index, params), (F, "glGetProgramEnvParameterfvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(GetProgramLocalParameterdvARB, (target, index, params), (F, "glGetProgramLocalParameterdvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramLocalParameterfvARB)(GLenum target, GLuint index, GLfloat * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(GetProgramLocalParameterfvARB, (target, index, params), (F, "glGetProgramLocalParameterfvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramStringARB)(GLenum target, GLenum pname, GLvoid * string)
{
    (void) target; (void) pname; (void) string;
   DISPATCH(GetProgramStringARB, (target, pname, string), (F, "glGetProgramStringARB(0x%x, 0x%x, %p);\n", target, pname, (const void *) string));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramivARB)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetProgramivARB, (target, pname, params), (F, "glGetProgramivARB(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsProgramARB)(GLuint program)
{
    (void) program;
   RETURN_DISPATCH(IsProgramARB, (program), (F, "glIsProgramARB(%d);\n", program));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_631)(GLuint program);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_631)(GLuint program)
{
    (void) program;
   RETURN_DISPATCH(IsProgramARB, (program), (F, "glIsProgramNV(%d);\n", program));
}

KEYWORD1 void KEYWORD2 NAME(ProgramEnvParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramEnvParameter4dARB, (target, index, x, y, z, w), (F, "glProgramEnvParameter4dARB(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_632)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_632)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramEnvParameter4dARB, (target, index, x, y, z, w), (F, "glProgramParameter4dNV(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(ProgramEnvParameter4dvARB)(GLenum target, GLuint index, const GLdouble * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramEnvParameter4dvARB, (target, index, params), (F, "glProgramEnvParameter4dvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_633)(GLenum target, GLuint index, const GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_633)(GLenum target, GLuint index, const GLdouble * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramEnvParameter4dvARB, (target, index, params), (F, "glProgramParameter4dvNV(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramEnvParameter4fARB, (target, index, x, y, z, w), (F, "glProgramEnvParameter4fARB(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_634)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_634)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramEnvParameter4fARB, (target, index, x, y, z, w), (F, "glProgramParameter4fNV(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(ProgramEnvParameter4fvARB)(GLenum target, GLuint index, const GLfloat * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramEnvParameter4fvARB, (target, index, params), (F, "glProgramEnvParameter4fvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_635)(GLenum target, GLuint index, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_635)(GLenum target, GLuint index, const GLfloat * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramEnvParameter4fvARB, (target, index, params), (F, "glProgramParameter4fvNV(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramLocalParameter4dARB, (target, index, x, y, z, w), (F, "glProgramLocalParameter4dARB(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(ProgramLocalParameter4dvARB)(GLenum target, GLuint index, const GLdouble * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramLocalParameter4dvARB, (target, index, params), (F, "glProgramLocalParameter4dvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) target; (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramLocalParameter4fARB, (target, index, x, y, z, w), (F, "glProgramLocalParameter4fARB(0x%x, %d, %f, %f, %f, %f);\n", target, index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(ProgramLocalParameter4fvARB)(GLenum target, GLuint index, const GLfloat * params)
{
    (void) target; (void) index; (void) params;
   DISPATCH(ProgramLocalParameter4fvARB, (target, index, params), (F, "glProgramLocalParameter4fvARB(0x%x, %d, %p);\n", target, index, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid * string)
{
    (void) target; (void) format; (void) len; (void) string;
   DISPATCH(ProgramStringARB, (target, format, len, string), (F, "glProgramStringARB(0x%x, 0x%x, %d, %p);\n", target, format, len, (const void *) string));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1f)(GLuint index, GLfloat x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1fARB, (index, x), (F, "glVertexAttrib1f(%d, %f);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1fARB)(GLuint index, GLfloat x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1fARB, (index, x), (F, "glVertexAttrib1fARB(%d, %f);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1fv)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1fvARB, (index, v), (F, "glVertexAttrib1fv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib1fvARB)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1fvARB, (index, v), (F, "glVertexAttrib1fvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2fARB, (index, x, y), (F, "glVertexAttrib2f(%d, %f, %f);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2fARB)(GLuint index, GLfloat x, GLfloat y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2fARB, (index, x, y), (F, "glVertexAttrib2fARB(%d, %f, %f);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2fv)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2fvARB, (index, v), (F, "glVertexAttrib2fv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib2fvARB)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2fvARB, (index, v), (F, "glVertexAttrib2fvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3fARB, (index, x, y, z), (F, "glVertexAttrib3f(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3fARB, (index, x, y, z), (F, "glVertexAttrib3fARB(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3fv)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3fvARB, (index, v), (F, "glVertexAttrib3fv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib3fvARB)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3fvARB, (index, v), (F, "glVertexAttrib3fvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4fARB, (index, x, y, z, w), (F, "glVertexAttrib4f(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4fARB, (index, x, y, z, w), (F, "glVertexAttrib4fARB(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4fv)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4fvARB, (index, v), (F, "glVertexAttrib4fv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttrib4fvARB)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4fvARB, (index, v), (F, "glVertexAttrib4fvARB(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(AttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj)
{
    (void) containerObj; (void) obj;
   DISPATCH(AttachObjectARB, (containerObj, obj), (F, "glAttachObjectARB(%d, %d);\n", containerObj, obj));
}

KEYWORD1 GLhandleARB KEYWORD2 NAME(CreateProgramObjectARB)(void)
{
   RETURN_DISPATCH(CreateProgramObjectARB, (), (F, "glCreateProgramObjectARB();\n"));
}

KEYWORD1 GLhandleARB KEYWORD2 NAME(CreateShaderObjectARB)(GLenum shaderType)
{
    (void) shaderType;
   RETURN_DISPATCH(CreateShaderObjectARB, (shaderType), (F, "glCreateShaderObjectARB(0x%x);\n", shaderType));
}

KEYWORD1 void KEYWORD2 NAME(DeleteObjectARB)(GLhandleARB obj)
{
    (void) obj;
   DISPATCH(DeleteObjectARB, (obj), (F, "glDeleteObjectARB(%d);\n", obj));
}

KEYWORD1 void KEYWORD2 NAME(DetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj)
{
    (void) containerObj; (void) attachedObj;
   DISPATCH(DetachObjectARB, (containerObj, attachedObj), (F, "glDetachObjectARB(%d, %d);\n", containerObj, attachedObj));
}

KEYWORD1 void KEYWORD2 NAME(GetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxLength, GLsizei * length, GLhandleARB * infoLog)
{
    (void) containerObj; (void) maxLength; (void) length; (void) infoLog;
   DISPATCH(GetAttachedObjectsARB, (containerObj, maxLength, length, infoLog), (F, "glGetAttachedObjectsARB(%d, %d, %p, %p);\n", containerObj, maxLength, (const void *) length, (const void *) infoLog));
}

KEYWORD1 GLhandleARB KEYWORD2 NAME(GetHandleARB)(GLenum pname)
{
    (void) pname;
   RETURN_DISPATCH(GetHandleARB, (pname), (F, "glGetHandleARB(0x%x);\n", pname));
}

KEYWORD1 void KEYWORD2 NAME(GetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * infoLog)
{
    (void) obj; (void) maxLength; (void) length; (void) infoLog;
   DISPATCH(GetInfoLogARB, (obj, maxLength, length, infoLog), (F, "glGetInfoLogARB(%d, %d, %p, %p);\n", obj, maxLength, (const void *) length, (const void *) infoLog));
}

KEYWORD1 void KEYWORD2 NAME(GetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat * params)
{
    (void) obj; (void) pname; (void) params;
   DISPATCH(GetObjectParameterfvARB, (obj, pname, params), (F, "glGetObjectParameterfvARB(%d, 0x%x, %p);\n", obj, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint * params)
{
    (void) obj; (void) pname; (void) params;
   DISPATCH(GetObjectParameterivARB, (obj, pname, params), (F, "glGetObjectParameterivARB(%d, 0x%x, %p);\n", obj, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysInstancedARB)(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    (void) mode; (void) first; (void) count; (void) primcount;
   DISPATCH(DrawArraysInstancedARB, (mode, first, count, primcount), (F, "glDrawArraysInstancedARB(0x%x, %d, %d, %d);\n", mode, first, count, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysInstancedEXT)(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    (void) mode; (void) first; (void) count; (void) primcount;
   DISPATCH(DrawArraysInstancedARB, (mode, first, count, primcount), (F, "glDrawArraysInstancedEXT(0x%x, %d, %d, %d);\n", mode, first, count, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    (void) mode; (void) first; (void) count; (void) primcount;
   DISPATCH(DrawArraysInstancedARB, (mode, first, count, primcount), (F, "glDrawArraysInstanced(0x%x, %d, %d, %d);\n", mode, first, count, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstancedARB)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount;
   DISPATCH(DrawElementsInstancedARB, (mode, count, type, indices, primcount), (F, "glDrawElementsInstancedARB(0x%x, %d, 0x%x, %p, %d);\n", mode, count, type, (const void *) indices, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstancedEXT)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount;
   DISPATCH(DrawElementsInstancedARB, (mode, count, type, indices, primcount), (F, "glDrawElementsInstancedEXT(0x%x, %d, 0x%x, %p, %d);\n", mode, count, type, (const void *) indices, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount;
   DISPATCH(DrawElementsInstancedARB, (mode, count, type, indices, primcount), (F, "glDrawElementsInstanced(0x%x, %d, 0x%x, %p, %d);\n", mode, count, type, (const void *) indices, primcount));
}

KEYWORD1 void KEYWORD2 NAME(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
    (void) target; (void) framebuffer;
   DISPATCH(BindFramebuffer, (target, framebuffer), (F, "glBindFramebuffer(0x%x, %d);\n", target, framebuffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_661)(GLenum target, GLuint framebuffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_661)(GLenum target, GLuint framebuffer)
{
    (void) target; (void) framebuffer;
   DISPATCH(BindFramebuffer, (target, framebuffer), (F, "glBindFramebufferOES(0x%x, %d);\n", target, framebuffer));
}

KEYWORD1 void KEYWORD2 NAME(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    (void) target; (void) renderbuffer;
   DISPATCH(BindRenderbuffer, (target, renderbuffer), (F, "glBindRenderbuffer(0x%x, %d);\n", target, renderbuffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_662)(GLenum target, GLuint renderbuffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_662)(GLenum target, GLuint renderbuffer)
{
    (void) target; (void) renderbuffer;
   DISPATCH(BindRenderbuffer, (target, renderbuffer), (F, "glBindRenderbufferOES(0x%x, %d);\n", target, renderbuffer));
}

KEYWORD1 void KEYWORD2 NAME(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    (void) srcX0; (void) srcY0; (void) srcX1; (void) srcY1; (void) dstX0; (void) dstY0; (void) dstX1; (void) dstY1; (void) mask; (void) filter;
   DISPATCH(BlitFramebuffer, (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter), (F, "glBlitFramebuffer(%d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x);\n", srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_663)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_663)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    (void) srcX0; (void) srcY0; (void) srcX1; (void) srcY1; (void) dstX0; (void) dstY0; (void) dstX1; (void) dstY1; (void) mask; (void) filter;
   DISPATCH(BlitFramebuffer, (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter), (F, "glBlitFramebufferEXT(%d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x);\n", srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter));
}

KEYWORD1 GLenum KEYWORD2 NAME(CheckFramebufferStatus)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(CheckFramebufferStatus, (target), (F, "glCheckFramebufferStatus(0x%x);\n", target));
}

KEYWORD1 GLenum KEYWORD2 NAME(CheckFramebufferStatusEXT)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(CheckFramebufferStatus, (target), (F, "glCheckFramebufferStatusEXT(0x%x);\n", target));
}

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_664)(GLenum target);

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_664)(GLenum target)
{
    (void) target;
   RETURN_DISPATCH(CheckFramebufferStatus, (target), (F, "glCheckFramebufferStatusOES(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(DeleteFramebuffers)(GLsizei n, const GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(DeleteFramebuffers, (n, framebuffers), (F, "glDeleteFramebuffers(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1 void KEYWORD2 NAME(DeleteFramebuffersEXT)(GLsizei n, const GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(DeleteFramebuffers, (n, framebuffers), (F, "glDeleteFramebuffersEXT(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_665)(GLsizei n, const GLuint * framebuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_665)(GLsizei n, const GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(DeleteFramebuffers, (n, framebuffers), (F, "glDeleteFramebuffersOES(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1 void KEYWORD2 NAME(DeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(DeleteRenderbuffers, (n, renderbuffers), (F, "glDeleteRenderbuffers(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1 void KEYWORD2 NAME(DeleteRenderbuffersEXT)(GLsizei n, const GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(DeleteRenderbuffers, (n, renderbuffers), (F, "glDeleteRenderbuffersEXT(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_666)(GLsizei n, const GLuint * renderbuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_666)(GLsizei n, const GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(DeleteRenderbuffers, (n, renderbuffers), (F, "glDeleteRenderbuffersOES(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    (void) target; (void) attachment; (void) renderbuffertarget; (void) renderbuffer;
   DISPATCH(FramebufferRenderbuffer, (target, attachment, renderbuffertarget, renderbuffer), (F, "glFramebufferRenderbuffer(0x%x, 0x%x, 0x%x, %d);\n", target, attachment, renderbuffertarget, renderbuffer));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    (void) target; (void) attachment; (void) renderbuffertarget; (void) renderbuffer;
   DISPATCH(FramebufferRenderbuffer, (target, attachment, renderbuffertarget, renderbuffer), (F, "glFramebufferRenderbufferEXT(0x%x, 0x%x, 0x%x, %d);\n", target, attachment, renderbuffertarget, renderbuffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_667)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_667)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    (void) target; (void) attachment; (void) renderbuffertarget; (void) renderbuffer;
   DISPATCH(FramebufferRenderbuffer, (target, attachment, renderbuffertarget, renderbuffer), (F, "glFramebufferRenderbufferOES(0x%x, 0x%x, 0x%x, %d);\n", target, attachment, renderbuffertarget, renderbuffer));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level;
   DISPATCH(FramebufferTexture1D, (target, attachment, textarget, texture, level), (F, "glFramebufferTexture1D(0x%x, 0x%x, 0x%x, %d, %d);\n", target, attachment, textarget, texture, level));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level;
   DISPATCH(FramebufferTexture1D, (target, attachment, textarget, texture, level), (F, "glFramebufferTexture1DEXT(0x%x, 0x%x, 0x%x, %d, %d);\n", target, attachment, textarget, texture, level));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level;
   DISPATCH(FramebufferTexture2D, (target, attachment, textarget, texture, level), (F, "glFramebufferTexture2D(0x%x, 0x%x, 0x%x, %d, %d);\n", target, attachment, textarget, texture, level));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level;
   DISPATCH(FramebufferTexture2D, (target, attachment, textarget, texture, level), (F, "glFramebufferTexture2DEXT(0x%x, 0x%x, 0x%x, %d, %d);\n", target, attachment, textarget, texture, level));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_669)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_669)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level;
   DISPATCH(FramebufferTexture2D, (target, attachment, textarget, texture, level), (F, "glFramebufferTexture2DOES(0x%x, 0x%x, 0x%x, %d, %d);\n", target, attachment, textarget, texture, level));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level; (void) layer;
   DISPATCH(FramebufferTexture3D, (target, attachment, textarget, texture, level, layer), (F, "glFramebufferTexture3D(0x%x, 0x%x, 0x%x, %d, %d, %d);\n", target, attachment, textarget, texture, level, layer));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level; (void) zoffset;
   DISPATCH(FramebufferTexture3D, (target, attachment, textarget, texture, level, zoffset), (F, "glFramebufferTexture3DEXT(0x%x, 0x%x, 0x%x, %d, %d, %d);\n", target, attachment, textarget, texture, level, zoffset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_670)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_670)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    (void) target; (void) attachment; (void) textarget; (void) texture; (void) level; (void) zoffset;
   DISPATCH(FramebufferTexture3D, (target, attachment, textarget, texture, level, zoffset), (F, "glFramebufferTexture3DOES(0x%x, 0x%x, 0x%x, %d, %d, %d);\n", target, attachment, textarget, texture, level, zoffset));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    (void) target; (void) attachment; (void) texture; (void) level; (void) layer;
   DISPATCH(FramebufferTextureLayer, (target, attachment, texture, level, layer), (F, "glFramebufferTextureLayer(0x%x, 0x%x, %d, %d, %d);\n", target, attachment, texture, level, layer));
}

KEYWORD1 void KEYWORD2 NAME(FramebufferTextureLayerEXT)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    (void) target; (void) attachment; (void) texture; (void) level; (void) layer;
   DISPATCH(FramebufferTextureLayer, (target, attachment, texture, level, layer), (F, "glFramebufferTextureLayerEXT(0x%x, 0x%x, %d, %d, %d);\n", target, attachment, texture, level, layer));
}

KEYWORD1 void KEYWORD2 NAME(GenFramebuffers)(GLsizei n, GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(GenFramebuffers, (n, framebuffers), (F, "glGenFramebuffers(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1 void KEYWORD2 NAME(GenFramebuffersEXT)(GLsizei n, GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(GenFramebuffers, (n, framebuffers), (F, "glGenFramebuffersEXT(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_672)(GLsizei n, GLuint * framebuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_672)(GLsizei n, GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(GenFramebuffers, (n, framebuffers), (F, "glGenFramebuffersOES(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1 void KEYWORD2 NAME(GenRenderbuffers)(GLsizei n, GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(GenRenderbuffers, (n, renderbuffers), (F, "glGenRenderbuffers(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1 void KEYWORD2 NAME(GenRenderbuffersEXT)(GLsizei n, GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(GenRenderbuffers, (n, renderbuffers), (F, "glGenRenderbuffersEXT(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_673)(GLsizei n, GLuint * renderbuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_673)(GLsizei n, GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(GenRenderbuffers, (n, renderbuffers), (F, "glGenRenderbuffersOES(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1 void KEYWORD2 NAME(GenerateMipmap)(GLenum target)
{
    (void) target;
   DISPATCH(GenerateMipmap, (target), (F, "glGenerateMipmap(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(GenerateMipmapEXT)(GLenum target)
{
    (void) target;
   DISPATCH(GenerateMipmap, (target), (F, "glGenerateMipmapEXT(0x%x);\n", target));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_674)(GLenum target);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_674)(GLenum target)
{
    (void) target;
   DISPATCH(GenerateMipmap, (target), (F, "glGenerateMipmapOES(0x%x);\n", target));
}

KEYWORD1 void KEYWORD2 NAME(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    (void) target; (void) attachment; (void) pname; (void) params;
   DISPATCH(GetFramebufferAttachmentParameteriv, (target, attachment, pname, params), (F, "glGetFramebufferAttachmentParameteriv(0x%x, 0x%x, 0x%x, %p);\n", target, attachment, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    (void) target; (void) attachment; (void) pname; (void) params;
   DISPATCH(GetFramebufferAttachmentParameteriv, (target, attachment, pname, params), (F, "glGetFramebufferAttachmentParameterivEXT(0x%x, 0x%x, 0x%x, %p);\n", target, attachment, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_675)(GLenum target, GLenum attachment, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_675)(GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    (void) target; (void) attachment; (void) pname; (void) params;
   DISPATCH(GetFramebufferAttachmentParameteriv, (target, attachment, pname, params), (F, "glGetFramebufferAttachmentParameterivOES(0x%x, 0x%x, 0x%x, %p);\n", target, attachment, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetRenderbufferParameteriv, (target, pname, params), (F, "glGetRenderbufferParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetRenderbufferParameteriv, (target, pname, params), (F, "glGetRenderbufferParameterivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_676)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_676)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetRenderbufferParameteriv, (target, pname, params), (F, "glGetRenderbufferParameterivOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsFramebuffer)(GLuint framebuffer)
{
    (void) framebuffer;
   RETURN_DISPATCH(IsFramebuffer, (framebuffer), (F, "glIsFramebuffer(%d);\n", framebuffer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsFramebufferEXT)(GLuint framebuffer)
{
    (void) framebuffer;
   RETURN_DISPATCH(IsFramebuffer, (framebuffer), (F, "glIsFramebufferEXT(%d);\n", framebuffer));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_677)(GLuint framebuffer);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_677)(GLuint framebuffer)
{
    (void) framebuffer;
   RETURN_DISPATCH(IsFramebuffer, (framebuffer), (F, "glIsFramebufferOES(%d);\n", framebuffer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsRenderbuffer)(GLuint renderbuffer)
{
    (void) renderbuffer;
   RETURN_DISPATCH(IsRenderbuffer, (renderbuffer), (F, "glIsRenderbuffer(%d);\n", renderbuffer));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsRenderbufferEXT)(GLuint renderbuffer)
{
    (void) renderbuffer;
   RETURN_DISPATCH(IsRenderbuffer, (renderbuffer), (F, "glIsRenderbufferEXT(%d);\n", renderbuffer));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_678)(GLuint renderbuffer);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_678)(GLuint renderbuffer)
{
    (void) renderbuffer;
   RETURN_DISPATCH(IsRenderbuffer, (renderbuffer), (F, "glIsRenderbufferOES(%d);\n", renderbuffer));
}

KEYWORD1 void KEYWORD2 NAME(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) target; (void) internalformat; (void) width; (void) height;
   DISPATCH(RenderbufferStorage, (target, internalformat, width, height), (F, "glRenderbufferStorage(0x%x, 0x%x, %d, %d);\n", target, internalformat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(RenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) target; (void) internalformat; (void) width; (void) height;
   DISPATCH(RenderbufferStorage, (target, internalformat, width, height), (F, "glRenderbufferStorageEXT(0x%x, 0x%x, %d, %d);\n", target, internalformat, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_679)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_679)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) target; (void) internalformat; (void) width; (void) height;
   DISPATCH(RenderbufferStorage, (target, internalformat, width, height), (F, "glRenderbufferStorageOES(0x%x, 0x%x, %d, %d);\n", target, internalformat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height;
   DISPATCH(RenderbufferStorageMultisample, (target, samples, internalformat, width, height), (F, "glRenderbufferStorageMultisample(0x%x, %d, 0x%x, %d, %d);\n", target, samples, internalformat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height;
   DISPATCH(RenderbufferStorageMultisample, (target, samples, internalformat, width, height), (F, "glRenderbufferStorageMultisampleEXT(0x%x, %d, 0x%x, %d, %d);\n", target, samples, internalformat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    (void) target; (void) offset; (void) length;
   DISPATCH(FlushMappedBufferRange, (target, offset, length), (F, "glFlushMappedBufferRange(0x%x, %d, %d);\n", target, offset, length));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_681)(GLenum target, GLintptr offset, GLsizeiptr length);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_681)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    (void) target; (void) offset; (void) length;
   DISPATCH(FlushMappedBufferRange, (target, offset, length), (F, "glFlushMappedBufferRangeEXT(0x%x, %d, %d);\n", target, offset, length));
}

KEYWORD1 GLvoid * KEYWORD2 NAME(MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    (void) target; (void) offset; (void) length; (void) access;
   RETURN_DISPATCH(MapBufferRange, (target, offset, length, access), (F, "glMapBufferRange(0x%x, %d, %d, %d);\n", target, offset, length, access));
}

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_682)(GLenum target, GLintptr offset, GLsizeiptr size, GLbitfield length);

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_682)(GLenum target, GLintptr offset, GLsizeiptr size, GLbitfield length)
{
    (void) target; (void) offset; (void) size; (void) length;
   RETURN_DISPATCH(MapBufferRange, (target, offset, size, length), (F, "glMapBufferRangeEXT(0x%x, %d, %d, %d);\n", target, offset, size, length));
}

KEYWORD1 void KEYWORD2 NAME(BindVertexArray)(GLuint array)
{
    (void) array;
   DISPATCH(BindVertexArray, (array), (F, "glBindVertexArray(%d);\n", array));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_683)(GLuint array);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_683)(GLuint array)
{
    (void) array;
   DISPATCH(BindVertexArray, (array), (F, "glBindVertexArrayOES(%d);\n", array));
}

KEYWORD1 void KEYWORD2 NAME(DeleteVertexArrays)(GLsizei n, const GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(DeleteVertexArrays, (n, arrays), (F, "glDeleteVertexArrays(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_684)(GLsizei n, const GLuint * arrays);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_684)(GLsizei n, const GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(DeleteVertexArrays, (n, arrays), (F, "glDeleteVertexArraysAPPLE(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1 void KEYWORD2 NAME(GenVertexArrays)(GLsizei n, GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(GenVertexArrays, (n, arrays), (F, "glGenVertexArrays(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_685)(GLsizei n, GLuint * arrays);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_685)(GLsizei n, GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(GenVertexArrays, (n, arrays), (F, "glGenVertexArraysOES(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsVertexArray)(GLuint array)
{
    (void) array;
   RETURN_DISPATCH(IsVertexArray, (array), (F, "glIsVertexArray(%d);\n", array));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_686)(GLuint array);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_686)(GLuint array)
{
    (void) array;
   RETURN_DISPATCH(IsVertexArray, (array), (F, "glIsVertexArrayAPPLE(%d);\n", array));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName)
{
    (void) program; (void) uniformBlockIndex; (void) bufSize; (void) length; (void) uniformBlockName;
   DISPATCH(GetActiveUniformBlockName, (program, uniformBlockIndex, bufSize, length, uniformBlockName), (F, "glGetActiveUniformBlockName(%d, %d, %d, %p, %p);\n", program, uniformBlockIndex, bufSize, (const void *) length, (const void *) uniformBlockName));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params)
{
    (void) program; (void) uniformBlockIndex; (void) pname; (void) params;
   DISPATCH(GetActiveUniformBlockiv, (program, uniformBlockIndex, pname, params), (F, "glGetActiveUniformBlockiv(%d, %d, 0x%x, %p);\n", program, uniformBlockIndex, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName)
{
    (void) program; (void) uniformIndex; (void) bufSize; (void) length; (void) uniformName;
   DISPATCH(GetActiveUniformName, (program, uniformIndex, bufSize, length, uniformName), (F, "glGetActiveUniformName(%d, %d, %d, %p, %p);\n", program, uniformIndex, bufSize, (const void *) length, (const void *) uniformName));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params)
{
    (void) program; (void) uniformCount; (void) uniformIndices; (void) pname; (void) params;
   DISPATCH(GetActiveUniformsiv, (program, uniformCount, uniformIndices, pname, params), (F, "glGetActiveUniformsiv(%d, %d, %p, 0x%x, %p);\n", program, uniformCount, (const void *) uniformIndices, pname, (const void *) params));
}

KEYWORD1 GLuint KEYWORD2 NAME(GetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName)
{
    (void) program; (void) uniformBlockName;
   RETURN_DISPATCH(GetUniformBlockIndex, (program, uniformBlockName), (F, "glGetUniformBlockIndex(%d, %p);\n", program, (const void *) uniformBlockName));
}

KEYWORD1 void KEYWORD2 NAME(GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar * const * uniformNames, GLuint * uniformIndices)
{
    (void) program; (void) uniformCount; (void) uniformNames; (void) uniformIndices;
   DISPATCH(GetUniformIndices, (program, uniformCount, uniformNames, uniformIndices), (F, "glGetUniformIndices(%d, %d, %p, %p);\n", program, uniformCount, (const void *) uniformNames, (const void *) uniformIndices));
}

KEYWORD1 void KEYWORD2 NAME(UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    (void) program; (void) uniformBlockIndex; (void) uniformBlockBinding;
   DISPATCH(UniformBlockBinding, (program, uniformBlockIndex, uniformBlockBinding), (F, "glUniformBlockBinding(%d, %d, %d);\n", program, uniformBlockIndex, uniformBlockBinding));
}

KEYWORD1 void KEYWORD2 NAME(CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    (void) readTarget; (void) writeTarget; (void) readOffset; (void) writeOffset; (void) size;
   DISPATCH(CopyBufferSubData, (readTarget, writeTarget, readOffset, writeOffset, size), (F, "glCopyBufferSubData(0x%x, 0x%x, %d, %d, %d);\n", readTarget, writeTarget, readOffset, writeOffset, size));
}

KEYWORD1 GLenum KEYWORD2 NAME(ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    (void) sync; (void) flags; (void) timeout;
   RETURN_DISPATCH(ClientWaitSync, (sync, flags, timeout), (F, "glClientWaitSync(%d, %d, %d);\n", sync, flags, timeout));
}

KEYWORD1 void KEYWORD2 NAME(DeleteSync)(GLsync sync)
{
    (void) sync;
   DISPATCH(DeleteSync, (sync), (F, "glDeleteSync(%d);\n", sync));
}

KEYWORD1 GLsync KEYWORD2 NAME(FenceSync)(GLenum condition, GLbitfield flags)
{
    (void) condition; (void) flags;
   RETURN_DISPATCH(FenceSync, (condition, flags), (F, "glFenceSync(0x%x, %d);\n", condition, flags));
}

KEYWORD1 void KEYWORD2 NAME(GetInteger64v)(GLenum pname, GLint64 * params)
{
    (void) pname; (void) params;
   DISPATCH(GetInteger64v, (pname, params), (F, "glGetInteger64v(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values)
{
    (void) sync; (void) pname; (void) bufSize; (void) length; (void) values;
   DISPATCH(GetSynciv, (sync, pname, bufSize, length, values), (F, "glGetSynciv(%d, 0x%x, %d, %p, %p);\n", sync, pname, bufSize, (const void *) length, (const void *) values));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsSync)(GLsync sync)
{
    (void) sync;
   RETURN_DISPATCH(IsSync, (sync), (F, "glIsSync(%d);\n", sync));
}

KEYWORD1 void KEYWORD2 NAME(WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    (void) sync; (void) flags; (void) timeout;
   DISPATCH(WaitSync, (sync, flags, timeout), (F, "glWaitSync(%d, %d, %d);\n", sync, flags, timeout));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) basevertex;
   DISPATCH(DrawElementsBaseVertex, (mode, count, type, indices, basevertex), (F, "glDrawElementsBaseVertex(0x%x, %d, 0x%x, %p, %d);\n", mode, count, type, (const void *) indices, basevertex));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_702)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_702)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) basevertex;
   DISPATCH(DrawElementsBaseVertex, (mode, count, type, indices, basevertex), (F, "glDrawElementsBaseVertexEXT(0x%x, %d, 0x%x, %p, %d);\n", mode, count, type, (const void *) indices, basevertex));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) basevertex;
   DISPATCH(DrawElementsInstancedBaseVertex, (mode, count, type, indices, primcount, basevertex), (F, "glDrawElementsInstancedBaseVertex(0x%x, %d, 0x%x, %p, %d, %d);\n", mode, count, type, (const void *) indices, primcount, basevertex));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_703)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_703)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) basevertex;
   DISPATCH(DrawElementsInstancedBaseVertex, (mode, count, type, indices, primcount, basevertex), (F, "glDrawElementsInstancedBaseVertexEXT(0x%x, %d, 0x%x, %p, %d, %d);\n", mode, count, type, (const void *) indices, primcount, basevertex));
}

KEYWORD1 void KEYWORD2 NAME(DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
    (void) mode; (void) start; (void) end; (void) count; (void) type; (void) indices; (void) basevertex;
   DISPATCH(DrawRangeElementsBaseVertex, (mode, start, end, count, type, indices, basevertex), (F, "glDrawRangeElementsBaseVertex(0x%x, %d, %d, %d, 0x%x, %p, %d);\n", mode, start, end, count, type, (const void *) indices, basevertex));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_704)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_704)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex)
{
    (void) mode; (void) start; (void) end; (void) count; (void) type; (void) indices; (void) basevertex;
   DISPATCH(DrawRangeElementsBaseVertex, (mode, start, end, count, type, indices, basevertex), (F, "glDrawRangeElementsBaseVertexEXT(0x%x, %d, %d, %d, 0x%x, %p, %d);\n", mode, start, end, count, type, (const void *) indices, basevertex));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, const GLint * basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) basevertex;
   DISPATCH(MultiDrawElementsBaseVertex, (mode, count, type, indices, primcount, basevertex), (F, "glMultiDrawElementsBaseVertex(0x%x, %p, 0x%x, %p, %d, %p);\n", mode, (const void *) count, type, (const void *) indices, primcount, (const void *) basevertex));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_705)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, const GLint * basevertex);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_705)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, const GLint * basevertex)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) basevertex;
   DISPATCH(MultiDrawElementsBaseVertex, (mode, count, type, indices, primcount, basevertex), (F, "glMultiDrawElementsBaseVertexEXT(0x%x, %p, 0x%x, %p, %d, %p);\n", mode, (const void *) count, type, (const void *) indices, primcount, (const void *) basevertex));
}

KEYWORD1 void KEYWORD2 NAME(ProvokingVertexEXT)(GLenum mode)
{
    (void) mode;
   DISPATCH(ProvokingVertex, (mode), (F, "glProvokingVertexEXT(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(ProvokingVertex)(GLenum mode)
{
    (void) mode;
   DISPATCH(ProvokingVertex, (mode), (F, "glProvokingVertex(0x%x);\n", mode));
}

KEYWORD1 void KEYWORD2 NAME(GetMultisamplefv)(GLenum pname, GLuint index, GLfloat * val)
{
    (void) pname; (void) index; (void) val;
   DISPATCH(GetMultisamplefv, (pname, index, val), (F, "glGetMultisamplefv(0x%x, %d, %p);\n", pname, index, (const void *) val));
}

KEYWORD1 void KEYWORD2 NAME(SampleMaski)(GLuint index, GLbitfield mask)
{
    (void) index; (void) mask;
   DISPATCH(SampleMaski, (index, mask), (F, "glSampleMaski(%d, %d);\n", index, mask));
}

KEYWORD1 void KEYWORD2 NAME(TexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height; (void) fixedsamplelocations;
   DISPATCH(TexImage2DMultisample, (target, samples, internalformat, width, height, fixedsamplelocations), (F, "glTexImage2DMultisample(0x%x, %d, 0x%x, %d, %d, %d);\n", target, samples, internalformat, width, height, fixedsamplelocations));
}

KEYWORD1 void KEYWORD2 NAME(TexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height; (void) depth; (void) fixedsamplelocations;
   DISPATCH(TexImage3DMultisample, (target, samples, internalformat, width, height, depth, fixedsamplelocations), (F, "glTexImage3DMultisample(0x%x, %d, 0x%x, %d, %d, %d, %d);\n", target, samples, internalformat, width, height, depth, fixedsamplelocations));
}

KEYWORD1 void KEYWORD2 NAME(BlendEquationSeparateiARB)(GLuint buf, GLenum modeRGB, GLenum modeA)
{
    (void) buf; (void) modeRGB; (void) modeA;
   DISPATCH(BlendEquationSeparateiARB, (buf, modeRGB, modeA), (F, "glBlendEquationSeparateiARB(%d, 0x%x, 0x%x);\n", buf, modeRGB, modeA));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_711)(GLuint buf, GLenum modeRGB, GLenum modeA);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_711)(GLuint buf, GLenum modeRGB, GLenum modeA)
{
    (void) buf; (void) modeRGB; (void) modeA;
   DISPATCH(BlendEquationSeparateiARB, (buf, modeRGB, modeA), (F, "glBlendEquationSeparateIndexedAMD(%d, 0x%x, 0x%x);\n", buf, modeRGB, modeA));
}

KEYWORD1 void KEYWORD2 NAME(BlendEquationiARB)(GLuint buf, GLenum mode)
{
    (void) buf; (void) mode;
   DISPATCH(BlendEquationiARB, (buf, mode), (F, "glBlendEquationiARB(%d, 0x%x);\n", buf, mode));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_712)(GLuint buf, GLenum mode);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_712)(GLuint buf, GLenum mode)
{
    (void) buf; (void) mode;
   DISPATCH(BlendEquationiARB, (buf, mode), (F, "glBlendEquationIndexedAMD(%d, 0x%x);\n", buf, mode));
}

KEYWORD1 void KEYWORD2 NAME(BlendFuncSeparateiARB)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcA, GLenum dstA)
{
    (void) buf; (void) srcRGB; (void) dstRGB; (void) srcA; (void) dstA;
   DISPATCH(BlendFuncSeparateiARB, (buf, srcRGB, dstRGB, srcA, dstA), (F, "glBlendFuncSeparateiARB(%d, 0x%x, 0x%x, 0x%x, 0x%x);\n", buf, srcRGB, dstRGB, srcA, dstA));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_713)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcA, GLenum dstA);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_713)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcA, GLenum dstA)
{
    (void) buf; (void) srcRGB; (void) dstRGB; (void) srcA; (void) dstA;
   DISPATCH(BlendFuncSeparateiARB, (buf, srcRGB, dstRGB, srcA, dstA), (F, "glBlendFuncSeparateIndexedAMD(%d, 0x%x, 0x%x, 0x%x, 0x%x);\n", buf, srcRGB, dstRGB, srcA, dstA));
}

KEYWORD1 void KEYWORD2 NAME(BlendFunciARB)(GLuint buf, GLenum src, GLenum dst)
{
    (void) buf; (void) src; (void) dst;
   DISPATCH(BlendFunciARB, (buf, src, dst), (F, "glBlendFunciARB(%d, 0x%x, 0x%x);\n", buf, src, dst));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_714)(GLuint buf, GLenum src, GLenum dst);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_714)(GLuint buf, GLenum src, GLenum dst)
{
    (void) buf; (void) src; (void) dst;
   DISPATCH(BlendFunciARB, (buf, src, dst), (F, "glBlendFuncIndexedAMD(%d, 0x%x, 0x%x);\n", buf, src, dst));
}

KEYWORD1 void KEYWORD2 NAME(BindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name)
{
    (void) program; (void) colorNumber; (void) index; (void) name;
   DISPATCH(BindFragDataLocationIndexed, (program, colorNumber, index, name), (F, "glBindFragDataLocationIndexed(%d, %d, %d, %p);\n", program, colorNumber, index, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_715)(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_715)(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name)
{
    (void) program; (void) colorNumber; (void) index; (void) name;
   DISPATCH(BindFragDataLocationIndexed, (program, colorNumber, index, name), (F, "glBindFragDataLocationIndexedEXT(%d, %d, %d, %p);\n", program, colorNumber, index, (const void *) name));
}

KEYWORD1 GLint KEYWORD2 NAME(GetFragDataIndex)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetFragDataIndex, (program, name), (F, "glGetFragDataIndex(%d, %p);\n", program, (const void *) name));
}

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_716)(GLuint program, const GLchar * name);

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_716)(GLuint program, const GLchar * name)
{
    (void) program; (void) name;
   RETURN_DISPATCH(GetFragDataIndex, (program, name), (F, "glGetFragDataIndexEXT(%d, %p);\n", program, (const void *) name));
}

KEYWORD1 void KEYWORD2 NAME(BindSampler)(GLuint unit, GLuint sampler)
{
    (void) unit; (void) sampler;
   DISPATCH(BindSampler, (unit, sampler), (F, "glBindSampler(%d, %d);\n", unit, sampler));
}

KEYWORD1 void KEYWORD2 NAME(DeleteSamplers)(GLsizei count, const GLuint * samplers)
{
    (void) count; (void) samplers;
   DISPATCH(DeleteSamplers, (count, samplers), (F, "glDeleteSamplers(%d, %p);\n", count, (const void *) samplers));
}

KEYWORD1 void KEYWORD2 NAME(GenSamplers)(GLsizei count, GLuint * samplers)
{
    (void) count; (void) samplers;
   DISPATCH(GenSamplers, (count, samplers), (F, "glGenSamplers(%d, %p);\n", count, (const void *) samplers));
}

KEYWORD1 void KEYWORD2 NAME(GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(GetSamplerParameterIiv, (sampler, pname, params), (F, "glGetSamplerParameterIiv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(GetSamplerParameterIuiv, (sampler, pname, params), (F, "glGetSamplerParameterIuiv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(GetSamplerParameterfv, (sampler, pname, params), (F, "glGetSamplerParameterfv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(GetSamplerParameteriv, (sampler, pname, params), (F, "glGetSamplerParameteriv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsSampler)(GLuint sampler)
{
    (void) sampler;
   RETURN_DISPATCH(IsSampler, (sampler), (F, "glIsSampler(%d);\n", sampler));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(SamplerParameterIiv, (sampler, pname, params), (F, "glSamplerParameterIiv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(SamplerParameterIuiv, (sampler, pname, params), (F, "glSamplerParameterIuiv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
    (void) sampler; (void) pname; (void) param;
   DISPATCH(SamplerParameterf, (sampler, pname, param), (F, "glSamplerParameterf(%d, 0x%x, %f);\n", sampler, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(SamplerParameterfv, (sampler, pname, params), (F, "glSamplerParameterfv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
    (void) sampler; (void) pname; (void) param;
   DISPATCH(SamplerParameteri, (sampler, pname, param), (F, "glSamplerParameteri(%d, 0x%x, %d);\n", sampler, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint * params)
{
    (void) sampler; (void) pname; (void) params;
   DISPATCH(SamplerParameteriv, (sampler, pname, params), (F, "glSamplerParameteriv(%d, 0x%x, %p);\n", sampler, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_731)(GLuint id, GLenum pname, GLint64 * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_731)(GLuint id, GLenum pname, GLint64 * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjecti64v, (id, pname, params), (F, "glGetQueryObjecti64v(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_732)(GLuint id, GLenum pname, GLuint64 * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_732)(GLuint id, GLenum pname, GLuint64 * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetQueryObjectui64v, (id, pname, params), (F, "glGetQueryObjectui64v(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_733)(GLuint id, GLenum target);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_733)(GLuint id, GLenum target)
{
    (void) id; (void) target;
   DISPATCH(QueryCounter, (id, target), (F, "glQueryCounter(%d, 0x%x);\n", id, target));
}

KEYWORD1 void KEYWORD2 NAME(ColorP3ui)(GLenum type, GLuint color)
{
    (void) type; (void) color;
   DISPATCH(ColorP3ui, (type, color), (F, "glColorP3ui(0x%x, %d);\n", type, color));
}

KEYWORD1 void KEYWORD2 NAME(ColorP3uiv)(GLenum type, const GLuint * color)
{
    (void) type; (void) color;
   DISPATCH(ColorP3uiv, (type, color), (F, "glColorP3uiv(0x%x, %p);\n", type, (const void *) color));
}

KEYWORD1 void KEYWORD2 NAME(ColorP4ui)(GLenum type, GLuint color)
{
    (void) type; (void) color;
   DISPATCH(ColorP4ui, (type, color), (F, "glColorP4ui(0x%x, %d);\n", type, color));
}

KEYWORD1 void KEYWORD2 NAME(ColorP4uiv)(GLenum type, const GLuint * color)
{
    (void) type; (void) color;
   DISPATCH(ColorP4uiv, (type, color), (F, "glColorP4uiv(0x%x, %p);\n", type, (const void *) color));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP1ui, (texture, type, coords), (F, "glMultiTexCoordP1ui(0x%x, 0x%x, %d);\n", texture, type, coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint * coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP1uiv, (texture, type, coords), (F, "glMultiTexCoordP1uiv(0x%x, 0x%x, %p);\n", texture, type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP2ui, (texture, type, coords), (F, "glMultiTexCoordP2ui(0x%x, 0x%x, %d);\n", texture, type, coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint * coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP2uiv, (texture, type, coords), (F, "glMultiTexCoordP2uiv(0x%x, 0x%x, %p);\n", texture, type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP3ui, (texture, type, coords), (F, "glMultiTexCoordP3ui(0x%x, 0x%x, %d);\n", texture, type, coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint * coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP3uiv, (texture, type, coords), (F, "glMultiTexCoordP3uiv(0x%x, 0x%x, %p);\n", texture, type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP4ui, (texture, type, coords), (F, "glMultiTexCoordP4ui(0x%x, 0x%x, %d);\n", texture, type, coords));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint * coords)
{
    (void) texture; (void) type; (void) coords;
   DISPATCH(MultiTexCoordP4uiv, (texture, type, coords), (F, "glMultiTexCoordP4uiv(0x%x, 0x%x, %p);\n", texture, type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(NormalP3ui)(GLenum type, GLuint coords)
{
    (void) type; (void) coords;
   DISPATCH(NormalP3ui, (type, coords), (F, "glNormalP3ui(0x%x, %d);\n", type, coords));
}

KEYWORD1 void KEYWORD2 NAME(NormalP3uiv)(GLenum type, const GLuint * coords)
{
    (void) type; (void) coords;
   DISPATCH(NormalP3uiv, (type, coords), (F, "glNormalP3uiv(0x%x, %p);\n", type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColorP3ui)(GLenum type, GLuint color)
{
    (void) type; (void) color;
   DISPATCH(SecondaryColorP3ui, (type, color), (F, "glSecondaryColorP3ui(0x%x, %d);\n", type, color));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColorP3uiv)(GLenum type, const GLuint * color)
{
    (void) type; (void) color;
   DISPATCH(SecondaryColorP3uiv, (type, color), (F, "glSecondaryColorP3uiv(0x%x, %p);\n", type, (const void *) color));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP1ui)(GLenum type, GLuint coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP1ui, (type, coords), (F, "glTexCoordP1ui(0x%x, %d);\n", type, coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP1uiv)(GLenum type, const GLuint * coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP1uiv, (type, coords), (F, "glTexCoordP1uiv(0x%x, %p);\n", type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP2ui)(GLenum type, GLuint coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP2ui, (type, coords), (F, "glTexCoordP2ui(0x%x, %d);\n", type, coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP2uiv)(GLenum type, const GLuint * coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP2uiv, (type, coords), (F, "glTexCoordP2uiv(0x%x, %p);\n", type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP3ui)(GLenum type, GLuint coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP3ui, (type, coords), (F, "glTexCoordP3ui(0x%x, %d);\n", type, coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP3uiv)(GLenum type, const GLuint * coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP3uiv, (type, coords), (F, "glTexCoordP3uiv(0x%x, %p);\n", type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP4ui)(GLenum type, GLuint coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP4ui, (type, coords), (F, "glTexCoordP4ui(0x%x, %d);\n", type, coords));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordP4uiv)(GLenum type, const GLuint * coords)
{
    (void) type; (void) coords;
   DISPATCH(TexCoordP4uiv, (type, coords), (F, "glTexCoordP4uiv(0x%x, %p);\n", type, (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP1ui, (index, type, normalized, value), (F, "glVertexAttribP1ui(%d, 0x%x, %d, %d);\n", index, type, normalized, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP1uiv, (index, type, normalized, value), (F, "glVertexAttribP1uiv(%d, 0x%x, %d, %p);\n", index, type, normalized, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP2ui, (index, type, normalized, value), (F, "glVertexAttribP2ui(%d, 0x%x, %d, %d);\n", index, type, normalized, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP2uiv, (index, type, normalized, value), (F, "glVertexAttribP2uiv(%d, 0x%x, %d, %p);\n", index, type, normalized, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP3ui, (index, type, normalized, value), (F, "glVertexAttribP3ui(%d, 0x%x, %d, %d);\n", index, type, normalized, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP3uiv, (index, type, normalized, value), (F, "glVertexAttribP3uiv(%d, 0x%x, %d, %p);\n", index, type, normalized, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP4ui, (index, type, normalized, value), (F, "glVertexAttribP4ui(%d, 0x%x, %d, %d);\n", index, type, normalized, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value)
{
    (void) index; (void) type; (void) normalized; (void) value;
   DISPATCH(VertexAttribP4uiv, (index, type, normalized, value), (F, "glVertexAttribP4uiv(%d, 0x%x, %d, %p);\n", index, type, normalized, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP2ui)(GLenum type, GLuint value)
{
    (void) type; (void) value;
   DISPATCH(VertexP2ui, (type, value), (F, "glVertexP2ui(0x%x, %d);\n", type, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP2uiv)(GLenum type, const GLuint * value)
{
    (void) type; (void) value;
   DISPATCH(VertexP2uiv, (type, value), (F, "glVertexP2uiv(0x%x, %p);\n", type, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP3ui)(GLenum type, GLuint value)
{
    (void) type; (void) value;
   DISPATCH(VertexP3ui, (type, value), (F, "glVertexP3ui(0x%x, %d);\n", type, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP3uiv)(GLenum type, const GLuint * value)
{
    (void) type; (void) value;
   DISPATCH(VertexP3uiv, (type, value), (F, "glVertexP3uiv(0x%x, %p);\n", type, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP4ui)(GLenum type, GLuint value)
{
    (void) type; (void) value;
   DISPATCH(VertexP4ui, (type, value), (F, "glVertexP4ui(0x%x, %d);\n", type, value));
}

KEYWORD1 void KEYWORD2 NAME(VertexP4uiv)(GLenum type, const GLuint * value)
{
    (void) type; (void) value;
   DISPATCH(VertexP4uiv, (type, value), (F, "glVertexP4uiv(0x%x, %p);\n", type, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysIndirect)(GLenum mode, const GLvoid * indirect)
{
    (void) mode; (void) indirect;
   DISPATCH(DrawArraysIndirect, (mode, indirect), (F, "glDrawArraysIndirect(0x%x, %p);\n", mode, (const void *) indirect));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid * indirect)
{
    (void) mode; (void) type; (void) indirect;
   DISPATCH(DrawElementsIndirect, (mode, type, indirect), (F, "glDrawElementsIndirect(0x%x, 0x%x, %p);\n", mode, type, (const void *) indirect));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_774)(GLuint program, GLint location, GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_774)(GLuint program, GLint location, GLdouble * params)
{
    (void) program; (void) location; (void) params;
   DISPATCH(GetUniformdv, (program, location, params), (F, "glGetUniformdv(%d, %d, %p);\n", program, location, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_775)(GLint location, GLdouble x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_775)(GLint location, GLdouble x)
{
    (void) location; (void) x;
   DISPATCH(Uniform1d, (location, x), (F, "glUniform1d(%d, %f);\n", location, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_776)(GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_776)(GLint location, GLsizei count, const GLdouble * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform1dv, (location, count, value), (F, "glUniform1dv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_777)(GLint location, GLdouble x, GLdouble y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_777)(GLint location, GLdouble x, GLdouble y)
{
    (void) location; (void) x; (void) y;
   DISPATCH(Uniform2d, (location, x, y), (F, "glUniform2d(%d, %f, %f);\n", location, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_778)(GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_778)(GLint location, GLsizei count, const GLdouble * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform2dv, (location, count, value), (F, "glUniform2dv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_779)(GLint location, GLdouble x, GLdouble y, GLdouble z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_779)(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    (void) location; (void) x; (void) y; (void) z;
   DISPATCH(Uniform3d, (location, x, y, z), (F, "glUniform3d(%d, %f, %f, %f);\n", location, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_780)(GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_780)(GLint location, GLsizei count, const GLdouble * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform3dv, (location, count, value), (F, "glUniform3dv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_781)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_781)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(Uniform4d, (location, x, y, z, w), (F, "glUniform4d(%d, %f, %f, %f, %f);\n", location, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_782)(GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_782)(GLint location, GLsizei count, const GLdouble * value)
{
    (void) location; (void) count; (void) value;
   DISPATCH(Uniform4dv, (location, count, value), (F, "glUniform4dv(%d, %d, %p);\n", location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_783)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_783)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2dv, (location, count, transpose, value), (F, "glUniformMatrix2dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_784)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_784)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2x3dv, (location, count, transpose, value), (F, "glUniformMatrix2x3dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_785)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_785)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix2x4dv, (location, count, transpose, value), (F, "glUniformMatrix2x4dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_786)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_786)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3dv, (location, count, transpose, value), (F, "glUniformMatrix3dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_787)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_787)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3x2dv, (location, count, transpose, value), (F, "glUniformMatrix3x2dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_788)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_788)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix3x4dv, (location, count, transpose, value), (F, "glUniformMatrix3x4dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_789)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_789)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4dv, (location, count, transpose, value), (F, "glUniformMatrix4dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_790)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_790)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4x2dv, (location, count, transpose, value), (F, "glUniformMatrix4x2dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_791)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_791)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(UniformMatrix4x3dv, (location, count, transpose, value), (F, "glUniformMatrix4x3dv(%d, %d, %d, %p);\n", location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_792)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_792)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name)
{
    (void) program; (void) shadertype; (void) index; (void) bufsize; (void) length; (void) name;
   DISPATCH(GetActiveSubroutineName, (program, shadertype, index, bufsize, length, name), (F, "glGetActiveSubroutineName(%d, 0x%x, %d, %d, %p, %p);\n", program, shadertype, index, bufsize, (const void *) length, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_793)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_793)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name)
{
    (void) program; (void) shadertype; (void) index; (void) bufsize; (void) length; (void) name;
   DISPATCH(GetActiveSubroutineUniformName, (program, shadertype, index, bufsize, length, name), (F, "glGetActiveSubroutineUniformName(%d, 0x%x, %d, %d, %p, %p);\n", program, shadertype, index, bufsize, (const void *) length, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_794)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_794)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint * values)
{
    (void) program; (void) shadertype; (void) index; (void) pname; (void) values;
   DISPATCH(GetActiveSubroutineUniformiv, (program, shadertype, index, pname, values), (F, "glGetActiveSubroutineUniformiv(%d, 0x%x, %d, 0x%x, %p);\n", program, shadertype, index, pname, (const void *) values));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_795)(GLuint program, GLenum shadertype, GLenum pname, GLint * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_795)(GLuint program, GLenum shadertype, GLenum pname, GLint * values)
{
    (void) program; (void) shadertype; (void) pname; (void) values;
   DISPATCH(GetProgramStageiv, (program, shadertype, pname, values), (F, "glGetProgramStageiv(%d, 0x%x, 0x%x, %p);\n", program, shadertype, pname, (const void *) values));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_796)(GLuint program, GLenum shadertype, const GLchar * name);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_796)(GLuint program, GLenum shadertype, const GLchar * name)
{
    (void) program; (void) shadertype; (void) name;
   RETURN_DISPATCH(GetSubroutineIndex, (program, shadertype, name), (F, "glGetSubroutineIndex(%d, 0x%x, %p);\n", program, shadertype, (const void *) name));
}

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_797)(GLuint program, GLenum shadertype, const GLchar * name);

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_797)(GLuint program, GLenum shadertype, const GLchar * name)
{
    (void) program; (void) shadertype; (void) name;
   RETURN_DISPATCH(GetSubroutineUniformLocation, (program, shadertype, name), (F, "glGetSubroutineUniformLocation(%d, 0x%x, %p);\n", program, shadertype, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_798)(GLenum shadertype, GLint location, GLuint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_798)(GLenum shadertype, GLint location, GLuint * params)
{
    (void) shadertype; (void) location; (void) params;
   DISPATCH(GetUniformSubroutineuiv, (shadertype, location, params), (F, "glGetUniformSubroutineuiv(0x%x, %d, %p);\n", shadertype, location, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_799)(GLenum shadertype, GLsizei count, const GLuint * indices);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_799)(GLenum shadertype, GLsizei count, const GLuint * indices)
{
    (void) shadertype; (void) count; (void) indices;
   DISPATCH(UniformSubroutinesuiv, (shadertype, count, indices), (F, "glUniformSubroutinesuiv(0x%x, %d, %p);\n", shadertype, count, (const void *) indices));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_800)(GLenum pname, const GLfloat * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_800)(GLenum pname, const GLfloat * values)
{
    (void) pname; (void) values;
   DISPATCH(PatchParameterfv, (pname, values), (F, "glPatchParameterfv(0x%x, %p);\n", pname, (const void *) values));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_801)(GLenum pname, GLint value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_801)(GLenum pname, GLint value)
{
    (void) pname; (void) value;
   DISPATCH(PatchParameteri, (pname, value), (F, "glPatchParameteri(0x%x, %d);\n", pname, value));
}

KEYWORD1 void KEYWORD2 NAME(BindTransformFeedback)(GLenum target, GLuint id)
{
    (void) target; (void) id;
   DISPATCH(BindTransformFeedback, (target, id), (F, "glBindTransformFeedback(0x%x, %d);\n", target, id));
}

KEYWORD1 void KEYWORD2 NAME(DeleteTransformFeedbacks)(GLsizei n, const GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(DeleteTransformFeedbacks, (n, ids), (F, "glDeleteTransformFeedbacks(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 void KEYWORD2 NAME(DrawTransformFeedback)(GLenum mode, GLuint id)
{
    (void) mode; (void) id;
   DISPATCH(DrawTransformFeedback, (mode, id), (F, "glDrawTransformFeedback(0x%x, %d);\n", mode, id));
}

KEYWORD1 void KEYWORD2 NAME(GenTransformFeedbacks)(GLsizei n, GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(GenTransformFeedbacks, (n, ids), (F, "glGenTransformFeedbacks(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsTransformFeedback)(GLuint id)
{
    (void) id;
   RETURN_DISPATCH(IsTransformFeedback, (id), (F, "glIsTransformFeedback(%d);\n", id));
}

KEYWORD1 void KEYWORD2 NAME(PauseTransformFeedback)(void)
{
   DISPATCH(PauseTransformFeedback, (), (F, "glPauseTransformFeedback();\n"));
}

KEYWORD1 void KEYWORD2 NAME(ResumeTransformFeedback)(void)
{
   DISPATCH(ResumeTransformFeedback, (), (F, "glResumeTransformFeedback();\n"));
}

KEYWORD1 void KEYWORD2 NAME(BeginQueryIndexed)(GLenum target, GLuint index, GLuint id)
{
    (void) target; (void) index; (void) id;
   DISPATCH(BeginQueryIndexed, (target, index, id), (F, "glBeginQueryIndexed(0x%x, %d, %d);\n", target, index, id));
}

KEYWORD1 void KEYWORD2 NAME(DrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream)
{
    (void) mode; (void) id; (void) stream;
   DISPATCH(DrawTransformFeedbackStream, (mode, id, stream), (F, "glDrawTransformFeedbackStream(0x%x, %d, %d);\n", mode, id, stream));
}

KEYWORD1 void KEYWORD2 NAME(EndQueryIndexed)(GLenum target, GLuint index)
{
    (void) target; (void) index;
   DISPATCH(EndQueryIndexed, (target, index), (F, "glEndQueryIndexed(0x%x, %d);\n", target, index));
}

KEYWORD1 void KEYWORD2 NAME(GetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint * params)
{
    (void) target; (void) index; (void) pname; (void) params;
   DISPATCH(GetQueryIndexediv, (target, index, pname, params), (F, "glGetQueryIndexediv(0x%x, %d, 0x%x, %p);\n", target, index, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ClearDepthf)(GLclampf depth)
{
    (void) depth;
   DISPATCH(ClearDepthf, (depth), (F, "glClearDepthf(%f);\n", depth));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_813)(GLclampf depth);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_813)(GLclampf depth)
{
    (void) depth;
   DISPATCH(ClearDepthf, (depth), (F, "glClearDepthfOES(%f);\n", depth));
}

KEYWORD1 void KEYWORD2 NAME(DepthRangef)(GLclampf zNear, GLclampf zFar)
{
    (void) zNear; (void) zFar;
   DISPATCH(DepthRangef, (zNear, zFar), (F, "glDepthRangef(%f, %f);\n", zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_814)(GLclampf zNear, GLclampf zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_814)(GLclampf zNear, GLclampf zFar)
{
    (void) zNear; (void) zFar;
   DISPATCH(DepthRangef, (zNear, zFar), (F, "glDepthRangefOES(%f, %f);\n", zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision)
{
    (void) shadertype; (void) precisiontype; (void) range; (void) precision;
   DISPATCH(GetShaderPrecisionFormat, (shadertype, precisiontype, range, precision), (F, "glGetShaderPrecisionFormat(0x%x, 0x%x, %p, %p);\n", shadertype, precisiontype, (const void *) range, (const void *) precision));
}

KEYWORD1 void KEYWORD2 NAME(ReleaseShaderCompiler)(void)
{
   DISPATCH(ReleaseShaderCompiler, (), (F, "glReleaseShaderCompiler();\n"));
}

KEYWORD1 void KEYWORD2 NAME(ShaderBinary)(GLsizei n, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length)
{
    (void) n; (void) shaders; (void) binaryformat; (void) binary; (void) length;
   DISPATCH(ShaderBinary, (n, shaders, binaryformat, binary, length), (F, "glShaderBinary(%d, %p, 0x%x, %p, %d);\n", n, (const void *) shaders, binaryformat, (const void *) binary, length));
}

KEYWORD1 void KEYWORD2 NAME(GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary)
{
    (void) program; (void) bufSize; (void) length; (void) binaryFormat; (void) binary;
   DISPATCH(GetProgramBinary, (program, bufSize, length, binaryFormat, binary), (F, "glGetProgramBinary(%d, %d, %p, %p, %p);\n", program, bufSize, (const void *) length, (const void *) binaryFormat, (const void *) binary));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_818)(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_818)(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary)
{
    (void) program; (void) bufSize; (void) length; (void) binaryFormat; (void) binary;
   DISPATCH(GetProgramBinary, (program, bufSize, length, binaryFormat, binary), (F, "glGetProgramBinaryOES(%d, %d, %p, %p, %p);\n", program, bufSize, (const void *) length, (const void *) binaryFormat, (const void *) binary));
}

KEYWORD1 void KEYWORD2 NAME(ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLsizei length)
{
    (void) program; (void) binaryFormat; (void) binary; (void) length;
   DISPATCH(ProgramBinary, (program, binaryFormat, binary, length), (F, "glProgramBinary(%d, 0x%x, %p, %d);\n", program, binaryFormat, (const void *) binary, length));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_819)(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLint length);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_819)(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLint length)
{
    (void) program; (void) binaryFormat; (void) binary; (void) length;
   DISPATCH(ProgramBinary, (program, binaryFormat, binary, length), (F, "glProgramBinaryOES(%d, 0x%x, %p, %d);\n", program, binaryFormat, (const void *) binary, length));
}

KEYWORD1 void KEYWORD2 NAME(ProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
    (void) program; (void) pname; (void) value;
   DISPATCH(ProgramParameteri, (program, pname, value), (F, "glProgramParameteri(%d, 0x%x, %d);\n", program, pname, value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_820)(GLuint program, GLenum pname, GLint value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_820)(GLuint program, GLenum pname, GLint value)
{
    (void) program; (void) pname; (void) value;
   DISPATCH(ProgramParameteri, (program, pname, value), (F, "glProgramParameteriEXT(%d, 0x%x, %d);\n", program, pname, value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_821)(GLuint index, GLenum pname, GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_821)(GLuint index, GLenum pname, GLdouble * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribLdv, (index, pname, params), (F, "glGetVertexAttribLdv(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_822)(GLuint index, GLdouble x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_822)(GLuint index, GLdouble x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttribL1d, (index, x), (F, "glVertexAttribL1d(%d, %f);\n", index, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_823)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_823)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribL1dv, (index, v), (F, "glVertexAttribL1dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_824)(GLuint index, GLdouble x, GLdouble y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_824)(GLuint index, GLdouble x, GLdouble y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttribL2d, (index, x, y), (F, "glVertexAttribL2d(%d, %f, %f);\n", index, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_825)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_825)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribL2dv, (index, v), (F, "glVertexAttribL2dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_826)(GLuint index, GLdouble x, GLdouble y, GLdouble z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_826)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttribL3d, (index, x, y, z), (F, "glVertexAttribL3d(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_827)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_827)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribL3dv, (index, v), (F, "glVertexAttribL3dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_828)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_828)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttribL4d, (index, x, y, z, w), (F, "glVertexAttribL4d(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_829)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_829)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribL4dv, (index, v), (F, "glVertexAttribL4dv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_830)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_830)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(VertexAttribLPointer, (index, size, type, stride, pointer), (F, "glVertexAttribLPointer(%d, %d, 0x%x, %d, %p);\n", index, size, type, stride, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(DepthRangeArrayv)(GLuint first, GLsizei count, const GLclampd * v)
{
    (void) first; (void) count; (void) v;
   DISPATCH(DepthRangeArrayv, (first, count, v), (F, "glDepthRangeArrayv(%d, %d, %p);\n", first, count, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(DepthRangeIndexed)(GLuint index, GLclampd n, GLclampd f)
{
    (void) index; (void) n; (void) f;
   DISPATCH(DepthRangeIndexed, (index, n, f), (F, "glDepthRangeIndexed(%d, %f, %f);\n", index, n, f));
}

KEYWORD1 void KEYWORD2 NAME(GetDoublei_v)(GLenum target, GLuint index, GLdouble * data)
{
    (void) target; (void) index; (void) data;
   DISPATCH(GetDoublei_v, (target, index, data), (F, "glGetDoublei_v(0x%x, %d, %p);\n", target, index, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(GetFloati_v)(GLenum target, GLuint index, GLfloat * data)
{
    (void) target; (void) index; (void) data;
   DISPATCH(GetFloati_v, (target, index, data), (F, "glGetFloati_v(0x%x, %d, %p);\n", target, index, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(ScissorArrayv)(GLuint first, GLsizei count, const int * v)
{
    (void) first; (void) count; (void) v;
   DISPATCH(ScissorArrayv, (first, count, v), (F, "glScissorArrayv(%d, %d, %p);\n", first, count, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(ScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
    (void) index; (void) left; (void) bottom; (void) width; (void) height;
   DISPATCH(ScissorIndexed, (index, left, bottom, width, height), (F, "glScissorIndexed(%d, %d, %d, %d, %d);\n", index, left, bottom, width, height));
}

KEYWORD1 void KEYWORD2 NAME(ScissorIndexedv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(ScissorIndexedv, (index, v), (F, "glScissorIndexedv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(ViewportArrayv)(GLuint first, GLsizei count, const GLfloat * v)
{
    (void) first; (void) count; (void) v;
   DISPATCH(ViewportArrayv, (first, count, v), (F, "glViewportArrayv(%d, %d, %p);\n", first, count, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(ViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    (void) index; (void) x; (void) y; (void) w; (void) h;
   DISPATCH(ViewportIndexedf, (index, x, y, w, h), (F, "glViewportIndexedf(%d, %f, %f, %f, %f);\n", index, x, y, w, h));
}

KEYWORD1 void KEYWORD2 NAME(ViewportIndexedfv)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(ViewportIndexedfv, (index, v), (F, "glViewportIndexedfv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 GLenum KEYWORD2 NAME(GetGraphicsResetStatusARB)(void)
{
   RETURN_DISPATCH(GetGraphicsResetStatusARB, (), (F, "glGetGraphicsResetStatusARB();\n"));
}

KEYWORD1 void KEYWORD2 NAME(GetnColorTableARB)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid * table)
{
    (void) target; (void) format; (void) type; (void) bufSize; (void) table;
   DISPATCH(GetnColorTableARB, (target, format, type, bufSize, table), (F, "glGetnColorTableARB(0x%x, 0x%x, 0x%x, %d, %p);\n", target, format, type, bufSize, (const void *) table));
}

KEYWORD1 void KEYWORD2 NAME(GetnCompressedTexImageARB)(GLenum target, GLint lod, GLsizei bufSize, GLvoid * img)
{
    (void) target; (void) lod; (void) bufSize; (void) img;
   DISPATCH(GetnCompressedTexImageARB, (target, lod, bufSize, img), (F, "glGetnCompressedTexImageARB(0x%x, %d, %d, %p);\n", target, lod, bufSize, (const void *) img));
}

KEYWORD1 void KEYWORD2 NAME(GetnConvolutionFilterARB)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid * image)
{
    (void) target; (void) format; (void) type; (void) bufSize; (void) image;
   DISPATCH(GetnConvolutionFilterARB, (target, format, type, bufSize, image), (F, "glGetnConvolutionFilterARB(0x%x, 0x%x, 0x%x, %d, %p);\n", target, format, type, bufSize, (const void *) image));
}

KEYWORD1 void KEYWORD2 NAME(GetnHistogramARB)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) bufSize; (void) values;
   DISPATCH(GetnHistogramARB, (target, reset, format, type, bufSize, values), (F, "glGetnHistogramARB(0x%x, %d, 0x%x, 0x%x, %d, %p);\n", target, reset, format, type, bufSize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetnMapdvARB)(GLenum target, GLenum query, GLsizei bufSize, GLdouble * v)
{
    (void) target; (void) query; (void) bufSize; (void) v;
   DISPATCH(GetnMapdvARB, (target, query, bufSize, v), (F, "glGetnMapdvARB(0x%x, 0x%x, %d, %p);\n", target, query, bufSize, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetnMapfvARB)(GLenum target, GLenum query, GLsizei bufSize, GLfloat * v)
{
    (void) target; (void) query; (void) bufSize; (void) v;
   DISPATCH(GetnMapfvARB, (target, query, bufSize, v), (F, "glGetnMapfvARB(0x%x, 0x%x, %d, %p);\n", target, query, bufSize, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetnMapivARB)(GLenum target, GLenum query, GLsizei bufSize, GLint * v)
{
    (void) target; (void) query; (void) bufSize; (void) v;
   DISPATCH(GetnMapivARB, (target, query, bufSize, v), (F, "glGetnMapivARB(0x%x, 0x%x, %d, %p);\n", target, query, bufSize, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(GetnMinmaxARB)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) bufSize; (void) values;
   DISPATCH(GetnMinmaxARB, (target, reset, format, type, bufSize, values), (F, "glGetnMinmaxARB(0x%x, %d, 0x%x, 0x%x, %d, %p);\n", target, reset, format, type, bufSize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetnPixelMapfvARB)(GLenum map, GLsizei bufSize, GLfloat * values)
{
    (void) map; (void) bufSize; (void) values;
   DISPATCH(GetnPixelMapfvARB, (map, bufSize, values), (F, "glGetnPixelMapfvARB(0x%x, %d, %p);\n", map, bufSize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetnPixelMapuivARB)(GLenum map, GLsizei bufSize, GLuint * values)
{
    (void) map; (void) bufSize; (void) values;
   DISPATCH(GetnPixelMapuivARB, (map, bufSize, values), (F, "glGetnPixelMapuivARB(0x%x, %d, %p);\n", map, bufSize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetnPixelMapusvARB)(GLenum map, GLsizei bufSize, GLushort * values)
{
    (void) map; (void) bufSize; (void) values;
   DISPATCH(GetnPixelMapusvARB, (map, bufSize, values), (F, "glGetnPixelMapusvARB(0x%x, %d, %p);\n", map, bufSize, (const void *) values));
}

KEYWORD1 void KEYWORD2 NAME(GetnPolygonStippleARB)(GLsizei bufSize, GLubyte * pattern)
{
    (void) bufSize; (void) pattern;
   DISPATCH(GetnPolygonStippleARB, (bufSize, pattern), (F, "glGetnPolygonStippleARB(%d, %p);\n", bufSize, (const void *) pattern));
}

KEYWORD1 void KEYWORD2 NAME(GetnSeparableFilterARB)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, GLvoid * row, GLsizei columnBufSize, GLvoid * column, GLvoid * span)
{
    (void) target; (void) format; (void) type; (void) rowBufSize; (void) row; (void) columnBufSize; (void) column; (void) span;
   DISPATCH(GetnSeparableFilterARB, (target, format, type, rowBufSize, row, columnBufSize, column, span), (F, "glGetnSeparableFilterARB(0x%x, 0x%x, 0x%x, %d, %p, %d, %p, %p);\n", target, format, type, rowBufSize, (const void *) row, columnBufSize, (const void *) column, (const void *) span));
}

KEYWORD1 void KEYWORD2 NAME(GetnTexImageARB)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid * img)
{
    (void) target; (void) level; (void) format; (void) type; (void) bufSize; (void) img;
   DISPATCH(GetnTexImageARB, (target, level, format, type, bufSize, img), (F, "glGetnTexImageARB(0x%x, %d, 0x%x, 0x%x, %d, %p);\n", target, level, format, type, bufSize, (const void *) img));
}

KEYWORD1 void KEYWORD2 NAME(GetnUniformdvARB)(GLuint program, GLint location, GLsizei bufSize, GLdouble * params)
{
    (void) program; (void) location; (void) bufSize; (void) params;
   DISPATCH(GetnUniformdvARB, (program, location, bufSize, params), (F, "glGetnUniformdvARB(%d, %d, %d, %p);\n", program, location, bufSize, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetnUniformfvARB)(GLuint program, GLint location, GLsizei bufSize, GLfloat * params)
{
    (void) program; (void) location; (void) bufSize; (void) params;
   DISPATCH(GetnUniformfvARB, (program, location, bufSize, params), (F, "glGetnUniformfvARB(%d, %d, %d, %p);\n", program, location, bufSize, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetnUniformivARB)(GLuint program, GLint location, GLsizei bufSize, GLint * params)
{
    (void) program; (void) location; (void) bufSize; (void) params;
   DISPATCH(GetnUniformivARB, (program, location, bufSize, params), (F, "glGetnUniformivARB(%d, %d, %d, %p);\n", program, location, bufSize, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetnUniformuivARB)(GLuint program, GLint location, GLsizei bufSize, GLuint * params)
{
    (void) program; (void) location; (void) bufSize; (void) params;
   DISPATCH(GetnUniformuivARB, (program, location, bufSize, params), (F, "glGetnUniformuivARB(%d, %d, %d, %p);\n", program, location, bufSize, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(ReadnPixelsARB)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid * data)
{
    (void) x; (void) y; (void) width; (void) height; (void) format; (void) type; (void) bufSize; (void) data;
   DISPATCH(ReadnPixelsARB, (x, y, width, height, format, type, bufSize, data), (F, "glReadnPixelsARB(%d, %d, %d, %d, 0x%x, 0x%x, %d, %p);\n", x, y, width, height, format, type, bufSize, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(DrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
{
    (void) mode; (void) first; (void) count; (void) primcount; (void) baseinstance;
   DISPATCH(DrawArraysInstancedBaseInstance, (mode, first, count, primcount, baseinstance), (F, "glDrawArraysInstancedBaseInstance(0x%x, %d, %d, %d, %d);\n", mode, first, count, primcount, baseinstance));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLuint baseinstance)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) baseinstance;
   DISPATCH(DrawElementsInstancedBaseInstance, (mode, count, type, indices, primcount, baseinstance), (F, "glDrawElementsInstancedBaseInstance(0x%x, %d, 0x%x, %p, %d, %d);\n", mode, count, type, (const void *) indices, primcount, baseinstance));
}

KEYWORD1 void KEYWORD2 NAME(DrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex, GLuint baseinstance)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) basevertex; (void) baseinstance;
   DISPATCH(DrawElementsInstancedBaseVertexBaseInstance, (mode, count, type, indices, primcount, basevertex, baseinstance), (F, "glDrawElementsInstancedBaseVertexBaseInstance(0x%x, %d, 0x%x, %p, %d, %d, %d);\n", mode, count, type, (const void *) indices, primcount, basevertex, baseinstance));
}

KEYWORD1 void KEYWORD2 NAME(DrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei primcount)
{
    (void) mode; (void) id; (void) primcount;
   DISPATCH(DrawTransformFeedbackInstanced, (mode, id, primcount), (F, "glDrawTransformFeedbackInstanced(0x%x, %d, %d);\n", mode, id, primcount));
}

KEYWORD1 void KEYWORD2 NAME(DrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei primcount)
{
    (void) mode; (void) id; (void) stream; (void) primcount;
   DISPATCH(DrawTransformFeedbackStreamInstanced, (mode, id, stream, primcount), (F, "glDrawTransformFeedbackStreamInstanced(0x%x, %d, %d, %d);\n", mode, id, stream, primcount));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_866)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_866)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params)
{
    (void) target; (void) internalformat; (void) pname; (void) bufSize; (void) params;
   DISPATCH(GetInternalformativ, (target, internalformat, pname, bufSize, params), (F, "glGetInternalformativ(0x%x, 0x%x, 0x%x, %d, %p);\n", target, internalformat, pname, bufSize, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint * params)
{
    (void) program; (void) bufferIndex; (void) pname; (void) params;
   DISPATCH(GetActiveAtomicCounterBufferiv, (program, bufferIndex, pname, params), (F, "glGetActiveAtomicCounterBufferiv(%d, %d, 0x%x, %p);\n", program, bufferIndex, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    (void) unit; (void) texture; (void) level; (void) layered; (void) layer; (void) access; (void) format;
   DISPATCH(BindImageTexture, (unit, texture, level, layered, layer, access, format), (F, "glBindImageTexture(%d, %d, %d, %d, %d, 0x%x, 0x%x);\n", unit, texture, level, layered, layer, access, format));
}

KEYWORD1 void KEYWORD2 NAME(MemoryBarrier)(GLbitfield barriers)
{
    (void) barriers;
   DISPATCH(MemoryBarrier, (barriers), (F, "glMemoryBarrier(%d);\n", barriers));
}

KEYWORD1 void KEYWORD2 NAME(TexStorage1D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width)
{
    (void) target; (void) levels; (void) internalFormat; (void) width;
   DISPATCH(TexStorage1D, (target, levels, internalFormat, width), (F, "glTexStorage1D(0x%x, %d, 0x%x, %d);\n", target, levels, internalFormat, width));
}

KEYWORD1 void KEYWORD2 NAME(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
    (void) target; (void) levels; (void) internalFormat; (void) width; (void) height;
   DISPATCH(TexStorage2D, (target, levels, internalFormat, width, height), (F, "glTexStorage2D(0x%x, %d, 0x%x, %d, %d);\n", target, levels, internalFormat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(TexStorage3D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth)
{
    (void) target; (void) levels; (void) internalFormat; (void) width; (void) height; (void) depth;
   DISPATCH(TexStorage3D, (target, levels, internalFormat, width, height, depth), (F, "glTexStorage3D(0x%x, %d, 0x%x, %d, %d, %d);\n", target, levels, internalFormat, width, height, depth));
}

KEYWORD1 void KEYWORD2 NAME(TextureStorage1DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width)
{
    (void) texture; (void) target; (void) levels; (void) internalFormat; (void) width;
   DISPATCH(TextureStorage1DEXT, (texture, target, levels, internalFormat, width), (F, "glTextureStorage1DEXT(%d, 0x%x, %d, 0x%x, %d);\n", texture, target, levels, internalFormat, width));
}

KEYWORD1 void KEYWORD2 NAME(TextureStorage2DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
    (void) texture; (void) target; (void) levels; (void) internalFormat; (void) width; (void) height;
   DISPATCH(TextureStorage2DEXT, (texture, target, levels, internalFormat, width, height), (F, "glTextureStorage2DEXT(%d, 0x%x, %d, 0x%x, %d, %d);\n", texture, target, levels, internalFormat, width, height));
}

KEYWORD1 void KEYWORD2 NAME(TextureStorage3DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth)
{
    (void) texture; (void) target; (void) levels; (void) internalFormat; (void) width; (void) height; (void) depth;
   DISPATCH(TextureStorage3DEXT, (texture, target, levels, internalFormat, width, height, depth), (F, "glTextureStorage3DEXT(%d, 0x%x, %d, 0x%x, %d, %d, %d);\n", texture, target, levels, internalFormat, width, height, depth));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const GLvoid * data)
{
    (void) target; (void) internalformat; (void) format; (void) type; (void) data;
   DISPATCH(ClearBufferData, (target, internalformat, format, type, data), (F, "glClearBufferData(0x%x, 0x%x, 0x%x, 0x%x, %p);\n", target, internalformat, format, type, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(ClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid * data)
{
    (void) target; (void) internalformat; (void) offset; (void) size; (void) format; (void) type; (void) data;
   DISPATCH(ClearBufferSubData, (target, internalformat, offset, size, format, type, data), (F, "glClearBufferSubData(0x%x, 0x%x, %d, %d, 0x%x, 0x%x, %p);\n", target, internalformat, offset, size, format, type, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    (void) num_groups_x; (void) num_groups_y; (void) num_groups_z;
   DISPATCH(DispatchCompute, (num_groups_x, num_groups_y, num_groups_z), (F, "glDispatchCompute(%d, %d, %d);\n", num_groups_x, num_groups_y, num_groups_z));
}

KEYWORD1 void KEYWORD2 NAME(DispatchComputeIndirect)(GLintptr indirect)
{
    (void) indirect;
   DISPATCH(DispatchComputeIndirect, (indirect), (F, "glDispatchComputeIndirect(%d);\n", indirect));
}

KEYWORD1 void KEYWORD2 NAME(CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    (void) srcName; (void) srcTarget; (void) srcLevel; (void) srcX; (void) srcY; (void) srcZ; (void) dstName; (void) dstTarget; (void) dstLevel; (void) dstX; (void) dstY; (void) dstZ; (void) srcWidth; (void) srcHeight; (void) srcDepth;
   DISPATCH(CopyImageSubData, (srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth), (F, "glCopyImageSubData(%d, 0x%x, %d, %d, %d, %d, %d, 0x%x, %d, %d, %d, %d, %d, %d, %d);\n", srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth));
}

KEYWORD1 void KEYWORD2 NAME(TextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    (void) texture; (void) target; (void) origtexture; (void) internalformat; (void) minlevel; (void) numlevels; (void) minlayer; (void) numlayers;
   DISPATCH(TextureView, (texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers), (F, "glTextureView(%d, 0x%x, %d, 0x%x, %d, %d, %d, %d);\n", texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers));
}

KEYWORD1 void KEYWORD2 NAME(BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    (void) bindingindex; (void) buffer; (void) offset; (void) stride;
   DISPATCH(BindVertexBuffer, (bindingindex, buffer, offset, stride), (F, "glBindVertexBuffer(%d, %d, %d, %d);\n", bindingindex, buffer, offset, stride));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
    (void) attribindex; (void) bindingindex;
   DISPATCH(VertexAttribBinding, (attribindex, bindingindex), (F, "glVertexAttribBinding(%d, %d);\n", attribindex, bindingindex));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    (void) attribindex; (void) size; (void) type; (void) normalized; (void) relativeoffset;
   DISPATCH(VertexAttribFormat, (attribindex, size, type, normalized, relativeoffset), (F, "glVertexAttribFormat(%d, %d, 0x%x, %d, %d);\n", attribindex, size, type, normalized, relativeoffset));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    (void) attribindex; (void) size; (void) type; (void) relativeoffset;
   DISPATCH(VertexAttribIFormat, (attribindex, size, type, relativeoffset), (F, "glVertexAttribIFormat(%d, %d, 0x%x, %d);\n", attribindex, size, type, relativeoffset));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    (void) attribindex; (void) size; (void) type; (void) relativeoffset;
   DISPATCH(VertexAttribLFormat, (attribindex, size, type, relativeoffset), (F, "glVertexAttribLFormat(%d, %d, 0x%x, %d);\n", attribindex, size, type, relativeoffset));
}

KEYWORD1 void KEYWORD2 NAME(VertexBindingDivisor)(GLuint attribindex, GLuint divisor)
{
    (void) attribindex; (void) divisor;
   DISPATCH(VertexBindingDivisor, (attribindex, divisor), (F, "glVertexBindingDivisor(%d, %d);\n", attribindex, divisor));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_888)(GLenum target, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_888)(GLenum target, GLenum pname, GLint param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(FramebufferParameteri, (target, pname, param), (F, "glFramebufferParameteri(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_889)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_889)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetFramebufferParameteriv, (target, pname, params), (F, "glGetFramebufferParameteriv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawArraysIndirect)(GLenum mode, const GLvoid * indirect, GLsizei primcount, GLsizei stride)
{
    (void) mode; (void) indirect; (void) primcount; (void) stride;
   DISPATCH(MultiDrawArraysIndirect, (mode, indirect, primcount, stride), (F, "glMultiDrawArraysIndirect(0x%x, %p, %d, %d);\n", mode, (const void *) indirect, primcount, stride));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid * indirect, GLsizei primcount, GLsizei stride)
{
    (void) mode; (void) type; (void) indirect; (void) primcount; (void) stride;
   DISPATCH(MultiDrawElementsIndirect, (mode, type, indirect, primcount, stride), (F, "glMultiDrawElementsIndirect(0x%x, 0x%x, %p, %d, %d);\n", mode, type, (const void *) indirect, primcount, stride));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_892)(GLuint program, GLenum programInterface, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_892)(GLuint program, GLenum programInterface, GLenum pname, GLint * params)
{
    (void) program; (void) programInterface; (void) pname; (void) params;
   DISPATCH(GetProgramInterfaceiv, (program, programInterface, pname, params), (F, "glGetProgramInterfaceiv(%d, 0x%x, 0x%x, %p);\n", program, programInterface, pname, (const void *) params));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_893)(GLuint program, GLenum programInterface, const GLchar * name);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_893)(GLuint program, GLenum programInterface, const GLchar * name)
{
    (void) program; (void) programInterface; (void) name;
   RETURN_DISPATCH(GetProgramResourceIndex, (program, programInterface, name), (F, "glGetProgramResourceIndex(%d, 0x%x, %p);\n", program, programInterface, (const void *) name));
}

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_894)(GLuint program, GLenum programInterface, const GLchar * name);

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_894)(GLuint program, GLenum programInterface, const GLchar * name)
{
    (void) program; (void) programInterface; (void) name;
   RETURN_DISPATCH(GetProgramResourceLocation, (program, programInterface, name), (F, "glGetProgramResourceLocation(%d, 0x%x, %p);\n", program, programInterface, (const void *) name));
}

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_895)(GLuint program, GLenum programInterface, const GLchar * name);

KEYWORD1_ALT GLint KEYWORD2 NAME(_dispatch_stub_895)(GLuint program, GLenum programInterface, const GLchar * name)
{
    (void) program; (void) programInterface; (void) name;
   RETURN_DISPATCH(GetProgramResourceLocationIndex, (program, programInterface, name), (F, "glGetProgramResourceLocationIndex(%d, 0x%x, %p);\n", program, programInterface, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_896)(GLuint program, GLenum programInterface, GLuint index, GLsizei  bufSize, GLsizei * length, GLchar * name);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_896)(GLuint program, GLenum programInterface, GLuint index, GLsizei  bufSize, GLsizei * length, GLchar * name)
{
    (void) program; (void) programInterface; (void) index; (void) bufSize; (void) length; (void) name;
   DISPATCH(GetProgramResourceName, (program, programInterface, index, bufSize, length, name), (F, "glGetProgramResourceName(%d, 0x%x, %d, %d, %p, %p);\n", program, programInterface, index, bufSize, (const void *) length, (const void *) name));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_897)(GLuint program, GLenum programInterface, GLuint index, GLsizei  propCount, const GLenum * props, GLsizei  bufSize, GLsizei * length, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_897)(GLuint program, GLenum programInterface, GLuint index, GLsizei  propCount, const GLenum * props, GLsizei  bufSize, GLsizei * length, GLint * params)
{
    (void) program; (void) programInterface; (void) index; (void) propCount; (void) props; (void) bufSize; (void) length; (void) params;
   DISPATCH(GetProgramResourceiv, (program, programInterface, index, propCount, props, bufSize, length, params), (F, "glGetProgramResourceiv(%d, 0x%x, %d, %d, %p, %d, %p, %p);\n", program, programInterface, index, propCount, (const void *) props, bufSize, (const void *) length, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_898)(GLuint program, GLuint shaderStorageBlockIndex, GLuint shaderStorageBlockBinding);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_898)(GLuint program, GLuint shaderStorageBlockIndex, GLuint shaderStorageBlockBinding)
{
    (void) program; (void) shaderStorageBlockIndex; (void) shaderStorageBlockBinding;
   DISPATCH(ShaderStorageBlockBinding, (program, shaderStorageBlockIndex, shaderStorageBlockBinding), (F, "glShaderStorageBlockBinding(%d, %d, %d);\n", program, shaderStorageBlockIndex, shaderStorageBlockBinding));
}

KEYWORD1 void KEYWORD2 NAME(TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    (void) target; (void) internalformat; (void) buffer; (void) offset; (void) size;
   DISPATCH(TexBufferRange, (target, internalformat, buffer, offset, size), (F, "glTexBufferRange(0x%x, 0x%x, %d, %d, %d);\n", target, internalformat, buffer, offset, size));
}

KEYWORD1 void KEYWORD2 NAME(TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height; (void) fixedsamplelocations;
   DISPATCH(TexStorage2DMultisample, (target, samples, internalformat, width, height, fixedsamplelocations), (F, "glTexStorage2DMultisample(0x%x, %d, 0x%x, %d, %d, %d);\n", target, samples, internalformat, width, height, fixedsamplelocations));
}

KEYWORD1 void KEYWORD2 NAME(TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height; (void) depth; (void) fixedsamplelocations;
   DISPATCH(TexStorage3DMultisample, (target, samples, internalformat, width, height, depth, fixedsamplelocations), (F, "glTexStorage3DMultisample(0x%x, %d, 0x%x, %d, %d, %d, %d);\n", target, samples, internalformat, width, height, depth, fixedsamplelocations));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_901)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_901)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    (void) target; (void) samples; (void) internalformat; (void) width; (void) height; (void) depth; (void) fixedsamplelocations;
   DISPATCH(TexStorage3DMultisample, (target, samples, internalformat, width, height, depth, fixedsamplelocations), (F, "glTexStorage3DMultisampleOES(0x%x, %d, 0x%x, %d, %d, %d, %d);\n", target, samples, internalformat, width, height, depth, fixedsamplelocations));
}

KEYWORD1 void KEYWORD2 NAME(BufferStorage)(GLenum target, GLsizeiptr size, const GLvoid * data, GLbitfield flags)
{
    (void) target; (void) size; (void) data; (void) flags;
   DISPATCH(BufferStorage, (target, size, data, flags), (F, "glBufferStorage(0x%x, %d, %p, %d);\n", target, size, (const void *) data, flags));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_902)(GLenum target, GLsizeiptr size, const GLvoid * data, GLbitfield flags);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_902)(GLenum target, GLsizeiptr size, const GLvoid * data, GLbitfield flags)
{
    (void) target; (void) size; (void) data; (void) flags;
   DISPATCH(BufferStorage, (target, size, data, flags), (F, "glBufferStorageEXT(0x%x, %d, %p, %d);\n", target, size, (const void *) data, flags));
}

KEYWORD1 void KEYWORD2 NAME(ClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const GLvoid * data)
{
    (void) texture; (void) level; (void) format; (void) type; (void) data;
   DISPATCH(ClearTexImage, (texture, level, format, type, data), (F, "glClearTexImage(%d, %d, 0x%x, 0x%x, %p);\n", texture, level, format, type, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(ClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) data;
   DISPATCH(ClearTexSubImage, (texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data), (F, "glClearTexSubImage(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (const void *) data));
}

KEYWORD1 void KEYWORD2 NAME(BindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers)
{
    (void) target; (void) first; (void) count; (void) buffers;
   DISPATCH(BindBuffersBase, (target, first, count, buffers), (F, "glBindBuffersBase(0x%x, %d, %d, %p);\n", target, first, count, (const void *) buffers));
}

KEYWORD1 void KEYWORD2 NAME(BindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizeiptr * sizes)
{
    (void) target; (void) first; (void) count; (void) buffers; (void) offsets; (void) sizes;
   DISPATCH(BindBuffersRange, (target, first, count, buffers, offsets, sizes), (F, "glBindBuffersRange(0x%x, %d, %d, %p, %p, %p);\n", target, first, count, (const void *) buffers, (const void *) offsets, (const void *) sizes));
}

KEYWORD1 void KEYWORD2 NAME(BindImageTextures)(GLuint first, GLsizei count, const GLuint * textures)
{
    (void) first; (void) count; (void) textures;
   DISPATCH(BindImageTextures, (first, count, textures), (F, "glBindImageTextures(%d, %d, %p);\n", first, count, (const void *) textures));
}

KEYWORD1 void KEYWORD2 NAME(BindSamplers)(GLuint first, GLsizei count, const GLuint * samplers)
{
    (void) first; (void) count; (void) samplers;
   DISPATCH(BindSamplers, (first, count, samplers), (F, "glBindSamplers(%d, %d, %p);\n", first, count, (const void *) samplers));
}

KEYWORD1 void KEYWORD2 NAME(BindTextures)(GLuint first, GLsizei count, const GLuint * textures)
{
    (void) first; (void) count; (void) textures;
   DISPATCH(BindTextures, (first, count, textures), (F, "glBindTextures(%d, %d, %p);\n", first, count, (const void *) textures));
}

KEYWORD1 void KEYWORD2 NAME(BindVertexBuffers)(GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides)
{
    (void) first; (void) count; (void) buffers; (void) offsets; (void) strides;
   DISPATCH(BindVertexBuffers, (first, count, buffers, offsets, strides), (F, "glBindVertexBuffers(%d, %d, %p, %p, %p);\n", first, count, (const void *) buffers, (const void *) offsets, (const void *) strides));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_911)(GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_911)(GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
    (void) mode; (void) indirect; (void) drawcount; (void) maxdrawcount; (void) stride;
   DISPATCH(MultiDrawArraysIndirectCountARB, (mode, indirect, drawcount, maxdrawcount, stride), (F, "glMultiDrawArraysIndirectCountARB(0x%x, %d, %d, %d, %d);\n", mode, indirect, drawcount, maxdrawcount, stride));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_912)(GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_912)(GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)
{
    (void) mode; (void) type; (void) indirect; (void) drawcount; (void) maxdrawcount; (void) stride;
   DISPATCH(MultiDrawElementsIndirectCountARB, (mode, type, indirect, drawcount, maxdrawcount, stride), (F, "glMultiDrawElementsIndirectCountARB(0x%x, 0x%x, %d, %d, %d, %d);\n", mode, type, indirect, drawcount, maxdrawcount, stride));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_913)(GLenum origin, GLenum depth);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_913)(GLenum origin, GLenum depth)
{
    (void) origin; (void) depth;
   DISPATCH(ClipControl, (origin, depth), (F, "glClipControl(0x%x, 0x%x);\n", origin, depth));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_914)(GLuint unit, GLuint texture);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_914)(GLuint unit, GLuint texture)
{
    (void) unit; (void) texture;
   DISPATCH(BindTextureUnit, (unit, texture), (F, "glBindTextureUnit(%d, %d);\n", unit, texture));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_915)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_915)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    (void) readFramebuffer; (void) drawFramebuffer; (void) srcX0; (void) srcY0; (void) srcX1; (void) srcY1; (void) dstX0; (void) dstY0; (void) dstX1; (void) dstY1; (void) mask; (void) filter;
   DISPATCH(BlitNamedFramebuffer, (readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter), (F, "glBlitNamedFramebuffer(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, 0x%x);\n", readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter));
}

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_916)(GLuint framebuffer, GLenum target);

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_916)(GLuint framebuffer, GLenum target)
{
    (void) framebuffer; (void) target;
   RETURN_DISPATCH(CheckNamedFramebufferStatus, (framebuffer, target), (F, "glCheckNamedFramebufferStatus(%d, 0x%x);\n", framebuffer, target));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_917)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_917)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const GLvoid * data)
{
    (void) buffer; (void) internalformat; (void) format; (void) type; (void) data;
   DISPATCH(ClearNamedBufferData, (buffer, internalformat, format, type, data), (F, "glClearNamedBufferData(%d, 0x%x, 0x%x, 0x%x, %p);\n", buffer, internalformat, format, type, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_918)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_918)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid * data)
{
    (void) buffer; (void) internalformat; (void) offset; (void) size; (void) format; (void) type; (void) data;
   DISPATCH(ClearNamedBufferSubData, (buffer, internalformat, offset, size, format, type, data), (F, "glClearNamedBufferSubData(%d, 0x%x, %d, %d, 0x%x, 0x%x, %p);\n", buffer, internalformat, offset, size, format, type, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_919)(GLuint framebuffer, GLenum buffer, GLfloat depth, GLint stencil);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_919)(GLuint framebuffer, GLenum buffer, GLfloat depth, GLint stencil)
{
    (void) framebuffer; (void) buffer; (void) depth; (void) stencil;
   DISPATCH(ClearNamedFramebufferfi, (framebuffer, buffer, depth, stencil), (F, "glClearNamedFramebufferfi(%d, 0x%x, %f, %d);\n", framebuffer, buffer, depth, stencil));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_920)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_920)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat * value)
{
    (void) framebuffer; (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearNamedFramebufferfv, (framebuffer, buffer, drawbuffer, value), (F, "glClearNamedFramebufferfv(%d, 0x%x, %d, %p);\n", framebuffer, buffer, drawbuffer, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_921)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_921)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint * value)
{
    (void) framebuffer; (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearNamedFramebufferiv, (framebuffer, buffer, drawbuffer, value), (F, "glClearNamedFramebufferiv(%d, 0x%x, %d, %p);\n", framebuffer, buffer, drawbuffer, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_922)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_922)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint * value)
{
    (void) framebuffer; (void) buffer; (void) drawbuffer; (void) value;
   DISPATCH(ClearNamedFramebufferuiv, (framebuffer, buffer, drawbuffer, value), (F, "glClearNamedFramebufferuiv(%d, 0x%x, %d, %p);\n", framebuffer, buffer, drawbuffer, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_923)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_923)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) texture; (void) level; (void) xoffset; (void) width; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTextureSubImage1D, (texture, level, xoffset, width, format, imageSize, data), (F, "glCompressedTextureSubImage1D(%d, %d, %d, %d, 0x%x, %d, %p);\n", texture, level, xoffset, width, format, imageSize, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_924)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_924)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTextureSubImage2D, (texture, level, xoffset, yoffset, width, height, format, imageSize, data), (F, "glCompressedTextureSubImage2D(%d, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", texture, level, xoffset, yoffset, width, height, format, imageSize, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_925)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_925)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) imageSize; (void) data;
   DISPATCH(CompressedTextureSubImage3D, (texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data), (F, "glCompressedTextureSubImage3D(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, %d, %p);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_926)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_926)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    (void) readBuffer; (void) writeBuffer; (void) readOffset; (void) writeOffset; (void) size;
   DISPATCH(CopyNamedBufferSubData, (readBuffer, writeBuffer, readOffset, writeOffset, size), (F, "glCopyNamedBufferSubData(%d, %d, %d, %d, %d);\n", readBuffer, writeBuffer, readOffset, writeOffset, size));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_927)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_927)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    (void) texture; (void) level; (void) xoffset; (void) x; (void) y; (void) width;
   DISPATCH(CopyTextureSubImage1D, (texture, level, xoffset, x, y, width), (F, "glCopyTextureSubImage1D(%d, %d, %d, %d, %d, %d);\n", texture, level, xoffset, x, y, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_928)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_928)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTextureSubImage2D, (texture, level, xoffset, yoffset, x, y, width, height), (F, "glCopyTextureSubImage2D(%d, %d, %d, %d, %d, %d, %d, %d);\n", texture, level, xoffset, yoffset, x, y, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_929)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_929)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(CopyTextureSubImage3D, (texture, level, xoffset, yoffset, zoffset, x, y, width, height), (F, "glCopyTextureSubImage3D(%d, %d, %d, %d, %d, %d, %d, %d, %d);\n", texture, level, xoffset, yoffset, zoffset, x, y, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_930)(GLsizei n, GLuint * buffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_930)(GLsizei n, GLuint * buffers)
{
    (void) n; (void) buffers;
   DISPATCH(CreateBuffers, (n, buffers), (F, "glCreateBuffers(%d, %p);\n", n, (const void *) buffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_931)(GLsizei n, GLuint * framebuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_931)(GLsizei n, GLuint * framebuffers)
{
    (void) n; (void) framebuffers;
   DISPATCH(CreateFramebuffers, (n, framebuffers), (F, "glCreateFramebuffers(%d, %p);\n", n, (const void *) framebuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_932)(GLsizei n, GLuint * pipelines);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_932)(GLsizei n, GLuint * pipelines)
{
    (void) n; (void) pipelines;
   DISPATCH(CreateProgramPipelines, (n, pipelines), (F, "glCreateProgramPipelines(%d, %p);\n", n, (const void *) pipelines));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_933)(GLenum target, GLsizei n, GLuint * ids);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_933)(GLenum target, GLsizei n, GLuint * ids)
{
    (void) target; (void) n; (void) ids;
   DISPATCH(CreateQueries, (target, n, ids), (F, "glCreateQueries(0x%x, %d, %p);\n", target, n, (const void *) ids));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_934)(GLsizei n, GLuint * renderbuffers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_934)(GLsizei n, GLuint * renderbuffers)
{
    (void) n; (void) renderbuffers;
   DISPATCH(CreateRenderbuffers, (n, renderbuffers), (F, "glCreateRenderbuffers(%d, %p);\n", n, (const void *) renderbuffers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_935)(GLsizei n, GLuint * samplers);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_935)(GLsizei n, GLuint * samplers)
{
    (void) n; (void) samplers;
   DISPATCH(CreateSamplers, (n, samplers), (F, "glCreateSamplers(%d, %p);\n", n, (const void *) samplers));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_936)(GLenum target, GLsizei n, GLuint * textures);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_936)(GLenum target, GLsizei n, GLuint * textures)
{
    (void) target; (void) n; (void) textures;
   DISPATCH(CreateTextures, (target, n, textures), (F, "glCreateTextures(0x%x, %d, %p);\n", target, n, (const void *) textures));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_937)(GLsizei n, GLuint * ids);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_937)(GLsizei n, GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(CreateTransformFeedbacks, (n, ids), (F, "glCreateTransformFeedbacks(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_938)(GLsizei n, GLuint * arrays);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_938)(GLsizei n, GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(CreateVertexArrays, (n, arrays), (F, "glCreateVertexArrays(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_939)(GLuint vaobj, GLuint index);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_939)(GLuint vaobj, GLuint index)
{
    (void) vaobj; (void) index;
   DISPATCH(DisableVertexArrayAttrib, (vaobj, index), (F, "glDisableVertexArrayAttrib(%d, %d);\n", vaobj, index));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_940)(GLuint vaobj, GLuint index);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_940)(GLuint vaobj, GLuint index)
{
    (void) vaobj; (void) index;
   DISPATCH(EnableVertexArrayAttrib, (vaobj, index), (F, "glEnableVertexArrayAttrib(%d, %d);\n", vaobj, index));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_941)(GLuint buffer, GLintptr offset, GLsizeiptr length);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_941)(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    (void) buffer; (void) offset; (void) length;
   DISPATCH(FlushMappedNamedBufferRange, (buffer, offset, length), (F, "glFlushMappedNamedBufferRange(%d, %d, %d);\n", buffer, offset, length));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_942)(GLuint texture);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_942)(GLuint texture)
{
    (void) texture;
   DISPATCH(GenerateTextureMipmap, (texture), (F, "glGenerateTextureMipmap(%d);\n", texture));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_943)(GLuint texture, GLint level, GLsizei bufSize, GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_943)(GLuint texture, GLint level, GLsizei bufSize, GLvoid * pixels)
{
    (void) texture; (void) level; (void) bufSize; (void) pixels;
   DISPATCH(GetCompressedTextureImage, (texture, level, bufSize, pixels), (F, "glGetCompressedTextureImage(%d, %d, %d, %p);\n", texture, level, bufSize, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_944)(GLuint buffer, GLenum pname, GLint64 * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_944)(GLuint buffer, GLenum pname, GLint64 * params)
{
    (void) buffer; (void) pname; (void) params;
   DISPATCH(GetNamedBufferParameteri64v, (buffer, pname, params), (F, "glGetNamedBufferParameteri64v(%d, 0x%x, %p);\n", buffer, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_945)(GLuint buffer, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_945)(GLuint buffer, GLenum pname, GLint * params)
{
    (void) buffer; (void) pname; (void) params;
   DISPATCH(GetNamedBufferParameteriv, (buffer, pname, params), (F, "glGetNamedBufferParameteriv(%d, 0x%x, %p);\n", buffer, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_946)(GLuint buffer, GLenum pname, GLvoid ** params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_946)(GLuint buffer, GLenum pname, GLvoid ** params)
{
    (void) buffer; (void) pname; (void) params;
   DISPATCH(GetNamedBufferPointerv, (buffer, pname, params), (F, "glGetNamedBufferPointerv(%d, 0x%x, %p);\n", buffer, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_947)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_947)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid * data)
{
    (void) buffer; (void) offset; (void) size; (void) data;
   DISPATCH(GetNamedBufferSubData, (buffer, offset, size, data), (F, "glGetNamedBufferSubData(%d, %d, %d, %p);\n", buffer, offset, size, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_948)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_948)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint * params)
{
    (void) framebuffer; (void) attachment; (void) pname; (void) params;
   DISPATCH(GetNamedFramebufferAttachmentParameteriv, (framebuffer, attachment, pname, params), (F, "glGetNamedFramebufferAttachmentParameteriv(%d, 0x%x, 0x%x, %p);\n", framebuffer, attachment, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_949)(GLuint framebuffer, GLenum pname, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_949)(GLuint framebuffer, GLenum pname, GLint * param)
{
    (void) framebuffer; (void) pname; (void) param;
   DISPATCH(GetNamedFramebufferParameteriv, (framebuffer, pname, param), (F, "glGetNamedFramebufferParameteriv(%d, 0x%x, %p);\n", framebuffer, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_950)(GLuint renderbuffer, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_950)(GLuint renderbuffer, GLenum pname, GLint * params)
{
    (void) renderbuffer; (void) pname; (void) params;
   DISPATCH(GetNamedRenderbufferParameteriv, (renderbuffer, pname, params), (F, "glGetNamedRenderbufferParameteriv(%d, 0x%x, %p);\n", renderbuffer, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_951)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_951)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    (void) id; (void) buffer; (void) pname; (void) offset;
   DISPATCH(GetQueryBufferObjecti64v, (id, buffer, pname, offset), (F, "glGetQueryBufferObjecti64v(%d, %d, 0x%x, %d);\n", id, buffer, pname, offset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_952)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_952)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    (void) id; (void) buffer; (void) pname; (void) offset;
   DISPATCH(GetQueryBufferObjectiv, (id, buffer, pname, offset), (F, "glGetQueryBufferObjectiv(%d, %d, 0x%x, %d);\n", id, buffer, pname, offset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_953)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_953)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    (void) id; (void) buffer; (void) pname; (void) offset;
   DISPATCH(GetQueryBufferObjectui64v, (id, buffer, pname, offset), (F, "glGetQueryBufferObjectui64v(%d, %d, 0x%x, %d);\n", id, buffer, pname, offset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_954)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_954)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset)
{
    (void) id; (void) buffer; (void) pname; (void) offset;
   DISPATCH(GetQueryBufferObjectuiv, (id, buffer, pname, offset), (F, "glGetQueryBufferObjectuiv(%d, %d, 0x%x, %d);\n", id, buffer, pname, offset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_955)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_955)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels)
{
    (void) texture; (void) level; (void) format; (void) type; (void) bufSize; (void) pixels;
   DISPATCH(GetTextureImage, (texture, level, format, type, bufSize, pixels), (F, "glGetTextureImage(%d, %d, 0x%x, 0x%x, %d, %p);\n", texture, level, format, type, bufSize, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_956)(GLuint texture, GLint level, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_956)(GLuint texture, GLint level, GLenum pname, GLfloat * params)
{
    (void) texture; (void) level; (void) pname; (void) params;
   DISPATCH(GetTextureLevelParameterfv, (texture, level, pname, params), (F, "glGetTextureLevelParameterfv(%d, %d, 0x%x, %p);\n", texture, level, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_957)(GLuint texture, GLint level, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_957)(GLuint texture, GLint level, GLenum pname, GLint * params)
{
    (void) texture; (void) level; (void) pname; (void) params;
   DISPATCH(GetTextureLevelParameteriv, (texture, level, pname, params), (F, "glGetTextureLevelParameteriv(%d, %d, 0x%x, %p);\n", texture, level, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_958)(GLuint texture, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_958)(GLuint texture, GLenum pname, GLint * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(GetTextureParameterIiv, (texture, pname, params), (F, "glGetTextureParameterIiv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_959)(GLuint texture, GLenum pname, GLuint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_959)(GLuint texture, GLenum pname, GLuint * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(GetTextureParameterIuiv, (texture, pname, params), (F, "glGetTextureParameterIuiv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_960)(GLuint texture, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_960)(GLuint texture, GLenum pname, GLfloat * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(GetTextureParameterfv, (texture, pname, params), (F, "glGetTextureParameterfv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_961)(GLuint texture, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_961)(GLuint texture, GLenum pname, GLint * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(GetTextureParameteriv, (texture, pname, params), (F, "glGetTextureParameteriv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_962)(GLuint xfb, GLenum pname, GLuint index, GLint64 * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_962)(GLuint xfb, GLenum pname, GLuint index, GLint64 * param)
{
    (void) xfb; (void) pname; (void) index; (void) param;
   DISPATCH(GetTransformFeedbacki64_v, (xfb, pname, index, param), (F, "glGetTransformFeedbacki64_v(%d, 0x%x, %d, %p);\n", xfb, pname, index, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_963)(GLuint xfb, GLenum pname, GLuint index, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_963)(GLuint xfb, GLenum pname, GLuint index, GLint * param)
{
    (void) xfb; (void) pname; (void) index; (void) param;
   DISPATCH(GetTransformFeedbacki_v, (xfb, pname, index, param), (F, "glGetTransformFeedbacki_v(%d, 0x%x, %d, %p);\n", xfb, pname, index, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_964)(GLuint xfb, GLenum pname, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_964)(GLuint xfb, GLenum pname, GLint * param)
{
    (void) xfb; (void) pname; (void) param;
   DISPATCH(GetTransformFeedbackiv, (xfb, pname, param), (F, "glGetTransformFeedbackiv(%d, 0x%x, %p);\n", xfb, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_965)(GLuint vaobj, GLuint index, GLenum pname, GLint64 * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_965)(GLuint vaobj, GLuint index, GLenum pname, GLint64 * param)
{
    (void) vaobj; (void) index; (void) pname; (void) param;
   DISPATCH(GetVertexArrayIndexed64iv, (vaobj, index, pname, param), (F, "glGetVertexArrayIndexed64iv(%d, %d, 0x%x, %p);\n", vaobj, index, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_966)(GLuint vaobj, GLuint index, GLenum pname, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_966)(GLuint vaobj, GLuint index, GLenum pname, GLint * param)
{
    (void) vaobj; (void) index; (void) pname; (void) param;
   DISPATCH(GetVertexArrayIndexediv, (vaobj, index, pname, param), (F, "glGetVertexArrayIndexediv(%d, %d, 0x%x, %p);\n", vaobj, index, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_967)(GLuint vaobj, GLenum pname, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_967)(GLuint vaobj, GLenum pname, GLint * param)
{
    (void) vaobj; (void) pname; (void) param;
   DISPATCH(GetVertexArrayiv, (vaobj, pname, param), (F, "glGetVertexArrayiv(%d, 0x%x, %p);\n", vaobj, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_968)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_968)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments)
{
    (void) framebuffer; (void) numAttachments; (void) attachments;
   DISPATCH(InvalidateNamedFramebufferData, (framebuffer, numAttachments, attachments), (F, "glInvalidateNamedFramebufferData(%d, %d, %p);\n", framebuffer, numAttachments, (const void *) attachments));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_969)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_969)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) framebuffer; (void) numAttachments; (void) attachments; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(InvalidateNamedFramebufferSubData, (framebuffer, numAttachments, attachments, x, y, width, height), (F, "glInvalidateNamedFramebufferSubData(%d, %d, %p, %d, %d, %d, %d);\n", framebuffer, numAttachments, (const void *) attachments, x, y, width, height));
}

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_970)(GLuint buffer, GLenum access);

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_970)(GLuint buffer, GLenum access)
{
    (void) buffer; (void) access;
   RETURN_DISPATCH(MapNamedBuffer, (buffer, access), (F, "glMapNamedBuffer(%d, 0x%x);\n", buffer, access));
}

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_971)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access);

KEYWORD1_ALT GLvoid * KEYWORD2 NAME(_dispatch_stub_971)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    (void) buffer; (void) offset; (void) length; (void) access;
   RETURN_DISPATCH(MapNamedBufferRange, (buffer, offset, length, access), (F, "glMapNamedBufferRange(%d, %d, %d, %d);\n", buffer, offset, length, access));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_972)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLenum usage);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_972)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLenum usage)
{
    (void) buffer; (void) size; (void) data; (void) usage;
   DISPATCH(NamedBufferData, (buffer, size, data, usage), (F, "glNamedBufferData(%d, %d, %p, 0x%x);\n", buffer, size, (const void *) data, usage));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_973)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLbitfield flags);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_973)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLbitfield flags)
{
    (void) buffer; (void) size; (void) data; (void) flags;
   DISPATCH(NamedBufferStorage, (buffer, size, data, flags), (F, "glNamedBufferStorage(%d, %d, %p, %d);\n", buffer, size, (const void *) data, flags));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_974)(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_974)(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
    (void) buffer; (void) offset; (void) size; (void) data;
   DISPATCH(NamedBufferSubData, (buffer, offset, size, data), (F, "glNamedBufferSubData(%d, %d, %d, %p);\n", buffer, offset, size, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_975)(GLuint framebuffer, GLenum buf);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_975)(GLuint framebuffer, GLenum buf)
{
    (void) framebuffer; (void) buf;
   DISPATCH(NamedFramebufferDrawBuffer, (framebuffer, buf), (F, "glNamedFramebufferDrawBuffer(%d, 0x%x);\n", framebuffer, buf));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_976)(GLuint framebuffer, GLsizei n, const GLenum * bufs);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_976)(GLuint framebuffer, GLsizei n, const GLenum * bufs)
{
    (void) framebuffer; (void) n; (void) bufs;
   DISPATCH(NamedFramebufferDrawBuffers, (framebuffer, n, bufs), (F, "glNamedFramebufferDrawBuffers(%d, %d, %p);\n", framebuffer, n, (const void *) bufs));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_977)(GLuint framebuffer, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_977)(GLuint framebuffer, GLenum pname, GLint param)
{
    (void) framebuffer; (void) pname; (void) param;
   DISPATCH(NamedFramebufferParameteri, (framebuffer, pname, param), (F, "glNamedFramebufferParameteri(%d, 0x%x, %d);\n", framebuffer, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_978)(GLuint framebuffer, GLenum buf);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_978)(GLuint framebuffer, GLenum buf)
{
    (void) framebuffer; (void) buf;
   DISPATCH(NamedFramebufferReadBuffer, (framebuffer, buf), (F, "glNamedFramebufferReadBuffer(%d, 0x%x);\n", framebuffer, buf));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_979)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_979)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    (void) framebuffer; (void) attachment; (void) renderbuffertarget; (void) renderbuffer;
   DISPATCH(NamedFramebufferRenderbuffer, (framebuffer, attachment, renderbuffertarget, renderbuffer), (F, "glNamedFramebufferRenderbuffer(%d, 0x%x, 0x%x, %d);\n", framebuffer, attachment, renderbuffertarget, renderbuffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_980)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_980)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
    (void) framebuffer; (void) attachment; (void) texture; (void) level;
   DISPATCH(NamedFramebufferTexture, (framebuffer, attachment, texture, level), (F, "glNamedFramebufferTexture(%d, 0x%x, %d, %d);\n", framebuffer, attachment, texture, level));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_981)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_981)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    (void) framebuffer; (void) attachment; (void) texture; (void) level; (void) layer;
   DISPATCH(NamedFramebufferTextureLayer, (framebuffer, attachment, texture, level, layer), (F, "glNamedFramebufferTextureLayer(%d, 0x%x, %d, %d, %d);\n", framebuffer, attachment, texture, level, layer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_982)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_982)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) renderbuffer; (void) internalformat; (void) width; (void) height;
   DISPATCH(NamedRenderbufferStorage, (renderbuffer, internalformat, width, height), (F, "glNamedRenderbufferStorage(%d, 0x%x, %d, %d);\n", renderbuffer, internalformat, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_983)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_983)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) renderbuffer; (void) samples; (void) internalformat; (void) width; (void) height;
   DISPATCH(NamedRenderbufferStorageMultisample, (renderbuffer, samples, internalformat, width, height), (F, "glNamedRenderbufferStorageMultisample(%d, %d, 0x%x, %d, %d);\n", renderbuffer, samples, internalformat, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_984)(GLuint texture, GLenum internalformat, GLuint buffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_984)(GLuint texture, GLenum internalformat, GLuint buffer)
{
    (void) texture; (void) internalformat; (void) buffer;
   DISPATCH(TextureBuffer, (texture, internalformat, buffer), (F, "glTextureBuffer(%d, 0x%x, %d);\n", texture, internalformat, buffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_985)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_985)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    (void) texture; (void) internalformat; (void) buffer; (void) offset; (void) size;
   DISPATCH(TextureBufferRange, (texture, internalformat, buffer, offset, size), (F, "glTextureBufferRange(%d, 0x%x, %d, %d, %d);\n", texture, internalformat, buffer, offset, size));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_986)(GLuint texture, GLenum pname, const GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_986)(GLuint texture, GLenum pname, const GLint * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(TextureParameterIiv, (texture, pname, params), (F, "glTextureParameterIiv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_987)(GLuint texture, GLenum pname, const GLuint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_987)(GLuint texture, GLenum pname, const GLuint * params)
{
    (void) texture; (void) pname; (void) params;
   DISPATCH(TextureParameterIuiv, (texture, pname, params), (F, "glTextureParameterIuiv(%d, 0x%x, %p);\n", texture, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_988)(GLuint texture, GLenum pname, GLfloat param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_988)(GLuint texture, GLenum pname, GLfloat param)
{
    (void) texture; (void) pname; (void) param;
   DISPATCH(TextureParameterf, (texture, pname, param), (F, "glTextureParameterf(%d, 0x%x, %f);\n", texture, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_989)(GLuint texture, GLenum pname, const GLfloat * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_989)(GLuint texture, GLenum pname, const GLfloat * param)
{
    (void) texture; (void) pname; (void) param;
   DISPATCH(TextureParameterfv, (texture, pname, param), (F, "glTextureParameterfv(%d, 0x%x, %p);\n", texture, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_990)(GLuint texture, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_990)(GLuint texture, GLenum pname, GLint param)
{
    (void) texture; (void) pname; (void) param;
   DISPATCH(TextureParameteri, (texture, pname, param), (F, "glTextureParameteri(%d, 0x%x, %d);\n", texture, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_991)(GLuint texture, GLenum pname, const GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_991)(GLuint texture, GLenum pname, const GLint * param)
{
    (void) texture; (void) pname; (void) param;
   DISPATCH(TextureParameteriv, (texture, pname, param), (F, "glTextureParameteriv(%d, 0x%x, %p);\n", texture, pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_992)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_992)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    (void) texture; (void) levels; (void) internalformat; (void) width;
   DISPATCH(TextureStorage1D, (texture, levels, internalformat, width), (F, "glTextureStorage1D(%d, %d, 0x%x, %d);\n", texture, levels, internalformat, width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_993)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_993)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    (void) texture; (void) levels; (void) internalformat; (void) width; (void) height;
   DISPATCH(TextureStorage2D, (texture, levels, internalformat, width, height), (F, "glTextureStorage2D(%d, %d, 0x%x, %d, %d);\n", texture, levels, internalformat, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_994)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_994)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    (void) texture; (void) samples; (void) internalformat; (void) width; (void) height; (void) fixedsamplelocations;
   DISPATCH(TextureStorage2DMultisample, (texture, samples, internalformat, width, height, fixedsamplelocations), (F, "glTextureStorage2DMultisample(%d, %d, 0x%x, %d, %d, %d);\n", texture, samples, internalformat, width, height, fixedsamplelocations));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_995)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_995)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    (void) texture; (void) levels; (void) internalformat; (void) width; (void) height; (void) depth;
   DISPATCH(TextureStorage3D, (texture, levels, internalformat, width, height, depth), (F, "glTextureStorage3D(%d, %d, 0x%x, %d, %d, %d);\n", texture, levels, internalformat, width, height, depth));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_996)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_996)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    (void) texture; (void) samples; (void) internalformat; (void) width; (void) height; (void) depth; (void) fixedsamplelocations;
   DISPATCH(TextureStorage3DMultisample, (texture, samples, internalformat, width, height, depth, fixedsamplelocations), (F, "glTextureStorage3DMultisample(%d, %d, 0x%x, %d, %d, %d, %d);\n", texture, samples, internalformat, width, height, depth, fixedsamplelocations));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_997)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_997)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) texture; (void) level; (void) xoffset; (void) width; (void) format; (void) type; (void) pixels;
   DISPATCH(TextureSubImage1D, (texture, level, xoffset, width, format, type, pixels), (F, "glTextureSubImage1D(%d, %d, %d, %d, 0x%x, 0x%x, %p);\n", texture, level, xoffset, width, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_998)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_998)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) width; (void) height; (void) format; (void) type; (void) pixels;
   DISPATCH(TextureSubImage2D, (texture, level, xoffset, yoffset, width, height, format, type, pixels), (F, "glTextureSubImage2D(%d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", texture, level, xoffset, yoffset, width, height, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_999)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_999)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) pixels;
   DISPATCH(TextureSubImage3D, (texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels), (F, "glTextureSubImage3D(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %p);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1000)(GLuint xfb, GLuint index, GLuint buffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1000)(GLuint xfb, GLuint index, GLuint buffer)
{
    (void) xfb; (void) index; (void) buffer;
   DISPATCH(TransformFeedbackBufferBase, (xfb, index, buffer), (F, "glTransformFeedbackBufferBase(%d, %d, %d);\n", xfb, index, buffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1001)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1001)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    (void) xfb; (void) index; (void) buffer; (void) offset; (void) size;
   DISPATCH(TransformFeedbackBufferRange, (xfb, index, buffer, offset, size), (F, "glTransformFeedbackBufferRange(%d, %d, %d, %d, %d);\n", xfb, index, buffer, offset, size));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1002)(GLuint buffer);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1002)(GLuint buffer)
{
    (void) buffer;
   RETURN_DISPATCH(UnmapNamedBuffer, (buffer), (F, "glUnmapNamedBuffer(%d);\n", buffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1003)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1003)(GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
    (void) vaobj; (void) attribindex; (void) bindingindex;
   DISPATCH(VertexArrayAttribBinding, (vaobj, attribindex, bindingindex), (F, "glVertexArrayAttribBinding(%d, %d, %d);\n", vaobj, attribindex, bindingindex));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1004)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1004)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    (void) vaobj; (void) attribindex; (void) size; (void) type; (void) normalized; (void) relativeoffset;
   DISPATCH(VertexArrayAttribFormat, (vaobj, attribindex, size, type, normalized, relativeoffset), (F, "glVertexArrayAttribFormat(%d, %d, %d, 0x%x, %d, %d);\n", vaobj, attribindex, size, type, normalized, relativeoffset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1005)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1005)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    (void) vaobj; (void) attribindex; (void) size; (void) type; (void) relativeoffset;
   DISPATCH(VertexArrayAttribIFormat, (vaobj, attribindex, size, type, relativeoffset), (F, "glVertexArrayAttribIFormat(%d, %d, %d, 0x%x, %d);\n", vaobj, attribindex, size, type, relativeoffset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1006)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1006)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    (void) vaobj; (void) attribindex; (void) size; (void) type; (void) relativeoffset;
   DISPATCH(VertexArrayAttribLFormat, (vaobj, attribindex, size, type, relativeoffset), (F, "glVertexArrayAttribLFormat(%d, %d, %d, 0x%x, %d);\n", vaobj, attribindex, size, type, relativeoffset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1007)(GLuint vaobj, GLuint bindingindex, GLuint divisor);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1007)(GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
    (void) vaobj; (void) bindingindex; (void) divisor;
   DISPATCH(VertexArrayBindingDivisor, (vaobj, bindingindex, divisor), (F, "glVertexArrayBindingDivisor(%d, %d, %d);\n", vaobj, bindingindex, divisor));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1008)(GLuint vaobj, GLuint buffer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1008)(GLuint vaobj, GLuint buffer)
{
    (void) vaobj; (void) buffer;
   DISPATCH(VertexArrayElementBuffer, (vaobj, buffer), (F, "glVertexArrayElementBuffer(%d, %d);\n", vaobj, buffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1009)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1009)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    (void) vaobj; (void) bindingindex; (void) buffer; (void) offset; (void) stride;
   DISPATCH(VertexArrayVertexBuffer, (vaobj, bindingindex, buffer, offset, stride), (F, "glVertexArrayVertexBuffer(%d, %d, %d, %d, %d);\n", vaobj, bindingindex, buffer, offset, stride));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1010)(GLuint vaobj, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1010)(GLuint vaobj, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides)
{
    (void) vaobj; (void) first; (void) count; (void) buffers; (void) offsets; (void) strides;
   DISPATCH(VertexArrayVertexBuffers, (vaobj, first, count, buffers, offsets, strides), (F, "glVertexArrayVertexBuffers(%d, %d, %d, %p, %p, %p);\n", vaobj, first, count, (const void *) buffers, (const void *) offsets, (const void *) strides));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1011)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1011)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, GLvoid * pixels)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) bufSize; (void) pixels;
   DISPATCH(GetCompressedTextureSubImage, (texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels), (F, "glGetCompressedTextureSubImage(%d, %d, %d, %d, %d, %d, %d, %d, %d, %p);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, (const void *) pixels));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1012)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1012)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth; (void) format; (void) type; (void) bufSize; (void) pixels;
   DISPATCH(GetTextureSubImage, (texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels), (F, "glGetTextureSubImage(%d, %d, %d, %d, %d, %d, %d, %d, 0x%x, 0x%x, %d, %p);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, (const void *) pixels));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateBufferData)(GLuint buffer)
{
    (void) buffer;
   DISPATCH(InvalidateBufferData, (buffer), (F, "glInvalidateBufferData(%d);\n", buffer));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    (void) buffer; (void) offset; (void) length;
   DISPATCH(InvalidateBufferSubData, (buffer, offset, length), (F, "glInvalidateBufferSubData(%d, %d, %d);\n", buffer, offset, length));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments)
{
    (void) target; (void) numAttachments; (void) attachments;
   DISPATCH(InvalidateFramebuffer, (target, numAttachments, attachments), (F, "glInvalidateFramebuffer(0x%x, %d, %p);\n", target, numAttachments, (const void *) attachments));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    (void) target; (void) numAttachments; (void) attachments; (void) x; (void) y; (void) width; (void) height;
   DISPATCH(InvalidateSubFramebuffer, (target, numAttachments, attachments, x, y, width, height), (F, "glInvalidateSubFramebuffer(0x%x, %d, %p, %d, %d, %d, %d);\n", target, numAttachments, (const void *) attachments, x, y, width, height));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateTexImage)(GLuint texture, GLint level)
{
    (void) texture; (void) level;
   DISPATCH(InvalidateTexImage, (texture, level), (F, "glInvalidateTexImage(%d, %d);\n", texture, level));
}

KEYWORD1 void KEYWORD2 NAME(InvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    (void) texture; (void) level; (void) xoffset; (void) yoffset; (void) zoffset; (void) width; (void) height; (void) depth;
   DISPATCH(InvalidateTexSubImage, (texture, level, xoffset, yoffset, zoffset, width, height, depth), (F, "glInvalidateTexSubImage(%d, %d, %d, %d, %d, %d, %d, %d);\n", texture, level, xoffset, yoffset, zoffset, width, height, depth));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1019)(GLfloat factor, GLfloat bias);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1019)(GLfloat factor, GLfloat bias)
{
    (void) factor; (void) bias;
   DISPATCH(PolygonOffsetEXT, (factor, bias), (F, "glPolygonOffsetEXT(%f, %f);\n", factor, bias));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1020)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1020)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
    (void) x; (void) y; (void) z; (void) width; (void) height;
   DISPATCH(DrawTexfOES, (x, y, z, width, height), (F, "glDrawTexfOES(%f, %f, %f, %f, %f);\n", x, y, z, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1021)(const GLfloat * coords);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1021)(const GLfloat * coords)
{
    (void) coords;
   DISPATCH(DrawTexfvOES, (coords), (F, "glDrawTexfvOES(%p);\n", (const void *) coords));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1022)(GLint x, GLint y, GLint z, GLint width, GLint height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1022)(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    (void) x; (void) y; (void) z; (void) width; (void) height;
   DISPATCH(DrawTexiOES, (x, y, z, width, height), (F, "glDrawTexiOES(%d, %d, %d, %d, %d);\n", x, y, z, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1023)(const GLint * coords);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1023)(const GLint * coords)
{
    (void) coords;
   DISPATCH(DrawTexivOES, (coords), (F, "glDrawTexivOES(%p);\n", (const void *) coords));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1024)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1024)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    (void) x; (void) y; (void) z; (void) width; (void) height;
   DISPATCH(DrawTexsOES, (x, y, z, width, height), (F, "glDrawTexsOES(%d, %d, %d, %d, %d);\n", x, y, z, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1025)(const GLshort * coords);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1025)(const GLshort * coords)
{
    (void) coords;
   DISPATCH(DrawTexsvOES, (coords), (F, "glDrawTexsvOES(%p);\n", (const void *) coords));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1026)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1026)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    (void) x; (void) y; (void) z; (void) width; (void) height;
   DISPATCH(DrawTexxOES, (x, y, z, width, height), (F, "glDrawTexxOES(%d, %d, %d, %d, %d);\n", x, y, z, width, height));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1027)(const GLfixed * coords);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1027)(const GLfixed * coords)
{
    (void) coords;
   DISPATCH(DrawTexxvOES, (coords), (F, "glDrawTexxvOES(%p);\n", (const void *) coords));
}

KEYWORD1 void KEYWORD2 NAME(PointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) pointer;
   DISPATCH(PointSizePointerOES, (type, stride, pointer), (F, "glPointSizePointerOES(0x%x, %d, %p);\n", type, stride, (const void *) pointer));
}

KEYWORD1_ALT GLbitfield KEYWORD2 NAME(_dispatch_stub_1029)(GLfixed * mantissa, GLint * exponent);

KEYWORD1_ALT GLbitfield KEYWORD2 NAME(_dispatch_stub_1029)(GLfixed * mantissa, GLint * exponent)
{
    (void) mantissa; (void) exponent;
   RETURN_DISPATCH(QueryMatrixxOES, (mantissa, exponent), (F, "glQueryMatrixxOES(%p, %p);\n", (const void *) mantissa, (const void *) exponent));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1030)(GLclampf value, GLboolean invert);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1030)(GLclampf value, GLboolean invert)
{
    (void) value; (void) invert;
   DISPATCH(SampleMaskSGIS, (value, invert), (F, "glSampleMaskSGIS(%f, %d);\n", value, invert));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1031)(GLenum pattern);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1031)(GLenum pattern)
{
    (void) pattern;
   DISPATCH(SamplePatternSGIS, (pattern), (F, "glSamplePatternSGIS(0x%x);\n", pattern));
}

KEYWORD1 void KEYWORD2 NAME(ColorPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) count; (void) pointer;
   DISPATCH(ColorPointerEXT, (size, type, stride, count, pointer), (F, "glColorPointerEXT(%d, 0x%x, %d, %d, %p);\n", size, type, stride, count, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(EdgeFlagPointerEXT)(GLsizei stride, GLsizei count, const GLboolean * pointer)
{
    (void) stride; (void) count; (void) pointer;
   DISPATCH(EdgeFlagPointerEXT, (stride, count, pointer), (F, "glEdgeFlagPointerEXT(%d, %d, %p);\n", stride, count, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(IndexPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) count; (void) pointer;
   DISPATCH(IndexPointerEXT, (type, stride, count, pointer), (F, "glIndexPointerEXT(0x%x, %d, %d, %p);\n", type, stride, count, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(NormalPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer)
{
    (void) type; (void) stride; (void) count; (void) pointer;
   DISPATCH(NormalPointerEXT, (type, stride, count, pointer), (F, "glNormalPointerEXT(0x%x, %d, %d, %p);\n", type, stride, count, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(TexCoordPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) count; (void) pointer;
   DISPATCH(TexCoordPointerEXT, (size, type, stride, count, pointer), (F, "glTexCoordPointerEXT(%d, 0x%x, %d, %d, %p);\n", size, type, stride, count, (const void *) pointer));
}

KEYWORD1 void KEYWORD2 NAME(VertexPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer)
{
    (void) size; (void) type; (void) stride; (void) count; (void) pointer;
   DISPATCH(VertexPointerEXT, (size, type, stride, count, pointer), (F, "glVertexPointerEXT(%d, 0x%x, %d, %d, %p);\n", size, type, stride, count, (const void *) pointer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1038)(GLenum target, GLsizei numAttachments, const GLenum * attachments);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1038)(GLenum target, GLsizei numAttachments, const GLenum * attachments)
{
    (void) target; (void) numAttachments; (void) attachments;
   DISPATCH(DiscardFramebufferEXT, (target, numAttachments, attachments), (F, "glDiscardFramebufferEXT(0x%x, %d, %p);\n", target, numAttachments, (const void *) attachments));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1039)(GLuint pipeline, GLuint program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1039)(GLuint pipeline, GLuint program)
{
    (void) pipeline; (void) program;
   DISPATCH(ActiveShaderProgram, (pipeline, program), (F, "glActiveShaderProgram(%d, %d);\n", pipeline, program));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1040)(GLuint pipeline);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1040)(GLuint pipeline)
{
    (void) pipeline;
   DISPATCH(BindProgramPipeline, (pipeline), (F, "glBindProgramPipeline(%d);\n", pipeline));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1041)(GLenum type, GLsizei count, const GLchar * const * strings);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1041)(GLenum type, GLsizei count, const GLchar * const * strings)
{
    (void) type; (void) count; (void) strings;
   RETURN_DISPATCH(CreateShaderProgramv, (type, count, strings), (F, "glCreateShaderProgramv(0x%x, %d, %p);\n", type, count, (const void *) strings));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1042)(GLsizei n, const GLuint * pipelines);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1042)(GLsizei n, const GLuint * pipelines)
{
    (void) n; (void) pipelines;
   DISPATCH(DeleteProgramPipelines, (n, pipelines), (F, "glDeleteProgramPipelines(%d, %p);\n", n, (const void *) pipelines));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1043)(GLsizei n, GLuint * pipelines);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1043)(GLsizei n, GLuint * pipelines)
{
    (void) n; (void) pipelines;
   DISPATCH(GenProgramPipelines, (n, pipelines), (F, "glGenProgramPipelines(%d, %p);\n", n, (const void *) pipelines));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1044)(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1044)(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    (void) pipeline; (void) bufSize; (void) length; (void) infoLog;
   DISPATCH(GetProgramPipelineInfoLog, (pipeline, bufSize, length, infoLog), (F, "glGetProgramPipelineInfoLog(%d, %d, %p, %p);\n", pipeline, bufSize, (const void *) length, (const void *) infoLog));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1045)(GLuint pipeline, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1045)(GLuint pipeline, GLenum pname, GLint * params)
{
    (void) pipeline; (void) pname; (void) params;
   DISPATCH(GetProgramPipelineiv, (pipeline, pname, params), (F, "glGetProgramPipelineiv(%d, 0x%x, %p);\n", pipeline, pname, (const void *) params));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1046)(GLuint pipeline);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1046)(GLuint pipeline)
{
    (void) pipeline;
   RETURN_DISPATCH(IsProgramPipeline, (pipeline), (F, "glIsProgramPipeline(%d);\n", pipeline));
}

KEYWORD1 void KEYWORD2 NAME(LockArraysEXT)(GLint first, GLsizei count)
{
    (void) first; (void) count;
   DISPATCH(LockArraysEXT, (first, count), (F, "glLockArraysEXT(%d, %d);\n", first, count));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1048)(GLuint program, GLint location, GLdouble x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1048)(GLuint program, GLint location, GLdouble x)
{
    (void) program; (void) location; (void) x;
   DISPATCH(ProgramUniform1d, (program, location, x), (F, "glProgramUniform1d(%d, %d, %f);\n", program, location, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1049)(GLuint program, GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1049)(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform1dv, (program, location, count, value), (F, "glProgramUniform1dv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1050)(GLuint program, GLint location, GLfloat x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1050)(GLuint program, GLint location, GLfloat x)
{
    (void) program; (void) location; (void) x;
   DISPATCH(ProgramUniform1f, (program, location, x), (F, "glProgramUniform1f(%d, %d, %f);\n", program, location, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1051)(GLuint program, GLint location, GLsizei count, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1051)(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform1fv, (program, location, count, value), (F, "glProgramUniform1fv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1052)(GLuint program, GLint location, GLint x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1052)(GLuint program, GLint location, GLint x)
{
    (void) program; (void) location; (void) x;
   DISPATCH(ProgramUniform1i, (program, location, x), (F, "glProgramUniform1i(%d, %d, %d);\n", program, location, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1053)(GLuint program, GLint location, GLsizei count, const GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1053)(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform1iv, (program, location, count, value), (F, "glProgramUniform1iv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1054)(GLuint program, GLint location, GLuint x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1054)(GLuint program, GLint location, GLuint x)
{
    (void) program; (void) location; (void) x;
   DISPATCH(ProgramUniform1ui, (program, location, x), (F, "glProgramUniform1ui(%d, %d, %d);\n", program, location, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1055)(GLuint program, GLint location, GLsizei count, const GLuint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1055)(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform1uiv, (program, location, count, value), (F, "glProgramUniform1uiv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1056)(GLuint program, GLint location, GLdouble x, GLdouble y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1056)(GLuint program, GLint location, GLdouble x, GLdouble y)
{
    (void) program; (void) location; (void) x; (void) y;
   DISPATCH(ProgramUniform2d, (program, location, x, y), (F, "glProgramUniform2d(%d, %d, %f, %f);\n", program, location, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1057)(GLuint program, GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1057)(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform2dv, (program, location, count, value), (F, "glProgramUniform2dv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1058)(GLuint program, GLint location, GLfloat x, GLfloat y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1058)(GLuint program, GLint location, GLfloat x, GLfloat y)
{
    (void) program; (void) location; (void) x; (void) y;
   DISPATCH(ProgramUniform2f, (program, location, x, y), (F, "glProgramUniform2f(%d, %d, %f, %f);\n", program, location, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1059)(GLuint program, GLint location, GLsizei count, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1059)(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform2fv, (program, location, count, value), (F, "glProgramUniform2fv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1060)(GLuint program, GLint location, GLint x, GLint y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1060)(GLuint program, GLint location, GLint x, GLint y)
{
    (void) program; (void) location; (void) x; (void) y;
   DISPATCH(ProgramUniform2i, (program, location, x, y), (F, "glProgramUniform2i(%d, %d, %d, %d);\n", program, location, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1061)(GLuint program, GLint location, GLsizei count, const GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1061)(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform2iv, (program, location, count, value), (F, "glProgramUniform2iv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1062)(GLuint program, GLint location, GLuint x, GLuint y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1062)(GLuint program, GLint location, GLuint x, GLuint y)
{
    (void) program; (void) location; (void) x; (void) y;
   DISPATCH(ProgramUniform2ui, (program, location, x, y), (F, "glProgramUniform2ui(%d, %d, %d, %d);\n", program, location, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1063)(GLuint program, GLint location, GLsizei count, const GLuint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1063)(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform2uiv, (program, location, count, value), (F, "glProgramUniform2uiv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1064)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1064)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    (void) program; (void) location; (void) x; (void) y; (void) z;
   DISPATCH(ProgramUniform3d, (program, location, x, y, z), (F, "glProgramUniform3d(%d, %d, %f, %f, %f);\n", program, location, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1065)(GLuint program, GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1065)(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform3dv, (program, location, count, value), (F, "glProgramUniform3dv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1066)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1066)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    (void) program; (void) location; (void) x; (void) y; (void) z;
   DISPATCH(ProgramUniform3f, (program, location, x, y, z), (F, "glProgramUniform3f(%d, %d, %f, %f, %f);\n", program, location, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1067)(GLuint program, GLint location, GLsizei count, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1067)(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform3fv, (program, location, count, value), (F, "glProgramUniform3fv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1068)(GLuint program, GLint location, GLint x, GLint y, GLint z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1068)(GLuint program, GLint location, GLint x, GLint y, GLint z)
{
    (void) program; (void) location; (void) x; (void) y; (void) z;
   DISPATCH(ProgramUniform3i, (program, location, x, y, z), (F, "glProgramUniform3i(%d, %d, %d, %d, %d);\n", program, location, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1069)(GLuint program, GLint location, GLsizei count, const GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1069)(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform3iv, (program, location, count, value), (F, "glProgramUniform3iv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1070)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1070)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z)
{
    (void) program; (void) location; (void) x; (void) y; (void) z;
   DISPATCH(ProgramUniform3ui, (program, location, x, y, z), (F, "glProgramUniform3ui(%d, %d, %d, %d, %d);\n", program, location, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1071)(GLuint program, GLint location, GLsizei count, const GLuint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1071)(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform3uiv, (program, location, count, value), (F, "glProgramUniform3uiv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1072)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1072)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) program; (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramUniform4d, (program, location, x, y, z, w), (F, "glProgramUniform4d(%d, %d, %f, %f, %f, %f);\n", program, location, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1073)(GLuint program, GLint location, GLsizei count, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1073)(GLuint program, GLint location, GLsizei count, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform4dv, (program, location, count, value), (F, "glProgramUniform4dv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1074)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1074)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) program; (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramUniform4f, (program, location, x, y, z, w), (F, "glProgramUniform4f(%d, %d, %f, %f, %f, %f);\n", program, location, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1075)(GLuint program, GLint location, GLsizei count, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1075)(GLuint program, GLint location, GLsizei count, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform4fv, (program, location, count, value), (F, "glProgramUniform4fv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1076)(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1076)(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    (void) program; (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramUniform4i, (program, location, x, y, z, w), (F, "glProgramUniform4i(%d, %d, %d, %d, %d, %d);\n", program, location, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1077)(GLuint program, GLint location, GLsizei count, const GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1077)(GLuint program, GLint location, GLsizei count, const GLint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform4iv, (program, location, count, value), (F, "glProgramUniform4iv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1078)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z, GLuint w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1078)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z, GLuint w)
{
    (void) program; (void) location; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramUniform4ui, (program, location, x, y, z, w), (F, "glProgramUniform4ui(%d, %d, %d, %d, %d, %d);\n", program, location, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1079)(GLuint program, GLint location, GLsizei count, const GLuint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1079)(GLuint program, GLint location, GLsizei count, const GLuint * value)
{
    (void) program; (void) location; (void) count; (void) value;
   DISPATCH(ProgramUniform4uiv, (program, location, count, value), (F, "glProgramUniform4uiv(%d, %d, %d, %p);\n", program, location, count, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1080)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1080)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1081)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1081)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1082)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1082)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2x3dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2x3dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1083)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1083)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2x3fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2x3fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1084)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1084)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2x4dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2x4dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1085)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1085)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix2x4fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix2x4fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1086)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1086)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1087)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1087)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1088)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1088)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3x2dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3x2dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1089)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1089)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3x2fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3x2fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1090)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1090)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3x4dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3x4dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1091)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1091)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix3x4fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix3x4fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1092)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1092)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1093)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1093)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1094)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1094)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4x2dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4x2dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1095)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1095)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4x2fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4x2fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1096)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1096)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4x3dv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4x3dv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1097)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1097)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    (void) program; (void) location; (void) count; (void) transpose; (void) value;
   DISPATCH(ProgramUniformMatrix4x3fv, (program, location, count, transpose, value), (F, "glProgramUniformMatrix4x3fv(%d, %d, %d, %d, %p);\n", program, location, count, transpose, (const void *) value));
}

KEYWORD1 void KEYWORD2 NAME(UnlockArraysEXT)(void)
{
   DISPATCH(UnlockArraysEXT, (), (F, "glUnlockArraysEXT();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1099)(GLuint pipeline, GLbitfield stages, GLuint program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1099)(GLuint pipeline, GLbitfield stages, GLuint program)
{
    (void) pipeline; (void) stages; (void) program;
   DISPATCH(UseProgramStages, (pipeline, stages, program), (F, "glUseProgramStages(%d, %d, %d);\n", pipeline, stages, program));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1100)(GLuint pipeline);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1100)(GLuint pipeline)
{
    (void) pipeline;
   DISPATCH(ValidateProgramPipeline, (pipeline), (F, "glValidateProgramPipeline(%d);\n", pipeline));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageCallbackARB)(GLDEBUGPROCARB callback, const GLvoid * userParam)
{
    (void) callback; (void) userParam;
   DISPATCH(DebugMessageCallback, (callback, userParam), (F, "glDebugMessageCallbackARB(%p, %p);\n", (const void *) callback, (const void *) userParam));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageCallback)(GLDEBUGPROC callback, const GLvoid * userParam)
{
    (void) callback; (void) userParam;
   DISPATCH(DebugMessageCallback, (callback, userParam), (F, "glDebugMessageCallback(%p, %p);\n", (const void *) callback, (const void *) userParam));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1101)(GLDEBUGPROC callback, const GLvoid * userParam);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1101)(GLDEBUGPROC callback, const GLvoid * userParam)
{
    (void) callback; (void) userParam;
   DISPATCH(DebugMessageCallback, (callback, userParam), (F, "glDebugMessageCallbackKHR(%p, %p);\n", (const void *) callback, (const void *) userParam));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageControlARB)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled)
{
    (void) source; (void) type; (void) severity; (void) count; (void) ids; (void) enabled;
   DISPATCH(DebugMessageControl, (source, type, severity, count, ids, enabled), (F, "glDebugMessageControlARB(0x%x, 0x%x, 0x%x, %d, %p, %d);\n", source, type, severity, count, (const void *) ids, enabled));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled)
{
    (void) source; (void) type; (void) severity; (void) count; (void) ids; (void) enabled;
   DISPATCH(DebugMessageControl, (source, type, severity, count, ids, enabled), (F, "glDebugMessageControl(0x%x, 0x%x, 0x%x, %d, %p, %d);\n", source, type, severity, count, (const void *) ids, enabled));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1102)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1102)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled)
{
    (void) source; (void) type; (void) severity; (void) count; (void) ids; (void) enabled;
   DISPATCH(DebugMessageControl, (source, type, severity, count, ids, enabled), (F, "glDebugMessageControlKHR(0x%x, 0x%x, 0x%x, %d, %p, %d);\n", source, type, severity, count, (const void *) ids, enabled));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageInsertARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLcharARB * buf)
{
    (void) source; (void) type; (void) id; (void) severity; (void) length; (void) buf;
   DISPATCH(DebugMessageInsert, (source, type, id, severity, length, buf), (F, "glDebugMessageInsertARB(0x%x, 0x%x, %d, 0x%x, %d, %p);\n", source, type, id, severity, length, (const void *) buf));
}

KEYWORD1 void KEYWORD2 NAME(DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf)
{
    (void) source; (void) type; (void) id; (void) severity; (void) length; (void) buf;
   DISPATCH(DebugMessageInsert, (source, type, id, severity, length, buf), (F, "glDebugMessageInsert(0x%x, 0x%x, %d, 0x%x, %d, %p);\n", source, type, id, severity, length, (const void *) buf));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1103)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1103)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf)
{
    (void) source; (void) type; (void) id; (void) severity; (void) length; (void) buf;
   DISPATCH(DebugMessageInsert, (source, type, id, severity, length, buf), (F, "glDebugMessageInsertKHR(0x%x, 0x%x, %d, 0x%x, %d, %p);\n", source, type, id, severity, length, (const void *) buf));
}

KEYWORD1 GLuint KEYWORD2 NAME(GetDebugMessageLogARB)(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLcharARB * messageLog)
{
    (void) count; (void) bufsize; (void) sources; (void) types; (void) ids; (void) severities; (void) lengths; (void) messageLog;
   RETURN_DISPATCH(GetDebugMessageLog, (count, bufsize, sources, types, ids, severities, lengths, messageLog), (F, "glGetDebugMessageLogARB(%d, %d, %p, %p, %p, %p, %p, %p);\n", count, bufsize, (const void *) sources, (const void *) types, (const void *) ids, (const void *) severities, (const void *) lengths, (const void *) messageLog));
}

KEYWORD1 GLuint KEYWORD2 NAME(GetDebugMessageLog)(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog)
{
    (void) count; (void) bufsize; (void) sources; (void) types; (void) ids; (void) severities; (void) lengths; (void) messageLog;
   RETURN_DISPATCH(GetDebugMessageLog, (count, bufsize, sources, types, ids, severities, lengths, messageLog), (F, "glGetDebugMessageLog(%d, %d, %p, %p, %p, %p, %p, %p);\n", count, bufsize, (const void *) sources, (const void *) types, (const void *) ids, (const void *) severities, (const void *) lengths, (const void *) messageLog));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1104)(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1104)(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog)
{
    (void) count; (void) bufsize; (void) sources; (void) types; (void) ids; (void) severities; (void) lengths; (void) messageLog;
   RETURN_DISPATCH(GetDebugMessageLog, (count, bufsize, sources, types, ids, severities, lengths, messageLog), (F, "glGetDebugMessageLogKHR(%d, %d, %p, %p, %p, %p, %p, %p);\n", count, bufsize, (const void *) sources, (const void *) types, (const void *) ids, (const void *) severities, (const void *) lengths, (const void *) messageLog));
}

KEYWORD1 void KEYWORD2 NAME(GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label)
{
    (void) identifier; (void) name; (void) bufSize; (void) length; (void) label;
   DISPATCH(GetObjectLabel, (identifier, name, bufSize, length, label), (F, "glGetObjectLabel(0x%x, %d, %d, %p, %p);\n", identifier, name, bufSize, (const void *) length, (const void *) label));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1105)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1105)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label)
{
    (void) identifier; (void) name; (void) bufSize; (void) length; (void) label;
   DISPATCH(GetObjectLabel, (identifier, name, bufSize, length, label), (F, "glGetObjectLabelKHR(0x%x, %d, %d, %p, %p);\n", identifier, name, bufSize, (const void *) length, (const void *) label));
}

KEYWORD1 void KEYWORD2 NAME(GetObjectPtrLabel)(const GLvoid * ptr, GLsizei bufSize, GLsizei * length, GLchar * label)
{
    (void) ptr; (void) bufSize; (void) length; (void) label;
   DISPATCH(GetObjectPtrLabel, (ptr, bufSize, length, label), (F, "glGetObjectPtrLabel(%p, %d, %p, %p);\n", (const void *) ptr, bufSize, (const void *) length, (const void *) label));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1106)(const GLvoid * ptr, GLsizei bufSize, GLsizei * length, GLchar * label);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1106)(const GLvoid * ptr, GLsizei bufSize, GLsizei * length, GLchar * label)
{
    (void) ptr; (void) bufSize; (void) length; (void) label;
   DISPATCH(GetObjectPtrLabel, (ptr, bufSize, length, label), (F, "glGetObjectPtrLabelKHR(%p, %d, %p, %p);\n", (const void *) ptr, bufSize, (const void *) length, (const void *) label));
}

KEYWORD1 void KEYWORD2 NAME(ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar * label)
{
    (void) identifier; (void) name; (void) length; (void) label;
   DISPATCH(ObjectLabel, (identifier, name, length, label), (F, "glObjectLabel(0x%x, %d, %d, %p);\n", identifier, name, length, (const void *) label));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1107)(GLenum identifier, GLuint name, GLsizei length, const GLchar * label);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1107)(GLenum identifier, GLuint name, GLsizei length, const GLchar * label)
{
    (void) identifier; (void) name; (void) length; (void) label;
   DISPATCH(ObjectLabel, (identifier, name, length, label), (F, "glObjectLabelKHR(0x%x, %d, %d, %p);\n", identifier, name, length, (const void *) label));
}

KEYWORD1 void KEYWORD2 NAME(ObjectPtrLabel)(const GLvoid * ptr, GLsizei length, const GLchar * label)
{
    (void) ptr; (void) length; (void) label;
   DISPATCH(ObjectPtrLabel, (ptr, length, label), (F, "glObjectPtrLabel(%p, %d, %p);\n", (const void *) ptr, length, (const void *) label));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1108)(const GLvoid * ptr, GLsizei length, const GLchar * label);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1108)(const GLvoid * ptr, GLsizei length, const GLchar * label)
{
    (void) ptr; (void) length; (void) label;
   DISPATCH(ObjectPtrLabel, (ptr, length, label), (F, "glObjectPtrLabelKHR(%p, %d, %p);\n", (const void *) ptr, length, (const void *) label));
}

KEYWORD1 void KEYWORD2 NAME(PopDebugGroup)(void)
{
   DISPATCH(PopDebugGroup, (), (F, "glPopDebugGroup();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1109)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1109)(void)
{
   DISPATCH(PopDebugGroup, (), (F, "glPopDebugGroupKHR();\n"));
}

KEYWORD1 void KEYWORD2 NAME(PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    (void) source; (void) id; (void) length; (void) message;
   DISPATCH(PushDebugGroup, (source, id, length, message), (F, "glPushDebugGroup(0x%x, %d, %d, %p);\n", source, id, length, (const void *) message));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1110)(GLenum source, GLuint id, GLsizei length, const GLchar * message);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1110)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    (void) source; (void) id; (void) length; (void) message;
   DISPATCH(PushDebugGroup, (source, id, length, message), (F, "glPushDebugGroupKHR(0x%x, %d, %d, %p);\n", source, id, length, (const void *) message));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3fEXT, (red, green, blue), (F, "glSecondaryColor3f(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3fEXT)(GLfloat red, GLfloat green, GLfloat blue)
{
    (void) red; (void) green; (void) blue;
   DISPATCH(SecondaryColor3fEXT, (red, green, blue), (F, "glSecondaryColor3fEXT(%f, %f, %f);\n", red, green, blue));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3fv)(const GLfloat * v)
{
    (void) v;
   DISPATCH(SecondaryColor3fvEXT, (v), (F, "glSecondaryColor3fv(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(SecondaryColor3fvEXT)(const GLfloat * v)
{
    (void) v;
   DISPATCH(SecondaryColor3fvEXT, (v), (F, "glSecondaryColor3fvEXT(%p);\n", (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawElements)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount;
   DISPATCH(MultiDrawElementsEXT, (mode, count, type, indices, primcount), (F, "glMultiDrawElements(0x%x, %p, 0x%x, %p, %d);\n", mode, (const void *) count, type, (const void *) indices, primcount));
}

KEYWORD1 void KEYWORD2 NAME(MultiDrawElementsEXT)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount;
   DISPATCH(MultiDrawElementsEXT, (mode, count, type, indices, primcount), (F, "glMultiDrawElementsEXT(0x%x, %p, 0x%x, %p, %d);\n", mode, (const void *) count, type, (const void *) indices, primcount));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordf)(GLfloat coord)
{
    (void) coord;
   DISPATCH(FogCoordfEXT, (coord), (F, "glFogCoordf(%f);\n", coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordfEXT)(GLfloat coord)
{
    (void) coord;
   DISPATCH(FogCoordfEXT, (coord), (F, "glFogCoordfEXT(%f);\n", coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordfv)(const GLfloat * coord)
{
    (void) coord;
   DISPATCH(FogCoordfvEXT, (coord), (F, "glFogCoordfv(%p);\n", (const void *) coord));
}

KEYWORD1 void KEYWORD2 NAME(FogCoordfvEXT)(const GLfloat * coord)
{
    (void) coord;
   DISPATCH(FogCoordfvEXT, (coord), (F, "glFogCoordfvEXT(%p);\n", (const void *) coord));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1116)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1116)(void)
{
   DISPATCH(ResizeBuffersMESA, (), (F, "glResizeBuffersMESA();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1117)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1117)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(WindowPos4dMESA, (x, y, z, w), (F, "glWindowPos4dMESA(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1118)(const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1118)(const GLdouble * v)
{
    (void) v;
   DISPATCH(WindowPos4dvMESA, (v), (F, "glWindowPos4dvMESA(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1119)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1119)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(WindowPos4fMESA, (x, y, z, w), (F, "glWindowPos4fMESA(%f, %f, %f, %f);\n", x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1120)(const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1120)(const GLfloat * v)
{
    (void) v;
   DISPATCH(WindowPos4fvMESA, (v), (F, "glWindowPos4fvMESA(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1121)(GLint x, GLint y, GLint z, GLint w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1121)(GLint x, GLint y, GLint z, GLint w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(WindowPos4iMESA, (x, y, z, w), (F, "glWindowPos4iMESA(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1122)(const GLint * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1122)(const GLint * v)
{
    (void) v;
   DISPATCH(WindowPos4ivMESA, (v), (F, "glWindowPos4ivMESA(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1123)(GLshort x, GLshort y, GLshort z, GLshort w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1123)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) x; (void) y; (void) z; (void) w;
   DISPATCH(WindowPos4sMESA, (x, y, z, w), (F, "glWindowPos4sMESA(%d, %d, %d, %d);\n", x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1124)(const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1124)(const GLshort * v)
{
    (void) v;
   DISPATCH(WindowPos4svMESA, (v), (F, "glWindowPos4svMESA(%p);\n", (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1125)(const GLenum * mode, const GLint * first, const GLsizei * count, GLsizei primcount, GLint modestride);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1125)(const GLenum * mode, const GLint * first, const GLsizei * count, GLsizei primcount, GLint modestride)
{
    (void) mode; (void) first; (void) count; (void) primcount; (void) modestride;
   DISPATCH(MultiModeDrawArraysIBM, (mode, first, count, primcount, modestride), (F, "glMultiModeDrawArraysIBM(%p, %p, %p, %d, %d);\n", (const void *) mode, (const void *) first, (const void *) count, primcount, modestride));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1126)(const GLenum * mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, GLint modestride);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1126)(const GLenum * mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, GLint modestride)
{
    (void) mode; (void) count; (void) type; (void) indices; (void) primcount; (void) modestride;
   DISPATCH(MultiModeDrawElementsIBM, (mode, count, type, indices, primcount, modestride), (F, "glMultiModeDrawElementsIBM(%p, %p, 0x%x, %p, %d, %d);\n", (const void *) mode, (const void *) count, type, (const void *) indices, primcount, modestride));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1127)(GLsizei n, const GLuint * ids, GLboolean * residences);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1127)(GLsizei n, const GLuint * ids, GLboolean * residences)
{
    (void) n; (void) ids; (void) residences;
   RETURN_DISPATCH(AreProgramsResidentNV, (n, ids, residences), (F, "glAreProgramsResidentNV(%d, %p, %p);\n", n, (const void *) ids, (const void *) residences));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1128)(GLenum target, GLuint id, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1128)(GLenum target, GLuint id, const GLfloat * params)
{
    (void) target; (void) id; (void) params;
   DISPATCH(ExecuteProgramNV, (target, id, params), (F, "glExecuteProgramNV(0x%x, %d, %p);\n", target, id, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1129)(GLenum target, GLuint index, GLenum pname, GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1129)(GLenum target, GLuint index, GLenum pname, GLdouble * params)
{
    (void) target; (void) index; (void) pname; (void) params;
   DISPATCH(GetProgramParameterdvNV, (target, index, pname, params), (F, "glGetProgramParameterdvNV(0x%x, %d, 0x%x, %p);\n", target, index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1130)(GLenum target, GLuint index, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1130)(GLenum target, GLuint index, GLenum pname, GLfloat * params)
{
    (void) target; (void) index; (void) pname; (void) params;
   DISPATCH(GetProgramParameterfvNV, (target, index, pname, params), (F, "glGetProgramParameterfvNV(0x%x, %d, 0x%x, %p);\n", target, index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1131)(GLuint id, GLenum pname, GLubyte * program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1131)(GLuint id, GLenum pname, GLubyte * program)
{
    (void) id; (void) pname; (void) program;
   DISPATCH(GetProgramStringNV, (id, pname, program), (F, "glGetProgramStringNV(%d, 0x%x, %p);\n", id, pname, (const void *) program));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1132)(GLuint id, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1132)(GLuint id, GLenum pname, GLint * params)
{
    (void) id; (void) pname; (void) params;
   DISPATCH(GetProgramivNV, (id, pname, params), (F, "glGetProgramivNV(%d, 0x%x, %p);\n", id, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1133)(GLenum target, GLuint address, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1133)(GLenum target, GLuint address, GLenum pname, GLint * params)
{
    (void) target; (void) address; (void) pname; (void) params;
   DISPATCH(GetTrackMatrixivNV, (target, address, pname, params), (F, "glGetTrackMatrixivNV(0x%x, %d, 0x%x, %p);\n", target, address, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1134)(GLuint index, GLenum pname, GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1134)(GLuint index, GLenum pname, GLdouble * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribdvNV, (index, pname, params), (F, "glGetVertexAttribdvNV(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1135)(GLuint index, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1135)(GLuint index, GLenum pname, GLfloat * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribfvNV, (index, pname, params), (F, "glGetVertexAttribfvNV(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1136)(GLuint index, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1136)(GLuint index, GLenum pname, GLint * params)
{
    (void) index; (void) pname; (void) params;
   DISPATCH(GetVertexAttribivNV, (index, pname, params), (F, "glGetVertexAttribivNV(%d, 0x%x, %p);\n", index, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1137)(GLenum target, GLuint id, GLsizei len, const GLubyte * program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1137)(GLenum target, GLuint id, GLsizei len, const GLubyte * program)
{
    (void) target; (void) id; (void) len; (void) program;
   DISPATCH(LoadProgramNV, (target, id, len, program), (F, "glLoadProgramNV(0x%x, %d, %d, %p);\n", target, id, len, (const void *) program));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1138)(GLenum target, GLuint index, GLsizei num, const GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1138)(GLenum target, GLuint index, GLsizei num, const GLdouble * params)
{
    (void) target; (void) index; (void) num; (void) params;
   DISPATCH(ProgramParameters4dvNV, (target, index, num, params), (F, "glProgramParameters4dvNV(0x%x, %d, %d, %p);\n", target, index, num, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1139)(GLenum target, GLuint index, GLsizei num, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1139)(GLenum target, GLuint index, GLsizei num, const GLfloat * params)
{
    (void) target; (void) index; (void) num; (void) params;
   DISPATCH(ProgramParameters4fvNV, (target, index, num, params), (F, "glProgramParameters4fvNV(0x%x, %d, %d, %p);\n", target, index, num, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1140)(GLsizei n, const GLuint * ids);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1140)(GLsizei n, const GLuint * ids)
{
    (void) n; (void) ids;
   DISPATCH(RequestResidentProgramsNV, (n, ids), (F, "glRequestResidentProgramsNV(%d, %p);\n", n, (const void *) ids));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1141)(GLenum target, GLuint address, GLenum matrix, GLenum transform);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1141)(GLenum target, GLuint address, GLenum matrix, GLenum transform)
{
    (void) target; (void) address; (void) matrix; (void) transform;
   DISPATCH(TrackMatrixNV, (target, address, matrix, transform), (F, "glTrackMatrixNV(0x%x, %d, 0x%x, 0x%x);\n", target, address, matrix, transform));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1142)(GLuint index, GLdouble x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1142)(GLuint index, GLdouble x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1dNV, (index, x), (F, "glVertexAttrib1dNV(%d, %f);\n", index, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1143)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1143)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1dvNV, (index, v), (F, "glVertexAttrib1dvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1144)(GLuint index, GLfloat x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1144)(GLuint index, GLfloat x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1fNV, (index, x), (F, "glVertexAttrib1fNV(%d, %f);\n", index, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1145)(GLuint index, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1145)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1fvNV, (index, v), (F, "glVertexAttrib1fvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1146)(GLuint index, GLshort x);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1146)(GLuint index, GLshort x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttrib1sNV, (index, x), (F, "glVertexAttrib1sNV(%d, %d);\n", index, x));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1147)(GLuint index, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1147)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib1svNV, (index, v), (F, "glVertexAttrib1svNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1148)(GLuint index, GLdouble x, GLdouble y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1148)(GLuint index, GLdouble x, GLdouble y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2dNV, (index, x, y), (F, "glVertexAttrib2dNV(%d, %f, %f);\n", index, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1149)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1149)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2dvNV, (index, v), (F, "glVertexAttrib2dvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1150)(GLuint index, GLfloat x, GLfloat y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1150)(GLuint index, GLfloat x, GLfloat y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2fNV, (index, x, y), (F, "glVertexAttrib2fNV(%d, %f, %f);\n", index, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1151)(GLuint index, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1151)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2fvNV, (index, v), (F, "glVertexAttrib2fvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1152)(GLuint index, GLshort x, GLshort y);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1152)(GLuint index, GLshort x, GLshort y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttrib2sNV, (index, x, y), (F, "glVertexAttrib2sNV(%d, %d, %d);\n", index, x, y));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1153)(GLuint index, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1153)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib2svNV, (index, v), (F, "glVertexAttrib2svNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1154)(GLuint index, GLdouble x, GLdouble y, GLdouble z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1154)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3dNV, (index, x, y, z), (F, "glVertexAttrib3dNV(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1155)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1155)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3dvNV, (index, v), (F, "glVertexAttrib3dvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1156)(GLuint index, GLfloat x, GLfloat y, GLfloat z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1156)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3fNV, (index, x, y, z), (F, "glVertexAttrib3fNV(%d, %f, %f, %f);\n", index, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1157)(GLuint index, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1157)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3fvNV, (index, v), (F, "glVertexAttrib3fvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1158)(GLuint index, GLshort x, GLshort y, GLshort z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1158)(GLuint index, GLshort x, GLshort y, GLshort z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttrib3sNV, (index, x, y, z), (F, "glVertexAttrib3sNV(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1159)(GLuint index, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1159)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib3svNV, (index, v), (F, "glVertexAttrib3svNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1160)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1160)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4dNV, (index, x, y, z, w), (F, "glVertexAttrib4dNV(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1161)(GLuint index, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1161)(GLuint index, const GLdouble * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4dvNV, (index, v), (F, "glVertexAttrib4dvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1162)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1162)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4fNV, (index, x, y, z, w), (F, "glVertexAttrib4fNV(%d, %f, %f, %f, %f);\n", index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1163)(GLuint index, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1163)(GLuint index, const GLfloat * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4fvNV, (index, v), (F, "glVertexAttrib4fvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1164)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1164)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4sNV, (index, x, y, z, w), (F, "glVertexAttrib4sNV(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1165)(GLuint index, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1165)(GLuint index, const GLshort * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4svNV, (index, v), (F, "glVertexAttrib4svNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1166)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1166)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttrib4ubNV, (index, x, y, z, w), (F, "glVertexAttrib4ubNV(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1167)(GLuint index, const GLubyte * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1167)(GLuint index, const GLubyte * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttrib4ubvNV, (index, v), (F, "glVertexAttrib4ubvNV(%d, %p);\n", index, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1168)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1168)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer)
{
    (void) index; (void) size; (void) type; (void) stride; (void) pointer;
   DISPATCH(VertexAttribPointerNV, (index, size, type, stride, pointer), (F, "glVertexAttribPointerNV(%d, %d, 0x%x, %d, %p);\n", index, size, type, stride, (const void *) pointer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1169)(GLuint index, GLsizei n, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1169)(GLuint index, GLsizei n, const GLdouble * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs1dvNV, (index, n, v), (F, "glVertexAttribs1dvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1170)(GLuint index, GLsizei n, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1170)(GLuint index, GLsizei n, const GLfloat * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs1fvNV, (index, n, v), (F, "glVertexAttribs1fvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1171)(GLuint index, GLsizei n, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1171)(GLuint index, GLsizei n, const GLshort * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs1svNV, (index, n, v), (F, "glVertexAttribs1svNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1172)(GLuint index, GLsizei n, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1172)(GLuint index, GLsizei n, const GLdouble * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs2dvNV, (index, n, v), (F, "glVertexAttribs2dvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1173)(GLuint index, GLsizei n, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1173)(GLuint index, GLsizei n, const GLfloat * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs2fvNV, (index, n, v), (F, "glVertexAttribs2fvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1174)(GLuint index, GLsizei n, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1174)(GLuint index, GLsizei n, const GLshort * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs2svNV, (index, n, v), (F, "glVertexAttribs2svNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1175)(GLuint index, GLsizei n, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1175)(GLuint index, GLsizei n, const GLdouble * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs3dvNV, (index, n, v), (F, "glVertexAttribs3dvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1176)(GLuint index, GLsizei n, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1176)(GLuint index, GLsizei n, const GLfloat * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs3fvNV, (index, n, v), (F, "glVertexAttribs3fvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1177)(GLuint index, GLsizei n, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1177)(GLuint index, GLsizei n, const GLshort * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs3svNV, (index, n, v), (F, "glVertexAttribs3svNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1178)(GLuint index, GLsizei n, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1178)(GLuint index, GLsizei n, const GLdouble * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs4dvNV, (index, n, v), (F, "glVertexAttribs4dvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1179)(GLuint index, GLsizei n, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1179)(GLuint index, GLsizei n, const GLfloat * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs4fvNV, (index, n, v), (F, "glVertexAttribs4fvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1180)(GLuint index, GLsizei n, const GLshort * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1180)(GLuint index, GLsizei n, const GLshort * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs4svNV, (index, n, v), (F, "glVertexAttribs4svNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1181)(GLuint index, GLsizei n, const GLubyte * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1181)(GLuint index, GLsizei n, const GLubyte * v)
{
    (void) index; (void) n; (void) v;
   DISPATCH(VertexAttribs4ubvNV, (index, n, v), (F, "glVertexAttribs4ubvNV(%d, %d, %p);\n", index, n, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1182)(GLenum pname, GLfloat * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1182)(GLenum pname, GLfloat * param)
{
    (void) pname; (void) param;
   DISPATCH(GetTexBumpParameterfvATI, (pname, param), (F, "glGetTexBumpParameterfvATI(0x%x, %p);\n", pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1183)(GLenum pname, GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1183)(GLenum pname, GLint * param)
{
    (void) pname; (void) param;
   DISPATCH(GetTexBumpParameterivATI, (pname, param), (F, "glGetTexBumpParameterivATI(0x%x, %p);\n", pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1184)(GLenum pname, const GLfloat * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1184)(GLenum pname, const GLfloat * param)
{
    (void) pname; (void) param;
   DISPATCH(TexBumpParameterfvATI, (pname, param), (F, "glTexBumpParameterfvATI(0x%x, %p);\n", pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1185)(GLenum pname, const GLint * param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1185)(GLenum pname, const GLint * param)
{
    (void) pname; (void) param;
   DISPATCH(TexBumpParameterivATI, (pname, param), (F, "glTexBumpParameterivATI(0x%x, %p);\n", pname, (const void *) param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1186)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1186)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
    (void) op; (void) dst; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod;
   DISPATCH(AlphaFragmentOp1ATI, (op, dst, dstMod, arg1, arg1Rep, arg1Mod), (F, "glAlphaFragmentOp1ATI(0x%x, %d, %d, %d, %d, %d);\n", op, dst, dstMod, arg1, arg1Rep, arg1Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1187)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1187)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
    (void) op; (void) dst; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod; (void) arg2; (void) arg2Rep; (void) arg2Mod;
   DISPATCH(AlphaFragmentOp2ATI, (op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod), (F, "glAlphaFragmentOp2ATI(0x%x, %d, %d, %d, %d, %d, %d, %d, %d);\n", op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1188)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1188)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
    (void) op; (void) dst; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod; (void) arg2; (void) arg2Rep; (void) arg2Mod; (void) arg3; (void) arg3Rep; (void) arg3Mod;
   DISPATCH(AlphaFragmentOp3ATI, (op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod), (F, "glAlphaFragmentOp3ATI(0x%x, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);\n", op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1189)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1189)(void)
{
   DISPATCH(BeginFragmentShaderATI, (), (F, "glBeginFragmentShaderATI();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1190)(GLuint id);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1190)(GLuint id)
{
    (void) id;
   DISPATCH(BindFragmentShaderATI, (id), (F, "glBindFragmentShaderATI(%d);\n", id));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1191)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1191)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)
{
    (void) op; (void) dst; (void) dstMask; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod;
   DISPATCH(ColorFragmentOp1ATI, (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod), (F, "glColorFragmentOp1ATI(0x%x, %d, %d, %d, %d, %d, %d);\n", op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1192)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1192)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)
{
    (void) op; (void) dst; (void) dstMask; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod; (void) arg2; (void) arg2Rep; (void) arg2Mod;
   DISPATCH(ColorFragmentOp2ATI, (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod), (F, "glColorFragmentOp2ATI(0x%x, %d, %d, %d, %d, %d, %d, %d, %d, %d);\n", op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1193)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1193)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)
{
    (void) op; (void) dst; (void) dstMask; (void) dstMod; (void) arg1; (void) arg1Rep; (void) arg1Mod; (void) arg2; (void) arg2Rep; (void) arg2Mod; (void) arg3; (void) arg3Rep; (void) arg3Mod;
   DISPATCH(ColorFragmentOp3ATI, (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod), (F, "glColorFragmentOp3ATI(0x%x, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d);\n", op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1194)(GLuint id);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1194)(GLuint id)
{
    (void) id;
   DISPATCH(DeleteFragmentShaderATI, (id), (F, "glDeleteFragmentShaderATI(%d);\n", id));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1195)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1195)(void)
{
   DISPATCH(EndFragmentShaderATI, (), (F, "glEndFragmentShaderATI();\n"));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1196)(GLuint range);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1196)(GLuint range)
{
    (void) range;
   RETURN_DISPATCH(GenFragmentShadersATI, (range), (F, "glGenFragmentShadersATI(%d);\n", range));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1197)(GLuint dst, GLuint coord, GLenum swizzle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1197)(GLuint dst, GLuint coord, GLenum swizzle)
{
    (void) dst; (void) coord; (void) swizzle;
   DISPATCH(PassTexCoordATI, (dst, coord, swizzle), (F, "glPassTexCoordATI(%d, %d, 0x%x);\n", dst, coord, swizzle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1198)(GLuint dst, GLuint interp, GLenum swizzle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1198)(GLuint dst, GLuint interp, GLenum swizzle)
{
    (void) dst; (void) interp; (void) swizzle;
   DISPATCH(SampleMapATI, (dst, interp, swizzle), (F, "glSampleMapATI(%d, %d, 0x%x);\n", dst, interp, swizzle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1199)(GLuint dst, const GLfloat * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1199)(GLuint dst, const GLfloat * value)
{
    (void) dst; (void) value;
   DISPATCH(SetFragmentShaderConstantATI, (dst, value), (F, "glSetFragmentShaderConstantATI(%d, %p);\n", dst, (const void *) value));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1200)(GLenum face);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1200)(GLenum face)
{
    (void) face;
   DISPATCH(ActiveStencilFaceEXT, (face), (F, "glActiveStencilFaceEXT(0x%x);\n", face));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1201)(GLuint array);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1201)(GLuint array)
{
    (void) array;
   DISPATCH(BindVertexArrayAPPLE, (array), (F, "glBindVertexArrayAPPLE(%d);\n", array));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1202)(GLsizei n, GLuint * arrays);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1202)(GLsizei n, GLuint * arrays)
{
    (void) n; (void) arrays;
   DISPATCH(GenVertexArraysAPPLE, (n, arrays), (F, "glGenVertexArraysAPPLE(%d, %p);\n", n, (const void *) arrays));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1203)(GLuint id, GLsizei len, const GLubyte * name, GLdouble * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1203)(GLuint id, GLsizei len, const GLubyte * name, GLdouble * params)
{
    (void) id; (void) len; (void) name; (void) params;
   DISPATCH(GetProgramNamedParameterdvNV, (id, len, name, params), (F, "glGetProgramNamedParameterdvNV(%d, %d, %p, %p);\n", id, len, (const void *) name, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1204)(GLuint id, GLsizei len, const GLubyte * name, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1204)(GLuint id, GLsizei len, const GLubyte * name, GLfloat * params)
{
    (void) id; (void) len; (void) name; (void) params;
   DISPATCH(GetProgramNamedParameterfvNV, (id, len, name, params), (F, "glGetProgramNamedParameterfvNV(%d, %d, %p, %p);\n", id, len, (const void *) name, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1205)(GLuint id, GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1205)(GLuint id, GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    (void) id; (void) len; (void) name; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramNamedParameter4dNV, (id, len, name, x, y, z, w), (F, "glProgramNamedParameter4dNV(%d, %d, %p, %f, %f, %f, %f);\n", id, len, (const void *) name, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1206)(GLuint id, GLsizei len, const GLubyte * name, const GLdouble * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1206)(GLuint id, GLsizei len, const GLubyte * name, const GLdouble * v)
{
    (void) id; (void) len; (void) name; (void) v;
   DISPATCH(ProgramNamedParameter4dvNV, (id, len, name, v), (F, "glProgramNamedParameter4dvNV(%d, %d, %p, %p);\n", id, len, (const void *) name, (const void *) v));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1207)(GLuint id, GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1207)(GLuint id, GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    (void) id; (void) len; (void) name; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(ProgramNamedParameter4fNV, (id, len, name, x, y, z, w), (F, "glProgramNamedParameter4fNV(%d, %d, %p, %f, %f, %f, %f);\n", id, len, (const void *) name, x, y, z, w));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1208)(GLuint id, GLsizei len, const GLubyte * name, const GLfloat * v);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1208)(GLuint id, GLsizei len, const GLubyte * name, const GLfloat * v)
{
    (void) id; (void) len; (void) name; (void) v;
   DISPATCH(ProgramNamedParameter4fvNV, (id, len, name, v), (F, "glProgramNamedParameter4fvNV(%d, %d, %p, %p);\n", id, len, (const void *) name, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(PrimitiveRestartNV)(void)
{
   DISPATCH(PrimitiveRestartNV, (), (F, "glPrimitiveRestartNV();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1210)(GLenum coord, GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1210)(GLenum coord, GLenum pname, GLfixed * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(GetTexGenxvOES, (coord, pname, params), (F, "glGetTexGenxvOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1211)(GLenum coord, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1211)(GLenum coord, GLenum pname, GLint param)
{
    (void) coord; (void) pname; (void) param;
   DISPATCH(TexGenxOES, (coord, pname, param), (F, "glTexGenxOES(0x%x, 0x%x, %d);\n", coord, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1212)(GLenum coord, GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1212)(GLenum coord, GLenum pname, const GLfixed * params)
{
    (void) coord; (void) pname; (void) params;
   DISPATCH(TexGenxvOES, (coord, pname, params), (F, "glTexGenxvOES(0x%x, 0x%x, %p);\n", coord, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1213)(GLclampd zmin, GLclampd zmax);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1213)(GLclampd zmin, GLclampd zmax)
{
    (void) zmin; (void) zmax;
   DISPATCH(DepthBoundsEXT, (zmin, zmax), (F, "glDepthBoundsEXT(%f, %f);\n", zmin, zmax));
}

KEYWORD1 void KEYWORD2 NAME(BindFramebufferEXT)(GLenum target, GLuint framebuffer)
{
    (void) target; (void) framebuffer;
   DISPATCH(BindFramebufferEXT, (target, framebuffer), (F, "glBindFramebufferEXT(0x%x, %d);\n", target, framebuffer));
}

KEYWORD1 void KEYWORD2 NAME(BindRenderbufferEXT)(GLenum target, GLuint renderbuffer)
{
    (void) target; (void) renderbuffer;
   DISPATCH(BindRenderbufferEXT, (target, renderbuffer), (F, "glBindRenderbufferEXT(0x%x, %d);\n", target, renderbuffer));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1216)(GLsizei len, const GLvoid * string);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1216)(GLsizei len, const GLvoid * string)
{
    (void) len; (void) string;
   DISPATCH(StringMarkerGREMEDY, (len, string), (F, "glStringMarkerGREMEDY(%d, %p);\n", len, (const void *) string));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1217)(GLenum target, GLenum pname, GLint param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1217)(GLenum target, GLenum pname, GLint param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(BufferParameteriAPPLE, (target, pname, param), (F, "glBufferParameteriAPPLE(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1218)(GLenum target, GLintptr offset, GLsizeiptr size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1218)(GLenum target, GLintptr offset, GLsizeiptr size)
{
    (void) target; (void) offset; (void) size;
   DISPATCH(FlushMappedBufferRangeAPPLE, (target, offset, size), (F, "glFlushMappedBufferRangeAPPLE(0x%x, %d, %d);\n", target, offset, size));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1iEXT)(GLuint index, GLint x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttribI1iEXT, (index, x), (F, "glVertexAttribI1iEXT(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1i)(GLuint index, GLint x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttribI1iEXT, (index, x), (F, "glVertexAttribI1i(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1uiEXT)(GLuint index, GLuint x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttribI1uiEXT, (index, x), (F, "glVertexAttribI1uiEXT(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI1ui)(GLuint index, GLuint x)
{
    (void) index; (void) x;
   DISPATCH(VertexAttribI1uiEXT, (index, x), (F, "glVertexAttribI1ui(%d, %d);\n", index, x));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2iEXT)(GLuint index, GLint x, GLint y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttribI2iEXT, (index, x, y), (F, "glVertexAttribI2iEXT(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2i)(GLuint index, GLint x, GLint y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttribI2iEXT, (index, x, y), (F, "glVertexAttribI2i(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2ivEXT)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI2ivEXT, (index, v), (F, "glVertexAttribI2ivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2iv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI2ivEXT, (index, v), (F, "glVertexAttribI2iv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttribI2uiEXT, (index, x, y), (F, "glVertexAttribI2uiEXT(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2ui)(GLuint index, GLuint x, GLuint y)
{
    (void) index; (void) x; (void) y;
   DISPATCH(VertexAttribI2uiEXT, (index, x, y), (F, "glVertexAttribI2ui(%d, %d, %d);\n", index, x, y));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2uivEXT)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI2uivEXT, (index, v), (F, "glVertexAttribI2uivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI2uiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI2uivEXT, (index, v), (F, "glVertexAttribI2uiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttribI3iEXT, (index, x, y, z), (F, "glVertexAttribI3iEXT(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttribI3iEXT, (index, x, y, z), (F, "glVertexAttribI3i(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3ivEXT)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI3ivEXT, (index, v), (F, "glVertexAttribI3ivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3iv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI3ivEXT, (index, v), (F, "glVertexAttribI3iv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttribI3uiEXT, (index, x, y, z), (F, "glVertexAttribI3uiEXT(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z)
{
    (void) index; (void) x; (void) y; (void) z;
   DISPATCH(VertexAttribI3uiEXT, (index, x, y, z), (F, "glVertexAttribI3ui(%d, %d, %d, %d);\n", index, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3uivEXT)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI3uivEXT, (index, v), (F, "glVertexAttribI3uivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI3uiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI3uivEXT, (index, v), (F, "glVertexAttribI3uiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttribI4iEXT, (index, x, y, z, w), (F, "glVertexAttribI4iEXT(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttribI4iEXT, (index, x, y, z, w), (F, "glVertexAttribI4i(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4ivEXT)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4ivEXT, (index, v), (F, "glVertexAttribI4ivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4iv)(GLuint index, const GLint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4ivEXT, (index, v), (F, "glVertexAttribI4iv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttribI4uiEXT, (index, x, y, z, w), (F, "glVertexAttribI4uiEXT(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    (void) index; (void) x; (void) y; (void) z; (void) w;
   DISPATCH(VertexAttribI4uiEXT, (index, x, y, z, w), (F, "glVertexAttribI4ui(%d, %d, %d, %d, %d);\n", index, x, y, z, w));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4uivEXT)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4uivEXT, (index, v), (F, "glVertexAttribI4uivEXT(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(VertexAttribI4uiv)(GLuint index, const GLuint * v)
{
    (void) index; (void) v;
   DISPATCH(VertexAttribI4uivEXT, (index, v), (F, "glVertexAttribI4uiv(%d, %p);\n", index, (const void *) v));
}

KEYWORD1 void KEYWORD2 NAME(ClearColorIiEXT)(GLint r, GLint g, GLint b, GLint a)
{
    (void) r; (void) g; (void) b; (void) a;
   DISPATCH(ClearColorIiEXT, (r, g, b, a), (F, "glClearColorIiEXT(%d, %d, %d, %d);\n", r, g, b, a));
}

KEYWORD1 void KEYWORD2 NAME(ClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a)
{
    (void) r; (void) g; (void) b; (void) a;
   DISPATCH(ClearColorIuiEXT, (r, g, b, a), (F, "glClearColorIuiEXT(%d, %d, %d, %d);\n", r, g, b, a));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1235)(GLenum target, GLuint index, GLuint buffer, GLintptr offset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1235)(GLenum target, GLuint index, GLuint buffer, GLintptr offset)
{
    (void) target; (void) index; (void) buffer; (void) offset;
   DISPATCH(BindBufferOffsetEXT, (target, index, buffer, offset), (F, "glBindBufferOffsetEXT(0x%x, %d, %d, %d);\n", target, index, buffer, offset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1236)(GLuint monitor);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1236)(GLuint monitor)
{
    (void) monitor;
   DISPATCH(BeginPerfMonitorAMD, (monitor), (F, "glBeginPerfMonitorAMD(%d);\n", monitor));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1237)(GLsizei n, GLuint * monitors);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1237)(GLsizei n, GLuint * monitors)
{
    (void) n; (void) monitors;
   DISPATCH(DeletePerfMonitorsAMD, (n, monitors), (F, "glDeletePerfMonitorsAMD(%d, %p);\n", n, (const void *) monitors));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1238)(GLuint monitor);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1238)(GLuint monitor)
{
    (void) monitor;
   DISPATCH(EndPerfMonitorAMD, (monitor), (F, "glEndPerfMonitorAMD(%d);\n", monitor));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1239)(GLsizei n, GLuint * monitors);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1239)(GLsizei n, GLuint * monitors)
{
    (void) n; (void) monitors;
   DISPATCH(GenPerfMonitorsAMD, (n, monitors), (F, "glGenPerfMonitorsAMD(%d, %p);\n", n, (const void *) monitors));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1240)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint * data, GLint * bytesWritten);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1240)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint * data, GLint * bytesWritten)
{
    (void) monitor; (void) pname; (void) dataSize; (void) data; (void) bytesWritten;
   DISPATCH(GetPerfMonitorCounterDataAMD, (monitor, pname, dataSize, data, bytesWritten), (F, "glGetPerfMonitorCounterDataAMD(%d, 0x%x, %d, %p, %p);\n", monitor, pname, dataSize, (const void *) data, (const void *) bytesWritten));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1241)(GLuint group, GLuint counter, GLenum pname, GLvoid * data);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1241)(GLuint group, GLuint counter, GLenum pname, GLvoid * data)
{
    (void) group; (void) counter; (void) pname; (void) data;
   DISPATCH(GetPerfMonitorCounterInfoAMD, (group, counter, pname, data), (F, "glGetPerfMonitorCounterInfoAMD(%d, %d, 0x%x, %p);\n", group, counter, pname, (const void *) data));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1242)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei * length, GLchar * counterString);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1242)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei * length, GLchar * counterString)
{
    (void) group; (void) counter; (void) bufSize; (void) length; (void) counterString;
   DISPATCH(GetPerfMonitorCounterStringAMD, (group, counter, bufSize, length, counterString), (F, "glGetPerfMonitorCounterStringAMD(%d, %d, %d, %p, %p);\n", group, counter, bufSize, (const void *) length, (const void *) counterString));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1243)(GLuint group, GLint * numCounters, GLint * maxActiveCounters, GLsizei countersSize, GLuint * counters);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1243)(GLuint group, GLint * numCounters, GLint * maxActiveCounters, GLsizei countersSize, GLuint * counters)
{
    (void) group; (void) numCounters; (void) maxActiveCounters; (void) countersSize; (void) counters;
   DISPATCH(GetPerfMonitorCountersAMD, (group, numCounters, maxActiveCounters, countersSize, counters), (F, "glGetPerfMonitorCountersAMD(%d, %p, %p, %d, %p);\n", group, (const void *) numCounters, (const void *) maxActiveCounters, countersSize, (const void *) counters));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1244)(GLuint group, GLsizei bufSize, GLsizei * length, GLchar * groupString);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1244)(GLuint group, GLsizei bufSize, GLsizei * length, GLchar * groupString)
{
    (void) group; (void) bufSize; (void) length; (void) groupString;
   DISPATCH(GetPerfMonitorGroupStringAMD, (group, bufSize, length, groupString), (F, "glGetPerfMonitorGroupStringAMD(%d, %d, %p, %p);\n", group, bufSize, (const void *) length, (const void *) groupString));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1245)(GLint * numGroups, GLsizei groupsSize, GLuint * groups);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1245)(GLint * numGroups, GLsizei groupsSize, GLuint * groups)
{
    (void) numGroups; (void) groupsSize; (void) groups;
   DISPATCH(GetPerfMonitorGroupsAMD, (numGroups, groupsSize, groups), (F, "glGetPerfMonitorGroupsAMD(%p, %d, %p);\n", (const void *) numGroups, groupsSize, (const void *) groups));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1246)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint * counterList);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1246)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint * counterList)
{
    (void) monitor; (void) enable; (void) group; (void) numCounters; (void) counterList;
   DISPATCH(SelectPerfMonitorCountersAMD, (monitor, enable, group, numCounters, counterList), (F, "glSelectPerfMonitorCountersAMD(%d, %d, %d, %d, %p);\n", monitor, enable, group, numCounters, (const void *) counterList));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1247)(GLenum objectType, GLuint name, GLenum pname, GLint * value);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1247)(GLenum objectType, GLuint name, GLenum pname, GLint * value)
{
    (void) objectType; (void) name; (void) pname; (void) value;
   DISPATCH(GetObjectParameterivAPPLE, (objectType, name, pname, value), (F, "glGetObjectParameterivAPPLE(0x%x, %d, 0x%x, %p);\n", objectType, name, pname, (const void *) value));
}

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_1248)(GLenum objectType, GLuint name, GLenum option);

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_1248)(GLenum objectType, GLuint name, GLenum option)
{
    (void) objectType; (void) name; (void) option;
   RETURN_DISPATCH(ObjectPurgeableAPPLE, (objectType, name, option), (F, "glObjectPurgeableAPPLE(0x%x, %d, 0x%x);\n", objectType, name, option));
}

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_1249)(GLenum objectType, GLuint name, GLenum option);

KEYWORD1_ALT GLenum KEYWORD2 NAME(_dispatch_stub_1249)(GLenum objectType, GLuint name, GLenum option)
{
    (void) objectType; (void) name; (void) option;
   RETURN_DISPATCH(ObjectUnpurgeableAPPLE, (objectType, name, option), (F, "glObjectUnpurgeableAPPLE(0x%x, %d, 0x%x);\n", objectType, name, option));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1250)(GLuint program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1250)(GLuint program)
{
    (void) program;
   DISPATCH(ActiveProgramEXT, (program), (F, "glActiveProgramEXT(%d);\n", program));
}

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1251)(GLenum type, const GLchar * string);

KEYWORD1_ALT GLuint KEYWORD2 NAME(_dispatch_stub_1251)(GLenum type, const GLchar * string)
{
    (void) type; (void) string;
   RETURN_DISPATCH(CreateShaderProgramEXT, (type, string), (F, "glCreateShaderProgramEXT(0x%x, %p);\n", type, (const void *) string));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1252)(GLenum type, GLuint program);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1252)(GLenum type, GLuint program)
{
    (void) type; (void) program;
   DISPATCH(UseShaderProgramEXT, (type, program), (F, "glUseShaderProgramEXT(0x%x, %d);\n", type, program));
}

KEYWORD1 void KEYWORD2 NAME(TextureBarrierNV)(void)
{
   DISPATCH(TextureBarrierNV, (), (F, "glTextureBarrierNV();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1253)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1253)(void)
{
   DISPATCH(TextureBarrierNV, (), (F, "glTextureBarrier();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1254)(void);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1254)(void)
{
   DISPATCH(VDPAUFiniNV, (), (F, "glVDPAUFiniNV();\n"));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1255)(GLintptr surface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1255)(GLintptr surface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values)
{
    (void) surface; (void) pname; (void) bufSize; (void) length; (void) values;
   DISPATCH(VDPAUGetSurfaceivNV, (surface, pname, bufSize, length, values), (F, "glVDPAUGetSurfaceivNV(%d, 0x%x, %d, %p, %p);\n", surface, pname, bufSize, (const void *) length, (const void *) values));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1256)(const GLvoid * vdpDevice, const GLvoid * getProcAddress);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1256)(const GLvoid * vdpDevice, const GLvoid * getProcAddress)
{
    (void) vdpDevice; (void) getProcAddress;
   DISPATCH(VDPAUInitNV, (vdpDevice, getProcAddress), (F, "glVDPAUInitNV(%p, %p);\n", (const void *) vdpDevice, (const void *) getProcAddress));
}

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1257)(GLintptr surface);

KEYWORD1_ALT GLboolean KEYWORD2 NAME(_dispatch_stub_1257)(GLintptr surface)
{
    (void) surface;
   RETURN_DISPATCH(VDPAUIsSurfaceNV, (surface), (F, "glVDPAUIsSurfaceNV(%d);\n", surface));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1258)(GLsizei numSurfaces, const GLintptr * surfaces);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1258)(GLsizei numSurfaces, const GLintptr * surfaces)
{
    (void) numSurfaces; (void) surfaces;
   DISPATCH(VDPAUMapSurfacesNV, (numSurfaces, surfaces), (F, "glVDPAUMapSurfacesNV(%d, %p);\n", numSurfaces, (const void *) surfaces));
}

KEYWORD1_ALT GLintptr KEYWORD2 NAME(_dispatch_stub_1259)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames);

KEYWORD1_ALT GLintptr KEYWORD2 NAME(_dispatch_stub_1259)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames)
{
    (void) vdpSurface; (void) target; (void) numTextureNames; (void) textureNames;
   RETURN_DISPATCH(VDPAURegisterOutputSurfaceNV, (vdpSurface, target, numTextureNames, textureNames), (F, "glVDPAURegisterOutputSurfaceNV(%p, 0x%x, %d, %p);\n", (const void *) vdpSurface, target, numTextureNames, (const void *) textureNames));
}

KEYWORD1_ALT GLintptr KEYWORD2 NAME(_dispatch_stub_1260)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames);

KEYWORD1_ALT GLintptr KEYWORD2 NAME(_dispatch_stub_1260)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames)
{
    (void) vdpSurface; (void) target; (void) numTextureNames; (void) textureNames;
   RETURN_DISPATCH(VDPAURegisterVideoSurfaceNV, (vdpSurface, target, numTextureNames, textureNames), (F, "glVDPAURegisterVideoSurfaceNV(%p, 0x%x, %d, %p);\n", (const void *) vdpSurface, target, numTextureNames, (const void *) textureNames));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1261)(GLintptr surface, GLenum access);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1261)(GLintptr surface, GLenum access)
{
    (void) surface; (void) access;
   DISPATCH(VDPAUSurfaceAccessNV, (surface, access), (F, "glVDPAUSurfaceAccessNV(%d, 0x%x);\n", surface, access));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1262)(GLsizei numSurfaces, const GLintptr * surfaces);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1262)(GLsizei numSurfaces, const GLintptr * surfaces)
{
    (void) numSurfaces; (void) surfaces;
   DISPATCH(VDPAUUnmapSurfacesNV, (numSurfaces, surfaces), (F, "glVDPAUUnmapSurfacesNV(%d, %p);\n", numSurfaces, (const void *) surfaces));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1263)(GLintptr surface);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1263)(GLintptr surface)
{
    (void) surface;
   DISPATCH(VDPAUUnregisterSurfaceNV, (surface), (F, "glVDPAUUnregisterSurfaceNV(%d);\n", surface));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1264)(GLuint queryHandle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1264)(GLuint queryHandle)
{
    (void) queryHandle;
   DISPATCH(BeginPerfQueryINTEL, (queryHandle), (F, "glBeginPerfQueryINTEL(%d);\n", queryHandle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1265)(GLuint queryId, GLuint * queryHandle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1265)(GLuint queryId, GLuint * queryHandle)
{
    (void) queryId; (void) queryHandle;
   DISPATCH(CreatePerfQueryINTEL, (queryId, queryHandle), (F, "glCreatePerfQueryINTEL(%d, %p);\n", queryId, (const void *) queryHandle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1266)(GLuint queryHandle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1266)(GLuint queryHandle)
{
    (void) queryHandle;
   DISPATCH(DeletePerfQueryINTEL, (queryHandle), (F, "glDeletePerfQueryINTEL(%d);\n", queryHandle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1267)(GLuint queryHandle);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1267)(GLuint queryHandle)
{
    (void) queryHandle;
   DISPATCH(EndPerfQueryINTEL, (queryHandle), (F, "glEndPerfQueryINTEL(%d);\n", queryHandle));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1268)(GLuint * queryId);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1268)(GLuint * queryId)
{
    (void) queryId;
   DISPATCH(GetFirstPerfQueryIdINTEL, (queryId), (F, "glGetFirstPerfQueryIdINTEL(%p);\n", (const void *) queryId));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1269)(GLuint queryId, GLuint * nextQueryId);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1269)(GLuint queryId, GLuint * nextQueryId)
{
    (void) queryId; (void) nextQueryId;
   DISPATCH(GetNextPerfQueryIdINTEL, (queryId, nextQueryId), (F, "glGetNextPerfQueryIdINTEL(%d, %p);\n", queryId, (const void *) nextQueryId));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1270)(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar * counterName, GLuint counterDescLength, GLchar * counterDesc, GLuint * counterOffset, GLuint * counterDataSize, GLuint * counterTypeEnum, GLuint * counterDataTypeEnum, GLuint64 * rawCounterMaxValue);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1270)(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar * counterName, GLuint counterDescLength, GLchar * counterDesc, GLuint * counterOffset, GLuint * counterDataSize, GLuint * counterTypeEnum, GLuint * counterDataTypeEnum, GLuint64 * rawCounterMaxValue)
{
    (void) queryId; (void) counterId; (void) counterNameLength; (void) counterName; (void) counterDescLength; (void) counterDesc; (void) counterOffset; (void) counterDataSize; (void) counterTypeEnum; (void) counterDataTypeEnum; (void) rawCounterMaxValue;
   DISPATCH(GetPerfCounterInfoINTEL, (queryId, counterId, counterNameLength, counterName, counterDescLength, counterDesc, counterOffset, counterDataSize, counterTypeEnum, counterDataTypeEnum, rawCounterMaxValue), (F, "glGetPerfCounterInfoINTEL(%d, %d, %d, %p, %d, %p, %p, %p, %p, %p, %p);\n", queryId, counterId, counterNameLength, (const void *) counterName, counterDescLength, (const void *) counterDesc, (const void *) counterOffset, (const void *) counterDataSize, (const void *) counterTypeEnum, (const void *) counterDataTypeEnum, (const void *) rawCounterMaxValue));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1271)(GLuint queryHandle, GLuint flags, GLsizei dataSize, GLvoid * data, GLuint * bytesWritten);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1271)(GLuint queryHandle, GLuint flags, GLsizei dataSize, GLvoid * data, GLuint * bytesWritten)
{
    (void) queryHandle; (void) flags; (void) dataSize; (void) data; (void) bytesWritten;
   DISPATCH(GetPerfQueryDataINTEL, (queryHandle, flags, dataSize, data, bytesWritten), (F, "glGetPerfQueryDataINTEL(%d, %d, %d, %p, %p);\n", queryHandle, flags, dataSize, (const void *) data, (const void *) bytesWritten));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1272)(GLchar * queryName, GLuint * queryId);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1272)(GLchar * queryName, GLuint * queryId)
{
    (void) queryName; (void) queryId;
   DISPATCH(GetPerfQueryIdByNameINTEL, (queryName, queryId), (F, "glGetPerfQueryIdByNameINTEL(%p, %p);\n", (const void *) queryName, (const void *) queryId));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1273)(GLuint queryId, GLuint queryNameLength, GLchar * queryName, GLuint * dataSize, GLuint * noCounters, GLuint * noInstances, GLuint * capsMask);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1273)(GLuint queryId, GLuint queryNameLength, GLchar * queryName, GLuint * dataSize, GLuint * noCounters, GLuint * noInstances, GLuint * capsMask)
{
    (void) queryId; (void) queryNameLength; (void) queryName; (void) dataSize; (void) noCounters; (void) noInstances; (void) capsMask;
   DISPATCH(GetPerfQueryInfoINTEL, (queryId, queryNameLength, queryName, dataSize, noCounters, noInstances, capsMask), (F, "glGetPerfQueryInfoINTEL(%d, %d, %p, %p, %p, %p, %p);\n", queryId, queryNameLength, (const void *) queryName, (const void *) dataSize, (const void *) noCounters, (const void *) noInstances, (const void *) capsMask));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1274)(GLfloat factor, GLfloat units, GLfloat clamp);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1274)(GLfloat factor, GLfloat units, GLfloat clamp)
{
    (void) factor; (void) units; (void) clamp;
   DISPATCH(PolygonOffsetClampEXT, (factor, units, clamp), (F, "glPolygonOffsetClampEXT(%f, %f, %f);\n", factor, units, clamp));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1275)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1275)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    (void) frontfunc; (void) backfunc; (void) ref; (void) mask;
   DISPATCH(StencilFuncSeparateATI, (frontfunc, backfunc, ref, mask), (F, "glStencilFuncSeparateATI(0x%x, 0x%x, %d, %d);\n", frontfunc, backfunc, ref, mask));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1276)(GLenum target, GLuint index, GLsizei count, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1276)(GLenum target, GLuint index, GLsizei count, const GLfloat * params)
{
    (void) target; (void) index; (void) count; (void) params;
   DISPATCH(ProgramEnvParameters4fvEXT, (target, index, count, params), (F, "glProgramEnvParameters4fvEXT(0x%x, %d, %d, %p);\n", target, index, count, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1277)(GLenum target, GLuint index, GLsizei count, const GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1277)(GLenum target, GLuint index, GLsizei count, const GLfloat * params)
{
    (void) target; (void) index; (void) count; (void) params;
   DISPATCH(ProgramLocalParameters4fvEXT, (target, index, count, params), (F, "glProgramLocalParameters4fvEXT(0x%x, %d, %d, %p);\n", target, index, count, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1278)(GLenum target, GLvoid * writeOffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1278)(GLenum target, GLvoid * writeOffset)
{
    (void) target; (void) writeOffset;
   DISPATCH(EGLImageTargetRenderbufferStorageOES, (target, writeOffset), (F, "glEGLImageTargetRenderbufferStorageOES(0x%x, %p);\n", target, (const void *) writeOffset));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1279)(GLenum target, GLvoid * writeOffset);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1279)(GLenum target, GLvoid * writeOffset)
{
    (void) target; (void) writeOffset;
   DISPATCH(EGLImageTargetTexture2DOES, (target, writeOffset), (F, "glEGLImageTargetTexture2DOES(0x%x, %p);\n", target, (const void *) writeOffset));
}

KEYWORD1 void KEYWORD2 NAME(AlphaFuncx)(GLenum func, GLclampx ref)
{
    (void) func; (void) ref;
   DISPATCH(AlphaFuncx, (func, ref), (F, "glAlphaFuncx(0x%x, %d);\n", func, ref));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1280)(GLenum func, GLclampx ref);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1280)(GLenum func, GLclampx ref)
{
    (void) func; (void) ref;
   DISPATCH(AlphaFuncx, (func, ref), (F, "glAlphaFuncxOES(0x%x, %d);\n", func, ref));
}

KEYWORD1 void KEYWORD2 NAME(ClearColorx)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(ClearColorx, (red, green, blue, alpha), (F, "glClearColorx(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1281)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1281)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(ClearColorx, (red, green, blue, alpha), (F, "glClearColorxOES(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(ClearDepthx)(GLclampx depth)
{
    (void) depth;
   DISPATCH(ClearDepthx, (depth), (F, "glClearDepthx(%d);\n", depth));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1282)(GLclampx depth);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1282)(GLclampx depth)
{
    (void) depth;
   DISPATCH(ClearDepthx, (depth), (F, "glClearDepthxOES(%d);\n", depth));
}

KEYWORD1 void KEYWORD2 NAME(Color4x)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4x, (red, green, blue, alpha), (F, "glColor4x(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1283)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1283)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    (void) red; (void) green; (void) blue; (void) alpha;
   DISPATCH(Color4x, (red, green, blue, alpha), (F, "glColor4xOES(%d, %d, %d, %d);\n", red, green, blue, alpha));
}

KEYWORD1 void KEYWORD2 NAME(DepthRangex)(GLclampx zNear, GLclampx zFar)
{
    (void) zNear; (void) zFar;
   DISPATCH(DepthRangex, (zNear, zFar), (F, "glDepthRangex(%d, %d);\n", zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1284)(GLclampx zNear, GLclampx zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1284)(GLclampx zNear, GLclampx zFar)
{
    (void) zNear; (void) zFar;
   DISPATCH(DepthRangex, (zNear, zFar), (F, "glDepthRangexOES(%d, %d);\n", zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(Fogx)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(Fogx, (pname, param), (F, "glFogx(0x%x, %d);\n", pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1285)(GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1285)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(Fogx, (pname, param), (F, "glFogxOES(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Fogxv)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(Fogxv, (pname, params), (F, "glFogxv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1286)(GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1286)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(Fogxv, (pname, params), (F, "glFogxvOES(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Frustumf)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Frustumf, (left, right, bottom, top, zNear, zFar), (F, "glFrustumf(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1287)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1287)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Frustumf, (left, right, bottom, top, zNear, zFar), (F, "glFrustumfOES(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(Frustumx)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Frustumx, (left, right, bottom, top, zNear, zFar), (F, "glFrustumx(%d, %d, %d, %d, %d, %d);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1288)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1288)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Frustumx, (left, right, bottom, top, zNear, zFar), (F, "glFrustumxOES(%d, %d, %d, %d, %d, %d);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(LightModelx)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(LightModelx, (pname, param), (F, "glLightModelx(0x%x, %d);\n", pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1289)(GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1289)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(LightModelx, (pname, param), (F, "glLightModelxOES(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(LightModelxv)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(LightModelxv, (pname, params), (F, "glLightModelxv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1290)(GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1290)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(LightModelxv, (pname, params), (F, "glLightModelxvOES(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(Lightx)(GLenum light, GLenum pname, GLfixed param)
{
    (void) light; (void) pname; (void) param;
   DISPATCH(Lightx, (light, pname, param), (F, "glLightx(0x%x, 0x%x, %d);\n", light, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1291)(GLenum light, GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1291)(GLenum light, GLenum pname, GLfixed param)
{
    (void) light; (void) pname; (void) param;
   DISPATCH(Lightx, (light, pname, param), (F, "glLightxOES(0x%x, 0x%x, %d);\n", light, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Lightxv)(GLenum light, GLenum pname, const GLfixed * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(Lightxv, (light, pname, params), (F, "glLightxv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1292)(GLenum light, GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1292)(GLenum light, GLenum pname, const GLfixed * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(Lightxv, (light, pname, params), (F, "glLightxvOES(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(LineWidthx)(GLfixed width)
{
    (void) width;
   DISPATCH(LineWidthx, (width), (F, "glLineWidthx(%d);\n", width));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1293)(GLfixed width);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1293)(GLfixed width)
{
    (void) width;
   DISPATCH(LineWidthx, (width), (F, "glLineWidthxOES(%d);\n", width));
}

KEYWORD1 void KEYWORD2 NAME(LoadMatrixx)(const GLfixed * m)
{
    (void) m;
   DISPATCH(LoadMatrixx, (m), (F, "glLoadMatrixx(%p);\n", (const void *) m));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1294)(const GLfixed * m);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1294)(const GLfixed * m)
{
    (void) m;
   DISPATCH(LoadMatrixx, (m), (F, "glLoadMatrixxOES(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(Materialx)(GLenum face, GLenum pname, GLfixed param)
{
    (void) face; (void) pname; (void) param;
   DISPATCH(Materialx, (face, pname, param), (F, "glMaterialx(0x%x, 0x%x, %d);\n", face, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1295)(GLenum face, GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1295)(GLenum face, GLenum pname, GLfixed param)
{
    (void) face; (void) pname; (void) param;
   DISPATCH(Materialx, (face, pname, param), (F, "glMaterialxOES(0x%x, 0x%x, %d);\n", face, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Materialxv)(GLenum face, GLenum pname, const GLfixed * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(Materialxv, (face, pname, params), (F, "glMaterialxv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1296)(GLenum face, GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1296)(GLenum face, GLenum pname, const GLfixed * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(Materialxv, (face, pname, params), (F, "glMaterialxvOES(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(MultMatrixx)(const GLfixed * m)
{
    (void) m;
   DISPATCH(MultMatrixx, (m), (F, "glMultMatrixx(%p);\n", (const void *) m));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1297)(const GLfixed * m);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1297)(const GLfixed * m)
{
    (void) m;
   DISPATCH(MultMatrixx, (m), (F, "glMultMatrixxOES(%p);\n", (const void *) m));
}

KEYWORD1 void KEYWORD2 NAME(MultiTexCoord4x)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4x, (target, s, t, r, q), (F, "glMultiTexCoord4x(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1298)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1298)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    (void) target; (void) s; (void) t; (void) r; (void) q;
   DISPATCH(MultiTexCoord4x, (target, s, t, r, q), (F, "glMultiTexCoord4xOES(0x%x, %d, %d, %d, %d);\n", target, s, t, r, q));
}

KEYWORD1 void KEYWORD2 NAME(Normal3x)(GLfixed nx, GLfixed ny, GLfixed nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3x, (nx, ny, nz), (F, "glNormal3x(%d, %d, %d);\n", nx, ny, nz));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1299)(GLfixed nx, GLfixed ny, GLfixed nz);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1299)(GLfixed nx, GLfixed ny, GLfixed nz)
{
    (void) nx; (void) ny; (void) nz;
   DISPATCH(Normal3x, (nx, ny, nz), (F, "glNormal3xOES(%d, %d, %d);\n", nx, ny, nz));
}

KEYWORD1 void KEYWORD2 NAME(Orthof)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Orthof, (left, right, bottom, top, zNear, zFar), (F, "glOrthof(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1300)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1300)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Orthof, (left, right, bottom, top, zNear, zFar), (F, "glOrthofOES(%f, %f, %f, %f, %f, %f);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(Orthox)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Orthox, (left, right, bottom, top, zNear, zFar), (F, "glOrthox(%d, %d, %d, %d, %d, %d);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1301)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1301)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    (void) left; (void) right; (void) bottom; (void) top; (void) zNear; (void) zFar;
   DISPATCH(Orthox, (left, right, bottom, top, zNear, zFar), (F, "glOrthoxOES(%d, %d, %d, %d, %d, %d);\n", left, right, bottom, top, zNear, zFar));
}

KEYWORD1 void KEYWORD2 NAME(PointSizex)(GLfixed size)
{
    (void) size;
   DISPATCH(PointSizex, (size), (F, "glPointSizex(%d);\n", size));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1302)(GLfixed size);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1302)(GLfixed size)
{
    (void) size;
   DISPATCH(PointSizex, (size), (F, "glPointSizexOES(%d);\n", size));
}

KEYWORD1 void KEYWORD2 NAME(PolygonOffsetx)(GLfixed factor, GLfixed units)
{
    (void) factor; (void) units;
   DISPATCH(PolygonOffsetx, (factor, units), (F, "glPolygonOffsetx(%d, %d);\n", factor, units));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1303)(GLfixed factor, GLfixed units);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1303)(GLfixed factor, GLfixed units)
{
    (void) factor; (void) units;
   DISPATCH(PolygonOffsetx, (factor, units), (F, "glPolygonOffsetxOES(%d, %d);\n", factor, units));
}

KEYWORD1 void KEYWORD2 NAME(Rotatex)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    (void) angle; (void) x; (void) y; (void) z;
   DISPATCH(Rotatex, (angle, x, y, z), (F, "glRotatex(%d, %d, %d, %d);\n", angle, x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1304)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1304)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    (void) angle; (void) x; (void) y; (void) z;
   DISPATCH(Rotatex, (angle, x, y, z), (F, "glRotatexOES(%d, %d, %d, %d);\n", angle, x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(SampleCoveragex)(GLclampx value, GLboolean invert)
{
    (void) value; (void) invert;
   DISPATCH(SampleCoveragex, (value, invert), (F, "glSampleCoveragex(%d, %d);\n", value, invert));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1305)(GLclampx value, GLboolean invert);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1305)(GLclampx value, GLboolean invert)
{
    (void) value; (void) invert;
   DISPATCH(SampleCoveragex, (value, invert), (F, "glSampleCoveragexOES(%d, %d);\n", value, invert));
}

KEYWORD1 void KEYWORD2 NAME(Scalex)(GLfixed x, GLfixed y, GLfixed z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Scalex, (x, y, z), (F, "glScalex(%d, %d, %d);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1306)(GLfixed x, GLfixed y, GLfixed z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1306)(GLfixed x, GLfixed y, GLfixed z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Scalex, (x, y, z), (F, "glScalexOES(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(TexEnvx)(GLenum target, GLenum pname, GLfixed param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexEnvx, (target, pname, param), (F, "glTexEnvx(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1307)(GLenum target, GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1307)(GLenum target, GLenum pname, GLfixed param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexEnvx, (target, pname, param), (F, "glTexEnvxOES(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(TexEnvxv)(GLenum target, GLenum pname, const GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexEnvxv, (target, pname, params), (F, "glTexEnvxv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1308)(GLenum target, GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1308)(GLenum target, GLenum pname, const GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexEnvxv, (target, pname, params), (F, "glTexEnvxvOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterx)(GLenum target, GLenum pname, GLfixed param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexParameterx, (target, pname, param), (F, "glTexParameterx(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1309)(GLenum target, GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1309)(GLenum target, GLenum pname, GLfixed param)
{
    (void) target; (void) pname; (void) param;
   DISPATCH(TexParameterx, (target, pname, param), (F, "glTexParameterxOES(0x%x, 0x%x, %d);\n", target, pname, param));
}

KEYWORD1 void KEYWORD2 NAME(Translatex)(GLfixed x, GLfixed y, GLfixed z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Translatex, (x, y, z), (F, "glTranslatex(%d, %d, %d);\n", x, y, z));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1310)(GLfixed x, GLfixed y, GLfixed z);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1310)(GLfixed x, GLfixed y, GLfixed z)
{
    (void) x; (void) y; (void) z;
   DISPATCH(Translatex, (x, y, z), (F, "glTranslatexOES(%d, %d, %d);\n", x, y, z));
}

KEYWORD1 void KEYWORD2 NAME(ClipPlanef)(GLenum plane, const GLfloat * equation)
{
    (void) plane; (void) equation;
   DISPATCH(ClipPlanef, (plane, equation), (F, "glClipPlanef(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1311)(GLenum plane, const GLfloat * equation);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1311)(GLenum plane, const GLfloat * equation)
{
    (void) plane; (void) equation;
   DISPATCH(ClipPlanef, (plane, equation), (F, "glClipPlanefOES(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(ClipPlanex)(GLenum plane, const GLfixed * equation)
{
    (void) plane; (void) equation;
   DISPATCH(ClipPlanex, (plane, equation), (F, "glClipPlanex(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1312)(GLenum plane, const GLfixed * equation);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1312)(GLenum plane, const GLfixed * equation)
{
    (void) plane; (void) equation;
   DISPATCH(ClipPlanex, (plane, equation), (F, "glClipPlanexOES(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(GetClipPlanef)(GLenum plane, GLfloat * equation)
{
    (void) plane; (void) equation;
   DISPATCH(GetClipPlanef, (plane, equation), (F, "glGetClipPlanef(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1313)(GLenum plane, GLfloat * equation);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1313)(GLenum plane, GLfloat * equation)
{
    (void) plane; (void) equation;
   DISPATCH(GetClipPlanef, (plane, equation), (F, "glGetClipPlanefOES(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(GetClipPlanex)(GLenum plane, GLfixed * equation)
{
    (void) plane; (void) equation;
   DISPATCH(GetClipPlanex, (plane, equation), (F, "glGetClipPlanex(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1314)(GLenum plane, GLfixed * equation);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1314)(GLenum plane, GLfixed * equation)
{
    (void) plane; (void) equation;
   DISPATCH(GetClipPlanex, (plane, equation), (F, "glGetClipPlanexOES(0x%x, %p);\n", plane, (const void *) equation));
}

KEYWORD1 void KEYWORD2 NAME(GetFixedv)(GLenum pname, GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(GetFixedv, (pname, params), (F, "glGetFixedv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1315)(GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1315)(GLenum pname, GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(GetFixedv, (pname, params), (F, "glGetFixedvOES(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetLightxv)(GLenum light, GLenum pname, GLfixed * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(GetLightxv, (light, pname, params), (F, "glGetLightxv(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1316)(GLenum light, GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1316)(GLenum light, GLenum pname, GLfixed * params)
{
    (void) light; (void) pname; (void) params;
   DISPATCH(GetLightxv, (light, pname, params), (F, "glGetLightxvOES(0x%x, 0x%x, %p);\n", light, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetMaterialxv)(GLenum face, GLenum pname, GLfixed * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(GetMaterialxv, (face, pname, params), (F, "glGetMaterialxv(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1317)(GLenum face, GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1317)(GLenum face, GLenum pname, GLfixed * params)
{
    (void) face; (void) pname; (void) params;
   DISPATCH(GetMaterialxv, (face, pname, params), (F, "glGetMaterialxvOES(0x%x, 0x%x, %p);\n", face, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexEnvxv)(GLenum target, GLenum pname, GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexEnvxv, (target, pname, params), (F, "glGetTexEnvxv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1318)(GLenum target, GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1318)(GLenum target, GLenum pname, GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexEnvxv, (target, pname, params), (F, "glGetTexEnvxvOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(GetTexParameterxv)(GLenum target, GLenum pname, GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterxv, (target, pname, params), (F, "glGetTexParameterxv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1319)(GLenum target, GLenum pname, GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1319)(GLenum target, GLenum pname, GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetTexParameterxv, (target, pname, params), (F, "glGetTexParameterxvOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterx)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterx, (pname, param), (F, "glPointParameterx(0x%x, %d);\n", pname, param));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1320)(GLenum pname, GLfixed param);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1320)(GLenum pname, GLfixed param)
{
    (void) pname; (void) param;
   DISPATCH(PointParameterx, (pname, param), (F, "glPointParameterxOES(0x%x, %d);\n", pname, param));
}

KEYWORD1 void KEYWORD2 NAME(PointParameterxv)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterxv, (pname, params), (F, "glPointParameterxv(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1321)(GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1321)(GLenum pname, const GLfixed * params)
{
    (void) pname; (void) params;
   DISPATCH(PointParameterxv, (pname, params), (F, "glPointParameterxvOES(0x%x, %p);\n", pname, (const void *) params));
}

KEYWORD1 void KEYWORD2 NAME(TexParameterxv)(GLenum target, GLenum pname, const GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterxv, (target, pname, params), (F, "glTexParameterxv(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1322)(GLenum target, GLenum pname, const GLfixed * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_1322)(GLenum target, GLenum pname, const GLfixed * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(TexParameterxv, (target, pname, params), (F, "glTexParameterxvOES(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}


#endif /* _GLAPI_SKIP_NORMAL_ENTRY_POINTS */

/* these entry points might require different protocols */
#ifndef _GLAPI_SKIP_PROTO_ENTRY_POINTS

KEYWORD1 GLboolean KEYWORD2 NAME(AreTexturesResidentEXT)(GLsizei n, const GLuint * textures, GLboolean * residences)
{
    (void) n; (void) textures; (void) residences;
   RETURN_DISPATCH(AreTexturesResident, (n, textures, residences), (F, "glAreTexturesResidentEXT(%d, %p, %p);\n", n, (const void *) textures, (const void *) residences));
}

KEYWORD1 void KEYWORD2 NAME(DeleteTexturesEXT)(GLsizei n, const GLuint * textures)
{
    (void) n; (void) textures;
   DISPATCH(DeleteTextures, (n, textures), (F, "glDeleteTexturesEXT(%d, %p);\n", n, (const void *) textures));
}

KEYWORD1 void KEYWORD2 NAME(GenTexturesEXT)(GLsizei n, GLuint * textures)
{
    (void) n; (void) textures;
   DISPATCH(GenTextures, (n, textures), (F, "glGenTexturesEXT(%d, %p);\n", n, (const void *) textures));
}

KEYWORD1 GLboolean KEYWORD2 NAME(IsTextureEXT)(GLuint texture)
{
    (void) texture;
   RETURN_DISPATCH(IsTexture, (texture), (F, "glIsTextureEXT(%d);\n", texture));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_343)(GLenum target, GLenum format, GLenum type, GLvoid * table);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_343)(GLenum target, GLenum format, GLenum type, GLvoid * table)
{
    (void) target; (void) format; (void) type; (void) table;
   DISPATCH(GetColorTable, (target, format, type, table), (F, "glGetColorTableSGI(0x%x, 0x%x, 0x%x, %p);\n", target, format, type, (const void *) table));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_344)(GLenum target, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_344)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetColorTableParameterfv, (target, pname, params), (F, "glGetColorTableParameterfvSGI(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_345)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_345)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetColorTableParameteriv, (target, pname, params), (F, "glGetColorTableParameterivSGI(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_356)(GLenum target, GLenum format, GLenum type, GLvoid * image);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_356)(GLenum target, GLenum format, GLenum type, GLvoid * image)
{
    (void) target; (void) format; (void) type; (void) image;
   DISPATCH(GetConvolutionFilter, (target, format, type, image), (F, "glGetConvolutionFilterEXT(0x%x, 0x%x, 0x%x, %p);\n", target, format, type, (const void *) image));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_357)(GLenum target, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_357)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetConvolutionParameterfv, (target, pname, params), (F, "glGetConvolutionParameterfvEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_358)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_358)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetConvolutionParameteriv, (target, pname, params), (F, "glGetConvolutionParameterivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_359)(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_359)(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span)
{
    (void) target; (void) format; (void) type; (void) row; (void) column; (void) span;
   DISPATCH(GetSeparableFilter, (target, format, type, row, column, span), (F, "glGetSeparableFilterEXT(0x%x, 0x%x, 0x%x, %p, %p, %p);\n", target, format, type, (const void *) row, (const void *) column, (const void *) span));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_361)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_361)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) values;
   DISPATCH(GetHistogram, (target, reset, format, type, values), (F, "glGetHistogramEXT(0x%x, %d, 0x%x, 0x%x, %p);\n", target, reset, format, type, (const void *) values));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_362)(GLenum target, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_362)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetHistogramParameterfv, (target, pname, params), (F, "glGetHistogramParameterfvEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_363)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_363)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetHistogramParameteriv, (target, pname, params), (F, "glGetHistogramParameterivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_364)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_364)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values)
{
    (void) target; (void) reset; (void) format; (void) type; (void) values;
   DISPATCH(GetMinmax, (target, reset, format, type, values), (F, "glGetMinmaxEXT(0x%x, %d, 0x%x, 0x%x, %p);\n", target, reset, format, type, (const void *) values));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_365)(GLenum target, GLenum pname, GLfloat * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_365)(GLenum target, GLenum pname, GLfloat * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetMinmaxParameterfv, (target, pname, params), (F, "glGetMinmaxParameterfvEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_366)(GLenum target, GLenum pname, GLint * params);

KEYWORD1_ALT void KEYWORD2 NAME(_dispatch_stub_366)(GLenum target, GLenum pname, GLint * params)
{
    (void) target; (void) pname; (void) params;
   DISPATCH(GetMinmaxParameteriv, (target, pname, params), (F, "glGetMinmaxParameterivEXT(0x%x, 0x%x, %p);\n", target, pname, (const void *) params));
}


#endif /* _GLAPI_SKIP_PROTO_ENTRY_POINTS */


#endif /* defined( NAME ) */

/*
 * This is how a dispatch table can be initialized with all the functions
 * we generated above.
 */
#ifdef DISPATCH_TABLE_NAME

#ifndef TABLE_ENTRY
#error TABLE_ENTRY must be defined
#endif

#ifdef _GLAPI_SKIP_NORMAL_ENTRY_POINTS
#error _GLAPI_SKIP_NORMAL_ENTRY_POINTS must not be defined
#endif

_glapi_proc DISPATCH_TABLE_NAME[] = {
   TABLE_ENTRY(NewList),
   TABLE_ENTRY(EndList),
   TABLE_ENTRY(CallList),
   TABLE_ENTRY(CallLists),
   TABLE_ENTRY(DeleteLists),
   TABLE_ENTRY(GenLists),
   TABLE_ENTRY(ListBase),
   TABLE_ENTRY(Begin),
   TABLE_ENTRY(Bitmap),
   TABLE_ENTRY(Color3b),
   TABLE_ENTRY(Color3bv),
   TABLE_ENTRY(Color3d),
   TABLE_ENTRY(Color3dv),
   TABLE_ENTRY(Color3f),
   TABLE_ENTRY(Color3fv),
   TABLE_ENTRY(Color3i),
   TABLE_ENTRY(Color3iv),
   TABLE_ENTRY(Color3s),
   TABLE_ENTRY(Color3sv),
   TABLE_ENTRY(Color3ub),
   TABLE_ENTRY(Color3ubv),
   TABLE_ENTRY(Color3ui),
   TABLE_ENTRY(Color3uiv),
   TABLE_ENTRY(Color3us),
   TABLE_ENTRY(Color3usv),
   TABLE_ENTRY(Color4b),
   TABLE_ENTRY(Color4bv),
   TABLE_ENTRY(Color4d),
   TABLE_ENTRY(Color4dv),
   TABLE_ENTRY(Color4f),
   TABLE_ENTRY(Color4fv),
   TABLE_ENTRY(Color4i),
   TABLE_ENTRY(Color4iv),
   TABLE_ENTRY(Color4s),
   TABLE_ENTRY(Color4sv),
   TABLE_ENTRY(Color4ub),
   TABLE_ENTRY(Color4ubv),
   TABLE_ENTRY(Color4ui),
   TABLE_ENTRY(Color4uiv),
   TABLE_ENTRY(Color4us),
   TABLE_ENTRY(Color4usv),
   TABLE_ENTRY(EdgeFlag),
   TABLE_ENTRY(EdgeFlagv),
   TABLE_ENTRY(End),
   TABLE_ENTRY(Indexd),
   TABLE_ENTRY(Indexdv),
   TABLE_ENTRY(Indexf),
   TABLE_ENTRY(Indexfv),
   TABLE_ENTRY(Indexi),
   TABLE_ENTRY(Indexiv),
   TABLE_ENTRY(Indexs),
   TABLE_ENTRY(Indexsv),
   TABLE_ENTRY(Normal3b),
   TABLE_ENTRY(Normal3bv),
   TABLE_ENTRY(Normal3d),
   TABLE_ENTRY(Normal3dv),
   TABLE_ENTRY(Normal3f),
   TABLE_ENTRY(Normal3fv),
   TABLE_ENTRY(Normal3i),
   TABLE_ENTRY(Normal3iv),
   TABLE_ENTRY(Normal3s),
   TABLE_ENTRY(Normal3sv),
   TABLE_ENTRY(RasterPos2d),
   TABLE_ENTRY(RasterPos2dv),
   TABLE_ENTRY(RasterPos2f),
   TABLE_ENTRY(RasterPos2fv),
   TABLE_ENTRY(RasterPos2i),
   TABLE_ENTRY(RasterPos2iv),
   TABLE_ENTRY(RasterPos2s),
   TABLE_ENTRY(RasterPos2sv),
   TABLE_ENTRY(RasterPos3d),
   TABLE_ENTRY(RasterPos3dv),
   TABLE_ENTRY(RasterPos3f),
   TABLE_ENTRY(RasterPos3fv),
   TABLE_ENTRY(RasterPos3i),
   TABLE_ENTRY(RasterPos3iv),
   TABLE_ENTRY(RasterPos3s),
   TABLE_ENTRY(RasterPos3sv),
   TABLE_ENTRY(RasterPos4d),
   TABLE_ENTRY(RasterPos4dv),
   TABLE_ENTRY(RasterPos4f),
   TABLE_ENTRY(RasterPos4fv),
   TABLE_ENTRY(RasterPos4i),
   TABLE_ENTRY(RasterPos4iv),
   TABLE_ENTRY(RasterPos4s),
   TABLE_ENTRY(RasterPos4sv),
   TABLE_ENTRY(Rectd),
   TABLE_ENTRY(Rectdv),
   TABLE_ENTRY(Rectf),
   TABLE_ENTRY(Rectfv),
   TABLE_ENTRY(Recti),
   TABLE_ENTRY(Rectiv),
   TABLE_ENTRY(Rects),
   TABLE_ENTRY(Rectsv),
   TABLE_ENTRY(TexCoord1d),
   TABLE_ENTRY(TexCoord1dv),
   TABLE_ENTRY(TexCoord1f),
   TABLE_ENTRY(TexCoord1fv),
   TABLE_ENTRY(TexCoord1i),
   TABLE_ENTRY(TexCoord1iv),
   TABLE_ENTRY(TexCoord1s),
   TABLE_ENTRY(TexCoord1sv),
   TABLE_ENTRY(TexCoord2d),
   TABLE_ENTRY(TexCoord2dv),
   TABLE_ENTRY(TexCoord2f),
   TABLE_ENTRY(TexCoord2fv),
   TABLE_ENTRY(TexCoord2i),
   TABLE_ENTRY(TexCoord2iv),
   TABLE_ENTRY(TexCoord2s),
   TABLE_ENTRY(TexCoord2sv),
   TABLE_ENTRY(TexCoord3d),
   TABLE_ENTRY(TexCoord3dv),
   TABLE_ENTRY(TexCoord3f),
   TABLE_ENTRY(TexCoord3fv),
   TABLE_ENTRY(TexCoord3i),
   TABLE_ENTRY(TexCoord3iv),
   TABLE_ENTRY(TexCoord3s),
   TABLE_ENTRY(TexCoord3sv),
   TABLE_ENTRY(TexCoord4d),
   TABLE_ENTRY(TexCoord4dv),
   TABLE_ENTRY(TexCoord4f),
   TABLE_ENTRY(TexCoord4fv),
   TABLE_ENTRY(TexCoord4i),
   TABLE_ENTRY(TexCoord4iv),
   TABLE_ENTRY(TexCoord4s),
   TABLE_ENTRY(TexCoord4sv),
   TABLE_ENTRY(Vertex2d),
   TABLE_ENTRY(Vertex2dv),
   TABLE_ENTRY(Vertex2f),
   TABLE_ENTRY(Vertex2fv),
   TABLE_ENTRY(Vertex2i),
   TABLE_ENTRY(Vertex2iv),
   TABLE_ENTRY(Vertex2s),
   TABLE_ENTRY(Vertex2sv),
   TABLE_ENTRY(Vertex3d),
   TABLE_ENTRY(Vertex3dv),
   TABLE_ENTRY(Vertex3f),
   TABLE_ENTRY(Vertex3fv),
   TABLE_ENTRY(Vertex3i),
   TABLE_ENTRY(Vertex3iv),
   TABLE_ENTRY(Vertex3s),
   TABLE_ENTRY(Vertex3sv),
   TABLE_ENTRY(Vertex4d),
   TABLE_ENTRY(Vertex4dv),
   TABLE_ENTRY(Vertex4f),
   TABLE_ENTRY(Vertex4fv),
   TABLE_ENTRY(Vertex4i),
   TABLE_ENTRY(Vertex4iv),
   TABLE_ENTRY(Vertex4s),
   TABLE_ENTRY(Vertex4sv),
   TABLE_ENTRY(ClipPlane),
   TABLE_ENTRY(ColorMaterial),
   TABLE_ENTRY(CullFace),
   TABLE_ENTRY(Fogf),
   TABLE_ENTRY(Fogfv),
   TABLE_ENTRY(Fogi),
   TABLE_ENTRY(Fogiv),
   TABLE_ENTRY(FrontFace),
   TABLE_ENTRY(Hint),
   TABLE_ENTRY(Lightf),
   TABLE_ENTRY(Lightfv),
   TABLE_ENTRY(Lighti),
   TABLE_ENTRY(Lightiv),
   TABLE_ENTRY(LightModelf),
   TABLE_ENTRY(LightModelfv),
   TABLE_ENTRY(LightModeli),
   TABLE_ENTRY(LightModeliv),
   TABLE_ENTRY(LineStipple),
   TABLE_ENTRY(LineWidth),
   TABLE_ENTRY(Materialf),
   TABLE_ENTRY(Materialfv),
   TABLE_ENTRY(Materiali),
   TABLE_ENTRY(Materialiv),
   TABLE_ENTRY(PointSize),
   TABLE_ENTRY(PolygonMode),
   TABLE_ENTRY(PolygonStipple),
   TABLE_ENTRY(Scissor),
   TABLE_ENTRY(ShadeModel),
   TABLE_ENTRY(TexParameterf),
   TABLE_ENTRY(TexParameterfv),
   TABLE_ENTRY(TexParameteri),
   TABLE_ENTRY(TexParameteriv),
   TABLE_ENTRY(TexImage1D),
   TABLE_ENTRY(TexImage2D),
   TABLE_ENTRY(TexEnvf),
   TABLE_ENTRY(TexEnvfv),
   TABLE_ENTRY(TexEnvi),
   TABLE_ENTRY(TexEnviv),
   TABLE_ENTRY(TexGend),
   TABLE_ENTRY(TexGendv),
   TABLE_ENTRY(TexGenf),
   TABLE_ENTRY(TexGenfv),
   TABLE_ENTRY(TexGeni),
   TABLE_ENTRY(TexGeniv),
   TABLE_ENTRY(FeedbackBuffer),
   TABLE_ENTRY(SelectBuffer),
   TABLE_ENTRY(RenderMode),
   TABLE_ENTRY(InitNames),
   TABLE_ENTRY(LoadName),
   TABLE_ENTRY(PassThrough),
   TABLE_ENTRY(PopName),
   TABLE_ENTRY(PushName),
   TABLE_ENTRY(DrawBuffer),
   TABLE_ENTRY(Clear),
   TABLE_ENTRY(ClearAccum),
   TABLE_ENTRY(ClearIndex),
   TABLE_ENTRY(ClearColor),
   TABLE_ENTRY(ClearStencil),
   TABLE_ENTRY(ClearDepth),
   TABLE_ENTRY(StencilMask),
   TABLE_ENTRY(ColorMask),
   TABLE_ENTRY(DepthMask),
   TABLE_ENTRY(IndexMask),
   TABLE_ENTRY(Accum),
   TABLE_ENTRY(Disable),
   TABLE_ENTRY(Enable),
   TABLE_ENTRY(Finish),
   TABLE_ENTRY(Flush),
   TABLE_ENTRY(PopAttrib),
   TABLE_ENTRY(PushAttrib),
   TABLE_ENTRY(Map1d),
   TABLE_ENTRY(Map1f),
   TABLE_ENTRY(Map2d),
   TABLE_ENTRY(Map2f),
   TABLE_ENTRY(MapGrid1d),
   TABLE_ENTRY(MapGrid1f),
   TABLE_ENTRY(MapGrid2d),
   TABLE_ENTRY(MapGrid2f),
   TABLE_ENTRY(EvalCoord1d),
   TABLE_ENTRY(EvalCoord1dv),
   TABLE_ENTRY(EvalCoord1f),
   TABLE_ENTRY(EvalCoord1fv),
   TABLE_ENTRY(EvalCoord2d),
   TABLE_ENTRY(EvalCoord2dv),
   TABLE_ENTRY(EvalCoord2f),
   TABLE_ENTRY(EvalCoord2fv),
   TABLE_ENTRY(EvalMesh1),
   TABLE_ENTRY(EvalPoint1),
   TABLE_ENTRY(EvalMesh2),
   TABLE_ENTRY(EvalPoint2),
   TABLE_ENTRY(AlphaFunc),
   TABLE_ENTRY(BlendFunc),
   TABLE_ENTRY(LogicOp),
   TABLE_ENTRY(StencilFunc),
   TABLE_ENTRY(StencilOp),
   TABLE_ENTRY(DepthFunc),
   TABLE_ENTRY(PixelZoom),
   TABLE_ENTRY(PixelTransferf),
   TABLE_ENTRY(PixelTransferi),
   TABLE_ENTRY(PixelStoref),
   TABLE_ENTRY(PixelStorei),
   TABLE_ENTRY(PixelMapfv),
   TABLE_ENTRY(PixelMapuiv),
   TABLE_ENTRY(PixelMapusv),
   TABLE_ENTRY(ReadBuffer),
   TABLE_ENTRY(CopyPixels),
   TABLE_ENTRY(ReadPixels),
   TABLE_ENTRY(DrawPixels),
   TABLE_ENTRY(GetBooleanv),
   TABLE_ENTRY(GetClipPlane),
   TABLE_ENTRY(GetDoublev),
   TABLE_ENTRY(GetError),
   TABLE_ENTRY(GetFloatv),
   TABLE_ENTRY(GetIntegerv),
   TABLE_ENTRY(GetLightfv),
   TABLE_ENTRY(GetLightiv),
   TABLE_ENTRY(GetMapdv),
   TABLE_ENTRY(GetMapfv),
   TABLE_ENTRY(GetMapiv),
   TABLE_ENTRY(GetMaterialfv),
   TABLE_ENTRY(GetMaterialiv),
   TABLE_ENTRY(GetPixelMapfv),
   TABLE_ENTRY(GetPixelMapuiv),
   TABLE_ENTRY(GetPixelMapusv),
   TABLE_ENTRY(GetPolygonStipple),
   TABLE_ENTRY(GetString),
   TABLE_ENTRY(GetTexEnvfv),
   TABLE_ENTRY(GetTexEnviv),
   TABLE_ENTRY(GetTexGendv),
   TABLE_ENTRY(GetTexGenfv),
   TABLE_ENTRY(GetTexGeniv),
   TABLE_ENTRY(GetTexImage),
   TABLE_ENTRY(GetTexParameterfv),
   TABLE_ENTRY(GetTexParameteriv),
   TABLE_ENTRY(GetTexLevelParameterfv),
   TABLE_ENTRY(GetTexLevelParameteriv),
   TABLE_ENTRY(IsEnabled),
   TABLE_ENTRY(IsList),
   TABLE_ENTRY(DepthRange),
   TABLE_ENTRY(Frustum),
   TABLE_ENTRY(LoadIdentity),
   TABLE_ENTRY(LoadMatrixf),
   TABLE_ENTRY(LoadMatrixd),
   TABLE_ENTRY(MatrixMode),
   TABLE_ENTRY(MultMatrixf),
   TABLE_ENTRY(MultMatrixd),
   TABLE_ENTRY(Ortho),
   TABLE_ENTRY(PopMatrix),
   TABLE_ENTRY(PushMatrix),
   TABLE_ENTRY(Rotated),
   TABLE_ENTRY(Rotatef),
   TABLE_ENTRY(Scaled),
   TABLE_ENTRY(Scalef),
   TABLE_ENTRY(Translated),
   TABLE_ENTRY(Translatef),
   TABLE_ENTRY(Viewport),
   TABLE_ENTRY(ArrayElement),
   TABLE_ENTRY(BindTexture),
   TABLE_ENTRY(ColorPointer),
   TABLE_ENTRY(DisableClientState),
   TABLE_ENTRY(DrawArrays),
   TABLE_ENTRY(DrawElements),
   TABLE_ENTRY(EdgeFlagPointer),
   TABLE_ENTRY(EnableClientState),
   TABLE_ENTRY(IndexPointer),
   TABLE_ENTRY(Indexub),
   TABLE_ENTRY(Indexubv),
   TABLE_ENTRY(InterleavedArrays),
   TABLE_ENTRY(NormalPointer),
   TABLE_ENTRY(PolygonOffset),
   TABLE_ENTRY(TexCoordPointer),
   TABLE_ENTRY(VertexPointer),
   TABLE_ENTRY(AreTexturesResident),
   TABLE_ENTRY(CopyTexImage1D),
   TABLE_ENTRY(CopyTexImage2D),
   TABLE_ENTRY(CopyTexSubImage1D),
   TABLE_ENTRY(CopyTexSubImage2D),
   TABLE_ENTRY(DeleteTextures),
   TABLE_ENTRY(GenTextures),
   TABLE_ENTRY(GetPointerv),
   TABLE_ENTRY(IsTexture),
   TABLE_ENTRY(PrioritizeTextures),
   TABLE_ENTRY(TexSubImage1D),
   TABLE_ENTRY(TexSubImage2D),
   TABLE_ENTRY(PopClientAttrib),
   TABLE_ENTRY(PushClientAttrib),
   TABLE_ENTRY(BlendColor),
   TABLE_ENTRY(BlendEquation),
   TABLE_ENTRY(DrawRangeElements),
   TABLE_ENTRY(ColorTable),
   TABLE_ENTRY(ColorTableParameterfv),
   TABLE_ENTRY(ColorTableParameteriv),
   TABLE_ENTRY(CopyColorTable),
   TABLE_ENTRY(GetColorTable),
   TABLE_ENTRY(GetColorTableParameterfv),
   TABLE_ENTRY(GetColorTableParameteriv),
   TABLE_ENTRY(ColorSubTable),
   TABLE_ENTRY(CopyColorSubTable),
   TABLE_ENTRY(ConvolutionFilter1D),
   TABLE_ENTRY(ConvolutionFilter2D),
   TABLE_ENTRY(ConvolutionParameterf),
   TABLE_ENTRY(ConvolutionParameterfv),
   TABLE_ENTRY(ConvolutionParameteri),
   TABLE_ENTRY(ConvolutionParameteriv),
   TABLE_ENTRY(CopyConvolutionFilter1D),
   TABLE_ENTRY(CopyConvolutionFilter2D),
   TABLE_ENTRY(GetConvolutionFilter),
   TABLE_ENTRY(GetConvolutionParameterfv),
   TABLE_ENTRY(GetConvolutionParameteriv),
   TABLE_ENTRY(GetSeparableFilter),
   TABLE_ENTRY(SeparableFilter2D),
   TABLE_ENTRY(GetHistogram),
   TABLE_ENTRY(GetHistogramParameterfv),
   TABLE_ENTRY(GetHistogramParameteriv),
   TABLE_ENTRY(GetMinmax),
   TABLE_ENTRY(GetMinmaxParameterfv),
   TABLE_ENTRY(GetMinmaxParameteriv),
   TABLE_ENTRY(Histogram),
   TABLE_ENTRY(Minmax),
   TABLE_ENTRY(ResetHistogram),
   TABLE_ENTRY(ResetMinmax),
   TABLE_ENTRY(TexImage3D),
   TABLE_ENTRY(TexSubImage3D),
   TABLE_ENTRY(CopyTexSubImage3D),
   TABLE_ENTRY(ActiveTexture),
   TABLE_ENTRY(ClientActiveTexture),
   TABLE_ENTRY(MultiTexCoord1d),
   TABLE_ENTRY(MultiTexCoord1dv),
   TABLE_ENTRY(MultiTexCoord1fARB),
   TABLE_ENTRY(MultiTexCoord1fvARB),
   TABLE_ENTRY(MultiTexCoord1i),
   TABLE_ENTRY(MultiTexCoord1iv),
   TABLE_ENTRY(MultiTexCoord1s),
   TABLE_ENTRY(MultiTexCoord1sv),
   TABLE_ENTRY(MultiTexCoord2d),
   TABLE_ENTRY(MultiTexCoord2dv),
   TABLE_ENTRY(MultiTexCoord2fARB),
   TABLE_ENTRY(MultiTexCoord2fvARB),
   TABLE_ENTRY(MultiTexCoord2i),
   TABLE_ENTRY(MultiTexCoord2iv),
   TABLE_ENTRY(MultiTexCoord2s),
   TABLE_ENTRY(MultiTexCoord2sv),
   TABLE_ENTRY(MultiTexCoord3d),
   TABLE_ENTRY(MultiTexCoord3dv),
   TABLE_ENTRY(MultiTexCoord3fARB),
   TABLE_ENTRY(MultiTexCoord3fvARB),
   TABLE_ENTRY(MultiTexCoord3i),
   TABLE_ENTRY(MultiTexCoord3iv),
   TABLE_ENTRY(MultiTexCoord3s),
   TABLE_ENTRY(MultiTexCoord3sv),
   TABLE_ENTRY(MultiTexCoord4d),
   TABLE_ENTRY(MultiTexCoord4dv),
   TABLE_ENTRY(MultiTexCoord4fARB),
   TABLE_ENTRY(MultiTexCoord4fvARB),
   TABLE_ENTRY(MultiTexCoord4i),
   TABLE_ENTRY(MultiTexCoord4iv),
   TABLE_ENTRY(MultiTexCoord4s),
   TABLE_ENTRY(MultiTexCoord4sv),
   TABLE_ENTRY(CompressedTexImage1D),
   TABLE_ENTRY(CompressedTexImage2D),
   TABLE_ENTRY(CompressedTexImage3D),
   TABLE_ENTRY(CompressedTexSubImage1D),
   TABLE_ENTRY(CompressedTexSubImage2D),
   TABLE_ENTRY(CompressedTexSubImage3D),
   TABLE_ENTRY(GetCompressedTexImage),
   TABLE_ENTRY(LoadTransposeMatrixd),
   TABLE_ENTRY(LoadTransposeMatrixf),
   TABLE_ENTRY(MultTransposeMatrixd),
   TABLE_ENTRY(MultTransposeMatrixf),
   TABLE_ENTRY(SampleCoverage),
   TABLE_ENTRY(BlendFuncSeparate),
   TABLE_ENTRY(FogCoordPointer),
   TABLE_ENTRY(FogCoordd),
   TABLE_ENTRY(FogCoorddv),
   TABLE_ENTRY(MultiDrawArrays),
   TABLE_ENTRY(PointParameterf),
   TABLE_ENTRY(PointParameterfv),
   TABLE_ENTRY(PointParameteri),
   TABLE_ENTRY(PointParameteriv),
   TABLE_ENTRY(SecondaryColor3b),
   TABLE_ENTRY(SecondaryColor3bv),
   TABLE_ENTRY(SecondaryColor3d),
   TABLE_ENTRY(SecondaryColor3dv),
   TABLE_ENTRY(SecondaryColor3i),
   TABLE_ENTRY(SecondaryColor3iv),
   TABLE_ENTRY(SecondaryColor3s),
   TABLE_ENTRY(SecondaryColor3sv),
   TABLE_ENTRY(SecondaryColor3ub),
   TABLE_ENTRY(SecondaryColor3ubv),
   TABLE_ENTRY(SecondaryColor3ui),
   TABLE_ENTRY(SecondaryColor3uiv),
   TABLE_ENTRY(SecondaryColor3us),
   TABLE_ENTRY(SecondaryColor3usv),
   TABLE_ENTRY(SecondaryColorPointer),
   TABLE_ENTRY(WindowPos2d),
   TABLE_ENTRY(WindowPos2dv),
   TABLE_ENTRY(WindowPos2f),
   TABLE_ENTRY(WindowPos2fv),
   TABLE_ENTRY(WindowPos2i),
   TABLE_ENTRY(WindowPos2iv),
   TABLE_ENTRY(WindowPos2s),
   TABLE_ENTRY(WindowPos2sv),
   TABLE_ENTRY(WindowPos3d),
   TABLE_ENTRY(WindowPos3dv),
   TABLE_ENTRY(WindowPos3f),
   TABLE_ENTRY(WindowPos3fv),
   TABLE_ENTRY(WindowPos3i),
   TABLE_ENTRY(WindowPos3iv),
   TABLE_ENTRY(WindowPos3s),
   TABLE_ENTRY(WindowPos3sv),
   TABLE_ENTRY(BeginQuery),
   TABLE_ENTRY(BindBuffer),
   TABLE_ENTRY(BufferData),
   TABLE_ENTRY(BufferSubData),
   TABLE_ENTRY(DeleteBuffers),
   TABLE_ENTRY(DeleteQueries),
   TABLE_ENTRY(EndQuery),
   TABLE_ENTRY(GenBuffers),
   TABLE_ENTRY(GenQueries),
   TABLE_ENTRY(GetBufferParameteriv),
   TABLE_ENTRY(GetBufferPointerv),
   TABLE_ENTRY(GetBufferSubData),
   TABLE_ENTRY(GetQueryObjectiv),
   TABLE_ENTRY(GetQueryObjectuiv),
   TABLE_ENTRY(GetQueryiv),
   TABLE_ENTRY(IsBuffer),
   TABLE_ENTRY(IsQuery),
   TABLE_ENTRY(MapBuffer),
   TABLE_ENTRY(UnmapBuffer),
   TABLE_ENTRY(AttachShader),
   TABLE_ENTRY(BindAttribLocation),
   TABLE_ENTRY(BlendEquationSeparate),
   TABLE_ENTRY(CompileShader),
   TABLE_ENTRY(CreateProgram),
   TABLE_ENTRY(CreateShader),
   TABLE_ENTRY(DeleteProgram),
   TABLE_ENTRY(DeleteShader),
   TABLE_ENTRY(DetachShader),
   TABLE_ENTRY(DisableVertexAttribArray),
   TABLE_ENTRY(DrawBuffers),
   TABLE_ENTRY(EnableVertexAttribArray),
   TABLE_ENTRY(GetActiveAttrib),
   TABLE_ENTRY(GetActiveUniform),
   TABLE_ENTRY(GetAttachedShaders),
   TABLE_ENTRY(GetAttribLocation),
   TABLE_ENTRY(GetProgramInfoLog),
   TABLE_ENTRY(GetProgramiv),
   TABLE_ENTRY(GetShaderInfoLog),
   TABLE_ENTRY(GetShaderSource),
   TABLE_ENTRY(GetShaderiv),
   TABLE_ENTRY(GetUniformLocation),
   TABLE_ENTRY(GetUniformfv),
   TABLE_ENTRY(GetUniformiv),
   TABLE_ENTRY(GetVertexAttribPointerv),
   TABLE_ENTRY(GetVertexAttribdv),
   TABLE_ENTRY(GetVertexAttribfv),
   TABLE_ENTRY(GetVertexAttribiv),
   TABLE_ENTRY(IsProgram),
   TABLE_ENTRY(IsShader),
   TABLE_ENTRY(LinkProgram),
   TABLE_ENTRY(ShaderSource),
   TABLE_ENTRY(StencilFuncSeparate),
   TABLE_ENTRY(StencilMaskSeparate),
   TABLE_ENTRY(StencilOpSeparate),
   TABLE_ENTRY(Uniform1f),
   TABLE_ENTRY(Uniform1fv),
   TABLE_ENTRY(Uniform1i),
   TABLE_ENTRY(Uniform1iv),
   TABLE_ENTRY(Uniform2f),
   TABLE_ENTRY(Uniform2fv),
   TABLE_ENTRY(Uniform2i),
   TABLE_ENTRY(Uniform2iv),
   TABLE_ENTRY(Uniform3f),
   TABLE_ENTRY(Uniform3fv),
   TABLE_ENTRY(Uniform3i),
   TABLE_ENTRY(Uniform3iv),
   TABLE_ENTRY(Uniform4f),
   TABLE_ENTRY(Uniform4fv),
   TABLE_ENTRY(Uniform4i),
   TABLE_ENTRY(Uniform4iv),
   TABLE_ENTRY(UniformMatrix2fv),
   TABLE_ENTRY(UniformMatrix3fv),
   TABLE_ENTRY(UniformMatrix4fv),
   TABLE_ENTRY(UseProgram),
   TABLE_ENTRY(ValidateProgram),
   TABLE_ENTRY(VertexAttrib1d),
   TABLE_ENTRY(VertexAttrib1dv),
   TABLE_ENTRY(VertexAttrib1s),
   TABLE_ENTRY(VertexAttrib1sv),
   TABLE_ENTRY(VertexAttrib2d),
   TABLE_ENTRY(VertexAttrib2dv),
   TABLE_ENTRY(VertexAttrib2s),
   TABLE_ENTRY(VertexAttrib2sv),
   TABLE_ENTRY(VertexAttrib3d),
   TABLE_ENTRY(VertexAttrib3dv),
   TABLE_ENTRY(VertexAttrib3s),
   TABLE_ENTRY(VertexAttrib3sv),
   TABLE_ENTRY(VertexAttrib4Nbv),
   TABLE_ENTRY(VertexAttrib4Niv),
   TABLE_ENTRY(VertexAttrib4Nsv),
   TABLE_ENTRY(VertexAttrib4Nub),
   TABLE_ENTRY(VertexAttrib4Nubv),
   TABLE_ENTRY(VertexAttrib4Nuiv),
   TABLE_ENTRY(VertexAttrib4Nusv),
   TABLE_ENTRY(VertexAttrib4bv),
   TABLE_ENTRY(VertexAttrib4d),
   TABLE_ENTRY(VertexAttrib4dv),
   TABLE_ENTRY(VertexAttrib4iv),
   TABLE_ENTRY(VertexAttrib4s),
   TABLE_ENTRY(VertexAttrib4sv),
   TABLE_ENTRY(VertexAttrib4ubv),
   TABLE_ENTRY(VertexAttrib4uiv),
   TABLE_ENTRY(VertexAttrib4usv),
   TABLE_ENTRY(VertexAttribPointer),
   TABLE_ENTRY(UniformMatrix2x3fv),
   TABLE_ENTRY(UniformMatrix2x4fv),
   TABLE_ENTRY(UniformMatrix3x2fv),
   TABLE_ENTRY(UniformMatrix3x4fv),
   TABLE_ENTRY(UniformMatrix4x2fv),
   TABLE_ENTRY(UniformMatrix4x3fv),
   TABLE_ENTRY(BeginConditionalRender),
   TABLE_ENTRY(BeginTransformFeedback),
   TABLE_ENTRY(BindBufferBase),
   TABLE_ENTRY(BindBufferRange),
   TABLE_ENTRY(BindFragDataLocation),
   TABLE_ENTRY(ClampColor),
   TABLE_ENTRY(ClearBufferfi),
   TABLE_ENTRY(ClearBufferfv),
   TABLE_ENTRY(ClearBufferiv),
   TABLE_ENTRY(ClearBufferuiv),
   TABLE_ENTRY(ColorMaski),
   TABLE_ENTRY(Disablei),
   TABLE_ENTRY(Enablei),
   TABLE_ENTRY(EndConditionalRender),
   TABLE_ENTRY(EndTransformFeedback),
   TABLE_ENTRY(GetBooleani_v),
   TABLE_ENTRY(GetFragDataLocation),
   TABLE_ENTRY(GetIntegeri_v),
   TABLE_ENTRY(GetStringi),
   TABLE_ENTRY(GetTexParameterIiv),
   TABLE_ENTRY(GetTexParameterIuiv),
   TABLE_ENTRY(GetTransformFeedbackVarying),
   TABLE_ENTRY(GetUniformuiv),
   TABLE_ENTRY(GetVertexAttribIiv),
   TABLE_ENTRY(GetVertexAttribIuiv),
   TABLE_ENTRY(IsEnabledi),
   TABLE_ENTRY(TexParameterIiv),
   TABLE_ENTRY(TexParameterIuiv),
   TABLE_ENTRY(TransformFeedbackVaryings),
   TABLE_ENTRY(Uniform1ui),
   TABLE_ENTRY(Uniform1uiv),
   TABLE_ENTRY(Uniform2ui),
   TABLE_ENTRY(Uniform2uiv),
   TABLE_ENTRY(Uniform3ui),
   TABLE_ENTRY(Uniform3uiv),
   TABLE_ENTRY(Uniform4ui),
   TABLE_ENTRY(Uniform4uiv),
   TABLE_ENTRY(VertexAttribI1iv),
   TABLE_ENTRY(VertexAttribI1uiv),
   TABLE_ENTRY(VertexAttribI4bv),
   TABLE_ENTRY(VertexAttribI4sv),
   TABLE_ENTRY(VertexAttribI4ubv),
   TABLE_ENTRY(VertexAttribI4usv),
   TABLE_ENTRY(VertexAttribIPointer),
   TABLE_ENTRY(PrimitiveRestartIndex),
   TABLE_ENTRY(TexBuffer),
   TABLE_ENTRY(FramebufferTexture),
   TABLE_ENTRY(GetBufferParameteri64v),
   TABLE_ENTRY(GetInteger64i_v),
   TABLE_ENTRY(VertexAttribDivisor),
   TABLE_ENTRY(MinSampleShading),
   TABLE_ENTRY(_dispatch_stub_621),
   TABLE_ENTRY(BindProgramARB),
   TABLE_ENTRY(DeleteProgramsARB),
   TABLE_ENTRY(GenProgramsARB),
   TABLE_ENTRY(GetProgramEnvParameterdvARB),
   TABLE_ENTRY(GetProgramEnvParameterfvARB),
   TABLE_ENTRY(GetProgramLocalParameterdvARB),
   TABLE_ENTRY(GetProgramLocalParameterfvARB),
   TABLE_ENTRY(GetProgramStringARB),
   TABLE_ENTRY(GetProgramivARB),
   TABLE_ENTRY(IsProgramARB),
   TABLE_ENTRY(ProgramEnvParameter4dARB),
   TABLE_ENTRY(ProgramEnvParameter4dvARB),
   TABLE_ENTRY(ProgramEnvParameter4fARB),
   TABLE_ENTRY(ProgramEnvParameter4fvARB),
   TABLE_ENTRY(ProgramLocalParameter4dARB),
   TABLE_ENTRY(ProgramLocalParameter4dvARB),
   TABLE_ENTRY(ProgramLocalParameter4fARB),
   TABLE_ENTRY(ProgramLocalParameter4fvARB),
   TABLE_ENTRY(ProgramStringARB),
   TABLE_ENTRY(VertexAttrib1fARB),
   TABLE_ENTRY(VertexAttrib1fvARB),
   TABLE_ENTRY(VertexAttrib2fARB),
   TABLE_ENTRY(VertexAttrib2fvARB),
   TABLE_ENTRY(VertexAttrib3fARB),
   TABLE_ENTRY(VertexAttrib3fvARB),
   TABLE_ENTRY(VertexAttrib4fARB),
   TABLE_ENTRY(VertexAttrib4fvARB),
   TABLE_ENTRY(AttachObjectARB),
   TABLE_ENTRY(CreateProgramObjectARB),
   TABLE_ENTRY(CreateShaderObjectARB),
   TABLE_ENTRY(DeleteObjectARB),
   TABLE_ENTRY(DetachObjectARB),
   TABLE_ENTRY(GetAttachedObjectsARB),
   TABLE_ENTRY(GetHandleARB),
   TABLE_ENTRY(GetInfoLogARB),
   TABLE_ENTRY(GetObjectParameterfvARB),
   TABLE_ENTRY(GetObjectParameterivARB),
   TABLE_ENTRY(DrawArraysInstancedARB),
   TABLE_ENTRY(DrawElementsInstancedARB),
   TABLE_ENTRY(BindFramebuffer),
   TABLE_ENTRY(BindRenderbuffer),
   TABLE_ENTRY(BlitFramebuffer),
   TABLE_ENTRY(CheckFramebufferStatus),
   TABLE_ENTRY(DeleteFramebuffers),
   TABLE_ENTRY(DeleteRenderbuffers),
   TABLE_ENTRY(FramebufferRenderbuffer),
   TABLE_ENTRY(FramebufferTexture1D),
   TABLE_ENTRY(FramebufferTexture2D),
   TABLE_ENTRY(FramebufferTexture3D),
   TABLE_ENTRY(FramebufferTextureLayer),
   TABLE_ENTRY(GenFramebuffers),
   TABLE_ENTRY(GenRenderbuffers),
   TABLE_ENTRY(GenerateMipmap),
   TABLE_ENTRY(GetFramebufferAttachmentParameteriv),
   TABLE_ENTRY(GetRenderbufferParameteriv),
   TABLE_ENTRY(IsFramebuffer),
   TABLE_ENTRY(IsRenderbuffer),
   TABLE_ENTRY(RenderbufferStorage),
   TABLE_ENTRY(RenderbufferStorageMultisample),
   TABLE_ENTRY(FlushMappedBufferRange),
   TABLE_ENTRY(MapBufferRange),
   TABLE_ENTRY(BindVertexArray),
   TABLE_ENTRY(DeleteVertexArrays),
   TABLE_ENTRY(GenVertexArrays),
   TABLE_ENTRY(IsVertexArray),
   TABLE_ENTRY(GetActiveUniformBlockName),
   TABLE_ENTRY(GetActiveUniformBlockiv),
   TABLE_ENTRY(GetActiveUniformName),
   TABLE_ENTRY(GetActiveUniformsiv),
   TABLE_ENTRY(GetUniformBlockIndex),
   TABLE_ENTRY(GetUniformIndices),
   TABLE_ENTRY(UniformBlockBinding),
   TABLE_ENTRY(CopyBufferSubData),
   TABLE_ENTRY(ClientWaitSync),
   TABLE_ENTRY(DeleteSync),
   TABLE_ENTRY(FenceSync),
   TABLE_ENTRY(GetInteger64v),
   TABLE_ENTRY(GetSynciv),
   TABLE_ENTRY(IsSync),
   TABLE_ENTRY(WaitSync),
   TABLE_ENTRY(DrawElementsBaseVertex),
   TABLE_ENTRY(DrawElementsInstancedBaseVertex),
   TABLE_ENTRY(DrawRangeElementsBaseVertex),
   TABLE_ENTRY(MultiDrawElementsBaseVertex),
   TABLE_ENTRY(ProvokingVertex),
   TABLE_ENTRY(GetMultisamplefv),
   TABLE_ENTRY(SampleMaski),
   TABLE_ENTRY(TexImage2DMultisample),
   TABLE_ENTRY(TexImage3DMultisample),
   TABLE_ENTRY(BlendEquationSeparateiARB),
   TABLE_ENTRY(BlendEquationiARB),
   TABLE_ENTRY(BlendFuncSeparateiARB),
   TABLE_ENTRY(BlendFunciARB),
   TABLE_ENTRY(BindFragDataLocationIndexed),
   TABLE_ENTRY(GetFragDataIndex),
   TABLE_ENTRY(BindSampler),
   TABLE_ENTRY(DeleteSamplers),
   TABLE_ENTRY(GenSamplers),
   TABLE_ENTRY(GetSamplerParameterIiv),
   TABLE_ENTRY(GetSamplerParameterIuiv),
   TABLE_ENTRY(GetSamplerParameterfv),
   TABLE_ENTRY(GetSamplerParameteriv),
   TABLE_ENTRY(IsSampler),
   TABLE_ENTRY(SamplerParameterIiv),
   TABLE_ENTRY(SamplerParameterIuiv),
   TABLE_ENTRY(SamplerParameterf),
   TABLE_ENTRY(SamplerParameterfv),
   TABLE_ENTRY(SamplerParameteri),
   TABLE_ENTRY(SamplerParameteriv),
   TABLE_ENTRY(_dispatch_stub_731),
   TABLE_ENTRY(_dispatch_stub_732),
   TABLE_ENTRY(_dispatch_stub_733),
   TABLE_ENTRY(ColorP3ui),
   TABLE_ENTRY(ColorP3uiv),
   TABLE_ENTRY(ColorP4ui),
   TABLE_ENTRY(ColorP4uiv),
   TABLE_ENTRY(MultiTexCoordP1ui),
   TABLE_ENTRY(MultiTexCoordP1uiv),
   TABLE_ENTRY(MultiTexCoordP2ui),
   TABLE_ENTRY(MultiTexCoordP2uiv),
   TABLE_ENTRY(MultiTexCoordP3ui),
   TABLE_ENTRY(MultiTexCoordP3uiv),
   TABLE_ENTRY(MultiTexCoordP4ui),
   TABLE_ENTRY(MultiTexCoordP4uiv),
   TABLE_ENTRY(NormalP3ui),
   TABLE_ENTRY(NormalP3uiv),
   TABLE_ENTRY(SecondaryColorP3ui),
   TABLE_ENTRY(SecondaryColorP3uiv),
   TABLE_ENTRY(TexCoordP1ui),
   TABLE_ENTRY(TexCoordP1uiv),
   TABLE_ENTRY(TexCoordP2ui),
   TABLE_ENTRY(TexCoordP2uiv),
   TABLE_ENTRY(TexCoordP3ui),
   TABLE_ENTRY(TexCoordP3uiv),
   TABLE_ENTRY(TexCoordP4ui),
   TABLE_ENTRY(TexCoordP4uiv),
   TABLE_ENTRY(VertexAttribP1ui),
   TABLE_ENTRY(VertexAttribP1uiv),
   TABLE_ENTRY(VertexAttribP2ui),
   TABLE_ENTRY(VertexAttribP2uiv),
   TABLE_ENTRY(VertexAttribP3ui),
   TABLE_ENTRY(VertexAttribP3uiv),
   TABLE_ENTRY(VertexAttribP4ui),
   TABLE_ENTRY(VertexAttribP4uiv),
   TABLE_ENTRY(VertexP2ui),
   TABLE_ENTRY(VertexP2uiv),
   TABLE_ENTRY(VertexP3ui),
   TABLE_ENTRY(VertexP3uiv),
   TABLE_ENTRY(VertexP4ui),
   TABLE_ENTRY(VertexP4uiv),
   TABLE_ENTRY(DrawArraysIndirect),
   TABLE_ENTRY(DrawElementsIndirect),
   TABLE_ENTRY(_dispatch_stub_774),
   TABLE_ENTRY(_dispatch_stub_775),
   TABLE_ENTRY(_dispatch_stub_776),
   TABLE_ENTRY(_dispatch_stub_777),
   TABLE_ENTRY(_dispatch_stub_778),
   TABLE_ENTRY(_dispatch_stub_779),
   TABLE_ENTRY(_dispatch_stub_780),
   TABLE_ENTRY(_dispatch_stub_781),
   TABLE_ENTRY(_dispatch_stub_782),
   TABLE_ENTRY(_dispatch_stub_783),
   TABLE_ENTRY(_dispatch_stub_784),
   TABLE_ENTRY(_dispatch_stub_785),
   TABLE_ENTRY(_dispatch_stub_786),
   TABLE_ENTRY(_dispatch_stub_787),
   TABLE_ENTRY(_dispatch_stub_788),
   TABLE_ENTRY(_dispatch_stub_789),
   TABLE_ENTRY(_dispatch_stub_790),
   TABLE_ENTRY(_dispatch_stub_791),
   TABLE_ENTRY(_dispatch_stub_792),
   TABLE_ENTRY(_dispatch_stub_793),
   TABLE_ENTRY(_dispatch_stub_794),
   TABLE_ENTRY(_dispatch_stub_795),
   TABLE_ENTRY(_dispatch_stub_796),
   TABLE_ENTRY(_dispatch_stub_797),
   TABLE_ENTRY(_dispatch_stub_798),
   TABLE_ENTRY(_dispatch_stub_799),
   TABLE_ENTRY(_dispatch_stub_800),
   TABLE_ENTRY(_dispatch_stub_801),
   TABLE_ENTRY(BindTransformFeedback),
   TABLE_ENTRY(DeleteTransformFeedbacks),
   TABLE_ENTRY(DrawTransformFeedback),
   TABLE_ENTRY(GenTransformFeedbacks),
   TABLE_ENTRY(IsTransformFeedback),
   TABLE_ENTRY(PauseTransformFeedback),
   TABLE_ENTRY(ResumeTransformFeedback),
   TABLE_ENTRY(BeginQueryIndexed),
   TABLE_ENTRY(DrawTransformFeedbackStream),
   TABLE_ENTRY(EndQueryIndexed),
   TABLE_ENTRY(GetQueryIndexediv),
   TABLE_ENTRY(ClearDepthf),
   TABLE_ENTRY(DepthRangef),
   TABLE_ENTRY(GetShaderPrecisionFormat),
   TABLE_ENTRY(ReleaseShaderCompiler),
   TABLE_ENTRY(ShaderBinary),
   TABLE_ENTRY(GetProgramBinary),
   TABLE_ENTRY(ProgramBinary),
   TABLE_ENTRY(ProgramParameteri),
   TABLE_ENTRY(_dispatch_stub_821),
   TABLE_ENTRY(_dispatch_stub_822),
   TABLE_ENTRY(_dispatch_stub_823),
   TABLE_ENTRY(_dispatch_stub_824),
   TABLE_ENTRY(_dispatch_stub_825),
   TABLE_ENTRY(_dispatch_stub_826),
   TABLE_ENTRY(_dispatch_stub_827),
   TABLE_ENTRY(_dispatch_stub_828),
   TABLE_ENTRY(_dispatch_stub_829),
   TABLE_ENTRY(_dispatch_stub_830),
   TABLE_ENTRY(DepthRangeArrayv),
   TABLE_ENTRY(DepthRangeIndexed),
   TABLE_ENTRY(GetDoublei_v),
   TABLE_ENTRY(GetFloati_v),
   TABLE_ENTRY(ScissorArrayv),
   TABLE_ENTRY(ScissorIndexed),
   TABLE_ENTRY(ScissorIndexedv),
   TABLE_ENTRY(ViewportArrayv),
   TABLE_ENTRY(ViewportIndexedf),
   TABLE_ENTRY(ViewportIndexedfv),
   TABLE_ENTRY(GetGraphicsResetStatusARB),
   TABLE_ENTRY(GetnColorTableARB),
   TABLE_ENTRY(GetnCompressedTexImageARB),
   TABLE_ENTRY(GetnConvolutionFilterARB),
   TABLE_ENTRY(GetnHistogramARB),
   TABLE_ENTRY(GetnMapdvARB),
   TABLE_ENTRY(GetnMapfvARB),
   TABLE_ENTRY(GetnMapivARB),
   TABLE_ENTRY(GetnMinmaxARB),
   TABLE_ENTRY(GetnPixelMapfvARB),
   TABLE_ENTRY(GetnPixelMapuivARB),
   TABLE_ENTRY(GetnPixelMapusvARB),
   TABLE_ENTRY(GetnPolygonStippleARB),
   TABLE_ENTRY(GetnSeparableFilterARB),
   TABLE_ENTRY(GetnTexImageARB),
   TABLE_ENTRY(GetnUniformdvARB),
   TABLE_ENTRY(GetnUniformfvARB),
   TABLE_ENTRY(GetnUniformivARB),
   TABLE_ENTRY(GetnUniformuivARB),
   TABLE_ENTRY(ReadnPixelsARB),
   TABLE_ENTRY(DrawArraysInstancedBaseInstance),
   TABLE_ENTRY(DrawElementsInstancedBaseInstance),
   TABLE_ENTRY(DrawElementsInstancedBaseVertexBaseInstance),
   TABLE_ENTRY(DrawTransformFeedbackInstanced),
   TABLE_ENTRY(DrawTransformFeedbackStreamInstanced),
   TABLE_ENTRY(_dispatch_stub_866),
   TABLE_ENTRY(GetActiveAtomicCounterBufferiv),
   TABLE_ENTRY(BindImageTexture),
   TABLE_ENTRY(MemoryBarrier),
   TABLE_ENTRY(TexStorage1D),
   TABLE_ENTRY(TexStorage2D),
   TABLE_ENTRY(TexStorage3D),
   TABLE_ENTRY(TextureStorage1DEXT),
   TABLE_ENTRY(TextureStorage2DEXT),
   TABLE_ENTRY(TextureStorage3DEXT),
   TABLE_ENTRY(ClearBufferData),
   TABLE_ENTRY(ClearBufferSubData),
   TABLE_ENTRY(DispatchCompute),
   TABLE_ENTRY(DispatchComputeIndirect),
   TABLE_ENTRY(CopyImageSubData),
   TABLE_ENTRY(TextureView),
   TABLE_ENTRY(BindVertexBuffer),
   TABLE_ENTRY(VertexAttribBinding),
   TABLE_ENTRY(VertexAttribFormat),
   TABLE_ENTRY(VertexAttribIFormat),
   TABLE_ENTRY(VertexAttribLFormat),
   TABLE_ENTRY(VertexBindingDivisor),
   TABLE_ENTRY(_dispatch_stub_888),
   TABLE_ENTRY(_dispatch_stub_889),
   TABLE_ENTRY(MultiDrawArraysIndirect),
   TABLE_ENTRY(MultiDrawElementsIndirect),
   TABLE_ENTRY(_dispatch_stub_892),
   TABLE_ENTRY(_dispatch_stub_893),
   TABLE_ENTRY(_dispatch_stub_894),
   TABLE_ENTRY(_dispatch_stub_895),
   TABLE_ENTRY(_dispatch_stub_896),
   TABLE_ENTRY(_dispatch_stub_897),
   TABLE_ENTRY(_dispatch_stub_898),
   TABLE_ENTRY(TexBufferRange),
   TABLE_ENTRY(TexStorage2DMultisample),
   TABLE_ENTRY(TexStorage3DMultisample),
   TABLE_ENTRY(BufferStorage),
   TABLE_ENTRY(ClearTexImage),
   TABLE_ENTRY(ClearTexSubImage),
   TABLE_ENTRY(BindBuffersBase),
   TABLE_ENTRY(BindBuffersRange),
   TABLE_ENTRY(BindImageTextures),
   TABLE_ENTRY(BindSamplers),
   TABLE_ENTRY(BindTextures),
   TABLE_ENTRY(BindVertexBuffers),
   TABLE_ENTRY(_dispatch_stub_911),
   TABLE_ENTRY(_dispatch_stub_912),
   TABLE_ENTRY(_dispatch_stub_913),
   TABLE_ENTRY(_dispatch_stub_914),
   TABLE_ENTRY(_dispatch_stub_915),
   TABLE_ENTRY(_dispatch_stub_916),
   TABLE_ENTRY(_dispatch_stub_917),
   TABLE_ENTRY(_dispatch_stub_918),
   TABLE_ENTRY(_dispatch_stub_919),
   TABLE_ENTRY(_dispatch_stub_920),
   TABLE_ENTRY(_dispatch_stub_921),
   TABLE_ENTRY(_dispatch_stub_922),
   TABLE_ENTRY(_dispatch_stub_923),
   TABLE_ENTRY(_dispatch_stub_924),
   TABLE_ENTRY(_dispatch_stub_925),
   TABLE_ENTRY(_dispatch_stub_926),
   TABLE_ENTRY(_dispatch_stub_927),
   TABLE_ENTRY(_dispatch_stub_928),
   TABLE_ENTRY(_dispatch_stub_929),
   TABLE_ENTRY(_dispatch_stub_930),
   TABLE_ENTRY(_dispatch_stub_931),
   TABLE_ENTRY(_dispatch_stub_932),
   TABLE_ENTRY(_dispatch_stub_933),
   TABLE_ENTRY(_dispatch_stub_934),
   TABLE_ENTRY(_dispatch_stub_935),
   TABLE_ENTRY(_dispatch_stub_936),
   TABLE_ENTRY(_dispatch_stub_937),
   TABLE_ENTRY(_dispatch_stub_938),
   TABLE_ENTRY(_dispatch_stub_939),
   TABLE_ENTRY(_dispatch_stub_940),
   TABLE_ENTRY(_dispatch_stub_941),
   TABLE_ENTRY(_dispatch_stub_942),
   TABLE_ENTRY(_dispatch_stub_943),
   TABLE_ENTRY(_dispatch_stub_944),
   TABLE_ENTRY(_dispatch_stub_945),
   TABLE_ENTRY(_dispatch_stub_946),
   TABLE_ENTRY(_dispatch_stub_947),
   TABLE_ENTRY(_dispatch_stub_948),
   TABLE_ENTRY(_dispatch_stub_949),
   TABLE_ENTRY(_dispatch_stub_950),
   TABLE_ENTRY(_dispatch_stub_951),
   TABLE_ENTRY(_dispatch_stub_952),
   TABLE_ENTRY(_dispatch_stub_953),
   TABLE_ENTRY(_dispatch_stub_954),
   TABLE_ENTRY(_dispatch_stub_955),
   TABLE_ENTRY(_dispatch_stub_956),
   TABLE_ENTRY(_dispatch_stub_957),
   TABLE_ENTRY(_dispatch_stub_958),
   TABLE_ENTRY(_dispatch_stub_959),
   TABLE_ENTRY(_dispatch_stub_960),
   TABLE_ENTRY(_dispatch_stub_961),
   TABLE_ENTRY(_dispatch_stub_962),
   TABLE_ENTRY(_dispatch_stub_963),
   TABLE_ENTRY(_dispatch_stub_964),
   TABLE_ENTRY(_dispatch_stub_965),
   TABLE_ENTRY(_dispatch_stub_966),
   TABLE_ENTRY(_dispatch_stub_967),
   TABLE_ENTRY(_dispatch_stub_968),
   TABLE_ENTRY(_dispatch_stub_969),
   TABLE_ENTRY(_dispatch_stub_970),
   TABLE_ENTRY(_dispatch_stub_971),
   TABLE_ENTRY(_dispatch_stub_972),
   TABLE_ENTRY(_dispatch_stub_973),
   TABLE_ENTRY(_dispatch_stub_974),
   TABLE_ENTRY(_dispatch_stub_975),
   TABLE_ENTRY(_dispatch_stub_976),
   TABLE_ENTRY(_dispatch_stub_977),
   TABLE_ENTRY(_dispatch_stub_978),
   TABLE_ENTRY(_dispatch_stub_979),
   TABLE_ENTRY(_dispatch_stub_980),
   TABLE_ENTRY(_dispatch_stub_981),
   TABLE_ENTRY(_dispatch_stub_982),
   TABLE_ENTRY(_dispatch_stub_983),
   TABLE_ENTRY(_dispatch_stub_984),
   TABLE_ENTRY(_dispatch_stub_985),
   TABLE_ENTRY(_dispatch_stub_986),
   TABLE_ENTRY(_dispatch_stub_987),
   TABLE_ENTRY(_dispatch_stub_988),
   TABLE_ENTRY(_dispatch_stub_989),
   TABLE_ENTRY(_dispatch_stub_990),
   TABLE_ENTRY(_dispatch_stub_991),
   TABLE_ENTRY(_dispatch_stub_992),
   TABLE_ENTRY(_dispatch_stub_993),
   TABLE_ENTRY(_dispatch_stub_994),
   TABLE_ENTRY(_dispatch_stub_995),
   TABLE_ENTRY(_dispatch_stub_996),
   TABLE_ENTRY(_dispatch_stub_997),
   TABLE_ENTRY(_dispatch_stub_998),
   TABLE_ENTRY(_dispatch_stub_999),
   TABLE_ENTRY(_dispatch_stub_1000),
   TABLE_ENTRY(_dispatch_stub_1001),
   TABLE_ENTRY(_dispatch_stub_1002),
   TABLE_ENTRY(_dispatch_stub_1003),
   TABLE_ENTRY(_dispatch_stub_1004),
   TABLE_ENTRY(_dispatch_stub_1005),
   TABLE_ENTRY(_dispatch_stub_1006),
   TABLE_ENTRY(_dispatch_stub_1007),
   TABLE_ENTRY(_dispatch_stub_1008),
   TABLE_ENTRY(_dispatch_stub_1009),
   TABLE_ENTRY(_dispatch_stub_1010),
   TABLE_ENTRY(_dispatch_stub_1011),
   TABLE_ENTRY(_dispatch_stub_1012),
   TABLE_ENTRY(InvalidateBufferData),
   TABLE_ENTRY(InvalidateBufferSubData),
   TABLE_ENTRY(InvalidateFramebuffer),
   TABLE_ENTRY(InvalidateSubFramebuffer),
   TABLE_ENTRY(InvalidateTexImage),
   TABLE_ENTRY(InvalidateTexSubImage),
   TABLE_ENTRY(_dispatch_stub_1019),
   TABLE_ENTRY(_dispatch_stub_1020),
   TABLE_ENTRY(_dispatch_stub_1021),
   TABLE_ENTRY(_dispatch_stub_1022),
   TABLE_ENTRY(_dispatch_stub_1023),
   TABLE_ENTRY(_dispatch_stub_1024),
   TABLE_ENTRY(_dispatch_stub_1025),
   TABLE_ENTRY(_dispatch_stub_1026),
   TABLE_ENTRY(_dispatch_stub_1027),
   TABLE_ENTRY(PointSizePointerOES),
   TABLE_ENTRY(_dispatch_stub_1029),
   TABLE_ENTRY(_dispatch_stub_1030),
   TABLE_ENTRY(_dispatch_stub_1031),
   TABLE_ENTRY(ColorPointerEXT),
   TABLE_ENTRY(EdgeFlagPointerEXT),
   TABLE_ENTRY(IndexPointerEXT),
   TABLE_ENTRY(NormalPointerEXT),
   TABLE_ENTRY(TexCoordPointerEXT),
   TABLE_ENTRY(VertexPointerEXT),
   TABLE_ENTRY(_dispatch_stub_1038),
   TABLE_ENTRY(_dispatch_stub_1039),
   TABLE_ENTRY(_dispatch_stub_1040),
   TABLE_ENTRY(_dispatch_stub_1041),
   TABLE_ENTRY(_dispatch_stub_1042),
   TABLE_ENTRY(_dispatch_stub_1043),
   TABLE_ENTRY(_dispatch_stub_1044),
   TABLE_ENTRY(_dispatch_stub_1045),
   TABLE_ENTRY(_dispatch_stub_1046),
   TABLE_ENTRY(LockArraysEXT),
   TABLE_ENTRY(_dispatch_stub_1048),
   TABLE_ENTRY(_dispatch_stub_1049),
   TABLE_ENTRY(_dispatch_stub_1050),
   TABLE_ENTRY(_dispatch_stub_1051),
   TABLE_ENTRY(_dispatch_stub_1052),
   TABLE_ENTRY(_dispatch_stub_1053),
   TABLE_ENTRY(_dispatch_stub_1054),
   TABLE_ENTRY(_dispatch_stub_1055),
   TABLE_ENTRY(_dispatch_stub_1056),
   TABLE_ENTRY(_dispatch_stub_1057),
   TABLE_ENTRY(_dispatch_stub_1058),
   TABLE_ENTRY(_dispatch_stub_1059),
   TABLE_ENTRY(_dispatch_stub_1060),
   TABLE_ENTRY(_dispatch_stub_1061),
   TABLE_ENTRY(_dispatch_stub_1062),
   TABLE_ENTRY(_dispatch_stub_1063),
   TABLE_ENTRY(_dispatch_stub_1064),
   TABLE_ENTRY(_dispatch_stub_1065),
   TABLE_ENTRY(_dispatch_stub_1066),
   TABLE_ENTRY(_dispatch_stub_1067),
   TABLE_ENTRY(_dispatch_stub_1068),
   TABLE_ENTRY(_dispatch_stub_1069),
   TABLE_ENTRY(_dispatch_stub_1070),
   TABLE_ENTRY(_dispatch_stub_1071),
   TABLE_ENTRY(_dispatch_stub_1072),
   TABLE_ENTRY(_dispatch_stub_1073),
   TABLE_ENTRY(_dispatch_stub_1074),
   TABLE_ENTRY(_dispatch_stub_1075),
   TABLE_ENTRY(_dispatch_stub_1076),
   TABLE_ENTRY(_dispatch_stub_1077),
   TABLE_ENTRY(_dispatch_stub_1078),
   TABLE_ENTRY(_dispatch_stub_1079),
   TABLE_ENTRY(_dispatch_stub_1080),
   TABLE_ENTRY(_dispatch_stub_1081),
   TABLE_ENTRY(_dispatch_stub_1082),
   TABLE_ENTRY(_dispatch_stub_1083),
   TABLE_ENTRY(_dispatch_stub_1084),
   TABLE_ENTRY(_dispatch_stub_1085),
   TABLE_ENTRY(_dispatch_stub_1086),
   TABLE_ENTRY(_dispatch_stub_1087),
   TABLE_ENTRY(_dispatch_stub_1088),
   TABLE_ENTRY(_dispatch_stub_1089),
   TABLE_ENTRY(_dispatch_stub_1090),
   TABLE_ENTRY(_dispatch_stub_1091),
   TABLE_ENTRY(_dispatch_stub_1092),
   TABLE_ENTRY(_dispatch_stub_1093),
   TABLE_ENTRY(_dispatch_stub_1094),
   TABLE_ENTRY(_dispatch_stub_1095),
   TABLE_ENTRY(_dispatch_stub_1096),
   TABLE_ENTRY(_dispatch_stub_1097),
   TABLE_ENTRY(UnlockArraysEXT),
   TABLE_ENTRY(_dispatch_stub_1099),
   TABLE_ENTRY(_dispatch_stub_1100),
   TABLE_ENTRY(DebugMessageCallback),
   TABLE_ENTRY(DebugMessageControl),
   TABLE_ENTRY(DebugMessageInsert),
   TABLE_ENTRY(GetDebugMessageLog),
   TABLE_ENTRY(GetObjectLabel),
   TABLE_ENTRY(GetObjectPtrLabel),
   TABLE_ENTRY(ObjectLabel),
   TABLE_ENTRY(ObjectPtrLabel),
   TABLE_ENTRY(PopDebugGroup),
   TABLE_ENTRY(PushDebugGroup),
   TABLE_ENTRY(SecondaryColor3fEXT),
   TABLE_ENTRY(SecondaryColor3fvEXT),
   TABLE_ENTRY(MultiDrawElementsEXT),
   TABLE_ENTRY(FogCoordfEXT),
   TABLE_ENTRY(FogCoordfvEXT),
   TABLE_ENTRY(_dispatch_stub_1116),
   TABLE_ENTRY(_dispatch_stub_1117),
   TABLE_ENTRY(_dispatch_stub_1118),
   TABLE_ENTRY(_dispatch_stub_1119),
   TABLE_ENTRY(_dispatch_stub_1120),
   TABLE_ENTRY(_dispatch_stub_1121),
   TABLE_ENTRY(_dispatch_stub_1122),
   TABLE_ENTRY(_dispatch_stub_1123),
   TABLE_ENTRY(_dispatch_stub_1124),
   TABLE_ENTRY(_dispatch_stub_1125),
   TABLE_ENTRY(_dispatch_stub_1126),
   TABLE_ENTRY(_dispatch_stub_1127),
   TABLE_ENTRY(_dispatch_stub_1128),
   TABLE_ENTRY(_dispatch_stub_1129),
   TABLE_ENTRY(_dispatch_stub_1130),
   TABLE_ENTRY(_dispatch_stub_1131),
   TABLE_ENTRY(_dispatch_stub_1132),
   TABLE_ENTRY(_dispatch_stub_1133),
   TABLE_ENTRY(_dispatch_stub_1134),
   TABLE_ENTRY(_dispatch_stub_1135),
   TABLE_ENTRY(_dispatch_stub_1136),
   TABLE_ENTRY(_dispatch_stub_1137),
   TABLE_ENTRY(_dispatch_stub_1138),
   TABLE_ENTRY(_dispatch_stub_1139),
   TABLE_ENTRY(_dispatch_stub_1140),
   TABLE_ENTRY(_dispatch_stub_1141),
   TABLE_ENTRY(_dispatch_stub_1142),
   TABLE_ENTRY(_dispatch_stub_1143),
   TABLE_ENTRY(_dispatch_stub_1144),
   TABLE_ENTRY(_dispatch_stub_1145),
   TABLE_ENTRY(_dispatch_stub_1146),
   TABLE_ENTRY(_dispatch_stub_1147),
   TABLE_ENTRY(_dispatch_stub_1148),
   TABLE_ENTRY(_dispatch_stub_1149),
   TABLE_ENTRY(_dispatch_stub_1150),
   TABLE_ENTRY(_dispatch_stub_1151),
   TABLE_ENTRY(_dispatch_stub_1152),
   TABLE_ENTRY(_dispatch_stub_1153),
   TABLE_ENTRY(_dispatch_stub_1154),
   TABLE_ENTRY(_dispatch_stub_1155),
   TABLE_ENTRY(_dispatch_stub_1156),
   TABLE_ENTRY(_dispatch_stub_1157),
   TABLE_ENTRY(_dispatch_stub_1158),
   TABLE_ENTRY(_dispatch_stub_1159),
   TABLE_ENTRY(_dispatch_stub_1160),
   TABLE_ENTRY(_dispatch_stub_1161),
   TABLE_ENTRY(_dispatch_stub_1162),
   TABLE_ENTRY(_dispatch_stub_1163),
   TABLE_ENTRY(_dispatch_stub_1164),
   TABLE_ENTRY(_dispatch_stub_1165),
   TABLE_ENTRY(_dispatch_stub_1166),
   TABLE_ENTRY(_dispatch_stub_1167),
   TABLE_ENTRY(_dispatch_stub_1168),
   TABLE_ENTRY(_dispatch_stub_1169),
   TABLE_ENTRY(_dispatch_stub_1170),
   TABLE_ENTRY(_dispatch_stub_1171),
   TABLE_ENTRY(_dispatch_stub_1172),
   TABLE_ENTRY(_dispatch_stub_1173),
   TABLE_ENTRY(_dispatch_stub_1174),
   TABLE_ENTRY(_dispatch_stub_1175),
   TABLE_ENTRY(_dispatch_stub_1176),
   TABLE_ENTRY(_dispatch_stub_1177),
   TABLE_ENTRY(_dispatch_stub_1178),
   TABLE_ENTRY(_dispatch_stub_1179),
   TABLE_ENTRY(_dispatch_stub_1180),
   TABLE_ENTRY(_dispatch_stub_1181),
   TABLE_ENTRY(_dispatch_stub_1182),
   TABLE_ENTRY(_dispatch_stub_1183),
   TABLE_ENTRY(_dispatch_stub_1184),
   TABLE_ENTRY(_dispatch_stub_1185),
   TABLE_ENTRY(_dispatch_stub_1186),
   TABLE_ENTRY(_dispatch_stub_1187),
   TABLE_ENTRY(_dispatch_stub_1188),
   TABLE_ENTRY(_dispatch_stub_1189),
   TABLE_ENTRY(_dispatch_stub_1190),
   TABLE_ENTRY(_dispatch_stub_1191),
   TABLE_ENTRY(_dispatch_stub_1192),
   TABLE_ENTRY(_dispatch_stub_1193),
   TABLE_ENTRY(_dispatch_stub_1194),
   TABLE_ENTRY(_dispatch_stub_1195),
   TABLE_ENTRY(_dispatch_stub_1196),
   TABLE_ENTRY(_dispatch_stub_1197),
   TABLE_ENTRY(_dispatch_stub_1198),
   TABLE_ENTRY(_dispatch_stub_1199),
   TABLE_ENTRY(_dispatch_stub_1200),
   TABLE_ENTRY(_dispatch_stub_1201),
   TABLE_ENTRY(_dispatch_stub_1202),
   TABLE_ENTRY(_dispatch_stub_1203),
   TABLE_ENTRY(_dispatch_stub_1204),
   TABLE_ENTRY(_dispatch_stub_1205),
   TABLE_ENTRY(_dispatch_stub_1206),
   TABLE_ENTRY(_dispatch_stub_1207),
   TABLE_ENTRY(_dispatch_stub_1208),
   TABLE_ENTRY(PrimitiveRestartNV),
   TABLE_ENTRY(_dispatch_stub_1210),
   TABLE_ENTRY(_dispatch_stub_1211),
   TABLE_ENTRY(_dispatch_stub_1212),
   TABLE_ENTRY(_dispatch_stub_1213),
   TABLE_ENTRY(BindFramebufferEXT),
   TABLE_ENTRY(BindRenderbufferEXT),
   TABLE_ENTRY(_dispatch_stub_1216),
   TABLE_ENTRY(_dispatch_stub_1217),
   TABLE_ENTRY(_dispatch_stub_1218),
   TABLE_ENTRY(VertexAttribI1iEXT),
   TABLE_ENTRY(VertexAttribI1uiEXT),
   TABLE_ENTRY(VertexAttribI2iEXT),
   TABLE_ENTRY(VertexAttribI2ivEXT),
   TABLE_ENTRY(VertexAttribI2uiEXT),
   TABLE_ENTRY(VertexAttribI2uivEXT),
   TABLE_ENTRY(VertexAttribI3iEXT),
   TABLE_ENTRY(VertexAttribI3ivEXT),
   TABLE_ENTRY(VertexAttribI3uiEXT),
   TABLE_ENTRY(VertexAttribI3uivEXT),
   TABLE_ENTRY(VertexAttribI4iEXT),
   TABLE_ENTRY(VertexAttribI4ivEXT),
   TABLE_ENTRY(VertexAttribI4uiEXT),
   TABLE_ENTRY(VertexAttribI4uivEXT),
   TABLE_ENTRY(ClearColorIiEXT),
   TABLE_ENTRY(ClearColorIuiEXT),
   TABLE_ENTRY(_dispatch_stub_1235),
   TABLE_ENTRY(_dispatch_stub_1236),
   TABLE_ENTRY(_dispatch_stub_1237),
   TABLE_ENTRY(_dispatch_stub_1238),
   TABLE_ENTRY(_dispatch_stub_1239),
   TABLE_ENTRY(_dispatch_stub_1240),
   TABLE_ENTRY(_dispatch_stub_1241),
   TABLE_ENTRY(_dispatch_stub_1242),
   TABLE_ENTRY(_dispatch_stub_1243),
   TABLE_ENTRY(_dispatch_stub_1244),
   TABLE_ENTRY(_dispatch_stub_1245),
   TABLE_ENTRY(_dispatch_stub_1246),
   TABLE_ENTRY(_dispatch_stub_1247),
   TABLE_ENTRY(_dispatch_stub_1248),
   TABLE_ENTRY(_dispatch_stub_1249),
   TABLE_ENTRY(_dispatch_stub_1250),
   TABLE_ENTRY(_dispatch_stub_1251),
   TABLE_ENTRY(_dispatch_stub_1252),
   TABLE_ENTRY(TextureBarrierNV),
   TABLE_ENTRY(_dispatch_stub_1254),
   TABLE_ENTRY(_dispatch_stub_1255),
   TABLE_ENTRY(_dispatch_stub_1256),
   TABLE_ENTRY(_dispatch_stub_1257),
   TABLE_ENTRY(_dispatch_stub_1258),
   TABLE_ENTRY(_dispatch_stub_1259),
   TABLE_ENTRY(_dispatch_stub_1260),
   TABLE_ENTRY(_dispatch_stub_1261),
   TABLE_ENTRY(_dispatch_stub_1262),
   TABLE_ENTRY(_dispatch_stub_1263),
   TABLE_ENTRY(_dispatch_stub_1264),
   TABLE_ENTRY(_dispatch_stub_1265),
   TABLE_ENTRY(_dispatch_stub_1266),
   TABLE_ENTRY(_dispatch_stub_1267),
   TABLE_ENTRY(_dispatch_stub_1268),
   TABLE_ENTRY(_dispatch_stub_1269),
   TABLE_ENTRY(_dispatch_stub_1270),
   TABLE_ENTRY(_dispatch_stub_1271),
   TABLE_ENTRY(_dispatch_stub_1272),
   TABLE_ENTRY(_dispatch_stub_1273),
   TABLE_ENTRY(_dispatch_stub_1274),
   TABLE_ENTRY(_dispatch_stub_1275),
   TABLE_ENTRY(_dispatch_stub_1276),
   TABLE_ENTRY(_dispatch_stub_1277),
   TABLE_ENTRY(_dispatch_stub_1278),
   TABLE_ENTRY(_dispatch_stub_1279),
   TABLE_ENTRY(AlphaFuncx),
   TABLE_ENTRY(ClearColorx),
   TABLE_ENTRY(ClearDepthx),
   TABLE_ENTRY(Color4x),
   TABLE_ENTRY(DepthRangex),
   TABLE_ENTRY(Fogx),
   TABLE_ENTRY(Fogxv),
   TABLE_ENTRY(Frustumf),
   TABLE_ENTRY(Frustumx),
   TABLE_ENTRY(LightModelx),
   TABLE_ENTRY(LightModelxv),
   TABLE_ENTRY(Lightx),
   TABLE_ENTRY(Lightxv),
   TABLE_ENTRY(LineWidthx),
   TABLE_ENTRY(LoadMatrixx),
   TABLE_ENTRY(Materialx),
   TABLE_ENTRY(Materialxv),
   TABLE_ENTRY(MultMatrixx),
   TABLE_ENTRY(MultiTexCoord4x),
   TABLE_ENTRY(Normal3x),
   TABLE_ENTRY(Orthof),
   TABLE_ENTRY(Orthox),
   TABLE_ENTRY(PointSizex),
   TABLE_ENTRY(PolygonOffsetx),
   TABLE_ENTRY(Rotatex),
   TABLE_ENTRY(SampleCoveragex),
   TABLE_ENTRY(Scalex),
   TABLE_ENTRY(TexEnvx),
   TABLE_ENTRY(TexEnvxv),
   TABLE_ENTRY(TexParameterx),
   TABLE_ENTRY(Translatex),
   TABLE_ENTRY(ClipPlanef),
   TABLE_ENTRY(ClipPlanex),
   TABLE_ENTRY(GetClipPlanef),
   TABLE_ENTRY(GetClipPlanex),
   TABLE_ENTRY(GetFixedv),
   TABLE_ENTRY(GetLightxv),
   TABLE_ENTRY(GetMaterialxv),
   TABLE_ENTRY(GetTexEnvxv),
   TABLE_ENTRY(GetTexParameterxv),
   TABLE_ENTRY(PointParameterx),
   TABLE_ENTRY(PointParameterxv),
   TABLE_ENTRY(TexParameterxv),
   /* A whole bunch of no-op functions.  These might be called
    * when someone tries to call a dynamically-registered
    * extension function without a current rendering context.
    */
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
   TABLE_ENTRY(Unused),
};
#endif /* DISPATCH_TABLE_NAME */


/*
 * This is just used to silence compiler warnings.
 * We list the functions which are not otherwise used.
 */
#ifdef UNUSED_TABLE_NAME
_glapi_proc UNUSED_TABLE_NAME[] = {
#ifndef _GLAPI_SKIP_NORMAL_ENTRY_POINTS
   TABLE_ENTRY(_dispatch_stub_190),
   TABLE_ENTRY(_dispatch_stub_191),
   TABLE_ENTRY(_dispatch_stub_192),
   TABLE_ENTRY(_dispatch_stub_193),
   TABLE_ENTRY(_dispatch_stub_254),
   TABLE_ENTRY(_dispatch_stub_279),
   TABLE_ENTRY(_dispatch_stub_280),
   TABLE_ENTRY(ArrayElementEXT),
   TABLE_ENTRY(BindTextureEXT),
   TABLE_ENTRY(DrawArraysEXT),
   TABLE_ENTRY(_dispatch_stub_323),
   TABLE_ENTRY(_dispatch_stub_324),
   TABLE_ENTRY(_dispatch_stub_325),
   TABLE_ENTRY(_dispatch_stub_326),
   TABLE_ENTRY(GetPointervEXT),
   TABLE_ENTRY(_dispatch_stub_329),
   TABLE_ENTRY(PrioritizeTexturesEXT),
   TABLE_ENTRY(_dispatch_stub_332),
   TABLE_ENTRY(_dispatch_stub_333),
   TABLE_ENTRY(BlendColorEXT),
   TABLE_ENTRY(BlendEquationEXT),
   TABLE_ENTRY(_dispatch_stub_337),
   TABLE_ENTRY(DrawRangeElementsEXT),
   TABLE_ENTRY(_dispatch_stub_339),
   TABLE_ENTRY(_dispatch_stub_340),
   TABLE_ENTRY(_dispatch_stub_341),
   TABLE_ENTRY(_dispatch_stub_342),
   TABLE_ENTRY(_dispatch_stub_346),
   TABLE_ENTRY(_dispatch_stub_347),
   TABLE_ENTRY(_dispatch_stub_348),
   TABLE_ENTRY(_dispatch_stub_349),
   TABLE_ENTRY(_dispatch_stub_350),
   TABLE_ENTRY(_dispatch_stub_351),
   TABLE_ENTRY(_dispatch_stub_352),
   TABLE_ENTRY(_dispatch_stub_353),
   TABLE_ENTRY(_dispatch_stub_354),
   TABLE_ENTRY(_dispatch_stub_355),
   TABLE_ENTRY(_dispatch_stub_360),
   TABLE_ENTRY(_dispatch_stub_367),
   TABLE_ENTRY(_dispatch_stub_368),
   TABLE_ENTRY(_dispatch_stub_369),
   TABLE_ENTRY(_dispatch_stub_370),
   TABLE_ENTRY(TexImage3DEXT),
   TABLE_ENTRY(_dispatch_stub_371),
   TABLE_ENTRY(TexSubImage3DEXT),
   TABLE_ENTRY(_dispatch_stub_372),
   TABLE_ENTRY(CopyTexSubImage3DEXT),
   TABLE_ENTRY(_dispatch_stub_373),
   TABLE_ENTRY(ActiveTextureARB),
   TABLE_ENTRY(ClientActiveTextureARB),
   TABLE_ENTRY(MultiTexCoord1dARB),
   TABLE_ENTRY(MultiTexCoord1dvARB),
   TABLE_ENTRY(MultiTexCoord1f),
   TABLE_ENTRY(MultiTexCoord1fv),
   TABLE_ENTRY(MultiTexCoord1iARB),
   TABLE_ENTRY(MultiTexCoord1ivARB),
   TABLE_ENTRY(MultiTexCoord1sARB),
   TABLE_ENTRY(MultiTexCoord1svARB),
   TABLE_ENTRY(MultiTexCoord2dARB),
   TABLE_ENTRY(MultiTexCoord2dvARB),
   TABLE_ENTRY(MultiTexCoord2f),
   TABLE_ENTRY(MultiTexCoord2fv),
   TABLE_ENTRY(MultiTexCoord2iARB),
   TABLE_ENTRY(MultiTexCoord2ivARB),
   TABLE_ENTRY(MultiTexCoord2sARB),
   TABLE_ENTRY(MultiTexCoord2svARB),
   TABLE_ENTRY(MultiTexCoord3dARB),
   TABLE_ENTRY(MultiTexCoord3dvARB),
   TABLE_ENTRY(MultiTexCoord3f),
   TABLE_ENTRY(MultiTexCoord3fv),
   TABLE_ENTRY(MultiTexCoord3iARB),
   TABLE_ENTRY(MultiTexCoord3ivARB),
   TABLE_ENTRY(MultiTexCoord3sARB),
   TABLE_ENTRY(MultiTexCoord3svARB),
   TABLE_ENTRY(MultiTexCoord4dARB),
   TABLE_ENTRY(MultiTexCoord4dvARB),
   TABLE_ENTRY(MultiTexCoord4f),
   TABLE_ENTRY(MultiTexCoord4fv),
   TABLE_ENTRY(MultiTexCoord4iARB),
   TABLE_ENTRY(MultiTexCoord4ivARB),
   TABLE_ENTRY(MultiTexCoord4sARB),
   TABLE_ENTRY(MultiTexCoord4svARB),
   TABLE_ENTRY(CompressedTexImage1DARB),
   TABLE_ENTRY(CompressedTexImage2DARB),
   TABLE_ENTRY(CompressedTexImage3DARB),
   TABLE_ENTRY(_dispatch_stub_410),
   TABLE_ENTRY(CompressedTexSubImage1DARB),
   TABLE_ENTRY(CompressedTexSubImage2DARB),
   TABLE_ENTRY(CompressedTexSubImage3DARB),
   TABLE_ENTRY(_dispatch_stub_413),
   TABLE_ENTRY(GetCompressedTexImageARB),
   TABLE_ENTRY(LoadTransposeMatrixdARB),
   TABLE_ENTRY(LoadTransposeMatrixfARB),
   TABLE_ENTRY(MultTransposeMatrixdARB),
   TABLE_ENTRY(MultTransposeMatrixfARB),
   TABLE_ENTRY(SampleCoverageARB),
   TABLE_ENTRY(BlendFuncSeparateEXT),
   TABLE_ENTRY(_dispatch_stub_420),
   TABLE_ENTRY(FogCoordPointerEXT),
   TABLE_ENTRY(FogCoorddEXT),
   TABLE_ENTRY(FogCoorddvEXT),
   TABLE_ENTRY(MultiDrawArraysEXT),
   TABLE_ENTRY(PointParameterfARB),
   TABLE_ENTRY(PointParameterfEXT),
   TABLE_ENTRY(_dispatch_stub_425),
   TABLE_ENTRY(PointParameterfvARB),
   TABLE_ENTRY(PointParameterfvEXT),
   TABLE_ENTRY(_dispatch_stub_426),
   TABLE_ENTRY(_dispatch_stub_427),
   TABLE_ENTRY(_dispatch_stub_428),
   TABLE_ENTRY(SecondaryColor3bEXT),
   TABLE_ENTRY(SecondaryColor3bvEXT),
   TABLE_ENTRY(SecondaryColor3dEXT),
   TABLE_ENTRY(SecondaryColor3dvEXT),
   TABLE_ENTRY(SecondaryColor3iEXT),
   TABLE_ENTRY(SecondaryColor3ivEXT),
   TABLE_ENTRY(SecondaryColor3sEXT),
   TABLE_ENTRY(SecondaryColor3svEXT),
   TABLE_ENTRY(SecondaryColor3ubEXT),
   TABLE_ENTRY(SecondaryColor3ubvEXT),
   TABLE_ENTRY(SecondaryColor3uiEXT),
   TABLE_ENTRY(SecondaryColor3uivEXT),
   TABLE_ENTRY(SecondaryColor3usEXT),
   TABLE_ENTRY(SecondaryColor3usvEXT),
   TABLE_ENTRY(SecondaryColorPointerEXT),
   TABLE_ENTRY(WindowPos2dARB),
   TABLE_ENTRY(_dispatch_stub_444),
   TABLE_ENTRY(WindowPos2dvARB),
   TABLE_ENTRY(_dispatch_stub_445),
   TABLE_ENTRY(WindowPos2fARB),
   TABLE_ENTRY(_dispatch_stub_446),
   TABLE_ENTRY(WindowPos2fvARB),
   TABLE_ENTRY(_dispatch_stub_447),
   TABLE_ENTRY(WindowPos2iARB),
   TABLE_ENTRY(_dispatch_stub_448),
   TABLE_ENTRY(WindowPos2ivARB),
   TABLE_ENTRY(_dispatch_stub_449),
   TABLE_ENTRY(WindowPos2sARB),
   TABLE_ENTRY(_dispatch_stub_450),
   TABLE_ENTRY(WindowPos2svARB),
   TABLE_ENTRY(_dispatch_stub_451),
   TABLE_ENTRY(WindowPos3dARB),
   TABLE_ENTRY(_dispatch_stub_452),
   TABLE_ENTRY(WindowPos3dvARB),
   TABLE_ENTRY(_dispatch_stub_453),
   TABLE_ENTRY(WindowPos3fARB),
   TABLE_ENTRY(_dispatch_stub_454),
   TABLE_ENTRY(WindowPos3fvARB),
   TABLE_ENTRY(_dispatch_stub_455),
   TABLE_ENTRY(WindowPos3iARB),
   TABLE_ENTRY(_dispatch_stub_456),
   TABLE_ENTRY(WindowPos3ivARB),
   TABLE_ENTRY(_dispatch_stub_457),
   TABLE_ENTRY(WindowPos3sARB),
   TABLE_ENTRY(_dispatch_stub_458),
   TABLE_ENTRY(WindowPos3svARB),
   TABLE_ENTRY(_dispatch_stub_459),
   TABLE_ENTRY(BeginQueryARB),
   TABLE_ENTRY(BindBufferARB),
   TABLE_ENTRY(BufferDataARB),
   TABLE_ENTRY(BufferSubDataARB),
   TABLE_ENTRY(DeleteBuffersARB),
   TABLE_ENTRY(DeleteQueriesARB),
   TABLE_ENTRY(EndQueryARB),
   TABLE_ENTRY(GenBuffersARB),
   TABLE_ENTRY(GenQueriesARB),
   TABLE_ENTRY(GetBufferParameterivARB),
   TABLE_ENTRY(GetBufferPointervARB),
   TABLE_ENTRY(_dispatch_stub_470),
   TABLE_ENTRY(GetBufferSubDataARB),
   TABLE_ENTRY(GetQueryObjectivARB),
   TABLE_ENTRY(GetQueryObjectuivARB),
   TABLE_ENTRY(GetQueryivARB),
   TABLE_ENTRY(IsBufferARB),
   TABLE_ENTRY(IsQueryARB),
   TABLE_ENTRY(MapBufferARB),
   TABLE_ENTRY(_dispatch_stub_477),
   TABLE_ENTRY(UnmapBufferARB),
   TABLE_ENTRY(_dispatch_stub_478),
   TABLE_ENTRY(BindAttribLocationARB),
   TABLE_ENTRY(_dispatch_stub_481),
   TABLE_ENTRY(CompileShaderARB),
   TABLE_ENTRY(DisableVertexAttribArrayARB),
   TABLE_ENTRY(DrawBuffersARB),
   TABLE_ENTRY(DrawBuffersATI),
   TABLE_ENTRY(_dispatch_stub_489),
   TABLE_ENTRY(EnableVertexAttribArrayARB),
   TABLE_ENTRY(GetActiveAttribARB),
   TABLE_ENTRY(GetActiveUniformARB),
   TABLE_ENTRY(GetAttribLocationARB),
   TABLE_ENTRY(GetShaderSourceARB),
   TABLE_ENTRY(GetUniformLocationARB),
   TABLE_ENTRY(GetUniformfvARB),
   TABLE_ENTRY(GetUniformivARB),
   TABLE_ENTRY(GetVertexAttribPointervARB),
   TABLE_ENTRY(_dispatch_stub_503),
   TABLE_ENTRY(GetVertexAttribdvARB),
   TABLE_ENTRY(GetVertexAttribfvARB),
   TABLE_ENTRY(GetVertexAttribivARB),
   TABLE_ENTRY(LinkProgramARB),
   TABLE_ENTRY(ShaderSourceARB),
   TABLE_ENTRY(_dispatch_stub_513),
   TABLE_ENTRY(Uniform1fARB),
   TABLE_ENTRY(Uniform1fvARB),
   TABLE_ENTRY(Uniform1iARB),
   TABLE_ENTRY(Uniform1ivARB),
   TABLE_ENTRY(Uniform2fARB),
   TABLE_ENTRY(Uniform2fvARB),
   TABLE_ENTRY(Uniform2iARB),
   TABLE_ENTRY(Uniform2ivARB),
   TABLE_ENTRY(Uniform3fARB),
   TABLE_ENTRY(Uniform3fvARB),
   TABLE_ENTRY(Uniform3iARB),
   TABLE_ENTRY(Uniform3ivARB),
   TABLE_ENTRY(Uniform4fARB),
   TABLE_ENTRY(Uniform4fvARB),
   TABLE_ENTRY(Uniform4iARB),
   TABLE_ENTRY(Uniform4ivARB),
   TABLE_ENTRY(UniformMatrix2fvARB),
   TABLE_ENTRY(UniformMatrix3fvARB),
   TABLE_ENTRY(UniformMatrix4fvARB),
   TABLE_ENTRY(UseProgramObjectARB),
   TABLE_ENTRY(ValidateProgramARB),
   TABLE_ENTRY(VertexAttrib1dARB),
   TABLE_ENTRY(VertexAttrib1dvARB),
   TABLE_ENTRY(VertexAttrib1sARB),
   TABLE_ENTRY(VertexAttrib1svARB),
   TABLE_ENTRY(VertexAttrib2dARB),
   TABLE_ENTRY(VertexAttrib2dvARB),
   TABLE_ENTRY(VertexAttrib2sARB),
   TABLE_ENTRY(VertexAttrib2svARB),
   TABLE_ENTRY(VertexAttrib3dARB),
   TABLE_ENTRY(VertexAttrib3dvARB),
   TABLE_ENTRY(VertexAttrib3sARB),
   TABLE_ENTRY(VertexAttrib3svARB),
   TABLE_ENTRY(VertexAttrib4NbvARB),
   TABLE_ENTRY(VertexAttrib4NivARB),
   TABLE_ENTRY(VertexAttrib4NsvARB),
   TABLE_ENTRY(VertexAttrib4NubARB),
   TABLE_ENTRY(VertexAttrib4NubvARB),
   TABLE_ENTRY(VertexAttrib4NuivARB),
   TABLE_ENTRY(VertexAttrib4NusvARB),
   TABLE_ENTRY(VertexAttrib4bvARB),
   TABLE_ENTRY(VertexAttrib4dARB),
   TABLE_ENTRY(VertexAttrib4dvARB),
   TABLE_ENTRY(VertexAttrib4ivARB),
   TABLE_ENTRY(VertexAttrib4sARB),
   TABLE_ENTRY(VertexAttrib4svARB),
   TABLE_ENTRY(VertexAttrib4ubvARB),
   TABLE_ENTRY(VertexAttrib4uivARB),
   TABLE_ENTRY(VertexAttrib4usvARB),
   TABLE_ENTRY(VertexAttribPointerARB),
   TABLE_ENTRY(BeginConditionalRenderNV),
   TABLE_ENTRY(_dispatch_stub_571),
   TABLE_ENTRY(_dispatch_stub_572),
   TABLE_ENTRY(_dispatch_stub_573),
   TABLE_ENTRY(BindFragDataLocationEXT),
   TABLE_ENTRY(ClampColorARB),
   TABLE_ENTRY(ColorMaskIndexedEXT),
   TABLE_ENTRY(DisableIndexedEXT),
   TABLE_ENTRY(EnableIndexedEXT),
   TABLE_ENTRY(EndConditionalRenderNV),
   TABLE_ENTRY(_dispatch_stub_584),
   TABLE_ENTRY(GetBooleanIndexedvEXT),
   TABLE_ENTRY(GetFragDataLocationEXT),
   TABLE_ENTRY(GetIntegerIndexedvEXT),
   TABLE_ENTRY(GetTexParameterIivEXT),
   TABLE_ENTRY(GetTexParameterIuivEXT),
   TABLE_ENTRY(_dispatch_stub_591),
   TABLE_ENTRY(GetUniformuivEXT),
   TABLE_ENTRY(GetVertexAttribIivEXT),
   TABLE_ENTRY(GetVertexAttribIuivEXT),
   TABLE_ENTRY(IsEnabledIndexedEXT),
   TABLE_ENTRY(TexParameterIivEXT),
   TABLE_ENTRY(TexParameterIuivEXT),
   TABLE_ENTRY(_dispatch_stub_598),
   TABLE_ENTRY(Uniform1uiEXT),
   TABLE_ENTRY(Uniform1uivEXT),
   TABLE_ENTRY(Uniform2uiEXT),
   TABLE_ENTRY(Uniform2uivEXT),
   TABLE_ENTRY(Uniform3uiEXT),
   TABLE_ENTRY(Uniform3uivEXT),
   TABLE_ENTRY(Uniform4uiEXT),
   TABLE_ENTRY(Uniform4uivEXT),
   TABLE_ENTRY(VertexAttribI1ivEXT),
   TABLE_ENTRY(VertexAttribI1uivEXT),
   TABLE_ENTRY(VertexAttribI4bvEXT),
   TABLE_ENTRY(VertexAttribI4svEXT),
   TABLE_ENTRY(VertexAttribI4ubvEXT),
   TABLE_ENTRY(VertexAttribI4usvEXT),
   TABLE_ENTRY(VertexAttribIPointerEXT),
   TABLE_ENTRY(PrimitiveRestartIndexNV),
   TABLE_ENTRY(TexBufferARB),
   TABLE_ENTRY(_dispatch_stub_616),
   TABLE_ENTRY(VertexAttribDivisorARB),
   TABLE_ENTRY(MinSampleShadingARB),
   TABLE_ENTRY(_dispatch_stub_622),
   TABLE_ENTRY(_dispatch_stub_623),
   TABLE_ENTRY(_dispatch_stub_624),
   TABLE_ENTRY(_dispatch_stub_631),
   TABLE_ENTRY(_dispatch_stub_632),
   TABLE_ENTRY(_dispatch_stub_633),
   TABLE_ENTRY(_dispatch_stub_634),
   TABLE_ENTRY(_dispatch_stub_635),
   TABLE_ENTRY(VertexAttrib1f),
   TABLE_ENTRY(VertexAttrib1fv),
   TABLE_ENTRY(VertexAttrib2f),
   TABLE_ENTRY(VertexAttrib2fv),
   TABLE_ENTRY(VertexAttrib3f),
   TABLE_ENTRY(VertexAttrib3fv),
   TABLE_ENTRY(VertexAttrib4f),
   TABLE_ENTRY(VertexAttrib4fv),
   TABLE_ENTRY(DrawArraysInstancedEXT),
   TABLE_ENTRY(DrawArraysInstanced),
   TABLE_ENTRY(DrawElementsInstancedEXT),
   TABLE_ENTRY(DrawElementsInstanced),
   TABLE_ENTRY(_dispatch_stub_661),
   TABLE_ENTRY(_dispatch_stub_662),
   TABLE_ENTRY(_dispatch_stub_663),
   TABLE_ENTRY(CheckFramebufferStatusEXT),
   TABLE_ENTRY(_dispatch_stub_664),
   TABLE_ENTRY(DeleteFramebuffersEXT),
   TABLE_ENTRY(_dispatch_stub_665),
   TABLE_ENTRY(DeleteRenderbuffersEXT),
   TABLE_ENTRY(_dispatch_stub_666),
   TABLE_ENTRY(FramebufferRenderbufferEXT),
   TABLE_ENTRY(_dispatch_stub_667),
   TABLE_ENTRY(FramebufferTexture1DEXT),
   TABLE_ENTRY(FramebufferTexture2DEXT),
   TABLE_ENTRY(_dispatch_stub_669),
   TABLE_ENTRY(FramebufferTexture3DEXT),
   TABLE_ENTRY(_dispatch_stub_670),
   TABLE_ENTRY(FramebufferTextureLayerEXT),
   TABLE_ENTRY(GenFramebuffersEXT),
   TABLE_ENTRY(_dispatch_stub_672),
   TABLE_ENTRY(GenRenderbuffersEXT),
   TABLE_ENTRY(_dispatch_stub_673),
   TABLE_ENTRY(GenerateMipmapEXT),
   TABLE_ENTRY(_dispatch_stub_674),
   TABLE_ENTRY(GetFramebufferAttachmentParameterivEXT),
   TABLE_ENTRY(_dispatch_stub_675),
   TABLE_ENTRY(GetRenderbufferParameterivEXT),
   TABLE_ENTRY(_dispatch_stub_676),
   TABLE_ENTRY(IsFramebufferEXT),
   TABLE_ENTRY(_dispatch_stub_677),
   TABLE_ENTRY(IsRenderbufferEXT),
   TABLE_ENTRY(_dispatch_stub_678),
   TABLE_ENTRY(RenderbufferStorageEXT),
   TABLE_ENTRY(_dispatch_stub_679),
   TABLE_ENTRY(RenderbufferStorageMultisampleEXT),
   TABLE_ENTRY(_dispatch_stub_681),
   TABLE_ENTRY(_dispatch_stub_682),
   TABLE_ENTRY(_dispatch_stub_683),
   TABLE_ENTRY(_dispatch_stub_684),
   TABLE_ENTRY(_dispatch_stub_685),
   TABLE_ENTRY(_dispatch_stub_686),
   TABLE_ENTRY(_dispatch_stub_702),
   TABLE_ENTRY(_dispatch_stub_703),
   TABLE_ENTRY(_dispatch_stub_704),
   TABLE_ENTRY(_dispatch_stub_705),
   TABLE_ENTRY(ProvokingVertexEXT),
   TABLE_ENTRY(_dispatch_stub_711),
   TABLE_ENTRY(_dispatch_stub_712),
   TABLE_ENTRY(_dispatch_stub_713),
   TABLE_ENTRY(_dispatch_stub_714),
   TABLE_ENTRY(_dispatch_stub_715),
   TABLE_ENTRY(_dispatch_stub_716),
   TABLE_ENTRY(_dispatch_stub_813),
   TABLE_ENTRY(_dispatch_stub_814),
   TABLE_ENTRY(_dispatch_stub_818),
   TABLE_ENTRY(_dispatch_stub_819),
   TABLE_ENTRY(_dispatch_stub_820),
   TABLE_ENTRY(_dispatch_stub_901),
   TABLE_ENTRY(_dispatch_stub_902),
   TABLE_ENTRY(DebugMessageCallbackARB),
   TABLE_ENTRY(_dispatch_stub_1101),
   TABLE_ENTRY(DebugMessageControlARB),
   TABLE_ENTRY(_dispatch_stub_1102),
   TABLE_ENTRY(DebugMessageInsertARB),
   TABLE_ENTRY(_dispatch_stub_1103),
   TABLE_ENTRY(GetDebugMessageLogARB),
   TABLE_ENTRY(_dispatch_stub_1104),
   TABLE_ENTRY(_dispatch_stub_1105),
   TABLE_ENTRY(_dispatch_stub_1106),
   TABLE_ENTRY(_dispatch_stub_1107),
   TABLE_ENTRY(_dispatch_stub_1108),
   TABLE_ENTRY(_dispatch_stub_1109),
   TABLE_ENTRY(_dispatch_stub_1110),
   TABLE_ENTRY(SecondaryColor3f),
   TABLE_ENTRY(SecondaryColor3fv),
   TABLE_ENTRY(MultiDrawElements),
   TABLE_ENTRY(FogCoordf),
   TABLE_ENTRY(FogCoordfv),
   TABLE_ENTRY(VertexAttribI1i),
   TABLE_ENTRY(VertexAttribI1ui),
   TABLE_ENTRY(VertexAttribI2i),
   TABLE_ENTRY(VertexAttribI2iv),
   TABLE_ENTRY(VertexAttribI2ui),
   TABLE_ENTRY(VertexAttribI2uiv),
   TABLE_ENTRY(VertexAttribI3i),
   TABLE_ENTRY(VertexAttribI3iv),
   TABLE_ENTRY(VertexAttribI3ui),
   TABLE_ENTRY(VertexAttribI3uiv),
   TABLE_ENTRY(VertexAttribI4i),
   TABLE_ENTRY(VertexAttribI4iv),
   TABLE_ENTRY(VertexAttribI4ui),
   TABLE_ENTRY(VertexAttribI4uiv),
   TABLE_ENTRY(_dispatch_stub_1253),
   TABLE_ENTRY(_dispatch_stub_1280),
   TABLE_ENTRY(_dispatch_stub_1281),
   TABLE_ENTRY(_dispatch_stub_1282),
   TABLE_ENTRY(_dispatch_stub_1283),
   TABLE_ENTRY(_dispatch_stub_1284),
   TABLE_ENTRY(_dispatch_stub_1285),
   TABLE_ENTRY(_dispatch_stub_1286),
   TABLE_ENTRY(_dispatch_stub_1287),
   TABLE_ENTRY(_dispatch_stub_1288),
   TABLE_ENTRY(_dispatch_stub_1289),
   TABLE_ENTRY(_dispatch_stub_1290),
   TABLE_ENTRY(_dispatch_stub_1291),
   TABLE_ENTRY(_dispatch_stub_1292),
   TABLE_ENTRY(_dispatch_stub_1293),
   TABLE_ENTRY(_dispatch_stub_1294),
   TABLE_ENTRY(_dispatch_stub_1295),
   TABLE_ENTRY(_dispatch_stub_1296),
   TABLE_ENTRY(_dispatch_stub_1297),
   TABLE_ENTRY(_dispatch_stub_1298),
   TABLE_ENTRY(_dispatch_stub_1299),
   TABLE_ENTRY(_dispatch_stub_1300),
   TABLE_ENTRY(_dispatch_stub_1301),
   TABLE_ENTRY(_dispatch_stub_1302),
   TABLE_ENTRY(_dispatch_stub_1303),
   TABLE_ENTRY(_dispatch_stub_1304),
   TABLE_ENTRY(_dispatch_stub_1305),
   TABLE_ENTRY(_dispatch_stub_1306),
   TABLE_ENTRY(_dispatch_stub_1307),
   TABLE_ENTRY(_dispatch_stub_1308),
   TABLE_ENTRY(_dispatch_stub_1309),
   TABLE_ENTRY(_dispatch_stub_1310),
   TABLE_ENTRY(_dispatch_stub_1311),
   TABLE_ENTRY(_dispatch_stub_1312),
   TABLE_ENTRY(_dispatch_stub_1313),
   TABLE_ENTRY(_dispatch_stub_1314),
   TABLE_ENTRY(_dispatch_stub_1315),
   TABLE_ENTRY(_dispatch_stub_1316),
   TABLE_ENTRY(_dispatch_stub_1317),
   TABLE_ENTRY(_dispatch_stub_1318),
   TABLE_ENTRY(_dispatch_stub_1319),
   TABLE_ENTRY(_dispatch_stub_1320),
   TABLE_ENTRY(_dispatch_stub_1321),
   TABLE_ENTRY(_dispatch_stub_1322),
#endif /* _GLAPI_SKIP_NORMAL_ENTRY_POINTS */
#ifndef _GLAPI_SKIP_PROTO_ENTRY_POINTS
   TABLE_ENTRY(AreTexturesResidentEXT),
   TABLE_ENTRY(DeleteTexturesEXT),
   TABLE_ENTRY(GenTexturesEXT),
   TABLE_ENTRY(IsTextureEXT),
   TABLE_ENTRY(_dispatch_stub_343),
   TABLE_ENTRY(_dispatch_stub_344),
   TABLE_ENTRY(_dispatch_stub_345),
   TABLE_ENTRY(_dispatch_stub_356),
   TABLE_ENTRY(_dispatch_stub_357),
   TABLE_ENTRY(_dispatch_stub_358),
   TABLE_ENTRY(_dispatch_stub_359),
   TABLE_ENTRY(_dispatch_stub_361),
   TABLE_ENTRY(_dispatch_stub_362),
   TABLE_ENTRY(_dispatch_stub_363),
   TABLE_ENTRY(_dispatch_stub_364),
   TABLE_ENTRY(_dispatch_stub_365),
   TABLE_ENTRY(_dispatch_stub_366),
#endif /* _GLAPI_SKIP_PROTO_ENTRY_POINTS */
};
#endif /*UNUSED_TABLE_NAME*/


#  undef KEYWORD1
#  undef KEYWORD1_ALT
#  undef KEYWORD2
#  undef NAME
#  undef DISPATCH
#  undef RETURN_DISPATCH
#  undef DISPATCH_TABLE_NAME
#  undef UNUSED_TABLE_NAME
#  undef TABLE_ENTRY
#  undef HIDDEN
