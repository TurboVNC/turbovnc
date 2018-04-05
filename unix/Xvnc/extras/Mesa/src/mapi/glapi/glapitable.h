/* DO NOT EDIT - This file generated automatically by gl_table.py (from Mesa) script */

/*
 * Copyright (C) 1999-2003  Brian Paul   All Rights Reserved.
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

#if !defined( _GLAPI_TABLE_H_ )
#  define _GLAPI_TABLE_H_

#ifndef GLAPIENTRYP
# ifndef GLAPIENTRY
#  define GLAPIENTRY
# endif

# define GLAPIENTRYP GLAPIENTRY *
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct _glapi_table
{
   void (GLAPIENTRYP NewList)(GLuint list, GLenum mode); /* 0 */
   void (GLAPIENTRYP EndList)(void); /* 1 */
   void (GLAPIENTRYP CallList)(GLuint list); /* 2 */
   void (GLAPIENTRYP CallLists)(GLsizei n, GLenum type, const GLvoid * lists); /* 3 */
   void (GLAPIENTRYP DeleteLists)(GLuint list, GLsizei range); /* 4 */
   GLuint (GLAPIENTRYP GenLists)(GLsizei range); /* 5 */
   void (GLAPIENTRYP ListBase)(GLuint base); /* 6 */
   void (GLAPIENTRYP Begin)(GLenum mode); /* 7 */
   void (GLAPIENTRYP Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap); /* 8 */
   void (GLAPIENTRYP Color3b)(GLbyte red, GLbyte green, GLbyte blue); /* 9 */
   void (GLAPIENTRYP Color3bv)(const GLbyte * v); /* 10 */
   void (GLAPIENTRYP Color3d)(GLdouble red, GLdouble green, GLdouble blue); /* 11 */
   void (GLAPIENTRYP Color3dv)(const GLdouble * v); /* 12 */
   void (GLAPIENTRYP Color3f)(GLfloat red, GLfloat green, GLfloat blue); /* 13 */
   void (GLAPIENTRYP Color3fv)(const GLfloat * v); /* 14 */
   void (GLAPIENTRYP Color3i)(GLint red, GLint green, GLint blue); /* 15 */
   void (GLAPIENTRYP Color3iv)(const GLint * v); /* 16 */
   void (GLAPIENTRYP Color3s)(GLshort red, GLshort green, GLshort blue); /* 17 */
   void (GLAPIENTRYP Color3sv)(const GLshort * v); /* 18 */
   void (GLAPIENTRYP Color3ub)(GLubyte red, GLubyte green, GLubyte blue); /* 19 */
   void (GLAPIENTRYP Color3ubv)(const GLubyte * v); /* 20 */
   void (GLAPIENTRYP Color3ui)(GLuint red, GLuint green, GLuint blue); /* 21 */
   void (GLAPIENTRYP Color3uiv)(const GLuint * v); /* 22 */
   void (GLAPIENTRYP Color3us)(GLushort red, GLushort green, GLushort blue); /* 23 */
   void (GLAPIENTRYP Color3usv)(const GLushort * v); /* 24 */
   void (GLAPIENTRYP Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha); /* 25 */
   void (GLAPIENTRYP Color4bv)(const GLbyte * v); /* 26 */
   void (GLAPIENTRYP Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha); /* 27 */
   void (GLAPIENTRYP Color4dv)(const GLdouble * v); /* 28 */
   void (GLAPIENTRYP Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); /* 29 */
   void (GLAPIENTRYP Color4fv)(const GLfloat * v); /* 30 */
   void (GLAPIENTRYP Color4i)(GLint red, GLint green, GLint blue, GLint alpha); /* 31 */
   void (GLAPIENTRYP Color4iv)(const GLint * v); /* 32 */
   void (GLAPIENTRYP Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha); /* 33 */
   void (GLAPIENTRYP Color4sv)(const GLshort * v); /* 34 */
   void (GLAPIENTRYP Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha); /* 35 */
   void (GLAPIENTRYP Color4ubv)(const GLubyte * v); /* 36 */
   void (GLAPIENTRYP Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha); /* 37 */
   void (GLAPIENTRYP Color4uiv)(const GLuint * v); /* 38 */
   void (GLAPIENTRYP Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha); /* 39 */
   void (GLAPIENTRYP Color4usv)(const GLushort * v); /* 40 */
   void (GLAPIENTRYP EdgeFlag)(GLboolean flag); /* 41 */
   void (GLAPIENTRYP EdgeFlagv)(const GLboolean * flag); /* 42 */
   void (GLAPIENTRYP End)(void); /* 43 */
   void (GLAPIENTRYP Indexd)(GLdouble c); /* 44 */
   void (GLAPIENTRYP Indexdv)(const GLdouble * c); /* 45 */
   void (GLAPIENTRYP Indexf)(GLfloat c); /* 46 */
   void (GLAPIENTRYP Indexfv)(const GLfloat * c); /* 47 */
   void (GLAPIENTRYP Indexi)(GLint c); /* 48 */
   void (GLAPIENTRYP Indexiv)(const GLint * c); /* 49 */
   void (GLAPIENTRYP Indexs)(GLshort c); /* 50 */
   void (GLAPIENTRYP Indexsv)(const GLshort * c); /* 51 */
   void (GLAPIENTRYP Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz); /* 52 */
   void (GLAPIENTRYP Normal3bv)(const GLbyte * v); /* 53 */
   void (GLAPIENTRYP Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz); /* 54 */
   void (GLAPIENTRYP Normal3dv)(const GLdouble * v); /* 55 */
   void (GLAPIENTRYP Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz); /* 56 */
   void (GLAPIENTRYP Normal3fv)(const GLfloat * v); /* 57 */
   void (GLAPIENTRYP Normal3i)(GLint nx, GLint ny, GLint nz); /* 58 */
   void (GLAPIENTRYP Normal3iv)(const GLint * v); /* 59 */
   void (GLAPIENTRYP Normal3s)(GLshort nx, GLshort ny, GLshort nz); /* 60 */
   void (GLAPIENTRYP Normal3sv)(const GLshort * v); /* 61 */
   void (GLAPIENTRYP RasterPos2d)(GLdouble x, GLdouble y); /* 62 */
   void (GLAPIENTRYP RasterPos2dv)(const GLdouble * v); /* 63 */
   void (GLAPIENTRYP RasterPos2f)(GLfloat x, GLfloat y); /* 64 */
   void (GLAPIENTRYP RasterPos2fv)(const GLfloat * v); /* 65 */
   void (GLAPIENTRYP RasterPos2i)(GLint x, GLint y); /* 66 */
   void (GLAPIENTRYP RasterPos2iv)(const GLint * v); /* 67 */
   void (GLAPIENTRYP RasterPos2s)(GLshort x, GLshort y); /* 68 */
   void (GLAPIENTRYP RasterPos2sv)(const GLshort * v); /* 69 */
   void (GLAPIENTRYP RasterPos3d)(GLdouble x, GLdouble y, GLdouble z); /* 70 */
   void (GLAPIENTRYP RasterPos3dv)(const GLdouble * v); /* 71 */
   void (GLAPIENTRYP RasterPos3f)(GLfloat x, GLfloat y, GLfloat z); /* 72 */
   void (GLAPIENTRYP RasterPos3fv)(const GLfloat * v); /* 73 */
   void (GLAPIENTRYP RasterPos3i)(GLint x, GLint y, GLint z); /* 74 */
   void (GLAPIENTRYP RasterPos3iv)(const GLint * v); /* 75 */
   void (GLAPIENTRYP RasterPos3s)(GLshort x, GLshort y, GLshort z); /* 76 */
   void (GLAPIENTRYP RasterPos3sv)(const GLshort * v); /* 77 */
   void (GLAPIENTRYP RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 78 */
   void (GLAPIENTRYP RasterPos4dv)(const GLdouble * v); /* 79 */
   void (GLAPIENTRYP RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 80 */
   void (GLAPIENTRYP RasterPos4fv)(const GLfloat * v); /* 81 */
   void (GLAPIENTRYP RasterPos4i)(GLint x, GLint y, GLint z, GLint w); /* 82 */
   void (GLAPIENTRYP RasterPos4iv)(const GLint * v); /* 83 */
   void (GLAPIENTRYP RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w); /* 84 */
   void (GLAPIENTRYP RasterPos4sv)(const GLshort * v); /* 85 */
   void (GLAPIENTRYP Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2); /* 86 */
   void (GLAPIENTRYP Rectdv)(const GLdouble * v1, const GLdouble * v2); /* 87 */
   void (GLAPIENTRYP Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2); /* 88 */
   void (GLAPIENTRYP Rectfv)(const GLfloat * v1, const GLfloat * v2); /* 89 */
   void (GLAPIENTRYP Recti)(GLint x1, GLint y1, GLint x2, GLint y2); /* 90 */
   void (GLAPIENTRYP Rectiv)(const GLint * v1, const GLint * v2); /* 91 */
   void (GLAPIENTRYP Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2); /* 92 */
   void (GLAPIENTRYP Rectsv)(const GLshort * v1, const GLshort * v2); /* 93 */
   void (GLAPIENTRYP TexCoord1d)(GLdouble s); /* 94 */
   void (GLAPIENTRYP TexCoord1dv)(const GLdouble * v); /* 95 */
   void (GLAPIENTRYP TexCoord1f)(GLfloat s); /* 96 */
   void (GLAPIENTRYP TexCoord1fv)(const GLfloat * v); /* 97 */
   void (GLAPIENTRYP TexCoord1i)(GLint s); /* 98 */
   void (GLAPIENTRYP TexCoord1iv)(const GLint * v); /* 99 */
   void (GLAPIENTRYP TexCoord1s)(GLshort s); /* 100 */
   void (GLAPIENTRYP TexCoord1sv)(const GLshort * v); /* 101 */
   void (GLAPIENTRYP TexCoord2d)(GLdouble s, GLdouble t); /* 102 */
   void (GLAPIENTRYP TexCoord2dv)(const GLdouble * v); /* 103 */
   void (GLAPIENTRYP TexCoord2f)(GLfloat s, GLfloat t); /* 104 */
   void (GLAPIENTRYP TexCoord2fv)(const GLfloat * v); /* 105 */
   void (GLAPIENTRYP TexCoord2i)(GLint s, GLint t); /* 106 */
   void (GLAPIENTRYP TexCoord2iv)(const GLint * v); /* 107 */
   void (GLAPIENTRYP TexCoord2s)(GLshort s, GLshort t); /* 108 */
   void (GLAPIENTRYP TexCoord2sv)(const GLshort * v); /* 109 */
   void (GLAPIENTRYP TexCoord3d)(GLdouble s, GLdouble t, GLdouble r); /* 110 */
   void (GLAPIENTRYP TexCoord3dv)(const GLdouble * v); /* 111 */
   void (GLAPIENTRYP TexCoord3f)(GLfloat s, GLfloat t, GLfloat r); /* 112 */
   void (GLAPIENTRYP TexCoord3fv)(const GLfloat * v); /* 113 */
   void (GLAPIENTRYP TexCoord3i)(GLint s, GLint t, GLint r); /* 114 */
   void (GLAPIENTRYP TexCoord3iv)(const GLint * v); /* 115 */
   void (GLAPIENTRYP TexCoord3s)(GLshort s, GLshort t, GLshort r); /* 116 */
   void (GLAPIENTRYP TexCoord3sv)(const GLshort * v); /* 117 */
   void (GLAPIENTRYP TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q); /* 118 */
   void (GLAPIENTRYP TexCoord4dv)(const GLdouble * v); /* 119 */
   void (GLAPIENTRYP TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q); /* 120 */
   void (GLAPIENTRYP TexCoord4fv)(const GLfloat * v); /* 121 */
   void (GLAPIENTRYP TexCoord4i)(GLint s, GLint t, GLint r, GLint q); /* 122 */
   void (GLAPIENTRYP TexCoord4iv)(const GLint * v); /* 123 */
   void (GLAPIENTRYP TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q); /* 124 */
   void (GLAPIENTRYP TexCoord4sv)(const GLshort * v); /* 125 */
   void (GLAPIENTRYP Vertex2d)(GLdouble x, GLdouble y); /* 126 */
   void (GLAPIENTRYP Vertex2dv)(const GLdouble * v); /* 127 */
   void (GLAPIENTRYP Vertex2f)(GLfloat x, GLfloat y); /* 128 */
   void (GLAPIENTRYP Vertex2fv)(const GLfloat * v); /* 129 */
   void (GLAPIENTRYP Vertex2i)(GLint x, GLint y); /* 130 */
   void (GLAPIENTRYP Vertex2iv)(const GLint * v); /* 131 */
   void (GLAPIENTRYP Vertex2s)(GLshort x, GLshort y); /* 132 */
   void (GLAPIENTRYP Vertex2sv)(const GLshort * v); /* 133 */
   void (GLAPIENTRYP Vertex3d)(GLdouble x, GLdouble y, GLdouble z); /* 134 */
   void (GLAPIENTRYP Vertex3dv)(const GLdouble * v); /* 135 */
   void (GLAPIENTRYP Vertex3f)(GLfloat x, GLfloat y, GLfloat z); /* 136 */
   void (GLAPIENTRYP Vertex3fv)(const GLfloat * v); /* 137 */
   void (GLAPIENTRYP Vertex3i)(GLint x, GLint y, GLint z); /* 138 */
   void (GLAPIENTRYP Vertex3iv)(const GLint * v); /* 139 */
   void (GLAPIENTRYP Vertex3s)(GLshort x, GLshort y, GLshort z); /* 140 */
   void (GLAPIENTRYP Vertex3sv)(const GLshort * v); /* 141 */
   void (GLAPIENTRYP Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 142 */
   void (GLAPIENTRYP Vertex4dv)(const GLdouble * v); /* 143 */
   void (GLAPIENTRYP Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 144 */
   void (GLAPIENTRYP Vertex4fv)(const GLfloat * v); /* 145 */
   void (GLAPIENTRYP Vertex4i)(GLint x, GLint y, GLint z, GLint w); /* 146 */
   void (GLAPIENTRYP Vertex4iv)(const GLint * v); /* 147 */
   void (GLAPIENTRYP Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w); /* 148 */
   void (GLAPIENTRYP Vertex4sv)(const GLshort * v); /* 149 */
   void (GLAPIENTRYP ClipPlane)(GLenum plane, const GLdouble * equation); /* 150 */
   void (GLAPIENTRYP ColorMaterial)(GLenum face, GLenum mode); /* 151 */
   void (GLAPIENTRYP CullFace)(GLenum mode); /* 152 */
   void (GLAPIENTRYP Fogf)(GLenum pname, GLfloat param); /* 153 */
   void (GLAPIENTRYP Fogfv)(GLenum pname, const GLfloat * params); /* 154 */
   void (GLAPIENTRYP Fogi)(GLenum pname, GLint param); /* 155 */
   void (GLAPIENTRYP Fogiv)(GLenum pname, const GLint * params); /* 156 */
   void (GLAPIENTRYP FrontFace)(GLenum mode); /* 157 */
   void (GLAPIENTRYP Hint)(GLenum target, GLenum mode); /* 158 */
   void (GLAPIENTRYP Lightf)(GLenum light, GLenum pname, GLfloat param); /* 159 */
   void (GLAPIENTRYP Lightfv)(GLenum light, GLenum pname, const GLfloat * params); /* 160 */
   void (GLAPIENTRYP Lighti)(GLenum light, GLenum pname, GLint param); /* 161 */
   void (GLAPIENTRYP Lightiv)(GLenum light, GLenum pname, const GLint * params); /* 162 */
   void (GLAPIENTRYP LightModelf)(GLenum pname, GLfloat param); /* 163 */
   void (GLAPIENTRYP LightModelfv)(GLenum pname, const GLfloat * params); /* 164 */
   void (GLAPIENTRYP LightModeli)(GLenum pname, GLint param); /* 165 */
   void (GLAPIENTRYP LightModeliv)(GLenum pname, const GLint * params); /* 166 */
   void (GLAPIENTRYP LineStipple)(GLint factor, GLushort pattern); /* 167 */
   void (GLAPIENTRYP LineWidth)(GLfloat width); /* 168 */
   void (GLAPIENTRYP Materialf)(GLenum face, GLenum pname, GLfloat param); /* 169 */
   void (GLAPIENTRYP Materialfv)(GLenum face, GLenum pname, const GLfloat * params); /* 170 */
   void (GLAPIENTRYP Materiali)(GLenum face, GLenum pname, GLint param); /* 171 */
   void (GLAPIENTRYP Materialiv)(GLenum face, GLenum pname, const GLint * params); /* 172 */
   void (GLAPIENTRYP PointSize)(GLfloat size); /* 173 */
   void (GLAPIENTRYP PolygonMode)(GLenum face, GLenum mode); /* 174 */
   void (GLAPIENTRYP PolygonStipple)(const GLubyte * mask); /* 175 */
   void (GLAPIENTRYP Scissor)(GLint x, GLint y, GLsizei width, GLsizei height); /* 176 */
   void (GLAPIENTRYP ShadeModel)(GLenum mode); /* 177 */
   void (GLAPIENTRYP TexParameterf)(GLenum target, GLenum pname, GLfloat param); /* 178 */
   void (GLAPIENTRYP TexParameterfv)(GLenum target, GLenum pname, const GLfloat * params); /* 179 */
   void (GLAPIENTRYP TexParameteri)(GLenum target, GLenum pname, GLint param); /* 180 */
   void (GLAPIENTRYP TexParameteriv)(GLenum target, GLenum pname, const GLint * params); /* 181 */
   void (GLAPIENTRYP TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels); /* 182 */
   void (GLAPIENTRYP TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels); /* 183 */
   void (GLAPIENTRYP TexEnvf)(GLenum target, GLenum pname, GLfloat param); /* 184 */
   void (GLAPIENTRYP TexEnvfv)(GLenum target, GLenum pname, const GLfloat * params); /* 185 */
   void (GLAPIENTRYP TexEnvi)(GLenum target, GLenum pname, GLint param); /* 186 */
   void (GLAPIENTRYP TexEnviv)(GLenum target, GLenum pname, const GLint * params); /* 187 */
   void (GLAPIENTRYP TexGend)(GLenum coord, GLenum pname, GLdouble param); /* 188 */
   void (GLAPIENTRYP TexGendv)(GLenum coord, GLenum pname, const GLdouble * params); /* 189 */
   void (GLAPIENTRYP TexGenf)(GLenum coord, GLenum pname, GLfloat param); /* 190 */
   void (GLAPIENTRYP TexGenfv)(GLenum coord, GLenum pname, const GLfloat * params); /* 191 */
   void (GLAPIENTRYP TexGeni)(GLenum coord, GLenum pname, GLint param); /* 192 */
   void (GLAPIENTRYP TexGeniv)(GLenum coord, GLenum pname, const GLint * params); /* 193 */
   void (GLAPIENTRYP FeedbackBuffer)(GLsizei size, GLenum type, GLfloat * buffer); /* 194 */
   void (GLAPIENTRYP SelectBuffer)(GLsizei size, GLuint * buffer); /* 195 */
   GLint (GLAPIENTRYP RenderMode)(GLenum mode); /* 196 */
   void (GLAPIENTRYP InitNames)(void); /* 197 */
   void (GLAPIENTRYP LoadName)(GLuint name); /* 198 */
   void (GLAPIENTRYP PassThrough)(GLfloat token); /* 199 */
   void (GLAPIENTRYP PopName)(void); /* 200 */
   void (GLAPIENTRYP PushName)(GLuint name); /* 201 */
   void (GLAPIENTRYP DrawBuffer)(GLenum mode); /* 202 */
   void (GLAPIENTRYP Clear)(GLbitfield mask); /* 203 */
   void (GLAPIENTRYP ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha); /* 204 */
   void (GLAPIENTRYP ClearIndex)(GLfloat c); /* 205 */
   void (GLAPIENTRYP ClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha); /* 206 */
   void (GLAPIENTRYP ClearStencil)(GLint s); /* 207 */
   void (GLAPIENTRYP ClearDepth)(GLclampd depth); /* 208 */
   void (GLAPIENTRYP StencilMask)(GLuint mask); /* 209 */
   void (GLAPIENTRYP ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha); /* 210 */
   void (GLAPIENTRYP DepthMask)(GLboolean flag); /* 211 */
   void (GLAPIENTRYP IndexMask)(GLuint mask); /* 212 */
   void (GLAPIENTRYP Accum)(GLenum op, GLfloat value); /* 213 */
   void (GLAPIENTRYP Disable)(GLenum cap); /* 214 */
   void (GLAPIENTRYP Enable)(GLenum cap); /* 215 */
   void (GLAPIENTRYP Finish)(void); /* 216 */
   void (GLAPIENTRYP Flush)(void); /* 217 */
   void (GLAPIENTRYP PopAttrib)(void); /* 218 */
   void (GLAPIENTRYP PushAttrib)(GLbitfield mask); /* 219 */
   void (GLAPIENTRYP Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points); /* 220 */
   void (GLAPIENTRYP Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points); /* 221 */
   void (GLAPIENTRYP Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points); /* 222 */
   void (GLAPIENTRYP Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points); /* 223 */
   void (GLAPIENTRYP MapGrid1d)(GLint un, GLdouble u1, GLdouble u2); /* 224 */
   void (GLAPIENTRYP MapGrid1f)(GLint un, GLfloat u1, GLfloat u2); /* 225 */
   void (GLAPIENTRYP MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2); /* 226 */
   void (GLAPIENTRYP MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2); /* 227 */
   void (GLAPIENTRYP EvalCoord1d)(GLdouble u); /* 228 */
   void (GLAPIENTRYP EvalCoord1dv)(const GLdouble * u); /* 229 */
   void (GLAPIENTRYP EvalCoord1f)(GLfloat u); /* 230 */
   void (GLAPIENTRYP EvalCoord1fv)(const GLfloat * u); /* 231 */
   void (GLAPIENTRYP EvalCoord2d)(GLdouble u, GLdouble v); /* 232 */
   void (GLAPIENTRYP EvalCoord2dv)(const GLdouble * u); /* 233 */
   void (GLAPIENTRYP EvalCoord2f)(GLfloat u, GLfloat v); /* 234 */
   void (GLAPIENTRYP EvalCoord2fv)(const GLfloat * u); /* 235 */
   void (GLAPIENTRYP EvalMesh1)(GLenum mode, GLint i1, GLint i2); /* 236 */
   void (GLAPIENTRYP EvalPoint1)(GLint i); /* 237 */
   void (GLAPIENTRYP EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2); /* 238 */
   void (GLAPIENTRYP EvalPoint2)(GLint i, GLint j); /* 239 */
   void (GLAPIENTRYP AlphaFunc)(GLenum func, GLclampf ref); /* 240 */
   void (GLAPIENTRYP BlendFunc)(GLenum sfactor, GLenum dfactor); /* 241 */
   void (GLAPIENTRYP LogicOp)(GLenum opcode); /* 242 */
   void (GLAPIENTRYP StencilFunc)(GLenum func, GLint ref, GLuint mask); /* 243 */
   void (GLAPIENTRYP StencilOp)(GLenum fail, GLenum zfail, GLenum zpass); /* 244 */
   void (GLAPIENTRYP DepthFunc)(GLenum func); /* 245 */
   void (GLAPIENTRYP PixelZoom)(GLfloat xfactor, GLfloat yfactor); /* 246 */
   void (GLAPIENTRYP PixelTransferf)(GLenum pname, GLfloat param); /* 247 */
   void (GLAPIENTRYP PixelTransferi)(GLenum pname, GLint param); /* 248 */
   void (GLAPIENTRYP PixelStoref)(GLenum pname, GLfloat param); /* 249 */
   void (GLAPIENTRYP PixelStorei)(GLenum pname, GLint param); /* 250 */
   void (GLAPIENTRYP PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat * values); /* 251 */
   void (GLAPIENTRYP PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint * values); /* 252 */
   void (GLAPIENTRYP PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort * values); /* 253 */
   void (GLAPIENTRYP ReadBuffer)(GLenum mode); /* 254 */
   void (GLAPIENTRYP CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type); /* 255 */
   void (GLAPIENTRYP ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels); /* 256 */
   void (GLAPIENTRYP DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels); /* 257 */
   void (GLAPIENTRYP GetBooleanv)(GLenum pname, GLboolean * params); /* 258 */
   void (GLAPIENTRYP GetClipPlane)(GLenum plane, GLdouble * equation); /* 259 */
   void (GLAPIENTRYP GetDoublev)(GLenum pname, GLdouble * params); /* 260 */
   GLenum (GLAPIENTRYP GetError)(void); /* 261 */
   void (GLAPIENTRYP GetFloatv)(GLenum pname, GLfloat * params); /* 262 */
   void (GLAPIENTRYP GetIntegerv)(GLenum pname, GLint * params); /* 263 */
   void (GLAPIENTRYP GetLightfv)(GLenum light, GLenum pname, GLfloat * params); /* 264 */
   void (GLAPIENTRYP GetLightiv)(GLenum light, GLenum pname, GLint * params); /* 265 */
   void (GLAPIENTRYP GetMapdv)(GLenum target, GLenum query, GLdouble * v); /* 266 */
   void (GLAPIENTRYP GetMapfv)(GLenum target, GLenum query, GLfloat * v); /* 267 */
   void (GLAPIENTRYP GetMapiv)(GLenum target, GLenum query, GLint * v); /* 268 */
   void (GLAPIENTRYP GetMaterialfv)(GLenum face, GLenum pname, GLfloat * params); /* 269 */
   void (GLAPIENTRYP GetMaterialiv)(GLenum face, GLenum pname, GLint * params); /* 270 */
   void (GLAPIENTRYP GetPixelMapfv)(GLenum map, GLfloat * values); /* 271 */
   void (GLAPIENTRYP GetPixelMapuiv)(GLenum map, GLuint * values); /* 272 */
   void (GLAPIENTRYP GetPixelMapusv)(GLenum map, GLushort * values); /* 273 */
   void (GLAPIENTRYP GetPolygonStipple)(GLubyte * mask); /* 274 */
   const GLubyte * (GLAPIENTRYP GetString)(GLenum name); /* 275 */
   void (GLAPIENTRYP GetTexEnvfv)(GLenum target, GLenum pname, GLfloat * params); /* 276 */
   void (GLAPIENTRYP GetTexEnviv)(GLenum target, GLenum pname, GLint * params); /* 277 */
   void (GLAPIENTRYP GetTexGendv)(GLenum coord, GLenum pname, GLdouble * params); /* 278 */
   void (GLAPIENTRYP GetTexGenfv)(GLenum coord, GLenum pname, GLfloat * params); /* 279 */
   void (GLAPIENTRYP GetTexGeniv)(GLenum coord, GLenum pname, GLint * params); /* 280 */
   void (GLAPIENTRYP GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels); /* 281 */
   void (GLAPIENTRYP GetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params); /* 282 */
   void (GLAPIENTRYP GetTexParameteriv)(GLenum target, GLenum pname, GLint * params); /* 283 */
   void (GLAPIENTRYP GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params); /* 284 */
   void (GLAPIENTRYP GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params); /* 285 */
   GLboolean (GLAPIENTRYP IsEnabled)(GLenum cap); /* 286 */
   GLboolean (GLAPIENTRYP IsList)(GLuint list); /* 287 */
   void (GLAPIENTRYP DepthRange)(GLclampd zNear, GLclampd zFar); /* 288 */
   void (GLAPIENTRYP Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar); /* 289 */
   void (GLAPIENTRYP LoadIdentity)(void); /* 290 */
   void (GLAPIENTRYP LoadMatrixf)(const GLfloat * m); /* 291 */
   void (GLAPIENTRYP LoadMatrixd)(const GLdouble * m); /* 292 */
   void (GLAPIENTRYP MatrixMode)(GLenum mode); /* 293 */
   void (GLAPIENTRYP MultMatrixf)(const GLfloat * m); /* 294 */
   void (GLAPIENTRYP MultMatrixd)(const GLdouble * m); /* 295 */
   void (GLAPIENTRYP Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar); /* 296 */
   void (GLAPIENTRYP PopMatrix)(void); /* 297 */
   void (GLAPIENTRYP PushMatrix)(void); /* 298 */
   void (GLAPIENTRYP Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z); /* 299 */
   void (GLAPIENTRYP Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z); /* 300 */
   void (GLAPIENTRYP Scaled)(GLdouble x, GLdouble y, GLdouble z); /* 301 */
   void (GLAPIENTRYP Scalef)(GLfloat x, GLfloat y, GLfloat z); /* 302 */
   void (GLAPIENTRYP Translated)(GLdouble x, GLdouble y, GLdouble z); /* 303 */
   void (GLAPIENTRYP Translatef)(GLfloat x, GLfloat y, GLfloat z); /* 304 */
   void (GLAPIENTRYP Viewport)(GLint x, GLint y, GLsizei width, GLsizei height); /* 305 */
   void (GLAPIENTRYP ArrayElement)(GLint i); /* 306 */
   void (GLAPIENTRYP BindTexture)(GLenum target, GLuint texture); /* 307 */
   void (GLAPIENTRYP ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 308 */
   void (GLAPIENTRYP DisableClientState)(GLenum array); /* 309 */
   void (GLAPIENTRYP DrawArrays)(GLenum mode, GLint first, GLsizei count); /* 310 */
   void (GLAPIENTRYP DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices); /* 311 */
   void (GLAPIENTRYP EdgeFlagPointer)(GLsizei stride, const GLvoid * pointer); /* 312 */
   void (GLAPIENTRYP EnableClientState)(GLenum array); /* 313 */
   void (GLAPIENTRYP IndexPointer)(GLenum type, GLsizei stride, const GLvoid * pointer); /* 314 */
   void (GLAPIENTRYP Indexub)(GLubyte c); /* 315 */
   void (GLAPIENTRYP Indexubv)(const GLubyte * c); /* 316 */
   void (GLAPIENTRYP InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid * pointer); /* 317 */
   void (GLAPIENTRYP NormalPointer)(GLenum type, GLsizei stride, const GLvoid * pointer); /* 318 */
   void (GLAPIENTRYP PolygonOffset)(GLfloat factor, GLfloat units); /* 319 */
   void (GLAPIENTRYP TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 320 */
   void (GLAPIENTRYP VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 321 */
   GLboolean (GLAPIENTRYP AreTexturesResident)(GLsizei n, const GLuint * textures, GLboolean * residences); /* 322 */
   void (GLAPIENTRYP CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border); /* 323 */
   void (GLAPIENTRYP CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border); /* 324 */
   void (GLAPIENTRYP CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width); /* 325 */
   void (GLAPIENTRYP CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height); /* 326 */
   void (GLAPIENTRYP DeleteTextures)(GLsizei n, const GLuint * textures); /* 327 */
   void (GLAPIENTRYP GenTextures)(GLsizei n, GLuint * textures); /* 328 */
   void (GLAPIENTRYP GetPointerv)(GLenum pname, GLvoid ** params); /* 329 */
   GLboolean (GLAPIENTRYP IsTexture)(GLuint texture); /* 330 */
   void (GLAPIENTRYP PrioritizeTextures)(GLsizei n, const GLuint * textures, const GLclampf * priorities); /* 331 */
   void (GLAPIENTRYP TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels); /* 332 */
   void (GLAPIENTRYP TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels); /* 333 */
   void (GLAPIENTRYP PopClientAttrib)(void); /* 334 */
   void (GLAPIENTRYP PushClientAttrib)(GLbitfield mask); /* 335 */
   void (GLAPIENTRYP BlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha); /* 336 */
   void (GLAPIENTRYP BlendEquation)(GLenum mode); /* 337 */
   void (GLAPIENTRYP DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices); /* 338 */
   void (GLAPIENTRYP ColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table); /* 339 */
   void (GLAPIENTRYP ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat * params); /* 340 */
   void (GLAPIENTRYP ColorTableParameteriv)(GLenum target, GLenum pname, const GLint * params); /* 341 */
   void (GLAPIENTRYP CopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width); /* 342 */
   void (GLAPIENTRYP GetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid * table); /* 343 */
   void (GLAPIENTRYP GetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat * params); /* 344 */
   void (GLAPIENTRYP GetColorTableParameteriv)(GLenum target, GLenum pname, GLint * params); /* 345 */
   void (GLAPIENTRYP ColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data); /* 346 */
   void (GLAPIENTRYP CopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width); /* 347 */
   void (GLAPIENTRYP ConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image); /* 348 */
   void (GLAPIENTRYP ConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image); /* 349 */
   void (GLAPIENTRYP ConvolutionParameterf)(GLenum target, GLenum pname, GLfloat params); /* 350 */
   void (GLAPIENTRYP ConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat * params); /* 351 */
   void (GLAPIENTRYP ConvolutionParameteri)(GLenum target, GLenum pname, GLint params); /* 352 */
   void (GLAPIENTRYP ConvolutionParameteriv)(GLenum target, GLenum pname, const GLint * params); /* 353 */
   void (GLAPIENTRYP CopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width); /* 354 */
   void (GLAPIENTRYP CopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height); /* 355 */
   void (GLAPIENTRYP GetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid * image); /* 356 */
   void (GLAPIENTRYP GetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat * params); /* 357 */
   void (GLAPIENTRYP GetConvolutionParameteriv)(GLenum target, GLenum pname, GLint * params); /* 358 */
   void (GLAPIENTRYP GetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span); /* 359 */
   void (GLAPIENTRYP SeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column); /* 360 */
   void (GLAPIENTRYP GetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values); /* 361 */
   void (GLAPIENTRYP GetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat * params); /* 362 */
   void (GLAPIENTRYP GetHistogramParameteriv)(GLenum target, GLenum pname, GLint * params); /* 363 */
   void (GLAPIENTRYP GetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values); /* 364 */
   void (GLAPIENTRYP GetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat * params); /* 365 */
   void (GLAPIENTRYP GetMinmaxParameteriv)(GLenum target, GLenum pname, GLint * params); /* 366 */
   void (GLAPIENTRYP Histogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink); /* 367 */
   void (GLAPIENTRYP Minmax)(GLenum target, GLenum internalformat, GLboolean sink); /* 368 */
   void (GLAPIENTRYP ResetHistogram)(GLenum target); /* 369 */
   void (GLAPIENTRYP ResetMinmax)(GLenum target); /* 370 */
   void (GLAPIENTRYP TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels); /* 371 */
   void (GLAPIENTRYP TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels); /* 372 */
   void (GLAPIENTRYP CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height); /* 373 */
   void (GLAPIENTRYP ActiveTexture)(GLenum texture); /* 374 */
   void (GLAPIENTRYP ClientActiveTexture)(GLenum texture); /* 375 */
   void (GLAPIENTRYP MultiTexCoord1d)(GLenum target, GLdouble s); /* 376 */
   void (GLAPIENTRYP MultiTexCoord1dv)(GLenum target, const GLdouble * v); /* 377 */
   void (GLAPIENTRYP MultiTexCoord1fARB)(GLenum target, GLfloat s); /* 378 */
   void (GLAPIENTRYP MultiTexCoord1fvARB)(GLenum target, const GLfloat * v); /* 379 */
   void (GLAPIENTRYP MultiTexCoord1i)(GLenum target, GLint s); /* 380 */
   void (GLAPIENTRYP MultiTexCoord1iv)(GLenum target, const GLint * v); /* 381 */
   void (GLAPIENTRYP MultiTexCoord1s)(GLenum target, GLshort s); /* 382 */
   void (GLAPIENTRYP MultiTexCoord1sv)(GLenum target, const GLshort * v); /* 383 */
   void (GLAPIENTRYP MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t); /* 384 */
   void (GLAPIENTRYP MultiTexCoord2dv)(GLenum target, const GLdouble * v); /* 385 */
   void (GLAPIENTRYP MultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t); /* 386 */
   void (GLAPIENTRYP MultiTexCoord2fvARB)(GLenum target, const GLfloat * v); /* 387 */
   void (GLAPIENTRYP MultiTexCoord2i)(GLenum target, GLint s, GLint t); /* 388 */
   void (GLAPIENTRYP MultiTexCoord2iv)(GLenum target, const GLint * v); /* 389 */
   void (GLAPIENTRYP MultiTexCoord2s)(GLenum target, GLshort s, GLshort t); /* 390 */
   void (GLAPIENTRYP MultiTexCoord2sv)(GLenum target, const GLshort * v); /* 391 */
   void (GLAPIENTRYP MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r); /* 392 */
   void (GLAPIENTRYP MultiTexCoord3dv)(GLenum target, const GLdouble * v); /* 393 */
   void (GLAPIENTRYP MultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r); /* 394 */
   void (GLAPIENTRYP MultiTexCoord3fvARB)(GLenum target, const GLfloat * v); /* 395 */
   void (GLAPIENTRYP MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r); /* 396 */
   void (GLAPIENTRYP MultiTexCoord3iv)(GLenum target, const GLint * v); /* 397 */
   void (GLAPIENTRYP MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r); /* 398 */
   void (GLAPIENTRYP MultiTexCoord3sv)(GLenum target, const GLshort * v); /* 399 */
   void (GLAPIENTRYP MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q); /* 400 */
   void (GLAPIENTRYP MultiTexCoord4dv)(GLenum target, const GLdouble * v); /* 401 */
   void (GLAPIENTRYP MultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q); /* 402 */
   void (GLAPIENTRYP MultiTexCoord4fvARB)(GLenum target, const GLfloat * v); /* 403 */
   void (GLAPIENTRYP MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q); /* 404 */
   void (GLAPIENTRYP MultiTexCoord4iv)(GLenum target, const GLint * v); /* 405 */
   void (GLAPIENTRYP MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q); /* 406 */
   void (GLAPIENTRYP MultiTexCoord4sv)(GLenum target, const GLshort * v); /* 407 */
   void (GLAPIENTRYP CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data); /* 408 */
   void (GLAPIENTRYP CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data); /* 409 */
   void (GLAPIENTRYP CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data); /* 410 */
   void (GLAPIENTRYP CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data); /* 411 */
   void (GLAPIENTRYP CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data); /* 412 */
   void (GLAPIENTRYP CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data); /* 413 */
   void (GLAPIENTRYP GetCompressedTexImage)(GLenum target, GLint level, GLvoid * img); /* 414 */
   void (GLAPIENTRYP LoadTransposeMatrixd)(const GLdouble * m); /* 415 */
   void (GLAPIENTRYP LoadTransposeMatrixf)(const GLfloat * m); /* 416 */
   void (GLAPIENTRYP MultTransposeMatrixd)(const GLdouble * m); /* 417 */
   void (GLAPIENTRYP MultTransposeMatrixf)(const GLfloat * m); /* 418 */
   void (GLAPIENTRYP SampleCoverage)(GLclampf value, GLboolean invert); /* 419 */
   void (GLAPIENTRYP BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha); /* 420 */
   void (GLAPIENTRYP FogCoordPointer)(GLenum type, GLsizei stride, const GLvoid * pointer); /* 421 */
   void (GLAPIENTRYP FogCoordd)(GLdouble coord); /* 422 */
   void (GLAPIENTRYP FogCoorddv)(const GLdouble * coord); /* 423 */
   void (GLAPIENTRYP MultiDrawArrays)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount); /* 424 */
   void (GLAPIENTRYP PointParameterf)(GLenum pname, GLfloat param); /* 425 */
   void (GLAPIENTRYP PointParameterfv)(GLenum pname, const GLfloat * params); /* 426 */
   void (GLAPIENTRYP PointParameteri)(GLenum pname, GLint param); /* 427 */
   void (GLAPIENTRYP PointParameteriv)(GLenum pname, const GLint * params); /* 428 */
   void (GLAPIENTRYP SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue); /* 429 */
   void (GLAPIENTRYP SecondaryColor3bv)(const GLbyte * v); /* 430 */
   void (GLAPIENTRYP SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue); /* 431 */
   void (GLAPIENTRYP SecondaryColor3dv)(const GLdouble * v); /* 432 */
   void (GLAPIENTRYP SecondaryColor3i)(GLint red, GLint green, GLint blue); /* 433 */
   void (GLAPIENTRYP SecondaryColor3iv)(const GLint * v); /* 434 */
   void (GLAPIENTRYP SecondaryColor3s)(GLshort red, GLshort green, GLshort blue); /* 435 */
   void (GLAPIENTRYP SecondaryColor3sv)(const GLshort * v); /* 436 */
   void (GLAPIENTRYP SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue); /* 437 */
   void (GLAPIENTRYP SecondaryColor3ubv)(const GLubyte * v); /* 438 */
   void (GLAPIENTRYP SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue); /* 439 */
   void (GLAPIENTRYP SecondaryColor3uiv)(const GLuint * v); /* 440 */
   void (GLAPIENTRYP SecondaryColor3us)(GLushort red, GLushort green, GLushort blue); /* 441 */
   void (GLAPIENTRYP SecondaryColor3usv)(const GLushort * v); /* 442 */
   void (GLAPIENTRYP SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 443 */
   void (GLAPIENTRYP WindowPos2d)(GLdouble x, GLdouble y); /* 444 */
   void (GLAPIENTRYP WindowPos2dv)(const GLdouble * v); /* 445 */
   void (GLAPIENTRYP WindowPos2f)(GLfloat x, GLfloat y); /* 446 */
   void (GLAPIENTRYP WindowPos2fv)(const GLfloat * v); /* 447 */
   void (GLAPIENTRYP WindowPos2i)(GLint x, GLint y); /* 448 */
   void (GLAPIENTRYP WindowPos2iv)(const GLint * v); /* 449 */
   void (GLAPIENTRYP WindowPos2s)(GLshort x, GLshort y); /* 450 */
   void (GLAPIENTRYP WindowPos2sv)(const GLshort * v); /* 451 */
   void (GLAPIENTRYP WindowPos3d)(GLdouble x, GLdouble y, GLdouble z); /* 452 */
   void (GLAPIENTRYP WindowPos3dv)(const GLdouble * v); /* 453 */
   void (GLAPIENTRYP WindowPos3f)(GLfloat x, GLfloat y, GLfloat z); /* 454 */
   void (GLAPIENTRYP WindowPos3fv)(const GLfloat * v); /* 455 */
   void (GLAPIENTRYP WindowPos3i)(GLint x, GLint y, GLint z); /* 456 */
   void (GLAPIENTRYP WindowPos3iv)(const GLint * v); /* 457 */
   void (GLAPIENTRYP WindowPos3s)(GLshort x, GLshort y, GLshort z); /* 458 */
   void (GLAPIENTRYP WindowPos3sv)(const GLshort * v); /* 459 */
   void (GLAPIENTRYP BeginQuery)(GLenum target, GLuint id); /* 460 */
   void (GLAPIENTRYP BindBuffer)(GLenum target, GLuint buffer); /* 461 */
   void (GLAPIENTRYP BufferData)(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage); /* 462 */
   void (GLAPIENTRYP BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data); /* 463 */
   void (GLAPIENTRYP DeleteBuffers)(GLsizei n, const GLuint * buffer); /* 464 */
   void (GLAPIENTRYP DeleteQueries)(GLsizei n, const GLuint * ids); /* 465 */
   void (GLAPIENTRYP EndQuery)(GLenum target); /* 466 */
   void (GLAPIENTRYP GenBuffers)(GLsizei n, GLuint * buffer); /* 467 */
   void (GLAPIENTRYP GenQueries)(GLsizei n, GLuint * ids); /* 468 */
   void (GLAPIENTRYP GetBufferParameteriv)(GLenum target, GLenum pname, GLint * params); /* 469 */
   void (GLAPIENTRYP GetBufferPointerv)(GLenum target, GLenum pname, GLvoid ** params); /* 470 */
   void (GLAPIENTRYP GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data); /* 471 */
   void (GLAPIENTRYP GetQueryObjectiv)(GLuint id, GLenum pname, GLint * params); /* 472 */
   void (GLAPIENTRYP GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params); /* 473 */
   void (GLAPIENTRYP GetQueryiv)(GLenum target, GLenum pname, GLint * params); /* 474 */
   GLboolean (GLAPIENTRYP IsBuffer)(GLuint buffer); /* 475 */
   GLboolean (GLAPIENTRYP IsQuery)(GLuint id); /* 476 */
   GLvoid * (GLAPIENTRYP MapBuffer)(GLenum target, GLenum access); /* 477 */
   GLboolean (GLAPIENTRYP UnmapBuffer)(GLenum target); /* 478 */
   void (GLAPIENTRYP AttachShader)(GLuint program, GLuint shader); /* 479 */
   void (GLAPIENTRYP BindAttribLocation)(GLuint program, GLuint index, const GLchar * name); /* 480 */
   void (GLAPIENTRYP BlendEquationSeparate)(GLenum modeRGB, GLenum modeA); /* 481 */
   void (GLAPIENTRYP CompileShader)(GLuint shader); /* 482 */
   GLuint (GLAPIENTRYP CreateProgram)(void); /* 483 */
   GLuint (GLAPIENTRYP CreateShader)(GLenum type); /* 484 */
   void (GLAPIENTRYP DeleteProgram)(GLuint program); /* 485 */
   void (GLAPIENTRYP DeleteShader)(GLuint program); /* 486 */
   void (GLAPIENTRYP DetachShader)(GLuint program, GLuint shader); /* 487 */
   void (GLAPIENTRYP DisableVertexAttribArray)(GLuint index); /* 488 */
   void (GLAPIENTRYP DrawBuffers)(GLsizei n, const GLenum * bufs); /* 489 */
   void (GLAPIENTRYP EnableVertexAttribArray)(GLuint index); /* 490 */
   void (GLAPIENTRYP GetActiveAttrib)(GLuint program, GLuint index, GLsizei  bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name); /* 491 */
   void (GLAPIENTRYP GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name); /* 492 */
   void (GLAPIENTRYP GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * obj); /* 493 */
   GLint (GLAPIENTRYP GetAttribLocation)(GLuint program, const GLchar * name); /* 494 */
   void (GLAPIENTRYP GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog); /* 495 */
   void (GLAPIENTRYP GetProgramiv)(GLuint program, GLenum pname, GLint * params); /* 496 */
   void (GLAPIENTRYP GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog); /* 497 */
   void (GLAPIENTRYP GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source); /* 498 */
   void (GLAPIENTRYP GetShaderiv)(GLuint shader, GLenum pname, GLint * params); /* 499 */
   GLint (GLAPIENTRYP GetUniformLocation)(GLuint program, const GLchar * name); /* 500 */
   void (GLAPIENTRYP GetUniformfv)(GLuint program, GLint location, GLfloat * params); /* 501 */
   void (GLAPIENTRYP GetUniformiv)(GLuint program, GLint location, GLint * params); /* 502 */
   void (GLAPIENTRYP GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid ** pointer); /* 503 */
   void (GLAPIENTRYP GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble * params); /* 504 */
   void (GLAPIENTRYP GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat * params); /* 505 */
   void (GLAPIENTRYP GetVertexAttribiv)(GLuint index, GLenum pname, GLint * params); /* 506 */
   GLboolean (GLAPIENTRYP IsProgram)(GLuint program); /* 507 */
   GLboolean (GLAPIENTRYP IsShader)(GLuint shader); /* 508 */
   void (GLAPIENTRYP LinkProgram)(GLuint program); /* 509 */
   void (GLAPIENTRYP ShaderSource)(GLuint shader, GLsizei count, const GLchar * const * string, const GLint * length); /* 510 */
   void (GLAPIENTRYP StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask); /* 511 */
   void (GLAPIENTRYP StencilMaskSeparate)(GLenum face, GLuint mask); /* 512 */
   void (GLAPIENTRYP StencilOpSeparate)(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass); /* 513 */
   void (GLAPIENTRYP Uniform1f)(GLint location, GLfloat v0); /* 514 */
   void (GLAPIENTRYP Uniform1fv)(GLint location, GLsizei count, const GLfloat * value); /* 515 */
   void (GLAPIENTRYP Uniform1i)(GLint location, GLint v0); /* 516 */
   void (GLAPIENTRYP Uniform1iv)(GLint location, GLsizei count, const GLint * value); /* 517 */
   void (GLAPIENTRYP Uniform2f)(GLint location, GLfloat v0, GLfloat v1); /* 518 */
   void (GLAPIENTRYP Uniform2fv)(GLint location, GLsizei count, const GLfloat * value); /* 519 */
   void (GLAPIENTRYP Uniform2i)(GLint location, GLint v0, GLint v1); /* 520 */
   void (GLAPIENTRYP Uniform2iv)(GLint location, GLsizei count, const GLint * value); /* 521 */
   void (GLAPIENTRYP Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2); /* 522 */
   void (GLAPIENTRYP Uniform3fv)(GLint location, GLsizei count, const GLfloat * value); /* 523 */
   void (GLAPIENTRYP Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2); /* 524 */
   void (GLAPIENTRYP Uniform3iv)(GLint location, GLsizei count, const GLint * value); /* 525 */
   void (GLAPIENTRYP Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3); /* 526 */
   void (GLAPIENTRYP Uniform4fv)(GLint location, GLsizei count, const GLfloat * value); /* 527 */
   void (GLAPIENTRYP Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3); /* 528 */
   void (GLAPIENTRYP Uniform4iv)(GLint location, GLsizei count, const GLint * value); /* 529 */
   void (GLAPIENTRYP UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 530 */
   void (GLAPIENTRYP UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 531 */
   void (GLAPIENTRYP UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 532 */
   void (GLAPIENTRYP UseProgram)(GLuint program); /* 533 */
   void (GLAPIENTRYP ValidateProgram)(GLuint program); /* 534 */
   void (GLAPIENTRYP VertexAttrib1d)(GLuint index, GLdouble x); /* 535 */
   void (GLAPIENTRYP VertexAttrib1dv)(GLuint index, const GLdouble * v); /* 536 */
   void (GLAPIENTRYP VertexAttrib1s)(GLuint index, GLshort x); /* 537 */
   void (GLAPIENTRYP VertexAttrib1sv)(GLuint index, const GLshort * v); /* 538 */
   void (GLAPIENTRYP VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y); /* 539 */
   void (GLAPIENTRYP VertexAttrib2dv)(GLuint index, const GLdouble * v); /* 540 */
   void (GLAPIENTRYP VertexAttrib2s)(GLuint index, GLshort x, GLshort y); /* 541 */
   void (GLAPIENTRYP VertexAttrib2sv)(GLuint index, const GLshort * v); /* 542 */
   void (GLAPIENTRYP VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z); /* 543 */
   void (GLAPIENTRYP VertexAttrib3dv)(GLuint index, const GLdouble * v); /* 544 */
   void (GLAPIENTRYP VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z); /* 545 */
   void (GLAPIENTRYP VertexAttrib3sv)(GLuint index, const GLshort * v); /* 546 */
   void (GLAPIENTRYP VertexAttrib4Nbv)(GLuint index, const GLbyte * v); /* 547 */
   void (GLAPIENTRYP VertexAttrib4Niv)(GLuint index, const GLint * v); /* 548 */
   void (GLAPIENTRYP VertexAttrib4Nsv)(GLuint index, const GLshort * v); /* 549 */
   void (GLAPIENTRYP VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w); /* 550 */
   void (GLAPIENTRYP VertexAttrib4Nubv)(GLuint index, const GLubyte * v); /* 551 */
   void (GLAPIENTRYP VertexAttrib4Nuiv)(GLuint index, const GLuint * v); /* 552 */
   void (GLAPIENTRYP VertexAttrib4Nusv)(GLuint index, const GLushort * v); /* 553 */
   void (GLAPIENTRYP VertexAttrib4bv)(GLuint index, const GLbyte * v); /* 554 */
   void (GLAPIENTRYP VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 555 */
   void (GLAPIENTRYP VertexAttrib4dv)(GLuint index, const GLdouble * v); /* 556 */
   void (GLAPIENTRYP VertexAttrib4iv)(GLuint index, const GLint * v); /* 557 */
   void (GLAPIENTRYP VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w); /* 558 */
   void (GLAPIENTRYP VertexAttrib4sv)(GLuint index, const GLshort * v); /* 559 */
   void (GLAPIENTRYP VertexAttrib4ubv)(GLuint index, const GLubyte * v); /* 560 */
   void (GLAPIENTRYP VertexAttrib4uiv)(GLuint index, const GLuint * v); /* 561 */
   void (GLAPIENTRYP VertexAttrib4usv)(GLuint index, const GLushort * v); /* 562 */
   void (GLAPIENTRYP VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer); /* 563 */
   void (GLAPIENTRYP UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 564 */
   void (GLAPIENTRYP UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 565 */
   void (GLAPIENTRYP UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 566 */
   void (GLAPIENTRYP UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 567 */
   void (GLAPIENTRYP UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 568 */
   void (GLAPIENTRYP UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 569 */
   void (GLAPIENTRYP BeginConditionalRender)(GLuint query, GLenum mode); /* 570 */
   void (GLAPIENTRYP BeginTransformFeedback)(GLenum mode); /* 571 */
   void (GLAPIENTRYP BindBufferBase)(GLenum target, GLuint index, GLuint buffer); /* 572 */
   void (GLAPIENTRYP BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size); /* 573 */
   void (GLAPIENTRYP BindFragDataLocation)(GLuint program, GLuint colorNumber, const GLchar * name); /* 574 */
   void (GLAPIENTRYP ClampColor)(GLenum target, GLenum clamp); /* 575 */
   void (GLAPIENTRYP ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil); /* 576 */
   void (GLAPIENTRYP ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat * value); /* 577 */
   void (GLAPIENTRYP ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint * value); /* 578 */
   void (GLAPIENTRYP ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint * value); /* 579 */
   void (GLAPIENTRYP ColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a); /* 580 */
   void (GLAPIENTRYP Disablei)(GLenum target, GLuint index); /* 581 */
   void (GLAPIENTRYP Enablei)(GLenum target, GLuint index); /* 582 */
   void (GLAPIENTRYP EndConditionalRender)(void); /* 583 */
   void (GLAPIENTRYP EndTransformFeedback)(void); /* 584 */
   void (GLAPIENTRYP GetBooleani_v)(GLenum value, GLuint index, GLboolean * data); /* 585 */
   GLint (GLAPIENTRYP GetFragDataLocation)(GLuint program, const GLchar * name); /* 586 */
   void (GLAPIENTRYP GetIntegeri_v)(GLenum value, GLuint index, GLint * data); /* 587 */
   const GLubyte * (GLAPIENTRYP GetStringi)(GLenum name, GLuint index); /* 588 */
   void (GLAPIENTRYP GetTexParameterIiv)(GLenum target, GLenum pname, GLint * params); /* 589 */
   void (GLAPIENTRYP GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint * params); /* 590 */
   void (GLAPIENTRYP GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name); /* 591 */
   void (GLAPIENTRYP GetUniformuiv)(GLuint program, GLint location, GLuint * params); /* 592 */
   void (GLAPIENTRYP GetVertexAttribIiv)(GLuint index, GLenum pname, GLint * params); /* 593 */
   void (GLAPIENTRYP GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint * params); /* 594 */
   GLboolean (GLAPIENTRYP IsEnabledi)(GLenum target, GLuint index); /* 595 */
   void (GLAPIENTRYP TexParameterIiv)(GLenum target, GLenum pname, const GLint * params); /* 596 */
   void (GLAPIENTRYP TexParameterIuiv)(GLenum target, GLenum pname, const GLuint * params); /* 597 */
   void (GLAPIENTRYP TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar * const * varyings, GLenum bufferMode); /* 598 */
   void (GLAPIENTRYP Uniform1ui)(GLint location, GLuint x); /* 599 */
   void (GLAPIENTRYP Uniform1uiv)(GLint location, GLsizei count, const GLuint * value); /* 600 */
   void (GLAPIENTRYP Uniform2ui)(GLint location, GLuint x, GLuint y); /* 601 */
   void (GLAPIENTRYP Uniform2uiv)(GLint location, GLsizei count, const GLuint * value); /* 602 */
   void (GLAPIENTRYP Uniform3ui)(GLint location, GLuint x, GLuint y, GLuint z); /* 603 */
   void (GLAPIENTRYP Uniform3uiv)(GLint location, GLsizei count, const GLuint * value); /* 604 */
   void (GLAPIENTRYP Uniform4ui)(GLint location, GLuint x, GLuint y, GLuint z, GLuint w); /* 605 */
   void (GLAPIENTRYP Uniform4uiv)(GLint location, GLsizei count, const GLuint * value); /* 606 */
   void (GLAPIENTRYP VertexAttribI1iv)(GLuint index, const GLint * v); /* 607 */
   void (GLAPIENTRYP VertexAttribI1uiv)(GLuint index, const GLuint * v); /* 608 */
   void (GLAPIENTRYP VertexAttribI4bv)(GLuint index, const GLbyte * v); /* 609 */
   void (GLAPIENTRYP VertexAttribI4sv)(GLuint index, const GLshort * v); /* 610 */
   void (GLAPIENTRYP VertexAttribI4ubv)(GLuint index, const GLubyte * v); /* 611 */
   void (GLAPIENTRYP VertexAttribI4usv)(GLuint index, const GLushort * v); /* 612 */
   void (GLAPIENTRYP VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 613 */
   void (GLAPIENTRYP PrimitiveRestartIndex)(GLuint index); /* 614 */
   void (GLAPIENTRYP TexBuffer)(GLenum target, GLenum internalFormat, GLuint buffer); /* 615 */
   void (GLAPIENTRYP FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level); /* 616 */
   void (GLAPIENTRYP GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 * params); /* 617 */
   void (GLAPIENTRYP GetInteger64i_v)(GLenum cap, GLuint index, GLint64 * data); /* 618 */
   void (GLAPIENTRYP VertexAttribDivisor)(GLuint index, GLuint divisor); /* 619 */
   void (GLAPIENTRYP MinSampleShading)(GLfloat value); /* 620 */
   void (GLAPIENTRYP MemoryBarrierByRegion)(GLbitfield barriers); /* 621 */
   void (GLAPIENTRYP BindProgramARB)(GLenum target, GLuint program); /* 622 */
   void (GLAPIENTRYP DeleteProgramsARB)(GLsizei n, const GLuint * programs); /* 623 */
   void (GLAPIENTRYP GenProgramsARB)(GLsizei n, GLuint * programs); /* 624 */
   void (GLAPIENTRYP GetProgramEnvParameterdvARB)(GLenum target, GLuint index, GLdouble * params); /* 625 */
   void (GLAPIENTRYP GetProgramEnvParameterfvARB)(GLenum target, GLuint index, GLfloat * params); /* 626 */
   void (GLAPIENTRYP GetProgramLocalParameterdvARB)(GLenum target, GLuint index, GLdouble * params); /* 627 */
   void (GLAPIENTRYP GetProgramLocalParameterfvARB)(GLenum target, GLuint index, GLfloat * params); /* 628 */
   void (GLAPIENTRYP GetProgramStringARB)(GLenum target, GLenum pname, GLvoid * string); /* 629 */
   void (GLAPIENTRYP GetProgramivARB)(GLenum target, GLenum pname, GLint * params); /* 630 */
   GLboolean (GLAPIENTRYP IsProgramARB)(GLuint program); /* 631 */
   void (GLAPIENTRYP ProgramEnvParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 632 */
   void (GLAPIENTRYP ProgramEnvParameter4dvARB)(GLenum target, GLuint index, const GLdouble * params); /* 633 */
   void (GLAPIENTRYP ProgramEnvParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 634 */
   void (GLAPIENTRYP ProgramEnvParameter4fvARB)(GLenum target, GLuint index, const GLfloat * params); /* 635 */
   void (GLAPIENTRYP ProgramLocalParameter4dARB)(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 636 */
   void (GLAPIENTRYP ProgramLocalParameter4dvARB)(GLenum target, GLuint index, const GLdouble * params); /* 637 */
   void (GLAPIENTRYP ProgramLocalParameter4fARB)(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 638 */
   void (GLAPIENTRYP ProgramLocalParameter4fvARB)(GLenum target, GLuint index, const GLfloat * params); /* 639 */
   void (GLAPIENTRYP ProgramStringARB)(GLenum target, GLenum format, GLsizei len, const GLvoid * string); /* 640 */
   void (GLAPIENTRYP VertexAttrib1fARB)(GLuint index, GLfloat x); /* 641 */
   void (GLAPIENTRYP VertexAttrib1fvARB)(GLuint index, const GLfloat * v); /* 642 */
   void (GLAPIENTRYP VertexAttrib2fARB)(GLuint index, GLfloat x, GLfloat y); /* 643 */
   void (GLAPIENTRYP VertexAttrib2fvARB)(GLuint index, const GLfloat * v); /* 644 */
   void (GLAPIENTRYP VertexAttrib3fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z); /* 645 */
   void (GLAPIENTRYP VertexAttrib3fvARB)(GLuint index, const GLfloat * v); /* 646 */
   void (GLAPIENTRYP VertexAttrib4fARB)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 647 */
   void (GLAPIENTRYP VertexAttrib4fvARB)(GLuint index, const GLfloat * v); /* 648 */
   void (GLAPIENTRYP AttachObjectARB)(GLhandleARB containerObj, GLhandleARB obj); /* 649 */
   GLhandleARB (GLAPIENTRYP CreateProgramObjectARB)(void); /* 650 */
   GLhandleARB (GLAPIENTRYP CreateShaderObjectARB)(GLenum shaderType); /* 651 */
   void (GLAPIENTRYP DeleteObjectARB)(GLhandleARB obj); /* 652 */
   void (GLAPIENTRYP DetachObjectARB)(GLhandleARB containerObj, GLhandleARB attachedObj); /* 653 */
   void (GLAPIENTRYP GetAttachedObjectsARB)(GLhandleARB containerObj, GLsizei maxLength, GLsizei * length, GLhandleARB * infoLog); /* 654 */
   GLhandleARB (GLAPIENTRYP GetHandleARB)(GLenum pname); /* 655 */
   void (GLAPIENTRYP GetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * infoLog); /* 656 */
   void (GLAPIENTRYP GetObjectParameterfvARB)(GLhandleARB obj, GLenum pname, GLfloat * params); /* 657 */
   void (GLAPIENTRYP GetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint * params); /* 658 */
   void (GLAPIENTRYP DrawArraysInstancedARB)(GLenum mode, GLint first, GLsizei count, GLsizei primcount); /* 659 */
   void (GLAPIENTRYP DrawElementsInstancedARB)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount); /* 660 */
   void (GLAPIENTRYP BindFramebuffer)(GLenum target, GLuint framebuffer); /* 661 */
   void (GLAPIENTRYP BindRenderbuffer)(GLenum target, GLuint renderbuffer); /* 662 */
   void (GLAPIENTRYP BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter); /* 663 */
   GLenum (GLAPIENTRYP CheckFramebufferStatus)(GLenum target); /* 664 */
   void (GLAPIENTRYP DeleteFramebuffers)(GLsizei n, const GLuint * framebuffers); /* 665 */
   void (GLAPIENTRYP DeleteRenderbuffers)(GLsizei n, const GLuint * renderbuffers); /* 666 */
   void (GLAPIENTRYP FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); /* 667 */
   void (GLAPIENTRYP FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); /* 668 */
   void (GLAPIENTRYP FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level); /* 669 */
   void (GLAPIENTRYP FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer); /* 670 */
   void (GLAPIENTRYP FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer); /* 671 */
   void (GLAPIENTRYP GenFramebuffers)(GLsizei n, GLuint * framebuffers); /* 672 */
   void (GLAPIENTRYP GenRenderbuffers)(GLsizei n, GLuint * renderbuffers); /* 673 */
   void (GLAPIENTRYP GenerateMipmap)(GLenum target); /* 674 */
   void (GLAPIENTRYP GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint * params); /* 675 */
   void (GLAPIENTRYP GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint * params); /* 676 */
   GLboolean (GLAPIENTRYP IsFramebuffer)(GLuint framebuffer); /* 677 */
   GLboolean (GLAPIENTRYP IsRenderbuffer)(GLuint renderbuffer); /* 678 */
   void (GLAPIENTRYP RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height); /* 679 */
   void (GLAPIENTRYP RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); /* 680 */
   void (GLAPIENTRYP FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length); /* 681 */
   GLvoid * (GLAPIENTRYP MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access); /* 682 */
   void (GLAPIENTRYP BindVertexArray)(GLuint array); /* 683 */
   void (GLAPIENTRYP DeleteVertexArrays)(GLsizei n, const GLuint * arrays); /* 684 */
   void (GLAPIENTRYP GenVertexArrays)(GLsizei n, GLuint * arrays); /* 685 */
   GLboolean (GLAPIENTRYP IsVertexArray)(GLuint array); /* 686 */
   void (GLAPIENTRYP GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName); /* 687 */
   void (GLAPIENTRYP GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params); /* 688 */
   void (GLAPIENTRYP GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName); /* 689 */
   void (GLAPIENTRYP GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params); /* 690 */
   GLuint (GLAPIENTRYP GetUniformBlockIndex)(GLuint program, const GLchar * uniformBlockName); /* 691 */
   void (GLAPIENTRYP GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar * const * uniformNames, GLuint * uniformIndices); /* 692 */
   void (GLAPIENTRYP UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding); /* 693 */
   void (GLAPIENTRYP CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size); /* 694 */
   GLenum (GLAPIENTRYP ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout); /* 695 */
   void (GLAPIENTRYP DeleteSync)(GLsync sync); /* 696 */
   GLsync (GLAPIENTRYP FenceSync)(GLenum condition, GLbitfield flags); /* 697 */
   void (GLAPIENTRYP GetInteger64v)(GLenum pname, GLint64 * params); /* 698 */
   void (GLAPIENTRYP GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values); /* 699 */
   GLboolean (GLAPIENTRYP IsSync)(GLsync sync); /* 700 */
   void (GLAPIENTRYP WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout); /* 701 */
   void (GLAPIENTRYP DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex); /* 702 */
   void (GLAPIENTRYP DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex); /* 703 */
   void (GLAPIENTRYP DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices, GLint basevertex); /* 704 */
   void (GLAPIENTRYP MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, const GLint * basevertex); /* 705 */
   void (GLAPIENTRYP ProvokingVertex)(GLenum mode); /* 706 */
   void (GLAPIENTRYP GetMultisamplefv)(GLenum pname, GLuint index, GLfloat * val); /* 707 */
   void (GLAPIENTRYP SampleMaski)(GLuint index, GLbitfield mask); /* 708 */
   void (GLAPIENTRYP TexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); /* 709 */
   void (GLAPIENTRYP TexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); /* 710 */
   void (GLAPIENTRYP BlendEquationSeparateiARB)(GLuint buf, GLenum modeRGB, GLenum modeA); /* 711 */
   void (GLAPIENTRYP BlendEquationiARB)(GLuint buf, GLenum mode); /* 712 */
   void (GLAPIENTRYP BlendFuncSeparateiARB)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcA, GLenum dstA); /* 713 */
   void (GLAPIENTRYP BlendFunciARB)(GLuint buf, GLenum src, GLenum dst); /* 714 */
   void (GLAPIENTRYP BindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name); /* 715 */
   GLint (GLAPIENTRYP GetFragDataIndex)(GLuint program, const GLchar * name); /* 716 */
   void (GLAPIENTRYP BindSampler)(GLuint unit, GLuint sampler); /* 717 */
   void (GLAPIENTRYP DeleteSamplers)(GLsizei count, const GLuint * samplers); /* 718 */
   void (GLAPIENTRYP GenSamplers)(GLsizei count, GLuint * samplers); /* 719 */
   void (GLAPIENTRYP GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint * params); /* 720 */
   void (GLAPIENTRYP GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint * params); /* 721 */
   void (GLAPIENTRYP GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat * params); /* 722 */
   void (GLAPIENTRYP GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint * params); /* 723 */
   GLboolean (GLAPIENTRYP IsSampler)(GLuint sampler); /* 724 */
   void (GLAPIENTRYP SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint * params); /* 725 */
   void (GLAPIENTRYP SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint * params); /* 726 */
   void (GLAPIENTRYP SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param); /* 727 */
   void (GLAPIENTRYP SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat * params); /* 728 */
   void (GLAPIENTRYP SamplerParameteri)(GLuint sampler, GLenum pname, GLint param); /* 729 */
   void (GLAPIENTRYP SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint * params); /* 730 */
   void (GLAPIENTRYP GetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 * params); /* 731 */
   void (GLAPIENTRYP GetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 * params); /* 732 */
   void (GLAPIENTRYP QueryCounter)(GLuint id, GLenum target); /* 733 */
   void (GLAPIENTRYP ColorP3ui)(GLenum type, GLuint color); /* 734 */
   void (GLAPIENTRYP ColorP3uiv)(GLenum type, const GLuint * color); /* 735 */
   void (GLAPIENTRYP ColorP4ui)(GLenum type, GLuint color); /* 736 */
   void (GLAPIENTRYP ColorP4uiv)(GLenum type, const GLuint * color); /* 737 */
   void (GLAPIENTRYP MultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords); /* 738 */
   void (GLAPIENTRYP MultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint * coords); /* 739 */
   void (GLAPIENTRYP MultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords); /* 740 */
   void (GLAPIENTRYP MultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint * coords); /* 741 */
   void (GLAPIENTRYP MultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords); /* 742 */
   void (GLAPIENTRYP MultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint * coords); /* 743 */
   void (GLAPIENTRYP MultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords); /* 744 */
   void (GLAPIENTRYP MultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint * coords); /* 745 */
   void (GLAPIENTRYP NormalP3ui)(GLenum type, GLuint coords); /* 746 */
   void (GLAPIENTRYP NormalP3uiv)(GLenum type, const GLuint * coords); /* 747 */
   void (GLAPIENTRYP SecondaryColorP3ui)(GLenum type, GLuint color); /* 748 */
   void (GLAPIENTRYP SecondaryColorP3uiv)(GLenum type, const GLuint * color); /* 749 */
   void (GLAPIENTRYP TexCoordP1ui)(GLenum type, GLuint coords); /* 750 */
   void (GLAPIENTRYP TexCoordP1uiv)(GLenum type, const GLuint * coords); /* 751 */
   void (GLAPIENTRYP TexCoordP2ui)(GLenum type, GLuint coords); /* 752 */
   void (GLAPIENTRYP TexCoordP2uiv)(GLenum type, const GLuint * coords); /* 753 */
   void (GLAPIENTRYP TexCoordP3ui)(GLenum type, GLuint coords); /* 754 */
   void (GLAPIENTRYP TexCoordP3uiv)(GLenum type, const GLuint * coords); /* 755 */
   void (GLAPIENTRYP TexCoordP4ui)(GLenum type, GLuint coords); /* 756 */
   void (GLAPIENTRYP TexCoordP4uiv)(GLenum type, const GLuint * coords); /* 757 */
   void (GLAPIENTRYP VertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value); /* 758 */
   void (GLAPIENTRYP VertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value); /* 759 */
   void (GLAPIENTRYP VertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value); /* 760 */
   void (GLAPIENTRYP VertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value); /* 761 */
   void (GLAPIENTRYP VertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value); /* 762 */
   void (GLAPIENTRYP VertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value); /* 763 */
   void (GLAPIENTRYP VertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value); /* 764 */
   void (GLAPIENTRYP VertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint * value); /* 765 */
   void (GLAPIENTRYP VertexP2ui)(GLenum type, GLuint value); /* 766 */
   void (GLAPIENTRYP VertexP2uiv)(GLenum type, const GLuint * value); /* 767 */
   void (GLAPIENTRYP VertexP3ui)(GLenum type, GLuint value); /* 768 */
   void (GLAPIENTRYP VertexP3uiv)(GLenum type, const GLuint * value); /* 769 */
   void (GLAPIENTRYP VertexP4ui)(GLenum type, GLuint value); /* 770 */
   void (GLAPIENTRYP VertexP4uiv)(GLenum type, const GLuint * value); /* 771 */
   void (GLAPIENTRYP DrawArraysIndirect)(GLenum mode, const GLvoid * indirect); /* 772 */
   void (GLAPIENTRYP DrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid * indirect); /* 773 */
   void (GLAPIENTRYP GetUniformdv)(GLuint program, GLint location, GLdouble * params); /* 774 */
   void (GLAPIENTRYP Uniform1d)(GLint location, GLdouble x); /* 775 */
   void (GLAPIENTRYP Uniform1dv)(GLint location, GLsizei count, const GLdouble * value); /* 776 */
   void (GLAPIENTRYP Uniform2d)(GLint location, GLdouble x, GLdouble y); /* 777 */
   void (GLAPIENTRYP Uniform2dv)(GLint location, GLsizei count, const GLdouble * value); /* 778 */
   void (GLAPIENTRYP Uniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z); /* 779 */
   void (GLAPIENTRYP Uniform3dv)(GLint location, GLsizei count, const GLdouble * value); /* 780 */
   void (GLAPIENTRYP Uniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 781 */
   void (GLAPIENTRYP Uniform4dv)(GLint location, GLsizei count, const GLdouble * value); /* 782 */
   void (GLAPIENTRYP UniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 783 */
   void (GLAPIENTRYP UniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 784 */
   void (GLAPIENTRYP UniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 785 */
   void (GLAPIENTRYP UniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 786 */
   void (GLAPIENTRYP UniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 787 */
   void (GLAPIENTRYP UniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 788 */
   void (GLAPIENTRYP UniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 789 */
   void (GLAPIENTRYP UniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 790 */
   void (GLAPIENTRYP UniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 791 */
   void (GLAPIENTRYP GetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name); /* 792 */
   void (GLAPIENTRYP GetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei * length, GLchar * name); /* 793 */
   void (GLAPIENTRYP GetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint * values); /* 794 */
   void (GLAPIENTRYP GetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint * values); /* 795 */
   GLuint (GLAPIENTRYP GetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar * name); /* 796 */
   GLint (GLAPIENTRYP GetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar * name); /* 797 */
   void (GLAPIENTRYP GetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint * params); /* 798 */
   void (GLAPIENTRYP UniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint * indices); /* 799 */
   void (GLAPIENTRYP PatchParameterfv)(GLenum pname, const GLfloat * values); /* 800 */
   void (GLAPIENTRYP PatchParameteri)(GLenum pname, GLint value); /* 801 */
   void (GLAPIENTRYP BindTransformFeedback)(GLenum target, GLuint id); /* 802 */
   void (GLAPIENTRYP DeleteTransformFeedbacks)(GLsizei n, const GLuint * ids); /* 803 */
   void (GLAPIENTRYP DrawTransformFeedback)(GLenum mode, GLuint id); /* 804 */
   void (GLAPIENTRYP GenTransformFeedbacks)(GLsizei n, GLuint * ids); /* 805 */
   GLboolean (GLAPIENTRYP IsTransformFeedback)(GLuint id); /* 806 */
   void (GLAPIENTRYP PauseTransformFeedback)(void); /* 807 */
   void (GLAPIENTRYP ResumeTransformFeedback)(void); /* 808 */
   void (GLAPIENTRYP BeginQueryIndexed)(GLenum target, GLuint index, GLuint id); /* 809 */
   void (GLAPIENTRYP DrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream); /* 810 */
   void (GLAPIENTRYP EndQueryIndexed)(GLenum target, GLuint index); /* 811 */
   void (GLAPIENTRYP GetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint * params); /* 812 */
   void (GLAPIENTRYP ClearDepthf)(GLclampf depth); /* 813 */
   void (GLAPIENTRYP DepthRangef)(GLclampf zNear, GLclampf zFar); /* 814 */
   void (GLAPIENTRYP GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision); /* 815 */
   void (GLAPIENTRYP ReleaseShaderCompiler)(void); /* 816 */
   void (GLAPIENTRYP ShaderBinary)(GLsizei n, const GLuint * shaders, GLenum binaryformat, const GLvoid * binary, GLsizei length); /* 817 */
   void (GLAPIENTRYP GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, GLvoid * binary); /* 818 */
   void (GLAPIENTRYP ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid * binary, GLsizei length); /* 819 */
   void (GLAPIENTRYP ProgramParameteri)(GLuint program, GLenum pname, GLint value); /* 820 */
   void (GLAPIENTRYP GetVertexAttribLdv)(GLuint index, GLenum pname, GLdouble * params); /* 821 */
   void (GLAPIENTRYP VertexAttribL1d)(GLuint index, GLdouble x); /* 822 */
   void (GLAPIENTRYP VertexAttribL1dv)(GLuint index, const GLdouble * v); /* 823 */
   void (GLAPIENTRYP VertexAttribL2d)(GLuint index, GLdouble x, GLdouble y); /* 824 */
   void (GLAPIENTRYP VertexAttribL2dv)(GLuint index, const GLdouble * v); /* 825 */
   void (GLAPIENTRYP VertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z); /* 826 */
   void (GLAPIENTRYP VertexAttribL3dv)(GLuint index, const GLdouble * v); /* 827 */
   void (GLAPIENTRYP VertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 828 */
   void (GLAPIENTRYP VertexAttribL4dv)(GLuint index, const GLdouble * v); /* 829 */
   void (GLAPIENTRYP VertexAttribLPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 830 */
   void (GLAPIENTRYP DepthRangeArrayv)(GLuint first, GLsizei count, const GLclampd * v); /* 831 */
   void (GLAPIENTRYP DepthRangeIndexed)(GLuint index, GLclampd n, GLclampd f); /* 832 */
   void (GLAPIENTRYP GetDoublei_v)(GLenum target, GLuint index, GLdouble * data); /* 833 */
   void (GLAPIENTRYP GetFloati_v)(GLenum target, GLuint index, GLfloat * data); /* 834 */
   void (GLAPIENTRYP ScissorArrayv)(GLuint first, GLsizei count, const int * v); /* 835 */
   void (GLAPIENTRYP ScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height); /* 836 */
   void (GLAPIENTRYP ScissorIndexedv)(GLuint index, const GLint * v); /* 837 */
   void (GLAPIENTRYP ViewportArrayv)(GLuint first, GLsizei count, const GLfloat * v); /* 838 */
   void (GLAPIENTRYP ViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h); /* 839 */
   void (GLAPIENTRYP ViewportIndexedfv)(GLuint index, const GLfloat * v); /* 840 */
   GLenum (GLAPIENTRYP GetGraphicsResetStatusARB)(void); /* 841 */
   void (GLAPIENTRYP GetnColorTableARB)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid * table); /* 842 */
   void (GLAPIENTRYP GetnCompressedTexImageARB)(GLenum target, GLint lod, GLsizei bufSize, GLvoid * img); /* 843 */
   void (GLAPIENTRYP GetnConvolutionFilterARB)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid * image); /* 844 */
   void (GLAPIENTRYP GetnHistogramARB)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid * values); /* 845 */
   void (GLAPIENTRYP GetnMapdvARB)(GLenum target, GLenum query, GLsizei bufSize, GLdouble * v); /* 846 */
   void (GLAPIENTRYP GetnMapfvARB)(GLenum target, GLenum query, GLsizei bufSize, GLfloat * v); /* 847 */
   void (GLAPIENTRYP GetnMapivARB)(GLenum target, GLenum query, GLsizei bufSize, GLint * v); /* 848 */
   void (GLAPIENTRYP GetnMinmaxARB)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid * values); /* 849 */
   void (GLAPIENTRYP GetnPixelMapfvARB)(GLenum map, GLsizei bufSize, GLfloat * values); /* 850 */
   void (GLAPIENTRYP GetnPixelMapuivARB)(GLenum map, GLsizei bufSize, GLuint * values); /* 851 */
   void (GLAPIENTRYP GetnPixelMapusvARB)(GLenum map, GLsizei bufSize, GLushort * values); /* 852 */
   void (GLAPIENTRYP GetnPolygonStippleARB)(GLsizei bufSize, GLubyte * pattern); /* 853 */
   void (GLAPIENTRYP GetnSeparableFilterARB)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, GLvoid * row, GLsizei columnBufSize, GLvoid * column, GLvoid * span); /* 854 */
   void (GLAPIENTRYP GetnTexImageARB)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid * img); /* 855 */
   void (GLAPIENTRYP GetnUniformdvARB)(GLuint program, GLint location, GLsizei bufSize, GLdouble * params); /* 856 */
   void (GLAPIENTRYP GetnUniformfvARB)(GLuint program, GLint location, GLsizei bufSize, GLfloat * params); /* 857 */
   void (GLAPIENTRYP GetnUniformivARB)(GLuint program, GLint location, GLsizei bufSize, GLint * params); /* 858 */
   void (GLAPIENTRYP GetnUniformuivARB)(GLuint program, GLint location, GLsizei bufSize, GLuint * params); /* 859 */
   void (GLAPIENTRYP ReadnPixelsARB)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid * data); /* 860 */
   void (GLAPIENTRYP DrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance); /* 861 */
   void (GLAPIENTRYP DrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLuint baseinstance); /* 862 */
   void (GLAPIENTRYP DrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices, GLsizei primcount, GLint basevertex, GLuint baseinstance); /* 863 */
   void (GLAPIENTRYP DrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei primcount); /* 864 */
   void (GLAPIENTRYP DrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei primcount); /* 865 */
   void (GLAPIENTRYP GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint * params); /* 866 */
   void (GLAPIENTRYP GetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint * params); /* 867 */
   void (GLAPIENTRYP BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format); /* 868 */
   void (GLAPIENTRYP MemoryBarrier)(GLbitfield barriers); /* 869 */
   void (GLAPIENTRYP TexStorage1D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width); /* 870 */
   void (GLAPIENTRYP TexStorage2D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height); /* 871 */
   void (GLAPIENTRYP TexStorage3D)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth); /* 872 */
   void (GLAPIENTRYP TextureStorage1DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width); /* 873 */
   void (GLAPIENTRYP TextureStorage2DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height); /* 874 */
   void (GLAPIENTRYP TextureStorage3DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth); /* 875 */
   void (GLAPIENTRYP ClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const GLvoid * data); /* 876 */
   void (GLAPIENTRYP ClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid * data); /* 877 */
   void (GLAPIENTRYP DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z); /* 878 */
   void (GLAPIENTRYP DispatchComputeIndirect)(GLintptr indirect); /* 879 */
   void (GLAPIENTRYP CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth); /* 880 */
   void (GLAPIENTRYP TextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers); /* 881 */
   void (GLAPIENTRYP BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride); /* 882 */
   void (GLAPIENTRYP VertexAttribBinding)(GLuint attribindex, GLuint bindingindex); /* 883 */
   void (GLAPIENTRYP VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset); /* 884 */
   void (GLAPIENTRYP VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); /* 885 */
   void (GLAPIENTRYP VertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); /* 886 */
   void (GLAPIENTRYP VertexBindingDivisor)(GLuint attribindex, GLuint divisor); /* 887 */
   void (GLAPIENTRYP FramebufferParameteri)(GLenum target, GLenum pname, GLint param); /* 888 */
   void (GLAPIENTRYP GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint * params); /* 889 */
   void (GLAPIENTRYP GetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 * params); /* 890 */
   void (GLAPIENTRYP MultiDrawArraysIndirect)(GLenum mode, const GLvoid * indirect, GLsizei primcount, GLsizei stride); /* 891 */
   void (GLAPIENTRYP MultiDrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid * indirect, GLsizei primcount, GLsizei stride); /* 892 */
   void (GLAPIENTRYP GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint * params); /* 893 */
   GLuint (GLAPIENTRYP GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar * name); /* 894 */
   GLint (GLAPIENTRYP GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar * name); /* 895 */
   GLint (GLAPIENTRYP GetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar * name); /* 896 */
   void (GLAPIENTRYP GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei  bufSize, GLsizei * length, GLchar * name); /* 897 */
   void (GLAPIENTRYP GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei  propCount, const GLenum * props, GLsizei  bufSize, GLsizei * length, GLint * params); /* 898 */
   void (GLAPIENTRYP ShaderStorageBlockBinding)(GLuint program, GLuint shaderStorageBlockIndex, GLuint shaderStorageBlockBinding); /* 899 */
   void (GLAPIENTRYP TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size); /* 900 */
   void (GLAPIENTRYP TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); /* 901 */
   void (GLAPIENTRYP TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); /* 902 */
   void (GLAPIENTRYP BufferStorage)(GLenum target, GLsizeiptr size, const GLvoid * data, GLbitfield flags); /* 903 */
   void (GLAPIENTRYP ClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const GLvoid * data); /* 904 */
   void (GLAPIENTRYP ClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data); /* 905 */
   void (GLAPIENTRYP BindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers); /* 906 */
   void (GLAPIENTRYP BindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizeiptr * sizes); /* 907 */
   void (GLAPIENTRYP BindImageTextures)(GLuint first, GLsizei count, const GLuint * textures); /* 908 */
   void (GLAPIENTRYP BindSamplers)(GLuint first, GLsizei count, const GLuint * samplers); /* 909 */
   void (GLAPIENTRYP BindTextures)(GLuint first, GLsizei count, const GLuint * textures); /* 910 */
   void (GLAPIENTRYP BindVertexBuffers)(GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides); /* 911 */
   GLuint64 (GLAPIENTRYP GetImageHandleARB)(GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format); /* 912 */
   GLuint64 (GLAPIENTRYP GetTextureHandleARB)(GLuint texture); /* 913 */
   GLuint64 (GLAPIENTRYP GetTextureSamplerHandleARB)(GLuint texture, GLuint sampler); /* 914 */
   void (GLAPIENTRYP GetVertexAttribLui64vARB)(GLuint index, GLenum pname, GLuint64EXT * params); /* 915 */
   GLboolean (GLAPIENTRYP IsImageHandleResidentARB)(GLuint64 handle); /* 916 */
   GLboolean (GLAPIENTRYP IsTextureHandleResidentARB)(GLuint64 handle); /* 917 */
   void (GLAPIENTRYP MakeImageHandleNonResidentARB)(GLuint64 handle); /* 918 */
   void (GLAPIENTRYP MakeImageHandleResidentARB)(GLuint64 handle, GLenum access); /* 919 */
   void (GLAPIENTRYP MakeTextureHandleNonResidentARB)(GLuint64 handle); /* 920 */
   void (GLAPIENTRYP MakeTextureHandleResidentARB)(GLuint64 handle); /* 921 */
   void (GLAPIENTRYP ProgramUniformHandleui64ARB)(GLuint program, GLint location, GLuint64 value); /* 922 */
   void (GLAPIENTRYP ProgramUniformHandleui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64 * value); /* 923 */
   void (GLAPIENTRYP UniformHandleui64ARB)(GLint location, GLuint64 value); /* 924 */
   void (GLAPIENTRYP UniformHandleui64vARB)(GLint location, GLsizei count, const GLuint64 * value); /* 925 */
   void (GLAPIENTRYP VertexAttribL1ui64ARB)(GLuint index, GLuint64EXT x); /* 926 */
   void (GLAPIENTRYP VertexAttribL1ui64vARB)(GLuint index, const GLuint64EXT * v); /* 927 */
   void (GLAPIENTRYP DispatchComputeGroupSizeARB)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z); /* 928 */
   void (GLAPIENTRYP MultiDrawArraysIndirectCountARB)(GLenum mode, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride); /* 929 */
   void (GLAPIENTRYP MultiDrawElementsIndirectCountARB)(GLenum mode, GLenum type, GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride); /* 930 */
   void (GLAPIENTRYP ClipControl)(GLenum origin, GLenum depth); /* 931 */
   void (GLAPIENTRYP BindTextureUnit)(GLuint unit, GLuint texture); /* 932 */
   void (GLAPIENTRYP BlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter); /* 933 */
   GLenum (GLAPIENTRYP CheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target); /* 934 */
   void (GLAPIENTRYP ClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const GLvoid * data); /* 935 */
   void (GLAPIENTRYP ClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const GLvoid * data); /* 936 */
   void (GLAPIENTRYP ClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil); /* 937 */
   void (GLAPIENTRYP ClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat * value); /* 938 */
   void (GLAPIENTRYP ClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint * value); /* 939 */
   void (GLAPIENTRYP ClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint * value); /* 940 */
   void (GLAPIENTRYP CompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data); /* 941 */
   void (GLAPIENTRYP CompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data); /* 942 */
   void (GLAPIENTRYP CompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data); /* 943 */
   void (GLAPIENTRYP CopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size); /* 944 */
   void (GLAPIENTRYP CopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width); /* 945 */
   void (GLAPIENTRYP CopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height); /* 946 */
   void (GLAPIENTRYP CopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height); /* 947 */
   void (GLAPIENTRYP CreateBuffers)(GLsizei n, GLuint * buffers); /* 948 */
   void (GLAPIENTRYP CreateFramebuffers)(GLsizei n, GLuint * framebuffers); /* 949 */
   void (GLAPIENTRYP CreateProgramPipelines)(GLsizei n, GLuint * pipelines); /* 950 */
   void (GLAPIENTRYP CreateQueries)(GLenum target, GLsizei n, GLuint * ids); /* 951 */
   void (GLAPIENTRYP CreateRenderbuffers)(GLsizei n, GLuint * renderbuffers); /* 952 */
   void (GLAPIENTRYP CreateSamplers)(GLsizei n, GLuint * samplers); /* 953 */
   void (GLAPIENTRYP CreateTextures)(GLenum target, GLsizei n, GLuint * textures); /* 954 */
   void (GLAPIENTRYP CreateTransformFeedbacks)(GLsizei n, GLuint * ids); /* 955 */
   void (GLAPIENTRYP CreateVertexArrays)(GLsizei n, GLuint * arrays); /* 956 */
   void (GLAPIENTRYP DisableVertexArrayAttrib)(GLuint vaobj, GLuint index); /* 957 */
   void (GLAPIENTRYP EnableVertexArrayAttrib)(GLuint vaobj, GLuint index); /* 958 */
   void (GLAPIENTRYP FlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length); /* 959 */
   void (GLAPIENTRYP GenerateTextureMipmap)(GLuint texture); /* 960 */
   void (GLAPIENTRYP GetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, GLvoid * pixels); /* 961 */
   void (GLAPIENTRYP GetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64 * params); /* 962 */
   void (GLAPIENTRYP GetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint * params); /* 963 */
   void (GLAPIENTRYP GetNamedBufferPointerv)(GLuint buffer, GLenum pname, GLvoid ** params); /* 964 */
   void (GLAPIENTRYP GetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid * data); /* 965 */
   void (GLAPIENTRYP GetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint * params); /* 966 */
   void (GLAPIENTRYP GetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint * param); /* 967 */
   void (GLAPIENTRYP GetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint * params); /* 968 */
   void (GLAPIENTRYP GetQueryBufferObjecti64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset); /* 969 */
   void (GLAPIENTRYP GetQueryBufferObjectiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset); /* 970 */
   void (GLAPIENTRYP GetQueryBufferObjectui64v)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset); /* 971 */
   void (GLAPIENTRYP GetQueryBufferObjectuiv)(GLuint id, GLuint buffer, GLenum pname, GLintptr offset); /* 972 */
   void (GLAPIENTRYP GetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels); /* 973 */
   void (GLAPIENTRYP GetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat * params); /* 974 */
   void (GLAPIENTRYP GetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint * params); /* 975 */
   void (GLAPIENTRYP GetTextureParameterIiv)(GLuint texture, GLenum pname, GLint * params); /* 976 */
   void (GLAPIENTRYP GetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint * params); /* 977 */
   void (GLAPIENTRYP GetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat * params); /* 978 */
   void (GLAPIENTRYP GetTextureParameteriv)(GLuint texture, GLenum pname, GLint * params); /* 979 */
   void (GLAPIENTRYP GetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64 * param); /* 980 */
   void (GLAPIENTRYP GetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint * param); /* 981 */
   void (GLAPIENTRYP GetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint * param); /* 982 */
   void (GLAPIENTRYP GetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64 * param); /* 983 */
   void (GLAPIENTRYP GetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint * param); /* 984 */
   void (GLAPIENTRYP GetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint * param); /* 985 */
   void (GLAPIENTRYP InvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments); /* 986 */
   void (GLAPIENTRYP InvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height); /* 987 */
   GLvoid * (GLAPIENTRYP MapNamedBuffer)(GLuint buffer, GLenum access); /* 988 */
   GLvoid * (GLAPIENTRYP MapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access); /* 989 */
   void (GLAPIENTRYP NamedBufferData)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLenum usage); /* 990 */
   void (GLAPIENTRYP NamedBufferStorage)(GLuint buffer, GLsizeiptr size, const GLvoid * data, GLbitfield flags); /* 991 */
   void (GLAPIENTRYP NamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid * data); /* 992 */
   void (GLAPIENTRYP NamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf); /* 993 */
   void (GLAPIENTRYP NamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum * bufs); /* 994 */
   void (GLAPIENTRYP NamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param); /* 995 */
   void (GLAPIENTRYP NamedFramebufferReadBuffer)(GLuint framebuffer, GLenum buf); /* 996 */
   void (GLAPIENTRYP NamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); /* 997 */
   void (GLAPIENTRYP NamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level); /* 998 */
   void (GLAPIENTRYP NamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer); /* 999 */
   void (GLAPIENTRYP NamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height); /* 1000 */
   void (GLAPIENTRYP NamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); /* 1001 */
   void (GLAPIENTRYP TextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer); /* 1002 */
   void (GLAPIENTRYP TextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size); /* 1003 */
   void (GLAPIENTRYP TextureParameterIiv)(GLuint texture, GLenum pname, const GLint * params); /* 1004 */
   void (GLAPIENTRYP TextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint * params); /* 1005 */
   void (GLAPIENTRYP TextureParameterf)(GLuint texture, GLenum pname, GLfloat param); /* 1006 */
   void (GLAPIENTRYP TextureParameterfv)(GLuint texture, GLenum pname, const GLfloat * param); /* 1007 */
   void (GLAPIENTRYP TextureParameteri)(GLuint texture, GLenum pname, GLint param); /* 1008 */
   void (GLAPIENTRYP TextureParameteriv)(GLuint texture, GLenum pname, const GLint * param); /* 1009 */
   void (GLAPIENTRYP TextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width); /* 1010 */
   void (GLAPIENTRYP TextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height); /* 1011 */
   void (GLAPIENTRYP TextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations); /* 1012 */
   void (GLAPIENTRYP TextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth); /* 1013 */
   void (GLAPIENTRYP TextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations); /* 1014 */
   void (GLAPIENTRYP TextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels); /* 1015 */
   void (GLAPIENTRYP TextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels); /* 1016 */
   void (GLAPIENTRYP TextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels); /* 1017 */
   void (GLAPIENTRYP TransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer); /* 1018 */
   void (GLAPIENTRYP TransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size); /* 1019 */
   GLboolean (GLAPIENTRYP UnmapNamedBuffer)(GLuint buffer); /* 1020 */
   void (GLAPIENTRYP VertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex); /* 1021 */
   void (GLAPIENTRYP VertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset); /* 1022 */
   void (GLAPIENTRYP VertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); /* 1023 */
   void (GLAPIENTRYP VertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset); /* 1024 */
   void (GLAPIENTRYP VertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor); /* 1025 */
   void (GLAPIENTRYP VertexArrayElementBuffer)(GLuint vaobj, GLuint buffer); /* 1026 */
   void (GLAPIENTRYP VertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride); /* 1027 */
   void (GLAPIENTRYP VertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint * buffers, const GLintptr * offsets, const GLsizei * strides); /* 1028 */
   void (GLAPIENTRYP GetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, GLvoid * pixels); /* 1029 */
   void (GLAPIENTRYP GetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, GLvoid * pixels); /* 1030 */
   void (GLAPIENTRYP BufferPageCommitmentARB)(GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit); /* 1031 */
   void (GLAPIENTRYP NamedBufferPageCommitmentARB)(GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit); /* 1032 */
   void (GLAPIENTRYP GetUniformi64vARB)(GLuint program, GLint location, GLint64 * params); /* 1033 */
   void (GLAPIENTRYP GetUniformui64vARB)(GLuint program, GLint location, GLuint64 * params); /* 1034 */
   void (GLAPIENTRYP GetnUniformi64vARB)(GLuint program, GLint location, GLsizei bufSize, GLint64 * params); /* 1035 */
   void (GLAPIENTRYP GetnUniformui64vARB)(GLuint program, GLint location, GLsizei bufSize, GLuint64 * params); /* 1036 */
   void (GLAPIENTRYP ProgramUniform1i64ARB)(GLuint program, GLint location, GLint64 x); /* 1037 */
   void (GLAPIENTRYP ProgramUniform1i64vARB)(GLuint program, GLint location, GLsizei count, const GLint64 * value); /* 1038 */
   void (GLAPIENTRYP ProgramUniform1ui64ARB)(GLuint program, GLint location, GLuint64 x); /* 1039 */
   void (GLAPIENTRYP ProgramUniform1ui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64 * value); /* 1040 */
   void (GLAPIENTRYP ProgramUniform2i64ARB)(GLuint program, GLint location, GLint64 x, GLint64 y); /* 1041 */
   void (GLAPIENTRYP ProgramUniform2i64vARB)(GLuint program, GLint location, GLsizei count, const GLint64 * value); /* 1042 */
   void (GLAPIENTRYP ProgramUniform2ui64ARB)(GLuint program, GLint location, GLuint64 x, GLuint64 y); /* 1043 */
   void (GLAPIENTRYP ProgramUniform2ui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64 * value); /* 1044 */
   void (GLAPIENTRYP ProgramUniform3i64ARB)(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z); /* 1045 */
   void (GLAPIENTRYP ProgramUniform3i64vARB)(GLuint program, GLint location, GLsizei count, const GLint64 * value); /* 1046 */
   void (GLAPIENTRYP ProgramUniform3ui64ARB)(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z); /* 1047 */
   void (GLAPIENTRYP ProgramUniform3ui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64 * value); /* 1048 */
   void (GLAPIENTRYP ProgramUniform4i64ARB)(GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w); /* 1049 */
   void (GLAPIENTRYP ProgramUniform4i64vARB)(GLuint program, GLint location, GLsizei count, const GLint64 * value); /* 1050 */
   void (GLAPIENTRYP ProgramUniform4ui64ARB)(GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w); /* 1051 */
   void (GLAPIENTRYP ProgramUniform4ui64vARB)(GLuint program, GLint location, GLsizei count, const GLuint64 * value); /* 1052 */
   void (GLAPIENTRYP Uniform1i64ARB)(GLint location, GLint64 x); /* 1053 */
   void (GLAPIENTRYP Uniform1i64vARB)(GLint location, GLsizei count, const GLint64 * value); /* 1054 */
   void (GLAPIENTRYP Uniform1ui64ARB)(GLint location, GLuint64 x); /* 1055 */
   void (GLAPIENTRYP Uniform1ui64vARB)(GLint location, GLsizei count, const GLuint64 * value); /* 1056 */
   void (GLAPIENTRYP Uniform2i64ARB)(GLint location, GLint64 x, GLint64 y); /* 1057 */
   void (GLAPIENTRYP Uniform2i64vARB)(GLint location, GLsizei count, const GLint64 * value); /* 1058 */
   void (GLAPIENTRYP Uniform2ui64ARB)(GLint location, GLuint64 x, GLuint64 y); /* 1059 */
   void (GLAPIENTRYP Uniform2ui64vARB)(GLint location, GLsizei count, const GLuint64 * value); /* 1060 */
   void (GLAPIENTRYP Uniform3i64ARB)(GLint location, GLint64 x, GLint64 y, GLint64 z); /* 1061 */
   void (GLAPIENTRYP Uniform3i64vARB)(GLint location, GLsizei count, const GLint64 * value); /* 1062 */
   void (GLAPIENTRYP Uniform3ui64ARB)(GLint location, GLuint64 x, GLuint64 y, GLuint64 z); /* 1063 */
   void (GLAPIENTRYP Uniform3ui64vARB)(GLint location, GLsizei count, const GLuint64 * value); /* 1064 */
   void (GLAPIENTRYP Uniform4i64ARB)(GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w); /* 1065 */
   void (GLAPIENTRYP Uniform4i64vARB)(GLint location, GLsizei count, const GLint64 * value); /* 1066 */
   void (GLAPIENTRYP Uniform4ui64ARB)(GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w); /* 1067 */
   void (GLAPIENTRYP Uniform4ui64vARB)(GLint location, GLsizei count, const GLuint64 * value); /* 1068 */
   void (GLAPIENTRYP InvalidateBufferData)(GLuint buffer); /* 1069 */
   void (GLAPIENTRYP InvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length); /* 1070 */
   void (GLAPIENTRYP InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments); /* 1071 */
   void (GLAPIENTRYP InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum * attachments, GLint x, GLint y, GLsizei width, GLsizei height); /* 1072 */
   void (GLAPIENTRYP InvalidateTexImage)(GLuint texture, GLint level); /* 1073 */
   void (GLAPIENTRYP InvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth); /* 1074 */
   void (GLAPIENTRYP PolygonOffsetEXT)(GLfloat factor, GLfloat bias); /* 1075 */
   void (GLAPIENTRYP DrawTexfOES)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height); /* 1076 */
   void (GLAPIENTRYP DrawTexfvOES)(const GLfloat * coords); /* 1077 */
   void (GLAPIENTRYP DrawTexiOES)(GLint x, GLint y, GLint z, GLint width, GLint height); /* 1078 */
   void (GLAPIENTRYP DrawTexivOES)(const GLint * coords); /* 1079 */
   void (GLAPIENTRYP DrawTexsOES)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height); /* 1080 */
   void (GLAPIENTRYP DrawTexsvOES)(const GLshort * coords); /* 1081 */
   void (GLAPIENTRYP DrawTexxOES)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height); /* 1082 */
   void (GLAPIENTRYP DrawTexxvOES)(const GLfixed * coords); /* 1083 */
   void (GLAPIENTRYP PointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid * pointer); /* 1084 */
   GLbitfield (GLAPIENTRYP QueryMatrixxOES)(GLfixed * mantissa, GLint * exponent); /* 1085 */
   void (GLAPIENTRYP SampleMaskSGIS)(GLclampf value, GLboolean invert); /* 1086 */
   void (GLAPIENTRYP SamplePatternSGIS)(GLenum pattern); /* 1087 */
   void (GLAPIENTRYP ColorPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer); /* 1088 */
   void (GLAPIENTRYP EdgeFlagPointerEXT)(GLsizei stride, GLsizei count, const GLboolean * pointer); /* 1089 */
   void (GLAPIENTRYP IndexPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer); /* 1090 */
   void (GLAPIENTRYP NormalPointerEXT)(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer); /* 1091 */
   void (GLAPIENTRYP TexCoordPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer); /* 1092 */
   void (GLAPIENTRYP VertexPointerEXT)(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer); /* 1093 */
   void (GLAPIENTRYP DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum * attachments); /* 1094 */
   void (GLAPIENTRYP ActiveShaderProgram)(GLuint pipeline, GLuint program); /* 1095 */
   void (GLAPIENTRYP BindProgramPipeline)(GLuint pipeline); /* 1096 */
   GLuint (GLAPIENTRYP CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar * const * strings); /* 1097 */
   void (GLAPIENTRYP DeleteProgramPipelines)(GLsizei n, const GLuint * pipelines); /* 1098 */
   void (GLAPIENTRYP GenProgramPipelines)(GLsizei n, GLuint * pipelines); /* 1099 */
   void (GLAPIENTRYP GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog); /* 1100 */
   void (GLAPIENTRYP GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint * params); /* 1101 */
   GLboolean (GLAPIENTRYP IsProgramPipeline)(GLuint pipeline); /* 1102 */
   void (GLAPIENTRYP LockArraysEXT)(GLint first, GLsizei count); /* 1103 */
   void (GLAPIENTRYP ProgramUniform1d)(GLuint program, GLint location, GLdouble x); /* 1104 */
   void (GLAPIENTRYP ProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value); /* 1105 */
   void (GLAPIENTRYP ProgramUniform1f)(GLuint program, GLint location, GLfloat x); /* 1106 */
   void (GLAPIENTRYP ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value); /* 1107 */
   void (GLAPIENTRYP ProgramUniform1i)(GLuint program, GLint location, GLint x); /* 1108 */
   void (GLAPIENTRYP ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint * value); /* 1109 */
   void (GLAPIENTRYP ProgramUniform1ui)(GLuint program, GLint location, GLuint x); /* 1110 */
   void (GLAPIENTRYP ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value); /* 1111 */
   void (GLAPIENTRYP ProgramUniform2d)(GLuint program, GLint location, GLdouble x, GLdouble y); /* 1112 */
   void (GLAPIENTRYP ProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value); /* 1113 */
   void (GLAPIENTRYP ProgramUniform2f)(GLuint program, GLint location, GLfloat x, GLfloat y); /* 1114 */
   void (GLAPIENTRYP ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value); /* 1115 */
   void (GLAPIENTRYP ProgramUniform2i)(GLuint program, GLint location, GLint x, GLint y); /* 1116 */
   void (GLAPIENTRYP ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint * value); /* 1117 */
   void (GLAPIENTRYP ProgramUniform2ui)(GLuint program, GLint location, GLuint x, GLuint y); /* 1118 */
   void (GLAPIENTRYP ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value); /* 1119 */
   void (GLAPIENTRYP ProgramUniform3d)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z); /* 1120 */
   void (GLAPIENTRYP ProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value); /* 1121 */
   void (GLAPIENTRYP ProgramUniform3f)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z); /* 1122 */
   void (GLAPIENTRYP ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value); /* 1123 */
   void (GLAPIENTRYP ProgramUniform3i)(GLuint program, GLint location, GLint x, GLint y, GLint z); /* 1124 */
   void (GLAPIENTRYP ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint * value); /* 1125 */
   void (GLAPIENTRYP ProgramUniform3ui)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z); /* 1126 */
   void (GLAPIENTRYP ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value); /* 1127 */
   void (GLAPIENTRYP ProgramUniform4d)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 1128 */
   void (GLAPIENTRYP ProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble * value); /* 1129 */
   void (GLAPIENTRYP ProgramUniform4f)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 1130 */
   void (GLAPIENTRYP ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat * value); /* 1131 */
   void (GLAPIENTRYP ProgramUniform4i)(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w); /* 1132 */
   void (GLAPIENTRYP ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint * value); /* 1133 */
   void (GLAPIENTRYP ProgramUniform4ui)(GLuint program, GLint location, GLuint x, GLuint y, GLuint z, GLuint w); /* 1134 */
   void (GLAPIENTRYP ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint * value); /* 1135 */
   void (GLAPIENTRYP ProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1136 */
   void (GLAPIENTRYP ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1137 */
   void (GLAPIENTRYP ProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1138 */
   void (GLAPIENTRYP ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1139 */
   void (GLAPIENTRYP ProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1140 */
   void (GLAPIENTRYP ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1141 */
   void (GLAPIENTRYP ProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1142 */
   void (GLAPIENTRYP ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1143 */
   void (GLAPIENTRYP ProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1144 */
   void (GLAPIENTRYP ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1145 */
   void (GLAPIENTRYP ProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1146 */
   void (GLAPIENTRYP ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1147 */
   void (GLAPIENTRYP ProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1148 */
   void (GLAPIENTRYP ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1149 */
   void (GLAPIENTRYP ProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1150 */
   void (GLAPIENTRYP ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1151 */
   void (GLAPIENTRYP ProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value); /* 1152 */
   void (GLAPIENTRYP ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value); /* 1153 */
   void (GLAPIENTRYP UnlockArraysEXT)(void); /* 1154 */
   void (GLAPIENTRYP UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program); /* 1155 */
   void (GLAPIENTRYP ValidateProgramPipeline)(GLuint pipeline); /* 1156 */
   void (GLAPIENTRYP DebugMessageCallback)(GLDEBUGPROC callback, const GLvoid * userParam); /* 1157 */
   void (GLAPIENTRYP DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled); /* 1158 */
   void (GLAPIENTRYP DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf); /* 1159 */
   GLuint (GLAPIENTRYP GetDebugMessageLog)(GLuint count, GLsizei bufsize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog); /* 1160 */
   void (GLAPIENTRYP GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label); /* 1161 */
   void (GLAPIENTRYP GetObjectPtrLabel)(const GLvoid * ptr, GLsizei bufSize, GLsizei * length, GLchar * label); /* 1162 */
   void (GLAPIENTRYP ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar * label); /* 1163 */
   void (GLAPIENTRYP ObjectPtrLabel)(const GLvoid * ptr, GLsizei length, const GLchar * label); /* 1164 */
   void (GLAPIENTRYP PopDebugGroup)(void); /* 1165 */
   void (GLAPIENTRYP PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message); /* 1166 */
   void (GLAPIENTRYP SecondaryColor3fEXT)(GLfloat red, GLfloat green, GLfloat blue); /* 1167 */
   void (GLAPIENTRYP SecondaryColor3fvEXT)(const GLfloat * v); /* 1168 */
   void (GLAPIENTRYP MultiDrawElementsEXT)(GLenum mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount); /* 1169 */
   void (GLAPIENTRYP FogCoordfEXT)(GLfloat coord); /* 1170 */
   void (GLAPIENTRYP FogCoordfvEXT)(const GLfloat * coord); /* 1171 */
   void (GLAPIENTRYP ResizeBuffersMESA)(void); /* 1172 */
   void (GLAPIENTRYP WindowPos4dMESA)(GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 1173 */
   void (GLAPIENTRYP WindowPos4dvMESA)(const GLdouble * v); /* 1174 */
   void (GLAPIENTRYP WindowPos4fMESA)(GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 1175 */
   void (GLAPIENTRYP WindowPos4fvMESA)(const GLfloat * v); /* 1176 */
   void (GLAPIENTRYP WindowPos4iMESA)(GLint x, GLint y, GLint z, GLint w); /* 1177 */
   void (GLAPIENTRYP WindowPos4ivMESA)(const GLint * v); /* 1178 */
   void (GLAPIENTRYP WindowPos4sMESA)(GLshort x, GLshort y, GLshort z, GLshort w); /* 1179 */
   void (GLAPIENTRYP WindowPos4svMESA)(const GLshort * v); /* 1180 */
   void (GLAPIENTRYP MultiModeDrawArraysIBM)(const GLenum * mode, const GLint * first, const GLsizei * count, GLsizei primcount, GLint modestride); /* 1181 */
   void (GLAPIENTRYP MultiModeDrawElementsIBM)(const GLenum * mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, GLint modestride); /* 1182 */
   GLboolean (GLAPIENTRYP AreProgramsResidentNV)(GLsizei n, const GLuint * ids, GLboolean * residences); /* 1183 */
   void (GLAPIENTRYP ExecuteProgramNV)(GLenum target, GLuint id, const GLfloat * params); /* 1184 */
   void (GLAPIENTRYP GetProgramParameterdvNV)(GLenum target, GLuint index, GLenum pname, GLdouble * params); /* 1185 */
   void (GLAPIENTRYP GetProgramParameterfvNV)(GLenum target, GLuint index, GLenum pname, GLfloat * params); /* 1186 */
   void (GLAPIENTRYP GetProgramStringNV)(GLuint id, GLenum pname, GLubyte * program); /* 1187 */
   void (GLAPIENTRYP GetProgramivNV)(GLuint id, GLenum pname, GLint * params); /* 1188 */
   void (GLAPIENTRYP GetTrackMatrixivNV)(GLenum target, GLuint address, GLenum pname, GLint * params); /* 1189 */
   void (GLAPIENTRYP GetVertexAttribdvNV)(GLuint index, GLenum pname, GLdouble * params); /* 1190 */
   void (GLAPIENTRYP GetVertexAttribfvNV)(GLuint index, GLenum pname, GLfloat * params); /* 1191 */
   void (GLAPIENTRYP GetVertexAttribivNV)(GLuint index, GLenum pname, GLint * params); /* 1192 */
   void (GLAPIENTRYP LoadProgramNV)(GLenum target, GLuint id, GLsizei len, const GLubyte * program); /* 1193 */
   void (GLAPIENTRYP ProgramParameters4dvNV)(GLenum target, GLuint index, GLsizei num, const GLdouble * params); /* 1194 */
   void (GLAPIENTRYP ProgramParameters4fvNV)(GLenum target, GLuint index, GLsizei num, const GLfloat * params); /* 1195 */
   void (GLAPIENTRYP RequestResidentProgramsNV)(GLsizei n, const GLuint * ids); /* 1196 */
   void (GLAPIENTRYP TrackMatrixNV)(GLenum target, GLuint address, GLenum matrix, GLenum transform); /* 1197 */
   void (GLAPIENTRYP VertexAttrib1dNV)(GLuint index, GLdouble x); /* 1198 */
   void (GLAPIENTRYP VertexAttrib1dvNV)(GLuint index, const GLdouble * v); /* 1199 */
   void (GLAPIENTRYP VertexAttrib1fNV)(GLuint index, GLfloat x); /* 1200 */
   void (GLAPIENTRYP VertexAttrib1fvNV)(GLuint index, const GLfloat * v); /* 1201 */
   void (GLAPIENTRYP VertexAttrib1sNV)(GLuint index, GLshort x); /* 1202 */
   void (GLAPIENTRYP VertexAttrib1svNV)(GLuint index, const GLshort * v); /* 1203 */
   void (GLAPIENTRYP VertexAttrib2dNV)(GLuint index, GLdouble x, GLdouble y); /* 1204 */
   void (GLAPIENTRYP VertexAttrib2dvNV)(GLuint index, const GLdouble * v); /* 1205 */
   void (GLAPIENTRYP VertexAttrib2fNV)(GLuint index, GLfloat x, GLfloat y); /* 1206 */
   void (GLAPIENTRYP VertexAttrib2fvNV)(GLuint index, const GLfloat * v); /* 1207 */
   void (GLAPIENTRYP VertexAttrib2sNV)(GLuint index, GLshort x, GLshort y); /* 1208 */
   void (GLAPIENTRYP VertexAttrib2svNV)(GLuint index, const GLshort * v); /* 1209 */
   void (GLAPIENTRYP VertexAttrib3dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z); /* 1210 */
   void (GLAPIENTRYP VertexAttrib3dvNV)(GLuint index, const GLdouble * v); /* 1211 */
   void (GLAPIENTRYP VertexAttrib3fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z); /* 1212 */
   void (GLAPIENTRYP VertexAttrib3fvNV)(GLuint index, const GLfloat * v); /* 1213 */
   void (GLAPIENTRYP VertexAttrib3sNV)(GLuint index, GLshort x, GLshort y, GLshort z); /* 1214 */
   void (GLAPIENTRYP VertexAttrib3svNV)(GLuint index, const GLshort * v); /* 1215 */
   void (GLAPIENTRYP VertexAttrib4dNV)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 1216 */
   void (GLAPIENTRYP VertexAttrib4dvNV)(GLuint index, const GLdouble * v); /* 1217 */
   void (GLAPIENTRYP VertexAttrib4fNV)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 1218 */
   void (GLAPIENTRYP VertexAttrib4fvNV)(GLuint index, const GLfloat * v); /* 1219 */
   void (GLAPIENTRYP VertexAttrib4sNV)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w); /* 1220 */
   void (GLAPIENTRYP VertexAttrib4svNV)(GLuint index, const GLshort * v); /* 1221 */
   void (GLAPIENTRYP VertexAttrib4ubNV)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w); /* 1222 */
   void (GLAPIENTRYP VertexAttrib4ubvNV)(GLuint index, const GLubyte * v); /* 1223 */
   void (GLAPIENTRYP VertexAttribPointerNV)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer); /* 1224 */
   void (GLAPIENTRYP VertexAttribs1dvNV)(GLuint index, GLsizei n, const GLdouble * v); /* 1225 */
   void (GLAPIENTRYP VertexAttribs1fvNV)(GLuint index, GLsizei n, const GLfloat * v); /* 1226 */
   void (GLAPIENTRYP VertexAttribs1svNV)(GLuint index, GLsizei n, const GLshort * v); /* 1227 */
   void (GLAPIENTRYP VertexAttribs2dvNV)(GLuint index, GLsizei n, const GLdouble * v); /* 1228 */
   void (GLAPIENTRYP VertexAttribs2fvNV)(GLuint index, GLsizei n, const GLfloat * v); /* 1229 */
   void (GLAPIENTRYP VertexAttribs2svNV)(GLuint index, GLsizei n, const GLshort * v); /* 1230 */
   void (GLAPIENTRYP VertexAttribs3dvNV)(GLuint index, GLsizei n, const GLdouble * v); /* 1231 */
   void (GLAPIENTRYP VertexAttribs3fvNV)(GLuint index, GLsizei n, const GLfloat * v); /* 1232 */
   void (GLAPIENTRYP VertexAttribs3svNV)(GLuint index, GLsizei n, const GLshort * v); /* 1233 */
   void (GLAPIENTRYP VertexAttribs4dvNV)(GLuint index, GLsizei n, const GLdouble * v); /* 1234 */
   void (GLAPIENTRYP VertexAttribs4fvNV)(GLuint index, GLsizei n, const GLfloat * v); /* 1235 */
   void (GLAPIENTRYP VertexAttribs4svNV)(GLuint index, GLsizei n, const GLshort * v); /* 1236 */
   void (GLAPIENTRYP VertexAttribs4ubvNV)(GLuint index, GLsizei n, const GLubyte * v); /* 1237 */
   void (GLAPIENTRYP GetTexBumpParameterfvATI)(GLenum pname, GLfloat * param); /* 1238 */
   void (GLAPIENTRYP GetTexBumpParameterivATI)(GLenum pname, GLint * param); /* 1239 */
   void (GLAPIENTRYP TexBumpParameterfvATI)(GLenum pname, const GLfloat * param); /* 1240 */
   void (GLAPIENTRYP TexBumpParameterivATI)(GLenum pname, const GLint * param); /* 1241 */
   void (GLAPIENTRYP AlphaFragmentOp1ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod); /* 1242 */
   void (GLAPIENTRYP AlphaFragmentOp2ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod); /* 1243 */
   void (GLAPIENTRYP AlphaFragmentOp3ATI)(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod); /* 1244 */
   void (GLAPIENTRYP BeginFragmentShaderATI)(void); /* 1245 */
   void (GLAPIENTRYP BindFragmentShaderATI)(GLuint id); /* 1246 */
   void (GLAPIENTRYP ColorFragmentOp1ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod); /* 1247 */
   void (GLAPIENTRYP ColorFragmentOp2ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod); /* 1248 */
   void (GLAPIENTRYP ColorFragmentOp3ATI)(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod); /* 1249 */
   void (GLAPIENTRYP DeleteFragmentShaderATI)(GLuint id); /* 1250 */
   void (GLAPIENTRYP EndFragmentShaderATI)(void); /* 1251 */
   GLuint (GLAPIENTRYP GenFragmentShadersATI)(GLuint range); /* 1252 */
   void (GLAPIENTRYP PassTexCoordATI)(GLuint dst, GLuint coord, GLenum swizzle); /* 1253 */
   void (GLAPIENTRYP SampleMapATI)(GLuint dst, GLuint interp, GLenum swizzle); /* 1254 */
   void (GLAPIENTRYP SetFragmentShaderConstantATI)(GLuint dst, const GLfloat * value); /* 1255 */
   void (GLAPIENTRYP DepthRangeArrayfvOES)(GLuint first, GLsizei count, const GLfloat * v); /* 1256 */
   void (GLAPIENTRYP DepthRangeIndexedfOES)(GLuint index, GLfloat n, GLfloat f); /* 1257 */
   void (GLAPIENTRYP ActiveStencilFaceEXT)(GLenum face); /* 1258 */
   void (GLAPIENTRYP GetProgramNamedParameterdvNV)(GLuint id, GLsizei len, const GLubyte * name, GLdouble * params); /* 1259 */
   void (GLAPIENTRYP GetProgramNamedParameterfvNV)(GLuint id, GLsizei len, const GLubyte * name, GLfloat * params); /* 1260 */
   void (GLAPIENTRYP ProgramNamedParameter4dNV)(GLuint id, GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z, GLdouble w); /* 1261 */
   void (GLAPIENTRYP ProgramNamedParameter4dvNV)(GLuint id, GLsizei len, const GLubyte * name, const GLdouble * v); /* 1262 */
   void (GLAPIENTRYP ProgramNamedParameter4fNV)(GLuint id, GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w); /* 1263 */
   void (GLAPIENTRYP ProgramNamedParameter4fvNV)(GLuint id, GLsizei len, const GLubyte * name, const GLfloat * v); /* 1264 */
   void (GLAPIENTRYP PrimitiveRestartNV)(void); /* 1265 */
   void (GLAPIENTRYP GetTexGenxvOES)(GLenum coord, GLenum pname, GLfixed * params); /* 1266 */
   void (GLAPIENTRYP TexGenxOES)(GLenum coord, GLenum pname, GLint param); /* 1267 */
   void (GLAPIENTRYP TexGenxvOES)(GLenum coord, GLenum pname, const GLfixed * params); /* 1268 */
   void (GLAPIENTRYP DepthBoundsEXT)(GLclampd zmin, GLclampd zmax); /* 1269 */
   void (GLAPIENTRYP BindFramebufferEXT)(GLenum target, GLuint framebuffer); /* 1270 */
   void (GLAPIENTRYP BindRenderbufferEXT)(GLenum target, GLuint renderbuffer); /* 1271 */
   void (GLAPIENTRYP StringMarkerGREMEDY)(GLsizei len, const GLvoid * string); /* 1272 */
   void (GLAPIENTRYP BufferParameteriAPPLE)(GLenum target, GLenum pname, GLint param); /* 1273 */
   void (GLAPIENTRYP FlushMappedBufferRangeAPPLE)(GLenum target, GLintptr offset, GLsizeiptr size); /* 1274 */
   void (GLAPIENTRYP VertexAttribI1iEXT)(GLuint index, GLint x); /* 1275 */
   void (GLAPIENTRYP VertexAttribI1uiEXT)(GLuint index, GLuint x); /* 1276 */
   void (GLAPIENTRYP VertexAttribI2iEXT)(GLuint index, GLint x, GLint y); /* 1277 */
   void (GLAPIENTRYP VertexAttribI2ivEXT)(GLuint index, const GLint * v); /* 1278 */
   void (GLAPIENTRYP VertexAttribI2uiEXT)(GLuint index, GLuint x, GLuint y); /* 1279 */
   void (GLAPIENTRYP VertexAttribI2uivEXT)(GLuint index, const GLuint * v); /* 1280 */
   void (GLAPIENTRYP VertexAttribI3iEXT)(GLuint index, GLint x, GLint y, GLint z); /* 1281 */
   void (GLAPIENTRYP VertexAttribI3ivEXT)(GLuint index, const GLint * v); /* 1282 */
   void (GLAPIENTRYP VertexAttribI3uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z); /* 1283 */
   void (GLAPIENTRYP VertexAttribI3uivEXT)(GLuint index, const GLuint * v); /* 1284 */
   void (GLAPIENTRYP VertexAttribI4iEXT)(GLuint index, GLint x, GLint y, GLint z, GLint w); /* 1285 */
   void (GLAPIENTRYP VertexAttribI4ivEXT)(GLuint index, const GLint * v); /* 1286 */
   void (GLAPIENTRYP VertexAttribI4uiEXT)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w); /* 1287 */
   void (GLAPIENTRYP VertexAttribI4uivEXT)(GLuint index, const GLuint * v); /* 1288 */
   void (GLAPIENTRYP ClearColorIiEXT)(GLint r, GLint g, GLint b, GLint a); /* 1289 */
   void (GLAPIENTRYP ClearColorIuiEXT)(GLuint r, GLuint g, GLuint b, GLuint a); /* 1290 */
   void (GLAPIENTRYP BindBufferOffsetEXT)(GLenum target, GLuint index, GLuint buffer, GLintptr offset); /* 1291 */
   void (GLAPIENTRYP BeginPerfMonitorAMD)(GLuint monitor); /* 1292 */
   void (GLAPIENTRYP DeletePerfMonitorsAMD)(GLsizei n, GLuint * monitors); /* 1293 */
   void (GLAPIENTRYP EndPerfMonitorAMD)(GLuint monitor); /* 1294 */
   void (GLAPIENTRYP GenPerfMonitorsAMD)(GLsizei n, GLuint * monitors); /* 1295 */
   void (GLAPIENTRYP GetPerfMonitorCounterDataAMD)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint * data, GLint * bytesWritten); /* 1296 */
   void (GLAPIENTRYP GetPerfMonitorCounterInfoAMD)(GLuint group, GLuint counter, GLenum pname, GLvoid * data); /* 1297 */
   void (GLAPIENTRYP GetPerfMonitorCounterStringAMD)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei * length, GLchar * counterString); /* 1298 */
   void (GLAPIENTRYP GetPerfMonitorCountersAMD)(GLuint group, GLint * numCounters, GLint * maxActiveCounters, GLsizei countersSize, GLuint * counters); /* 1299 */
   void (GLAPIENTRYP GetPerfMonitorGroupStringAMD)(GLuint group, GLsizei bufSize, GLsizei * length, GLchar * groupString); /* 1300 */
   void (GLAPIENTRYP GetPerfMonitorGroupsAMD)(GLint * numGroups, GLsizei groupsSize, GLuint * groups); /* 1301 */
   void (GLAPIENTRYP SelectPerfMonitorCountersAMD)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint * counterList); /* 1302 */
   void (GLAPIENTRYP GetObjectParameterivAPPLE)(GLenum objectType, GLuint name, GLenum pname, GLint * value); /* 1303 */
   GLenum (GLAPIENTRYP ObjectPurgeableAPPLE)(GLenum objectType, GLuint name, GLenum option); /* 1304 */
   GLenum (GLAPIENTRYP ObjectUnpurgeableAPPLE)(GLenum objectType, GLuint name, GLenum option); /* 1305 */
   void (GLAPIENTRYP ActiveProgramEXT)(GLuint program); /* 1306 */
   GLuint (GLAPIENTRYP CreateShaderProgramEXT)(GLenum type, const GLchar * string); /* 1307 */
   void (GLAPIENTRYP UseShaderProgramEXT)(GLenum type, GLuint program); /* 1308 */
   void (GLAPIENTRYP TextureBarrierNV)(void); /* 1309 */
   void (GLAPIENTRYP VDPAUFiniNV)(void); /* 1310 */
   void (GLAPIENTRYP VDPAUGetSurfaceivNV)(GLintptr surface, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values); /* 1311 */
   void (GLAPIENTRYP VDPAUInitNV)(const GLvoid * vdpDevice, const GLvoid * getProcAddress); /* 1312 */
   GLboolean (GLAPIENTRYP VDPAUIsSurfaceNV)(GLintptr surface); /* 1313 */
   void (GLAPIENTRYP VDPAUMapSurfacesNV)(GLsizei numSurfaces, const GLintptr * surfaces); /* 1314 */
   GLintptr (GLAPIENTRYP VDPAURegisterOutputSurfaceNV)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames); /* 1315 */
   GLintptr (GLAPIENTRYP VDPAURegisterVideoSurfaceNV)(const GLvoid * vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint * textureNames); /* 1316 */
   void (GLAPIENTRYP VDPAUSurfaceAccessNV)(GLintptr surface, GLenum access); /* 1317 */
   void (GLAPIENTRYP VDPAUUnmapSurfacesNV)(GLsizei numSurfaces, const GLintptr * surfaces); /* 1318 */
   void (GLAPIENTRYP VDPAUUnregisterSurfaceNV)(GLintptr surface); /* 1319 */
   void (GLAPIENTRYP BeginPerfQueryINTEL)(GLuint queryHandle); /* 1320 */
   void (GLAPIENTRYP CreatePerfQueryINTEL)(GLuint queryId, GLuint * queryHandle); /* 1321 */
   void (GLAPIENTRYP DeletePerfQueryINTEL)(GLuint queryHandle); /* 1322 */
   void (GLAPIENTRYP EndPerfQueryINTEL)(GLuint queryHandle); /* 1323 */
   void (GLAPIENTRYP GetFirstPerfQueryIdINTEL)(GLuint * queryId); /* 1324 */
   void (GLAPIENTRYP GetNextPerfQueryIdINTEL)(GLuint queryId, GLuint * nextQueryId); /* 1325 */
   void (GLAPIENTRYP GetPerfCounterInfoINTEL)(GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar * counterName, GLuint counterDescLength, GLchar * counterDesc, GLuint * counterOffset, GLuint * counterDataSize, GLuint * counterTypeEnum, GLuint * counterDataTypeEnum, GLuint64 * rawCounterMaxValue); /* 1326 */
   void (GLAPIENTRYP GetPerfQueryDataINTEL)(GLuint queryHandle, GLuint flags, GLsizei dataSize, GLvoid * data, GLuint * bytesWritten); /* 1327 */
   void (GLAPIENTRYP GetPerfQueryIdByNameINTEL)(GLchar * queryName, GLuint * queryId); /* 1328 */
   void (GLAPIENTRYP GetPerfQueryInfoINTEL)(GLuint queryId, GLuint queryNameLength, GLchar * queryName, GLuint * dataSize, GLuint * noCounters, GLuint * noInstances, GLuint * capsMask); /* 1329 */
   void (GLAPIENTRYP PolygonOffsetClampEXT)(GLfloat factor, GLfloat units, GLfloat clamp); /* 1330 */
   void (GLAPIENTRYP WindowRectanglesEXT)(GLenum mode, GLsizei count, const GLint * box); /* 1331 */
   void (GLAPIENTRYP StencilFuncSeparateATI)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask); /* 1332 */
   void (GLAPIENTRYP ProgramEnvParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat * params); /* 1333 */
   void (GLAPIENTRYP ProgramLocalParameters4fvEXT)(GLenum target, GLuint index, GLsizei count, const GLfloat * params); /* 1334 */
   void (GLAPIENTRYP EGLImageTargetRenderbufferStorageOES)(GLenum target, GLvoid * writeOffset); /* 1335 */
   void (GLAPIENTRYP EGLImageTargetTexture2DOES)(GLenum target, GLvoid * writeOffset); /* 1336 */
   void (GLAPIENTRYP AlphaFuncx)(GLenum func, GLclampx ref); /* 1337 */
   void (GLAPIENTRYP ClearColorx)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha); /* 1338 */
   void (GLAPIENTRYP ClearDepthx)(GLclampx depth); /* 1339 */
   void (GLAPIENTRYP Color4x)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha); /* 1340 */
   void (GLAPIENTRYP DepthRangex)(GLclampx zNear, GLclampx zFar); /* 1341 */
   void (GLAPIENTRYP Fogx)(GLenum pname, GLfixed param); /* 1342 */
   void (GLAPIENTRYP Fogxv)(GLenum pname, const GLfixed * params); /* 1343 */
   void (GLAPIENTRYP Frustumf)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar); /* 1344 */
   void (GLAPIENTRYP Frustumx)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar); /* 1345 */
   void (GLAPIENTRYP LightModelx)(GLenum pname, GLfixed param); /* 1346 */
   void (GLAPIENTRYP LightModelxv)(GLenum pname, const GLfixed * params); /* 1347 */
   void (GLAPIENTRYP Lightx)(GLenum light, GLenum pname, GLfixed param); /* 1348 */
   void (GLAPIENTRYP Lightxv)(GLenum light, GLenum pname, const GLfixed * params); /* 1349 */
   void (GLAPIENTRYP LineWidthx)(GLfixed width); /* 1350 */
   void (GLAPIENTRYP LoadMatrixx)(const GLfixed * m); /* 1351 */
   void (GLAPIENTRYP Materialx)(GLenum face, GLenum pname, GLfixed param); /* 1352 */
   void (GLAPIENTRYP Materialxv)(GLenum face, GLenum pname, const GLfixed * params); /* 1353 */
   void (GLAPIENTRYP MultMatrixx)(const GLfixed * m); /* 1354 */
   void (GLAPIENTRYP MultiTexCoord4x)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q); /* 1355 */
   void (GLAPIENTRYP Normal3x)(GLfixed nx, GLfixed ny, GLfixed nz); /* 1356 */
   void (GLAPIENTRYP Orthof)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar); /* 1357 */
   void (GLAPIENTRYP Orthox)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar); /* 1358 */
   void (GLAPIENTRYP PointSizex)(GLfixed size); /* 1359 */
   void (GLAPIENTRYP PolygonOffsetx)(GLfixed factor, GLfixed units); /* 1360 */
   void (GLAPIENTRYP Rotatex)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z); /* 1361 */
   void (GLAPIENTRYP SampleCoveragex)(GLclampx value, GLboolean invert); /* 1362 */
   void (GLAPIENTRYP Scalex)(GLfixed x, GLfixed y, GLfixed z); /* 1363 */
   void (GLAPIENTRYP TexEnvx)(GLenum target, GLenum pname, GLfixed param); /* 1364 */
   void (GLAPIENTRYP TexEnvxv)(GLenum target, GLenum pname, const GLfixed * params); /* 1365 */
   void (GLAPIENTRYP TexParameterx)(GLenum target, GLenum pname, GLfixed param); /* 1366 */
   void (GLAPIENTRYP Translatex)(GLfixed x, GLfixed y, GLfixed z); /* 1367 */
   void (GLAPIENTRYP ClipPlanef)(GLenum plane, const GLfloat * equation); /* 1368 */
   void (GLAPIENTRYP ClipPlanex)(GLenum plane, const GLfixed * equation); /* 1369 */
   void (GLAPIENTRYP GetClipPlanef)(GLenum plane, GLfloat * equation); /* 1370 */
   void (GLAPIENTRYP GetClipPlanex)(GLenum plane, GLfixed * equation); /* 1371 */
   void (GLAPIENTRYP GetFixedv)(GLenum pname, GLfixed * params); /* 1372 */
   void (GLAPIENTRYP GetLightxv)(GLenum light, GLenum pname, GLfixed * params); /* 1373 */
   void (GLAPIENTRYP GetMaterialxv)(GLenum face, GLenum pname, GLfixed * params); /* 1374 */
   void (GLAPIENTRYP GetTexEnvxv)(GLenum target, GLenum pname, GLfixed * params); /* 1375 */
   void (GLAPIENTRYP GetTexParameterxv)(GLenum target, GLenum pname, GLfixed * params); /* 1376 */
   void (GLAPIENTRYP PointParameterx)(GLenum pname, GLfixed param); /* 1377 */
   void (GLAPIENTRYP PointParameterxv)(GLenum pname, const GLfixed * params); /* 1378 */
   void (GLAPIENTRYP TexParameterxv)(GLenum target, GLenum pname, const GLfixed * params); /* 1379 */
   void (GLAPIENTRYP BlendBarrier)(void); /* 1380 */
   void (GLAPIENTRYP PrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW); /* 1381 */
};

#ifdef __cplusplus
}
#endif

#endif /* !defined( _GLAPI_TABLE_H_ ) */
