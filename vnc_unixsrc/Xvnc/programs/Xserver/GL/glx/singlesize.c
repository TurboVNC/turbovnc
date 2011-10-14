/* $XFree86: xc/programs/Xserver/GL/glx/singlesize.c,v 1.7tsi Exp $ */
/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
** 
** http://oss.sgi.com/projects/FreeB
** 
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
** 
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
** 
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
**
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <GL/gl.h>
#include "glxserver.h"
#include "singlesize.h"

/*
** These routines compute the size of variable-size returned parameters.
** Unlike the similar routines that do the same thing for variable-size
** incoming parameters, the samplegl library itself doesn't use these routines.
** Hence, they are located here, in the GLX extension library.
*/

GLint __glReadPixels_size(GLenum format, GLenum type, GLint w, GLint h)
{
    return __glXImageSize( format, type, 0, w, h, 1, 0, 0, 0, 0, 4 );
}

/**
 * Determine the number of data elements that go with the specified \c pname
 * to a \c glGetTexEnvfv or \c glGetTexEnviv call.
 * 
 * \todo
 * Replace this function with a call to \c __glTexEnvfv_size.  Make that there
 * aren't any values of \c pname that are valid for one but not the other.
 */
GLint __glGetTexEnvfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
      case GL_TEXTURE_LOD_BIAS:
      case GL_COMBINE_RGB:
      case GL_COMBINE_ALPHA:
      case GL_SOURCE0_RGB:
      case GL_SOURCE1_RGB:
      case GL_SOURCE2_RGB:
      case GL_SOURCE0_ALPHA:
      case GL_SOURCE1_ALPHA:
      case GL_SOURCE2_ALPHA:
      case GL_OPERAND0_RGB:
      case GL_OPERAND1_RGB:
      case GL_OPERAND2_RGB:
      case GL_OPERAND0_ALPHA:
      case GL_OPERAND1_ALPHA:
      case GL_OPERAND2_ALPHA:
      case GL_RGB_SCALE:
      case GL_ALPHA_SCALE:
	
      /* GL_ARB_point_sprite / GL_NV_point_sprite */
      case GL_COORD_REPLACE_ARB:

      /* GL_NV_texture_env_combine4 */
      case GL_SOURCE3_RGB_NV:
      case GL_SOURCE3_ALPHA_NV:
      case GL_OPERAND3_RGB_NV:
      case GL_OPERAND3_ALPHA_NV:
	return 1;

      case GL_TEXTURE_ENV_COLOR:
	return 4;
      default:
	return -1;
    }
}


GLint __glGetTexEnviv_size(GLenum pname)
{
    return __glGetTexEnvfv_size(pname);
}

GLint __glGetTexGenfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	return 1;
      case GL_OBJECT_PLANE:
	return 4;
      case GL_EYE_PLANE:
	return 4;
      default:
	return -1;
    }
}

GLint __glGetTexGendv_size(GLenum pname)
{
    return __glGetTexGenfv_size(pname);
}

GLint __glGetTexGeniv_size(GLenum pname)
{
    return __glGetTexGenfv_size(pname);
}

GLint __glGetTexParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_BORDER_COLOR:
	return 4;

      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
      case GL_TEXTURE_RESIDENT:
       
      /* GL_SGIS_texture_lod / GL_EXT_texture_lod / GL 1.2 */
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:

      /* GL_SGIX_texture_lod_bias */
      case GL_TEXTURE_LOD_BIAS_S_SGIX:
      case GL_TEXTURE_LOD_BIAS_T_SGIX:
      case GL_TEXTURE_LOD_BIAS_R_SGIX:

      /* GL_ARB_shadow / GL 1.4 */
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:

      /* GL_SGIS_generate_mipmap / GL 1.4 */
      case GL_GENERATE_MIPMAP:

      /* GL_ARB_depth_texture / GL 1.4 */
      case GL_DEPTH_TEXTURE_MODE:

      /* GL_EXT_texture_lod_bias / GL 1.4 */
      case GL_TEXTURE_LOD_BIAS:

      /* GL_SGIX_shadow_ambient / GL_ARB_shadow_ambient */
      case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:

      /* GL_SGIX_shadow */
      case GL_TEXTURE_COMPARE_SGIX:
      case GL_TEXTURE_COMPARE_OPERATOR_SGIX:

      /* GL_SGIX_texture_coordinate_clamp */
      case GL_TEXTURE_MAX_CLAMP_S_SGIX:
      case GL_TEXTURE_MAX_CLAMP_T_SGIX:
      case GL_TEXTURE_MAX_CLAMP_R_SGIX:

      /* GL_EXT_texture_filter_anisotropic */
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:

      /* GL_NV_texture_expand_normal */
      case GL_TEXTURE_UNSIGNED_REMAP_MODE_NV:
	return 1;

      default:
	return -1;
    }
}

GLint __glGetTexParameteriv_size(GLenum pname)
{
    return __glGetTexParameterfv_size(pname);
}

GLint __glGetLightfv_size(GLenum pname)
{
    switch (pname) {
      case GL_AMBIENT:
	return 4;
      case GL_DIFFUSE:
	return 4;
      case GL_SPECULAR:
	return 4;
      case GL_POSITION:
	return 4;
      case GL_SPOT_DIRECTION:
	return 3;
      case GL_SPOT_EXPONENT:
	return 1;
      case GL_SPOT_CUTOFF:
	return 1;
      case GL_CONSTANT_ATTENUATION:
	return 1;
      case GL_LINEAR_ATTENUATION:
	return 1;
      case GL_QUADRATIC_ATTENUATION:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetLightiv_size(GLenum pname)
{
    return __glGetLightfv_size(pname);
}

GLint __glGetMap_size(GLenum target, GLenum query)
{
    GLint k, order=0, majorMinor[2];

    /*
    ** Assume target and query are both valid.
    */
    switch (target) {
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
	switch (query) {
	  case GL_COEFF:
	    k = __glMap1d_size(target);
	    glGetMapiv(target, GL_ORDER, &order);
	    /*
	    ** The query above might fail, but then order will be zero anyway.
	    */
	    return (order * k);
	  case GL_DOMAIN:
	    return 2;
	  case GL_ORDER:
	    return 1;
	}
	break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
	switch (query) {
	  case GL_COEFF:
	    k = __glMap2d_size(target);
	    majorMinor[0] = majorMinor[1] = 0;
	    glGetMapiv(target, GL_ORDER, majorMinor);
	    /*
	    ** The query above might fail, but then majorMinor will be zeroes
	    */
	    return (majorMinor[0] * majorMinor[1] * k);
	  case GL_DOMAIN:
	    return 4;
	  case GL_ORDER:
	    return 2;
	}
	break;
    }
    return -1;
}

GLint __glGetMapdv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMapfv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMapiv_size(GLenum target, GLenum query)
{
    return __glGetMap_size(target, query);
}

GLint __glGetMaterialfv_size(GLenum pname)
{
    switch (pname) {
      case GL_SHININESS:
	return 1;
      case GL_COLOR_INDEXES:
	return 3;
      case GL_EMISSION:
	return 4;
      case GL_AMBIENT:
	return 4;
      case GL_DIFFUSE:
	return 4;
      case GL_SPECULAR:
	return 4;
      case GL_AMBIENT_AND_DIFFUSE:
	return 4;
      default:
	return -1;
    }
}

GLint __glGetMaterialiv_size(GLenum pname)
{
    return __glGetMaterialfv_size(pname);
}

GLint __glGetPixelMap_size(GLenum map)
{
    GLint size;
    GLenum query;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
	query = GL_PIXEL_MAP_I_TO_I_SIZE;
	break;
      case GL_PIXEL_MAP_S_TO_S:
	query = GL_PIXEL_MAP_S_TO_S_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_R:
	query = GL_PIXEL_MAP_I_TO_R_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_G:
	query = GL_PIXEL_MAP_I_TO_G_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_B:
	query = GL_PIXEL_MAP_I_TO_B_SIZE;
	break;
      case GL_PIXEL_MAP_I_TO_A:
	query = GL_PIXEL_MAP_I_TO_A_SIZE;
	break;
      case GL_PIXEL_MAP_R_TO_R:
	query = GL_PIXEL_MAP_R_TO_R_SIZE;
	break;
      case GL_PIXEL_MAP_G_TO_G:
	query = GL_PIXEL_MAP_G_TO_G_SIZE;
	break;
      case GL_PIXEL_MAP_B_TO_B:
	query = GL_PIXEL_MAP_B_TO_B_SIZE;
	break;
      case GL_PIXEL_MAP_A_TO_A:
	query = GL_PIXEL_MAP_A_TO_A_SIZE;
	break;
      default:
	return -1;
    }
    glGetIntegerv(query, &size);
    return size;
}

GLint __glGetPixelMapfv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

GLint __glGetPixelMapuiv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

GLint __glGetPixelMapusv_size(GLenum map)
{
    return __glGetPixelMap_size(map);
}

/*
** Return the number of words needed to pass back the requested
** value.
*/
GLint __glGet_size(GLenum sq)
{
    switch (sq) {
      case GL_MAX_TEXTURE_SIZE:
	return 1;
      case GL_SUBPIXEL_BITS:
	return 1;
      case GL_MAX_LIST_NESTING:
	return 1;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_INDEX:
      case GL_MAP1_NORMAL:
      case GL_MAP1_TEXTURE_COORD_1:
      case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3:
      case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3:
      case GL_MAP1_VERTEX_4:
	return 1;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_INDEX:
      case GL_MAP2_NORMAL:
      case GL_MAP2_TEXTURE_COORD_1:
      case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3:
      case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3:
      case GL_MAP2_VERTEX_4:
	return 1;
      case GL_AUTO_NORMAL:
	return 1;
      case GL_CURRENT_COLOR:
	return 4;
      case GL_CURRENT_INDEX:
	return 1;
      case GL_CURRENT_NORMAL:
	return 3;
      case GL_CURRENT_TEXTURE_COORDS:
	return 4;
      case GL_CURRENT_RASTER_INDEX:
	return 1;
      case GL_CURRENT_RASTER_COLOR:
	return 4;
      case GL_CURRENT_RASTER_TEXTURE_COORDS:
	return 4;
      case GL_CURRENT_RASTER_POSITION:
	return 4;
      case GL_CURRENT_RASTER_POSITION_VALID:
	return 1;
      case GL_CURRENT_RASTER_DISTANCE:
	return 1;
      case GL_POINT_SIZE:
	return 1;
      case GL_POINT_SIZE_RANGE:
      /* case GL_SMOOTH_POINT_SIZE_RANGE: */ /* alias */
	return 2;
      case GL_POINT_SIZE_GRANULARITY:
      /* case GL_SMOOTH_POINT_SIZE_GRANULARITY: */ /* alias */
	return 1;
      case GL_ALIASED_POINT_SIZE_RANGE:
	return 2;
      case GL_POINT_SMOOTH:
	return 1;
      case GL_LINE_SMOOTH:
	return 1;
      case GL_LINE_WIDTH:
	return 1;
      case GL_LINE_WIDTH_RANGE:
      /* case GL_SMOOTH_LINE_WIDTH_RANGE: */ /* alias */
	return 2;
      case GL_LINE_WIDTH_GRANULARITY:
      /* case GL_SMOOTH_LINE_WIDTH_GRANULARITY: */ /* alias */
	return 1;
      case GL_ALIASED_LINE_WIDTH_RANGE:
	return 2;
      case GL_LINE_STIPPLE_PATTERN:
	return 1;
      case GL_LINE_STIPPLE_REPEAT:
	return 1;
      case GL_LINE_STIPPLE:
	return 1;
      case GL_POLYGON_MODE:
	return 2;
      case GL_POLYGON_SMOOTH:
	return 1;
      case GL_POLYGON_STIPPLE:
	return 1;
      case GL_EDGE_FLAG:
	return 1;
      case GL_CULL_FACE:
	return 1;
      case GL_CULL_FACE_MODE:
	return 1;
      case GL_FRONT_FACE:
	return 1;
      case GL_LIGHTING:
	return 1;
      case GL_LIGHT_MODEL_LOCAL_VIEWER:
	return 1;
      case GL_LIGHT_MODEL_TWO_SIDE:
	return 1;
      case GL_LIGHT_MODEL_AMBIENT:
	return 4;
      case GL_LIGHT_MODEL_COLOR_CONTROL:
	return 1;
      case GL_COLOR_MATERIAL:
	return 1;
      case GL_COLOR_MATERIAL_FACE:
	return 1;
      case GL_COLOR_MATERIAL_PARAMETER:
	return 1;
      case GL_SHADE_MODEL:
	return 1;
      case GL_FOG:
	return 1;
      case GL_FOG_INDEX:
	return 1;
      case GL_FOG_DENSITY:
	return 1;
      case GL_FOG_START:
	return 1;
      case GL_FOG_END:
	return 1;
      case GL_FOG_MODE:
	return 1;
      case GL_FOG_COLOR:
	return 4;
      case GL_DEPTH_RANGE:
	return 2;
      case GL_DEPTH_TEST:
	return 1;
      case GL_DEPTH_WRITEMASK:
	return 1;
      case GL_DEPTH_CLEAR_VALUE:
	return 1;
      case GL_DEPTH_FUNC:
	return 1;
      case GL_ACCUM_CLEAR_VALUE:
	return 4;
      case GL_STENCIL_TEST:
	return 1;
      case GL_STENCIL_CLEAR_VALUE:
	return 1;
      case GL_STENCIL_FUNC:
	return 1;
      case GL_STENCIL_VALUE_MASK:
	return 1;
      case GL_STENCIL_FAIL:
	return 1;
      case GL_STENCIL_PASS_DEPTH_FAIL:
	return 1;
      case GL_STENCIL_PASS_DEPTH_PASS:
	return 1;
      case GL_STENCIL_REF:
	return 1;
      case GL_STENCIL_WRITEMASK:
	return 1;
      case GL_MATRIX_MODE:
	return 1;
      case GL_NORMALIZE:
	return 1;
      case GL_VIEWPORT:
	return 4;
      case GL_ATTRIB_STACK_DEPTH:
	return 1;
      case GL_MODELVIEW_STACK_DEPTH:
	return 1;
      case GL_PROJECTION_STACK_DEPTH:
	return 1;
      case GL_TEXTURE_STACK_DEPTH:
	return 1;
      case GL_MODELVIEW_MATRIX:
	return 16;
      case GL_PROJECTION_MATRIX:
	return 16;
      case GL_TEXTURE_MATRIX:
	return 16;
      case GL_ALPHA_TEST:
	return 1;
      case GL_ALPHA_TEST_FUNC:
	return 1;
      case GL_ALPHA_TEST_REF:
	return 1;
      case GL_DITHER:
	return 1;
      case GL_BLEND_DST:
	return 1;
      case GL_BLEND_SRC:
	return 1;
      case GL_BLEND:
	return 1;
      case GL_LOGIC_OP_MODE:
	return 1;
      case GL_LOGIC_OP:
	return 1;
      case GL_DRAW_BUFFER:
	return 1;
      case GL_READ_BUFFER:
	return 1;
      case GL_SCISSOR_TEST:
	return 1;
      case GL_SCISSOR_BOX:
	return 4;
      case GL_INDEX_CLEAR_VALUE:
	return 1;
      case GL_INDEX_MODE:
	return 1;
      case GL_INDEX_WRITEMASK:
	return 1;
      case GL_COLOR_CLEAR_VALUE:
	return 4;
      case GL_RGBA_MODE:
	return 1;
      case GL_COLOR_WRITEMASK:
	return 4;
      case GL_RENDER_MODE:
	return 1;
      case GL_PERSPECTIVE_CORRECTION_HINT:
	return 1;
      case GL_POINT_SMOOTH_HINT:
	return 1;
      case GL_LINE_SMOOTH_HINT:
	return 1;
      case GL_POLYGON_SMOOTH_HINT:
	return 1;
      case GL_FOG_HINT:
	return 1;
      case GL_LIST_BASE:
	return 1;
      case GL_LIST_INDEX:
	return 1;
      case GL_LIST_MODE:
	return 1;
      case GL_TEXTURE_GEN_S:
	return 1;
      case GL_TEXTURE_GEN_T:
	return 1;
      case GL_TEXTURE_GEN_R:
	return 1;
      case GL_TEXTURE_GEN_Q:
	return 1;
      case GL_PACK_SWAP_BYTES:
	return 1;
      case GL_PACK_LSB_FIRST:
	return 1;
      case GL_PACK_ROW_LENGTH:
	return 1;
      case GL_PACK_IMAGE_HEIGHT:
	return 1;
      case GL_PACK_SKIP_ROWS:
	return 1;
      case GL_PACK_SKIP_PIXELS:
	return 1;
      case GL_PACK_SKIP_IMAGES:
	return 1;
      case GL_PACK_ALIGNMENT:
	return 1;
      case GL_UNPACK_SWAP_BYTES:
	return 1;
      case GL_UNPACK_LSB_FIRST:
	return 1;
      case GL_UNPACK_ROW_LENGTH:
	return 1;
      case GL_UNPACK_IMAGE_HEIGHT:
	return 1;
      case GL_UNPACK_SKIP_ROWS:
	return 1;
      case GL_UNPACK_SKIP_PIXELS:
	return 1;
      case GL_UNPACK_SKIP_IMAGES:
	return 1;
      case GL_UNPACK_ALIGNMENT:
	return 1;
      case GL_MAP_COLOR:
	return 1;
      case GL_MAP_STENCIL:
	return 1;
      case GL_INDEX_SHIFT:
	return 1;
      case GL_INDEX_OFFSET:
	return 1;
      case GL_RED_SCALE:
      case GL_GREEN_SCALE:
      case GL_BLUE_SCALE:
      case GL_ALPHA_SCALE:
      case GL_DEPTH_SCALE:
	return 1;
      case GL_RED_BIAS:
      case GL_GREEN_BIAS:
      case GL_BLUE_BIAS:
      case GL_ALPHA_BIAS:
      case GL_DEPTH_BIAS:
	return 1;
      case GL_ZOOM_X:
      case GL_ZOOM_Y:
	return 1;
      case GL_PIXEL_MAP_I_TO_I_SIZE:	  case GL_PIXEL_MAP_S_TO_S_SIZE:
      case GL_PIXEL_MAP_I_TO_R_SIZE:	  case GL_PIXEL_MAP_I_TO_G_SIZE:
      case GL_PIXEL_MAP_I_TO_B_SIZE:	  case GL_PIXEL_MAP_I_TO_A_SIZE:
      case GL_PIXEL_MAP_R_TO_R_SIZE:	  case GL_PIXEL_MAP_G_TO_G_SIZE:
      case GL_PIXEL_MAP_B_TO_B_SIZE:	  case GL_PIXEL_MAP_A_TO_A_SIZE:
	return 1;
      case GL_MAX_EVAL_ORDER:
	return 1;
      case GL_MAX_LIGHTS:
	return 1;
      case GL_MAX_CLIP_PLANES:
	return 1;
      case GL_MAX_PIXEL_MAP_TABLE:
	return 1;
      case GL_MAX_ATTRIB_STACK_DEPTH:
	return 1;
      case GL_MAX_MODELVIEW_STACK_DEPTH:
	return 1;
      case GL_MAX_NAME_STACK_DEPTH:
	return 1;
      case GL_MAX_PROJECTION_STACK_DEPTH:
	return 1;
      case GL_MAX_TEXTURE_STACK_DEPTH:
	return 1;
      case GL_INDEX_BITS:
	return 1;
      case GL_RED_BITS:
	return 1;
      case GL_GREEN_BITS:
	return 1;
      case GL_BLUE_BITS:
	return 1;
      case GL_ALPHA_BITS:
	return 1;
      case GL_DEPTH_BITS:
	return 1;
      case GL_STENCIL_BITS:
	return 1;
      case GL_ACCUM_RED_BITS:
      case GL_ACCUM_GREEN_BITS:
      case GL_ACCUM_BLUE_BITS:
      case GL_ACCUM_ALPHA_BITS:
	return 1;
      case GL_MAP1_GRID_DOMAIN:
	return 2;
      case GL_MAP1_GRID_SEGMENTS:
	return 1;
      case GL_MAP2_GRID_DOMAIN:
	return 4;
      case GL_MAP2_GRID_SEGMENTS:
	return 2;
      case GL_TEXTURE_1D:
      case GL_TEXTURE_2D:
      case GL_TEXTURE_3D:
	return 1;
      case GL_NAME_STACK_DEPTH:
	return 1;
      case GL_MAX_VIEWPORT_DIMS:
	return 2;
      case GL_DOUBLEBUFFER:
	return 1;
      case GL_AUX_BUFFERS:
	return 1;
      case GL_STEREO:
	return 1;
      case GL_CLIP_PLANE0:	case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2:	case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4:	case GL_CLIP_PLANE5:
	return 1;
      case GL_LIGHT0:	case GL_LIGHT1:
      case GL_LIGHT2:	case GL_LIGHT3:
      case GL_LIGHT4:	case GL_LIGHT5:
      case GL_LIGHT6:	case GL_LIGHT7:
	return 1;
      case GL_VERTEX_ARRAY:
      case GL_VERTEX_ARRAY_SIZE:
      case GL_VERTEX_ARRAY_TYPE:
      case GL_VERTEX_ARRAY_STRIDE:
      case GL_NORMAL_ARRAY:
      case GL_NORMAL_ARRAY_TYPE:
      case GL_NORMAL_ARRAY_STRIDE:
      case GL_COLOR_ARRAY:
      case GL_COLOR_ARRAY_SIZE:
      case GL_COLOR_ARRAY_TYPE:
      case GL_COLOR_ARRAY_STRIDE:
      case GL_INDEX_ARRAY:
      case GL_INDEX_ARRAY_TYPE:
      case GL_INDEX_ARRAY_STRIDE:
      case GL_TEXTURE_COORD_ARRAY:
      case GL_TEXTURE_COORD_ARRAY_SIZE:
      case GL_TEXTURE_COORD_ARRAY_TYPE:
      case GL_TEXTURE_COORD_ARRAY_STRIDE:
      case GL_EDGE_FLAG_ARRAY:
      case GL_EDGE_FLAG_ARRAY_STRIDE:
	return 1;
      case GL_TEXTURE_BINDING_1D:
      case GL_TEXTURE_BINDING_2D:
      case GL_TEXTURE_BINDING_3D:
	return 1;
      case GL_BLEND_COLOR:
	return 4;
      case GL_BLEND_EQUATION:
	return 1;
      case GL_COLOR_MATRIX:
	return 16;
      case GL_COLOR_MATRIX_STACK_DEPTH:
	return 1;
      case GL_COLOR_TABLE:
      case GL_POST_CONVOLUTION_COLOR_TABLE:
      case GL_POST_COLOR_MATRIX_COLOR_TABLE:
      case GL_CONVOLUTION_1D:
      case GL_CONVOLUTION_2D:
      case GL_SEPARABLE_2D:
      case GL_HISTOGRAM:
      case GL_MINMAX:
	return 1;
      case GL_POLYGON_OFFSET_FACTOR:
      case GL_POLYGON_OFFSET_UNITS:
	return 1;
      case GL_POST_CONVOLUTION_RED_SCALE:
      case GL_POST_CONVOLUTION_GREEN_SCALE:
      case GL_POST_CONVOLUTION_BLUE_SCALE:
      case GL_POST_CONVOLUTION_ALPHA_SCALE:
      case GL_POST_CONVOLUTION_RED_BIAS:
      case GL_POST_CONVOLUTION_GREEN_BIAS:
      case GL_POST_CONVOLUTION_BLUE_BIAS:
      case GL_POST_CONVOLUTION_ALPHA_BIAS:
	return 1;
      case GL_POST_COLOR_MATRIX_RED_SCALE:
      case GL_POST_COLOR_MATRIX_GREEN_SCALE:
      case GL_POST_COLOR_MATRIX_BLUE_SCALE:
      case GL_POST_COLOR_MATRIX_ALPHA_SCALE:
      case GL_POST_COLOR_MATRIX_RED_BIAS:
      case GL_POST_COLOR_MATRIX_GREEN_BIAS:
      case GL_POST_COLOR_MATRIX_BLUE_BIAS:
      case GL_POST_COLOR_MATRIX_ALPHA_BIAS:
	return 1;
      case GL_RESCALE_NORMAL:
	return 1;
      case GL_MAX_ELEMENTS_INDICES:
      case GL_MAX_ELEMENTS_VERTICES:
         return 1;
      case GL_ACTIVE_TEXTURE_ARB:
      case GL_CLIENT_ACTIVE_TEXTURE_ARB:
      case GL_MAX_TEXTURE_UNITS_ARB:
	return 1;
      case GL_MAX_COLOR_MATRIX_STACK_DEPTH:
      case GL_MAX_CONVOLUTION_WIDTH:
      case GL_MAX_CONVOLUTION_HEIGHT:
	return 1;
      case GL_OCCLUSION_TEST_RESULT_HP:
      case GL_OCCLUSION_TEST_HP:
	return 1;
      case GL_PACK_INVERT_MESA:
	return 1;
      case GL_CULL_VERTEX_IBM:
	return 1;
      case GL_RASTER_POSITION_UNCLIPPED_IBM:
	return 1;

      /* GL_ARB_texture_cube_map / GL 1.3 */
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_BINDING_CUBE_MAP:
      case GL_MAX_CUBE_MAP_TEXTURE_SIZE:

      /* GL_ARB_multisample / GL 1.3 */
      case GL_MULTISAMPLE:
      case GL_SAMPLE_ALPHA_TO_COVERAGE:
      case GL_SAMPLE_ALPHA_TO_ONE:
      case GL_SAMPLE_COVERAGE:
      case GL_SAMPLE_BUFFERS:
      case GL_SAMPLES:
      case GL_SAMPLE_COVERAGE_VALUE:
      case GL_SAMPLE_COVERAGE_INVERT:

      /* GL_ARB_texture_comrpession / GL 1.3 */
      case GL_TEXTURE_COMPRESSION_HINT:
      case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
	
      /* GL_EXT_blend_func_separate / GL 1.4 */
      case GL_BLEND_DST_RGB:
      case GL_BLEND_SRC_RGB:
      case GL_BLEND_DST_ALPHA:
      case GL_BLEND_SRC_ALPHA:

      /* GL_EXT_fog_coord / GL 1.4 */
      case GL_CURRENT_FOG_COORD:
      case GL_FOG_COORD_ARRAY_TYPE:
      case GL_FOG_COORD_ARRAY_STRIDE:
      case GL_FOG_COORD_ARRAY:
      case GL_FOG_COORD_SRC:

      /* GL_EXT_secondary_color / GL 1.4 */
      case GL_COLOR_SUM:
      case GL_SECONDARY_COLOR_ARRAY_SIZE:
      case GL_SECONDARY_COLOR_ARRAY_TYPE:
      case GL_SECONDARY_COLOR_ARRAY_STRIDE:
      case GL_SECONDARY_COLOR_ARRAY:

      /* GL_EXT_texture_lod_bias / GL 1.4 */
      case GL_MAX_TEXTURE_LOD_BIAS:

      /* GL_ARB_point_sprite */
      case GL_POINT_SPRITE_ARB:

      /* GL_ARB_vertex_blend */
      case GL_MAX_VERTEX_UNITS_ARB:
      case GL_ACTIVE_VERTEX_UNITS_ARB:
      case GL_WEIGHT_SUM_UNITY_ARB:
      case GL_VERTEX_BLEND_ARB:
      case GL_CURRENT_WEIGHT_ARB:
      case GL_WEIGHT_ARRAY_ARB:
      case GL_WEIGHT_ARRAY_TYPE_ARB:
      case GL_WEIGHT_ARRAY_STRIDE_ARB:
      case GL_WEIGHT_ARRAY_SIZE_ARB:

      /* GL_ARB_matrix_palette */
      case GL_MATRIX_PALETTE_ARB:
      case GL_MAX_MATRIX_PALETTE_STACK_DEPTH_ARB:
      case GL_MAX_PALETTE_MATRICES_ARB:
      case GL_CURRENT_PALETTE_MATRIX_ARB:
      case GL_CURRENT_MATRIX_INDEX_ARB:
      case GL_MATRIX_INDEX_ARRAY_ARB:
      case GL_MATRIX_INDEX_ARRAY_SIZE_ARB:
      case GL_MATRIX_INDEX_ARRAY_TYPE_ARB:
      case GL_MATRIX_INDEX_ARRAY_STRIDE_ARB:

      /* GL_EXT_clip_volume_hint */
      case GL_CLIP_VOLUME_CLIPPING_HINT_EXT:

      /* GL_EXT_depth_bounds_test */
      case GL_DEPTH_BOUNDS_TEST_EXT:

      /* GL_EXT_stencil_two_size */
      case GL_STENCIL_TEST_TWO_SIDE_EXT:
      case GL_ACTIVE_STENCIL_FACE_EXT:

      /* GL_EXT_vertex_weighting */
      case GL_VERTEX_WEIGHTING_EXT:
      case GL_MODELVIEW0_EXT:
      case GL_MODELVIEW1_EXT:
      case GL_CURRENT_VERTEX_WEIGHT_EXT:
      case GL_VERTEX_WEIGHT_ARRAY_EXT:
      case GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT:
      case GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT:
      case GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT:
      /* case GL_MODELVIEW0_STACK_DEPTH_EXT: */ /* alias */
      case GL_MODELVIEW1_STACK_DEPTH_EXT:

      /* GL_EXT_blend_equation_separate */
      /* case GL_BLEND_EQUATION_RGB_EXT: */ /* alias */
      case GL_BLEND_EQUATION_ALPHA_EXT:

      /* GL_ATI_vertex_streams */
      case GL_MAX_VERTEX_STREAMS_ATI:

      /* GL_ATI_draw_buffers */
      case GL_MAX_DRAW_BUFFERS_ATI:
      case GL_DRAW_BUFFER0_ATI:
      case GL_DRAW_BUFFER1_ATI:
      case GL_DRAW_BUFFER2_ATI:
      case GL_DRAW_BUFFER3_ATI:
      case GL_DRAW_BUFFER4_ATI:
      case GL_DRAW_BUFFER5_ATI:
      case GL_DRAW_BUFFER6_ATI:
      case GL_DRAW_BUFFER7_ATI:
      case GL_DRAW_BUFFER8_ATI:
      case GL_DRAW_BUFFER9_ATI:
      case GL_DRAW_BUFFER10_ATI:
      case GL_DRAW_BUFFER11_ATI:
      case GL_DRAW_BUFFER12_ATI:
      case GL_DRAW_BUFFER13_ATI:
      case GL_DRAW_BUFFER14_ATI:
      case GL_DRAW_BUFFER15_ATI:

      /* GL_ATI_separate_stencil */
      case GL_STENCIL_BACK_FUNC_ATI:
      case GL_STENCIL_BACK_FAIL_ATI:
      case GL_STENCIL_BACK_PASS_DEPTH_FAIL_ATI:
      case GL_STENCIL_BACK_PASS_DEPTH_PASS_ATI:

      /* GL_NV_depth_clamp */
      case GL_DEPTH_CLAMP_NV:

      /* GL_NV_fog_distance */
      case GL_FOG_DISTANCE_MODE_NV:

      /* GL_NV_light_max_exponent */
      case GL_MAX_SHININESS_NV:
      case GL_MAX_SPOT_EXPONENT_NV:

      /* GL_NV_multisample_filter_hint */
      case GL_MULTISAMPLE_FILTER_HINT_NV:

      /* GL_NV_point_sprite */
      /* case GL_POINT_SPRITE_NV: */ /* alias */
      case GL_POINT_SPRITE_R_MODE_NV:

      /* GL_NV_primitive_restart */
      case GL_PRIMITIVE_RESTART_NV:
      case GL_PRIMITIVE_RESTART_INDEX_NV:

      /* GL_NV_register_combiners */
      case GL_REGISTER_COMBINERS_NV:
      case GL_NUM_GENERAL_COMBINERS_NV:
      case GL_COLOR_SUM_CLAMP_NV:
      case GL_MAX_GENERAL_COMBINERS_NV:

      /* GL_NV_register_combiners2 */
      case GL_PER_STAGE_CONSTANTS_NV:

      /* GL_NV_texture_rectangle */
      case GL_TEXTURE_RECTANGLE_NV:
      case GL_TEXTURE_BINDING_RECTANGLE_NV:
      case GL_MAX_RECTANGLE_TEXTURE_SIZE_NV:
	return 1;

      /* GL_EXT_depth_bounds_test */
      case GL_DEPTH_BOUNDS_EXT:
	return 2;

      /* GL_EXT_secondary_color / GL 1.4 */
      case GL_CURRENT_SECONDARY_COLOR:

      /* GL_NV_register_combiners */
      case GL_CONSTANT_COLOR0_NV:
      case GL_CONSTANT_COLOR1_NV:
	return 4;

      /* GL_ARB_vertex_blend */
      /* case GL_MODELVIEW0_ARB: */ /* alias */
      /* case GL_MODELVIEW1_ARB: */ /* alias */
      case GL_MODELVIEW2_ARB:
      case GL_MODELVIEW3_ARB:
      case GL_MODELVIEW4_ARB:
      case GL_MODELVIEW5_ARB:
      case GL_MODELVIEW6_ARB:
      case GL_MODELVIEW7_ARB:
      case GL_MODELVIEW8_ARB:
      case GL_MODELVIEW9_ARB:
      case GL_MODELVIEW10_ARB:
      case GL_MODELVIEW11_ARB:
      case GL_MODELVIEW12_ARB:
      case GL_MODELVIEW13_ARB:
      case GL_MODELVIEW14_ARB:
      case GL_MODELVIEW15_ARB:
      case GL_MODELVIEW16_ARB:
      case GL_MODELVIEW17_ARB:
      case GL_MODELVIEW18_ARB:
      case GL_MODELVIEW19_ARB:
      case GL_MODELVIEW20_ARB:
      case GL_MODELVIEW21_ARB:
      case GL_MODELVIEW22_ARB:
      case GL_MODELVIEW23_ARB:
      case GL_MODELVIEW24_ARB:
      case GL_MODELVIEW25_ARB:
      case GL_MODELVIEW26_ARB:
      case GL_MODELVIEW27_ARB:
      case GL_MODELVIEW28_ARB:
      case GL_MODELVIEW29_ARB:
      case GL_MODELVIEW30_ARB:
      case GL_MODELVIEW31_ARB:

      /* GL_EXT_vertex_weighting */
      /* case GL_MODELVIEW0_MATRIX_EXT: */ /* alias */
      case GL_MODELVIEW1_MATRIX_EXT:
	return 32;

      /* GL_ARB_texture_comrpession / GL 1.3 */
      case GL_COMPRESSED_TEXTURE_FORMATS: {
	  GLint temp;
	  
	  glGetIntegerv( GL_NUM_COMPRESSED_TEXTURE_FORMATS, & temp );
	  return temp;
      }

      default:
	return -1;
    }
}

GLint __glGetDoublev_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetFloatv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetIntegerv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetBooleanv_size(GLenum sq)
{
    return __glGet_size(sq);
}

GLint __glGetTexLevelParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_TEXTURE_WIDTH:
      case GL_TEXTURE_HEIGHT:
      case GL_TEXTURE_DEPTH:
      case GL_TEXTURE_COMPONENTS:
      case GL_TEXTURE_BORDER:
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
      case GL_TEXTURE_INTENSITY_SIZE:

      /* GL_ARB_texture_compression / GL 1.3 */
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
      case GL_TEXTURE_COMPRESSED:

      /* GL_ARB_depth_texture / GL 1.4 */
      case GL_TEXTURE_DEPTH_SIZE:
	return 1;

      default:
	return -1;
    }
}

GLint __glGetTexLevelParameteriv_size(GLenum pname)
{
    return __glGetTexLevelParameterfv_size(pname);
}

GLint __glGetTexImage_size(GLenum target, GLint level, GLenum format,
			   GLenum type, GLint width, GLint height, GLint depth)
{
    return __glXImageSize( format, type, target, width, height, depth,
			   0, 0, 0, 0, 4 );
}

GLint __glGetConvolutionParameteriv_size(GLenum pname)
{
    switch (pname) {
      case GL_CONVOLUTION_BORDER_COLOR:
      case GL_CONVOLUTION_FILTER_SCALE:
      case GL_CONVOLUTION_FILTER_BIAS:
	return 4;
      case GL_CONVOLUTION_BORDER_MODE:
      case GL_CONVOLUTION_FORMAT:
      case GL_CONVOLUTION_WIDTH:
      case GL_CONVOLUTION_HEIGHT:
      case GL_MAX_CONVOLUTION_WIDTH:
      case GL_MAX_CONVOLUTION_HEIGHT:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetConvolutionParameterfv_size(GLenum pname)
{
    return __glGetConvolutionParameteriv_size(pname);
}


GLint __glGetHistogramParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_HISTOGRAM_WIDTH:
      case GL_HISTOGRAM_FORMAT:
      case GL_HISTOGRAM_RED_SIZE:
      case GL_HISTOGRAM_GREEN_SIZE:
      case GL_HISTOGRAM_BLUE_SIZE:
      case GL_HISTOGRAM_ALPHA_SIZE:
      case GL_HISTOGRAM_LUMINANCE_SIZE:
      case GL_HISTOGRAM_SINK:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetHistogramParameteriv_size(GLenum pname)
{
    return __glGetHistogramParameterfv_size(pname);
}

GLint __glGetMinmaxParameterfv_size(GLenum pname)
{
    switch (pname) {
      case GL_MINMAX_FORMAT:
      case GL_MINMAX_SINK:
	return 1;
      default:
	return -1;
    }
}

GLint __glGetMinmaxParameteriv_size(GLenum pname)
{
    return __glGetMinmaxParameterfv_size(pname);
}

GLint __glGetColorTableParameterfv_size(GLenum pname)
{
    switch(pname) {

    case GL_COLOR_TABLE_SCALE: /* return RGBA */
    case GL_COLOR_TABLE_BIAS:
	return 4;
    case GL_COLOR_TABLE_FORMAT:
    case GL_COLOR_TABLE_WIDTH:
    case GL_COLOR_TABLE_RED_SIZE:
    case GL_COLOR_TABLE_GREEN_SIZE:
    case GL_COLOR_TABLE_BLUE_SIZE:
    case GL_COLOR_TABLE_ALPHA_SIZE:
    case GL_COLOR_TABLE_LUMINANCE_SIZE:
    case GL_COLOR_TABLE_INTENSITY_SIZE:
	return 1;
    default:
	return 0;
    }
}

GLint __glGetColorTableParameteriv_size(GLenum pname)
{
    return __glGetColorTableParameterfv_size(pname);
}

