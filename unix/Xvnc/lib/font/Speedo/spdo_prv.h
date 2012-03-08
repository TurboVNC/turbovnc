/* $Xorg: spdo_prv.h,v 1.3 2000/08/17 19:46:27 cpqbld Exp $ */

/*

Copyright 1989-1991, Bitstream Inc., Cambridge, MA.
You are hereby granted permission under all Bitstream propriety rights to
use, copy, modify, sublicense, sell, and redistribute the Bitstream Speedo
software and the Bitstream Charter outline font for any purpose and without
restrictions; provided, that this notice is left intact on all copies of such
software or font and that Bitstream's trademark is acknowledged as shown below
on all unmodified copies of such font.

BITSTREAM CHARTER is a registered trademark of Bitstream Inc.


BITSTREAM INC. DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
WITHOUT LIMITATION THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE.  BITSTREAM SHALL NOT BE LIABLE FOR ANY DIRECT OR INDIRECT
DAMAGES, INCLUDING BUT NOT LIMITED TO LOST PROFITS, LOST DATA, OR ANY OTHER
INCIDENTAL OR CONSEQUENTIAL DAMAGES, ARISING OUT OF OR IN ANY WAY CONNECTED
WITH THE SPEEDO SOFTWARE OR THE BITSTREAM CHARTER OUTLINE FONT.

*/
/* $XFree86: xc/lib/font/Speedo/spdo_prv.h,v 1.5 2001/01/17 19:43:18 dawes Exp $ */



/***************************** S P D O _ P R V . H *******************************/
 
#include "speedo.h"  /* include public definitions */

/*****  CONFIGURATION DEFINITIONS *****/


/***** PRIVATE FONT HEADER OFFSET CONSTANTS  *****/
#define  FH_ORUMX    0      /* U   Max ORU value  2 bytes                   */
#define  FH_PIXMX    2      /* U   Max Pixel value  2 bytes                 */
#define  FH_CUSNR    4      /* U   Customer Number  2 bytes                 */
#define  FH_OFFCD    6      /* E   Offset to Char Directory  3 bytes        */
#define  FH_OFCNS    9      /* E   Offset to Constraint Data  3 bytes       */
#define  FH_OFFTK   12      /* E   Offset to Track Kerning  3 bytes         */
#define  FH_OFFPK   15      /* E   Offset to Pair Kerning  3 bytes          */
#define  FH_OCHRD   18      /* E   Offset to Character Data  3 bytes        */
#define  FH_NBYTE   21      /* E   Number of Bytes in File  3 bytes         */


/***** MODE FLAGS CONSTANTS *****/
#define CURVES_OUT     0X0008  /* Output module accepts curves              */
#define BOGUS_MODE     0X0010  /* Linear scaling mode                       */
#define CONSTR_OFF     0X0020  /* Inhibit constraint table                  */
#define IMPORT_WIDTHS  0X0040  /* Imported width mode                       */
#define SQUEEZE_LEFT   0X0100  /* Squeeze left mode                         */
#define SQUEEZE_RIGHT  0X0200  /* Squeeze right mode                        */
#define SQUEEZE_TOP    0X0400  /* Squeeze top mode                          */
#define SQUEEZE_BOTTOM 0X0800  /* Squeeze bottom mode                       */
#define CLIP_LEFT      0X1000  /* Clip left mode                            */
#define CLIP_RIGHT     0X2000  /* Clip right mode                           */
#define CLIP_TOP       0X4000  /* Clip top mode                             */
#define CLIP_BOTTOM    0X8000  /* Clip bottom mode                          */


/***** MACRO DEFINITIONS *****/

#define SQUEEZE_MULT(A,B) (((fix31)A * (fix31)B) / (1 << 16))

#define NEXT_BYTE(A) (*(A)++)

#define NEXT_WORD(A) \
    ((fix15)(sp_globals.key32 ^ ((A) += 2, \
				 ((fix15)((A)[-1]) << 8) | (fix15)((A)[-2]) | \
				 ((A)[-1] & 0x80? ~0xFFFF : 0))))

#if INCL_EXT                       /* Extended fonts supported? */

#define NEXT_BYTES(A, B) \
    (((B = (ufix16)(*(A)++) ^ sp_globals.key7) >= 248)? \
     ((ufix16)(B & 0x07) << 8) + ((*(A)++) ^ sp_globals.key8) + 248: \
     B)

#else                              /* Compact fonts only supported? */

#define NEXT_BYTES(A, B) ((*(A)++) ^ sp_globals.key7)

#endif


#define NEXT_BYTE_U(A) (*(A)++) 

#define NEXT_WORD_U(A, B) \
    (fix15)(B = (*(A)++) << 8, (fix15)(*(A)++) + B)

#define NEXT_CHNDX(A, B) \
    ((B)? (ufix16)NEXT_WORD(A): (ufix16)NEXT_BYTE(A))

/* Multiply (fix15)X by (fix15)Y to produce (fix31)product */
#define MULT16(X, Y) \
    ((fix31)X * (fix31)Y)

/* Multiply (fix15)X by (fix15)MULT, add (fix31)OFFSET, 
 * shift right SHIFT bits to produce (fix15)result */
#define TRANS(X, MULT, OFFSET, SHIFT) \
    ((fix15)((((fix31)X * (fix31)MULT) + OFFSET) / (1 << SHIFT)))

/******************************************************************************
 *
 *      the following block of definitions redefines every function
 *      reference to be prefixed with an "sp_".  In addition, if this 
 *      is a reentrant version, the parameter sp_globals will be added
 *      as the first parameter.
 *
 *****************************************************************************/

#if STATIC_ALLOC || DYNAMIC_ALLOC

#define GDECL

#define get_char_id(char_index) sp_get_char_id(char_index)
#define get_char_width(char_index) sp_get_char_width(char_index)
#define get_track_kern(track,point_size) sp_get_track_kern(track,point_size)
#define get_pair_kern(char_index1,char_index2) sp_get_pair_kern(char_index1,char_index2)
#define get_char_bbox(char_index,bbox) sp_get_char_bbox(char_index,bbox)
#define make_char(char_index) sp_make_char(char_index)
#if INCL_ISW
#define compute_isw_scale() sp_compute_isw_scale()
#define do_make_char(char_index) sp_do_make_char(char_index)
#define make_char_isw(char_index,imported_width) sp_make_char_isw(char_index,imported_width)
#define reset_xmax(xmax) sp_reset_xmax(xmax)
#endif
#if INCL_ISW || INCL_SQUEEZING
#define preview_bounding_box(pointer,format) sp_preview_bounding_box(pointer,format)
#endif
#define make_simp_char(pointer,format) sp_make_simp_char(pointer,format)
#define make_comp_char(pointer) sp_make_comp_char(pointer)
#define get_char_org(char_index,top_level) sp_get_char_org(char_index,top_level)
#define get_posn_arg(ppointer,format) sp_get_posn_arg(ppointer,format)
#define get_scale_arg(ppointer,format) sp_get_scale_arg(ppointer,format)
#define read_bbox(ppointer,pPmin,pPmax,set_flag) sp_read_bbox(ppointer,pPmin,pPmax,set_flag)
#define proc_outl_data(pointer) sp_proc_outl_data(pointer)
#define split_curve(P1,P2,P3,depth) sp_split_curve(P1,P2,P3,depth)
#define get_args(ppointer,format,pP) sp_get_args(ppointer,format,pP)

#define init_black(specsarg) sp_init_black(specsarg)
#define begin_char_black(Psw,Pmin,Pmax) sp_begin_char_black(Psw,Pmin,Pmax)
#define begin_contour_black(P1,outside) sp_begin_contour_black(P1,outside)
#define line_black(P1) sp_line_black(P1)
#define end_char_black() sp_end_char_black()
#define add_intercept_black(y,x) sp_add_intercept_black(y,x)
#define proc_intercepts_black() sp_proc_intercepts_black()

#define init_screen(specsarg) sp_init_screen(specsarg)
#define begin_char_screen(Psw,Pmin,Pmax) sp_begin_char_screen(Psw,Pmin,Pmax)
#define begin_contour_screen(P1,outside) sp_begin_contour_screen(P1,outside)
#define curve_screen(P1,P2,P3,depth) sp_curve_screen(P1,P2,P3,depth)
#define scan_curve_screen(X0,Y0,X1,Y1,X2,Y2,X3,Y3) sp_scan_curve_screen(X0,Y0,X1,Y1,X2,Y2,X3,Y3) 
#define vert_line_screen(x,y1,y2) sp_vert_line_screen(x,y1,y2)
#define line_screen(P1) sp_line_screen(P1)
#define end_contour_screen() sp_end_contour_screen()
#define end_char_screen() sp_end_char_screen()
#define add_intercept_screen(y,x) sp_add_intercept_screen(y,x)
#define proc_intercepts_screen() sp_proc_intercepts_screen()

#define init_outline(specsarg) sp_init_outline(specsarg)
#define begin_char_outline(Psw,Pmin,Pmax) sp_begin_char_outline(Psw,Pmin,Pmax)
#define begin_sub_char_outline(Psw,Pmin,Pmax) sp_begin_sub_char_outline(Psw,Pmin,Pmax)
#define begin_contour_outline(P1,outside) sp_begin_contour_outline(P1,outside)
#define curve_outline(P1,P2,P3,depth) sp_curve_outline(P1,P2,P3,depth)
#define line_outline(P1) sp_line_outline(P1)
#define end_contour_outline() sp_end_contour_outline()
#define end_sub_char_outline() sp_end_sub_char_outline()
#define end_char_outline() sp_end_char_outline()

#define init_2d(specsarg) sp_init_2d(specsarg)
#define begin_char_2d(Psw, Pmin, Pmax) sp_begin_char_2d(Psw, Pmin, Pmax)
#define begin_contour_2d(P1, outside) sp_begin_contour_2d(P1, outside)
#define line_2d(P1) sp_line_2d(P1)
#define end_char_2d() sp_end_char_2d()
#define add_intercept_2d(y, x) sp_add_intercept_2d(y, x)
#define proc_intercepts_2d() sp_proc_intercepts_2d()
#define draw_vector_to_2d(x0, y0, x1, y1, band) sp_draw_vector_to_2d(x0, y0, x1, y1, band)

#define init_char_out(Psw,Pmin,Pmax) sp_init_char_out(Psw,Pmin,Pmax)
#define begin_sub_char_out(Psw,Pmin,Pmax) sp_begin_sub_char_out(Psw,Pmin,Pmax)
#define curve_out(P1,P2,P3,depth) sp_curve_out(P1,P2,P3,depth)
#define end_contour_out() sp_end_contour_out()
#define end_sub_char_out() sp_end_sub_char_out()
#define init_intercepts_out() sp_init_intercepts_out()
#define restart_intercepts_out() sp_restart_intercepts_out()
#define set_first_band_out(Pmin,Pmax) sp_set_first_band_out(Pmin,Pmax)
#define reduce_band_size_out() sp_reduce_band_size_out()
#define next_band_out() sp_next_band_out()

#define init_userout(specsarg) sp_init_userout(specsarg)

#define reset() sp_reset()
#define set_key(key) sp_set_key(key)
#define get_cust_no(font_buff) sp_get_cust_no(font_buff)
#define set_specs(specsarg) sp_set_specs(specsarg)
#define setup_consts(xmin,xmax,ymin,ymax) sp_setup_consts(xmin,xmax,ymin,ymax)
#define setup_tcb(ptcb) sp_setup_tcb(ptcb)
#define setup_mult(input_mult) sp_setup_mult(input_mult)
#define setup_offset(input_offset) sp_setup_offset(input_offset)
#define type_tcb(ptcb) sp_type_tcb(ptcb)
#define read_long(pointer) sp_read_long(pointer)
#define read_word_u(pointer) sp_read_word_u(pointer)
#define init_tcb() sp_init_tcb()
#define scale_tcb(ptcb,x_pos,y_pos,x_scale,y_scale) sp_scale_tcb(ptcb,x_pos,y_pos,x_scale,y_scale)
#define plaid_tcb(ppointer,format) sp_plaid_tcb(ppointer,format)
#define skip_orus(ppointer,short_form,no_ctrl_zones) sp_skip_orus(ppointer,short_form,no_ctrl_zones)
#define skip_interpolation_table(ppointer,format) sp_skip_interpolation_table(ppointer,format)
#define skip_control_zone(ppointer,format) sp_skip_control_zone(ppointer,format)
#define constr_update() sp_constr_update()
#define read_oru_table(ppointer) sp_read_oru_table(ppointer)
#define calculate_x_pix(start_edge,end_edge,constr_nr,x_scale,x_offset,ppo,setwidth_pix) sp_calculate_x_pix(start_edge,end_edge,constr_nr,x_scale,x_offset,ppo,setwidth_pix)
#define calculate_y_pix(start_edge,end_edge,constr_nr,top_scale,bottom_scale,ppo,emtop_pix,embot_pix) sp_calculate_y_pix(start_edge,end_edge,constr_nr,top_scale,bottom_scale,ppo,emtop_pix,embot_pix)
#define calculate_x_scale(x_factor,x_offset,no_x_ctrl_zones) sp_calculate_x_scale(x_factor,x_offset,no_x_ctrl_zones)
#define calculate_y_scale(top_scale,bottom_scale,first_y_zone,no_Y_ctrl_zones) sp_calculate_y_scale(top_scale,bottom_scale,first_y_zone,no_Y_ctrl_zones) 
#define setup_pix_table(ppointer,short_form,no_X_ctrl_zones,no_Y_ctrl_zones) sp_setup_pix_table(ppointer,short_form,no_X_ctrl_zones,no_Y_ctrl_zones)
#define setup_int_table(ppointer,no_X_int_zones, no_Y_int_zones) sp_setup_int_table(ppointer,no_X_int_zones, no_Y_int_zones)

#define fn_init_out(specsarg) (*sp_globals.init_out)(specsarg)  
#define fn_begin_char(Psw,Pmin,Pmax) (*sp_globals.begin_char)(Psw,Pmin,Pmax)
#define fn_begin_sub_char(Psw,Pmin,Pmax) (*sp_globals.begin_sub_char)(Psw,Pmin,Pmax)
#define fn_end_sub_char() (*sp_globals.end_sub_char)()
#define fn_end_char() (*sp_globals.end_char)()
#define fn_line(P1) (*sp_globals.line)(P1)
#define fn_end_contour() (*sp_globals.end_contour)()
#define fn_begin_contour(P0,fmt) (*sp_globals.begin_contour)(P0,fmt)
#define fn_curve(P1,P2,P3,depth) (*sp_globals.curve)(P1,P2,P3,depth)

#define load_char_data(offset, no_bytes, buff_off) sp_load_char_data(offset, no_bytes, buff_off)
#define report_error(n) sp_report_error(n)

#if INCL_MULTIDEV

#define set_bitmap_device(bfuncs,size) sp_set_bitmap_device(bfuncs,size)
#define set_outline_device(ofuncs,size) sp_set_outline_device(ofuncs,size)

#define open_bitmap(x_set_width, y_set_width, xmin, xmax, ymin, ymax) (*sp_globals.bitmap_device.p_open_bitmap)(x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define set_bitmap_bits(y, xbit1, xbit2) (*sp_globals.bitmap_device.p_set_bits)(y, xbit1, xbit2)
#define close_bitmap() (*sp_globals.bitmap_device.p_close_bitmap)()

#define open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax) (*sp_globals.outline_device.p_open_outline)(x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define start_new_char() (*sp_globals.outline_device.p_start_char)()
#define start_contour(x,y,outside) (*sp_globals.outline_device.p_start_contour)(x,y,outside)
#define curve_to(x1,y1,x2,y2,x3,y3) (*sp_globals.outline_device.p_curve)(x1,y1,x2,y2,x3,y3)
#define line_to(x,y) (*sp_globals.outline_device.p_line)(x,y)
#define close_contour() (*sp_globals.outline_device.p_close_contour)()
#define close_outline() (*sp_globals.outline_device.p_close_outline)()

#else

#define open_bitmap(x_set_width, y_set_width, xmin, xmax, ymin, ymax) sp_open_bitmap(x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define set_bitmap_bits(y, xbit1, xbit2) sp_set_bitmap_bits(y, xbit1, xbit2)
#define close_bitmap() sp_close_bitmap()

#define open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax) sp_open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define start_new_char() sp_start_new_char()
#define start_contour(x,y,outside) sp_start_contour(x,y,outside)
#define curve_to(x1,y1,x2,y2,x3,y3) sp_curve_to(x1,y1,x2,y2,x3,y3)
#define line_to(x,y) sp_line_to(x,y)
#define close_contour() sp_close_contour()
#define close_outline() sp_close_outline()

#endif

#else

#define GDECL SPEEDO_GLOBALS* sp_global_ptr,

#define get_char_id(char_index) sp_get_char_id(sp_global_ptr,char_index)
#define get_char_width(char_index) sp_get_char_width(sp_global_ptr,char_index)
#define get_track_kern(track,point_size) sp_get_track_kern(sp_global_ptr,track,point_size)
#define get_pair_kern(char_index1,char_index2) sp_get_pair_kern(sp_global_ptr,char_index1,char_index2)
#define get_char_bbox(char_index,bbox) sp_get_char_bbox(sp_global_ptr,char_index,bbox)
#define make_char(char_index) sp_make_char(sp_global_ptr,char_index)
#if INCL_ISW
#define compute_isw_scale() sp_compute_isw_scale(sp_global_ptr)
#define do_make_char(char_index) sp_do_make_char(sp_global_ptr,char_index)
#define make_char_isw(char_index,imported_width) sp_make_char_isw(sp_global_ptr,char_index,imported_width)
#define reset_xmax(xmax) sp_reset_xmax(sp_global_ptr,xmax)
#endif
#if INCL_ISW || INCL_SQUEEZING
#define preview_bounding_box(pointer,format) sp_preview_bounding_box(sp_global_ptr,pointer,format)
#endif
#define make_simp_char(pointer,format) sp_make_simp_char(sp_global_ptr,pointer,format)
#define make_comp_char(pointer) sp_make_comp_char(sp_global_ptr,pointer)
#define get_char_org(char_index,top_level) sp_get_char_org(sp_global_ptr,char_index,top_level)
#define get_posn_arg(ppointer,format) sp_get_posn_arg(sp_global_ptr,ppointer,format)
#define get_scale_arg(ppointer,format) sp_get_scale_arg(sp_global_ptr,ppointer,format)
#define read_bbox(ppointer,pPmin,pPmax,set_flag) sp_read_bbox(sp_global_ptr,ppointer,pPmin,pPmax,set_flag)
#define proc_outl_data(pointer) sp_proc_outl_data(sp_global_ptr,pointer)
#define split_curve(P1,P2,P3,depth) sp_split_curve(sp_global_ptr,P1,P2,P3,depth)
#define get_args(ppointer,format,pP) sp_get_args(sp_global_ptr,ppointer,format,pP)

#define init_black(specsarg) sp_init_black(sp_global_ptr,specsarg)
#define begin_char_black(Psw,Pmin,Pmax) sp_begin_char_black(sp_global_ptr,Psw,Pmin,Pmax)
#define begin_contour_black(P1,outside) sp_begin_contour_black(sp_global_ptr,P1,outside)
#define line_black(P1) sp_line_black(sp_global_ptr,P1)
#define end_char_black() sp_end_char_black(sp_global_ptr)
#define add_intercept_black(y,x) sp_add_intercept_black(sp_global_ptr,y,x)
#define proc_intercepts_black() sp_proc_intercepts_black(sp_global_ptr)

#define init_screen(specsarg) sp_init_screen(sp_global_ptr,specsarg)
#define begin_char_screen(Psw,Pmin,Pmax) sp_begin_char_screen(sp_global_ptr,Psw,Pmin,Pmax)
#define begin_contour_screen(P1,outside) sp_begin_contour_screen(sp_global_ptr,P1,outside)
#define curve_screen(P1,P2,P3,depth) sp_curve_screen(sp_global_ptr,P1,P2,P3,depth)
#define scan_curve_screen(X0,Y0,X1,Y1,X2,Y2,X3,Y3) sp_scan_curve_screen(sp_global_ptr,X0,Y0,X1,Y1,X2,Y2,X3,Y3) 
#define vert_line_screen(x,y1,y2) sp_vert_line_screen(sp_global_ptr,x,y1,y2)
#define line_screen(P1) sp_line_screen(sp_global_ptr,P1)
#define end_char_screen() sp_end_char_screen(sp_global_ptr)
#define end_contour_screen() sp_end_contour_screen(sp_global_ptr)
#define add_intercept_screen(y,x) sp_add_intercept_screen(sp_global_ptr,y,x)
#define proc_intercepts_screen() sp_proc_intercepts_screen(sp_global_ptr)

#define init_outline(specsarg) sp_init_outline(sp_global_ptr,specsarg)
#define begin_char_outline(Psw,Pmin,Pmax) sp_begin_char_outline(sp_global_ptr,Psw,Pmin,Pmax)
#define begin_sub_char_outline(Psw,Pmin,Pmax) sp_begin_sub_char_outline(sp_global_ptr,Psw,Pmin,Pmax)
#define begin_contour_outline(P1,outside) sp_begin_contour_outline(sp_global_ptr,P1,outside)
#define curve_outline(P1,P2,P3,depth) sp_curve_outline(sp_global_ptr,P1,P2,P3,depth)
#define line_outline(P1) sp_line_outline(sp_global_ptr,P1)
#define end_contour_outline() sp_end_contour_outline(sp_global_ptr)
#define end_sub_char_outline() sp_end_sub_char_outline(sp_global_ptr)
#define end_char_outline() sp_end_char_outline(sp_global_ptr)

#define init_2d(specsarg) sp_init_2d(sp_global_ptr,specsarg)
#define begin_char_2d(Psw, Pmin, Pmax) sp_begin_char_2d(sp_global_ptr,Psw, Pmin, Pmax)
#define begin_contour_2d(P1, outside) sp_begin_contour_2d(sp_global_ptr,P1, outside)
#define line_2d(P1) sp_line_2d(sp_global_ptr,P1)
#define end_char_2d() sp_end_char_2d(sp_global_ptr)
#define add_intercept_2d(y, x) sp_add_intercept_2d(sp_global_ptr,y, x)
#define proc_intercepts_2d() sp_proc_intercepts_2d(sp_global_ptr)
#define draw_vector_to_2d(x0, y0, x1, y1, band) sp_draw_vector_to_2d(sp_global_ptr,x0, y0, x1, y1, band)

#define init_char_out(Psw,Pmin,Pmax) sp_init_char_out(sp_global_ptr,Psw,Pmin,Pmax)
#define begin_sub_char_out(Psw,Pmin,Pmax) sp_begin_sub_char_out(sp_global_ptr,Psw,Pmin,Pmax)
#define curve_out(P1,P2,P3,depth) sp_curve_out(sp_global_ptr,P1,P2,P3,depth)
#define end_contour_out() sp_end_contour_out(sp_global_ptr)
#define end_sub_char_out() sp_end_sub_char_out(sp_global_ptr)
#define init_intercepts_out() sp_init_intercepts_out(sp_global_ptr)
#define restart_intercepts_out() sp_restart_intercepts_out(sp_global_ptr)
#define set_first_band_out(Pmin,Pmax) sp_set_first_band_out(sp_global_ptr,Pmin,Pmax)
#define reduce_band_size_out() sp_reduce_band_size_out(sp_global_ptr)
#define next_band_out() sp_next_band_out(sp_global_ptr)

#define init_userout(specsarg) sp_init_userout(sp_global_ptr,specsarg)

#define reset() sp_reset(sp_global_ptr)
#define set_key(key) sp_set_key(sp_global_ptr,key)
#define get_cust_no(font_buff) sp_get_cust_no(sp_global_ptr,font_buff)
#define set_specs(specsarg) sp_set_specs(sp_global_ptr,specsarg)
#define setup_consts(xmin,xmax,ymin,ymax) sp_setup_consts(sp_global_ptr,xmin,xmax,ymin,ymax)
#define setup_tcb(ptcb) sp_setup_tcb(sp_global_ptr,ptcb)
#define setup_mult(input_mult) sp_setup_mult(sp_global_ptr,input_mult)
#define setup_offset(input_offset) sp_setup_offset(sp_global_ptr,input_offset)
#define type_tcb(ptcb) sp_type_tcb(sp_global_ptr,ptcb)
#define read_long(pointer) sp_read_long(sp_global_ptr,pointer)
#define read_word_u(pointer) sp_read_word_u(sp_global_ptr,pointer)
#define init_tcb() sp_init_tcb(sp_global_ptr)
#define scale_tcb(ptcb,x_pos,y_pos,x_scale,y_scale) sp_scale_tcb(sp_global_ptr,ptcb,x_pos,y_pos,x_scale,y_scale)
#define plaid_tcb(ppointer,format) sp_plaid_tcb(sp_global_ptr,ppointer,format)
#define skip_orus(ppointer,short_form,no_ctrl_zones) sp_skip_orus(sp_global_ptr,ppointer,short_form,no_ctrl_zones)
#define skip_interpolation_table(ppointer,format) sp_skip_interpolation_table(sp_global_ptr,ppointer,format)
#define skip_control_zone(ppointer,format) sp_skip_control_zone(sp_global_ptr,ppointer,format)
#define constr_update() sp_constr_update(sp_global_ptr)
#define read_oru_table(ppointer) sp_read_oru_table(sp_global_ptr,ppointer)
#define calculate_x_pix(start_edge,end_edge,constr_nr,x_scale,x_offset,ppo,setwidth_pix) sp_calculate_x_pix(sp_global_ptr,start_edge,end_edge,constr_nr,x_scale,x_offset,ppo,setwidth_pix)
#define calculate_y_pix(start_edge,end_edge,constr_nr,top_scale,bottom_scale,ppo,emtop_pix,embot_pix) sp_calculate_y_pix(sp_global_ptr,start_edge,end_edge,constr_nr,top_scale,bottom_scale,ppo,emtop_pix,embot_pix)
#define calculate_x_scale(x_factor,x_offset,no_x_ctrl_zones) sp_calculate_x_scale(sp_global_ptr,x_factor,x_offset,no_x_ctrl_zones)
#define calculate_y_scale(top_scale,bottom_scale,first_y_zone,no_Y_ctrl_zones) sp_calculate_y_scale(sp_global_ptr,top_scale,bottom_scale,first_y_zone,no_Y_ctrl_zones) 
#define setup_pix_table(ppointer,short_form,no_X_ctrl_zones,no_Y_ctrl_zones) sp_setup_pix_table(sp_global_ptr,ppointer,short_form,no_X_ctrl_zones,no_Y_ctrl_zones)
#define setup_int_table(ppointer,no_X_int_zones, no_Y_int_zones) sp_setup_int_table(sp_global_ptr,ppointer,no_X_int_zones, no_Y_int_zones)

#define fn_init_out(specsarg) (*sp_globals.init_out)(sp_global_ptr,specsarg)  
#define fn_begin_char(Psw,Pmin,Pmax) (*sp_globals.begin_char)(sp_global_ptr,Psw,Pmin,Pmax)
#define fn_begin_sub_char(Psw,Pmin,Pmax) (*sp_globals.begin_sub_char)(sp_global_ptr,Psw,Pmin,Pmax)
#define fn_end_sub_char() (*sp_globals.end_sub_char)(sp_global_ptr)
#define fn_end_char() (*sp_globals.end_char)(sp_global_ptr)
#define fn_line(P1) (*sp_globals.line)(sp_global_ptr,P1)
#define fn_end_contour() (*sp_globals.end_contour)(sp_global_ptr)
#define fn_begin_contour(P0,fmt) (*sp_globals.begin_contour)(sp_global_ptr,P0,fmt)
#define fn_curve(P1,P2,P3,depth) (*sp_globals.curve)(sp_global_ptr,P1,P2,P3,depth)


#define load_char_data(offset, no_bytes, buff_off) sp_load_char_data(sp_global_ptr, offset, no_bytes, buff_off)
#define report_error(n) sp_report_error(sp_global_ptr, n)

#if INCL_MULTIDEV

#define set_bitmap_device(bfuncs,size) sp_set_bitmap_device(sp_global_ptr,bfuncs,size)
#define set_outline_device(ofuncs,size) sp_set_outline_device(sp_global_ptr,ofuncs,size)

#define open_bitmap(x_set_width, y_set_width, xmin, xmax, ymin, ymax)(*sp_globals.bitmap_device.p_open_bitmap)(sp_global_ptr,x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define set_bitmap_bits(y, xbit1, xbit2)(*sp_globals.bitmap_device.p_set_bits)(sp_global_ptr,y, xbit1, xbit2)
#define close_bitmap()(*sp_globals.bitmap_device.p_close_bitmap)(sp_global_ptr)

#define open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax)(*sp_globals.outline_device.p_open_outline)(sp_global_ptr,x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define start_new_char()(*sp_globals.outline_device.p_start_char)(sp_global_ptr)
#define start_contour(x,y,outside)(*sp_globals.outline_device.p_start_contour)(sp_global_ptr,x,y,outside)
#define curve_to(x1,y1,x2,y2,x3,y3)(*sp_globals.outline_device.p_curve)(sp_global_ptr,x1,y1,x2,y2,x3,y3)
#define line_to(x,y)(*sp_globals.outline_device.p_line)(sp_global_ptr,x,y)
#define close_contour()(*sp_globals.outline_device.p_close_contour)(sp_global_ptr)
#define close_outline()(*sp_globals.outline_device.p_close_outline)(sp_global_ptr)

#else

#define open_bitmap(x_set_width, y_set_width, xmin, xmax, ymin, ymax) sp_open_bitmap(sp_global_ptr, x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define set_bitmap_bits(y, xbit1, xbit2) sp_set_bitmap_bits(sp_global_ptr, y, xbit1, xbit2)
#define close_bitmap() sp_close_bitmap(sp_global_ptr)

#define open_outline(x_set_width, y_set_width, xmin, xmax, ymin, ymax) sp_open_outline(sp_global_ptr, x_set_width, y_set_width, xmin, xmax, ymin, ymax)
#define start_new_char() sp_start_new_char(sp_global_ptr )
#define start_contour(x,y,outside) sp_start_contour(sp_global_ptr, x,y,outside)
#define curve_to(x1,y1,x2,y2,x3,y3) sp_curve_to(sp_global_ptr, x1,y1,x2,y2,x3,y3)
#define line_to(x,y) sp_line_to(sp_global_ptr, x,y)
#define close_contour() sp_close_contour(sp_global_ptr)
#define close_outline() sp_close_outline(sp_global_ptr)

#endif
#endif


