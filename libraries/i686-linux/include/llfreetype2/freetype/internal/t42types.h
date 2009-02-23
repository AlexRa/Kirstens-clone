/***************************************************************************/
/*                                                                         */
/*  t42types.h                                                             */
/*                                                                         */
/*    Type 42 font data types (specification only).                        */
/*                                                                         */
/*  Copyright 2002 by Roberto Alameda.                                     */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T42TYPES_H__
#define __T42TYPES_H__


#include "llfreetype2/freetype/ft2build.h"
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_INTERNAL_TYPE1_TYPES_H
#include FT_INTERNAL_POSTSCRIPT_NAMES_H
#include FT_INTERNAL_POSTSCRIPT_HINTS_H


FT_BEGIN_HEADER


  typedef struct  T42_FaceRec_
  {
    FT_FaceRec     root;
    T1_FontRec     type1;
    const void*    psnames;
    const void*    psaux;
    const void*    afm_data;
    FT_Byte*       ttf_data;
    FT_ULong       ttf_size;
    FT_Face        ttf_face;
    FT_CharMapRec  charmaprecs[2];
    FT_CharMap     charmaps[2];
    PS_Unicodes    unicode_map;

  } T42_FaceRec, *T42_Face;


FT_END_HEADER

#endif /* __T1TYPES_H__ */


/* END */
