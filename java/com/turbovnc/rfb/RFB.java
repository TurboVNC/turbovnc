/* Copyright (C) 2011-2012, 2015-2018, 2021-2022 D. R. Commander.
 *                                               All Rights Reserved.
 * Copyright 2009, 2011, 2019 Pierre Ossman for Cendio AB
 * Copyright (C) 2011-2012 Brian P. Hinz
 * Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

package com.turbovnc.rfb;

public final class RFB {

  //***************************************************************************
  // Security types
  //***************************************************************************

  public static final int SECTYPE_INVALID  = 0;
  public static final int SECTYPE_NONE     = 1;
  public static final int SECTYPE_VNCAUTH  = 2;

  public static final int SECTYPE_RA2      = 5;
  public static final int SECTYPE_RA2NE    = 6;

  public static final int SECTYPE_SSPI     = 7;
  public static final int SECTYPE_SSPINE   = 8;

  public static final int SECTYPE_TIGHT    = 16;
  public static final int SECTYPE_ULTRA    = 17;
  public static final int SECTYPE_TLS      = 18;
  public static final int SECTYPE_VENCRYPT = 19;

  // Tight subtypes
  public static final int SECTYPE_UNIX_LOGIN = 129;

  // VeNCrypt subtypes
  public static final int SECTYPE_PLAIN      = 256;
  public static final int SECTYPE_TLS_NONE   = 257;
  public static final int SECTYPE_TLS_VNC    = 258;
  public static final int SECTYPE_TLS_PLAIN  = 259;
  public static final int SECTYPE_X509_NONE  = 260;
  public static final int SECTYPE_X509_VNC   = 261;
  public static final int SECTYPE_X509_PLAIN = 262;

  public static int secTypeNum(String name) {
    if (name.equalsIgnoreCase("None"))      return SECTYPE_NONE;
    if (name.equalsIgnoreCase("VncAuth"))   return SECTYPE_VNCAUTH;
    if (name.equalsIgnoreCase("VNC"))       return SECTYPE_VNCAUTH;
    if (name.equalsIgnoreCase("Tight"))     return SECTYPE_TIGHT;
    if (name.equalsIgnoreCase("RA2"))       return SECTYPE_RA2;
    if (name.equalsIgnoreCase("RA2ne"))     return SECTYPE_RA2NE;
    if (name.equalsIgnoreCase("SSPI"))      return SECTYPE_SSPI;
    if (name.equalsIgnoreCase("SSPIne"))    return SECTYPE_SSPINE;
    //if (name.equalsIgnoreCase("Ultra"))     return SECTYPE_ULTRA;
    if (name.equalsIgnoreCase("TLS"))       return SECTYPE_TLS;
    if (name.equalsIgnoreCase("VeNCrypt"))  return SECTYPE_VENCRYPT;

    // Tight subtypes
    if (name.equalsIgnoreCase("UnixLogin")) return SECTYPE_UNIX_LOGIN;

    // VeNCrypt subtypes
    if (name.equalsIgnoreCase("Plain"))     return SECTYPE_PLAIN;
    if (name.equalsIgnoreCase("TLSNone"))   return SECTYPE_TLS_NONE;
    if (name.equalsIgnoreCase("TLSVnc"))    return SECTYPE_TLS_VNC;
    if (name.equalsIgnoreCase("TLSPlain"))  return SECTYPE_TLS_PLAIN;
    if (name.equalsIgnoreCase("X509None"))  return SECTYPE_X509_NONE;
    if (name.equalsIgnoreCase("X509Vnc"))   return SECTYPE_X509_VNC;
    if (name.equalsIgnoreCase("X509Plain")) return SECTYPE_X509_PLAIN;

    return SECTYPE_INVALID;
  }

  public static String secTypeName(int num) {
    switch (num) {
      case SECTYPE_NONE:        return "None";
      case SECTYPE_VNCAUTH:     return "VncAuth";
      case SECTYPE_TIGHT:       return "Tight";
      case SECTYPE_RA2:         return "RA2";
      case SECTYPE_RA2NE:       return "RA2ne";
      case SECTYPE_SSPI:        return "SSPI";
      case SECTYPE_SSPINE:      return "SSPIne";
      //case SECTYPE_ULTRA:       return "Ultra";
      case SECTYPE_TLS:         return "TLS";
      case SECTYPE_VENCRYPT:    return "VeNCrypt";

      // Tight subtypes
      case SECTYPE_UNIX_LOGIN:  return "UnixLogin";

      // VeNCrypt subtypes
      case SECTYPE_PLAIN:       return "Plain";
      case SECTYPE_TLS_NONE:    return "TLSNone";
      case SECTYPE_TLS_VNC:     return "TLSVnc";
      case SECTYPE_TLS_PLAIN:   return "TLSPlain";
      case SECTYPE_X509_NONE:   return "X509None";
      case SECTYPE_X509_VNC:    return "X509Vnc";
      case SECTYPE_X509_PLAIN:  return "X509Plain";
      default:                  return "[unknown SecType]";
    }
  }

  public static boolean isEncrypted(int secType) {
    switch (secType) {
      case SECTYPE_TLS:
      case SECTYPE_TLS_NONE:
      case SECTYPE_TLS_VNC:
      case SECTYPE_TLS_PLAIN:
      case SECTYPE_X509_NONE:
      case SECTYPE_X509_VNC:
      case SECTYPE_X509_PLAIN:
        return true;
      default:
        return false;
    }
  }

  //***************************************************************************
  // Authentication result codes
  //***************************************************************************

  public static final int AUTH_OK       = 0;
  public static final int AUTH_FAILED   = 1;
  public static final int AUTH_TOO_MANY = 2;  // deprecated

  //***************************************************************************
  // Message types
  //***************************************************************************

  // Server -> Client
  public static final int FRAMEBUFFER_UPDATE        = 0;
  public static final int SET_COLOUR_MAP_ENTRIES    = 1;
  public static final int BELL                      = 2;
  public static final int SERVER_CUT_TEXT           = 3;

  public static final int END_OF_CONTINUOUS_UPDATES = 150;

  // Client -> Server
  public static final int SET_PIXEL_FORMAT           = 0;
  public static final int FIX_COLOUR_MAP_ENTRIES     = 1;
  public static final int SET_ENCODINGS              = 2;
  public static final int FRAMEBUFFER_UPDATE_REQUEST = 3;
  public static final int KEY_EVENT                  = 4;
  public static final int POINTER_EVENT              = 5;
  public static final int CLIENT_CUT_TEXT            = 6;

  public static final int ENABLE_CONTINUOUS_UPDATES  = 150;
  public static final int SET_DESKTOP_SIZE           = 251;

  // Server -> Client and Client -> Server
  public static final int FENCE = 248;
  public static final int GII   = 253;
  public static final int QEMU  = 255;

  //***************************************************************************
  // Encoding types
  //***************************************************************************

  public static final int ENCODING_RAW      = 0;
  public static final int ENCODING_COPYRECT = 1;
  public static final int ENCODING_RRE      = 2;
  public static final int ENCODING_CORRE    = 4;
  public static final int ENCODING_HEXTILE  = 5;
  public static final int ENCODING_TIGHT    = 7;
  public static final int ENCODING_ZRLE     = 16;
  public static final int ENCODING_LAST     = ENCODING_ZRLE;

  public static final int ENCODING_MAX      = 255;

  public static int encodingNum(String name) {
    if (name.equalsIgnoreCase("Raw"))      return ENCODING_RAW;
    if (name.equalsIgnoreCase("RRE"))      return ENCODING_RRE;
    if (name.equalsIgnoreCase("CoRRE"))    return ENCODING_CORRE;
    if (name.equalsIgnoreCase("Hextile"))  return ENCODING_HEXTILE;
    if (name.equalsIgnoreCase("Tight"))    return ENCODING_TIGHT;
    if (name.equalsIgnoreCase("ZRLE"))     return ENCODING_ZRLE;
    return -1;
  }

  public static String encodingName(int num) {
    switch (num) {
      case ENCODING_RAW:       return "Raw";
      case ENCODING_RRE:       return "RRE";
      case ENCODING_CORRE:     return "CoRRE";
      case ENCODING_HEXTILE:   return "Hextile";
      case ENCODING_TIGHT:     return "Tight";
      case ENCODING_ZRLE:      return "ZRLE";
      default:                 return "[unknown encoding]";
    }
  }

  //***************************************************************************
  // Pseudo-encodings
  //***************************************************************************

  public static final int ENCODING_VMWARE_LED_STATE        = 0x574D5668;

  public static final int ENCODING_EXTENDED_CLIPBOARD      = 0xC0A1E5CE;

  public static final int ENCODING_SUBSAMP_1X              = -768;
  public static final int ENCODING_SUBSAMP_4X              = -767;
  public static final int ENCODING_SUBSAMP_2X              = -766;
  public static final int ENCODING_SUBSAMP_GRAY            = -765;
  public static final int ENCODING_SUBSAMP_8X              = -764;
  public static final int ENCODING_SUBSAMP_16X             = -763;
  public static final int ENCODING_FINE_QUALITY_LEVEL_0    = -512;
  public static final int ENCODING_FINE_QUALITY_LEVEL_100  = -412;

  public static final int ENCODING_CONTINUOUS_UPDATES      = -313;
  public static final int ENCODING_FENCE                   = -312;

  public static final int ENCODING_CLIENT_REDIRECT         = -311;

  public static final int ENCODING_EXTENDED_DESKTOP_SIZE   = -308;

  public static final int ENCODING_DESKTOP_NAME            = -307;

  public static final int ENCODING_GII                     = -305;

  public static final int ENCODING_QEMU_LED_STATE          = -261;
  public static final int ENCODING_QEMU_EXTENDED_KEY_EVENT = -258;

  public static final int ENCODING_COMPRESS_LEVEL_0        = -256;
  public static final int ENCODING_COMPRESS_LEVEL_9        = -247;

  public static final int ENCODING_X_CURSOR                = -240;
  public static final int ENCODING_RICH_CURSOR             = -239;

  public static final int ENCODING_LAST_RECT               = -224;
  public static final int ENCODING_NEW_FB_SIZE             = -223;

  public static final int ENCODING_QUALITY_LEVEL_0         = -32;
  public static final int ENCODING_QUALITY_LEVEL_9         = -23;

  //***************************************************************************
  // Fence flags
  //***************************************************************************

  public static final int FENCE_FLAG_BLOCK_BEFORE = 1 << 0;
  public static final int FENCE_FLAG_BLOCK_AFTER  = 1 << 1;
  public static final int FENCE_FLAG_SYNC_NEXT    = 1 << 2;

  public static final int FENCE_FLAG_REQUEST      = 1 << 31;

  public static final int FENCE_FLAGS_SUPPORTED   = (FENCE_FLAG_BLOCK_BEFORE |
                                                     FENCE_FLAG_BLOCK_AFTER |
                                                     FENCE_FLAG_SYNC_NEXT |
                                                     FENCE_FLAG_REQUEST);

  //***************************************************************************
  // Extended Clipboard
  //***************************************************************************

  // Formats
  public static final int EXTCLIP_FORMAT_UTF8  = 1 << 0;
  public static final int EXTCLIP_FORMAT_RTF   = 1 << 1;
  public static final int EXTCLIP_FORMAT_HTML  = 1 << 2;
  public static final int EXTCLIP_FORMAT_DIB   = 1 << 3;
  public static final int EXTCLIP_FORMAT_FILES = 1 << 4;

  public static final int EXTCLIP_FORMAT_MASK  = 0x0000ffff;

  // Actions
  public static final int EXTCLIP_ACTION_CAPS    = 1 << 24;
  public static final int EXTCLIP_ACTION_REQUEST = 1 << 25;
  public static final int EXTCLIP_ACTION_PEEK    = 1 << 26;
  public static final int EXTCLIP_ACTION_NOTIFY  = 1 << 27;
  public static final int EXTCLIP_ACTION_PROVIDE = 1 << 28;

  public static final int EXTCLIP_ACTION_MASK    = 0xff000000;

  //***************************************************************************
  // Hextile subencoding types
  //***************************************************************************

  public static final int HEXTILE_RAW                  = (1 << 0);
  public static final int HEXTILE_BACKGROUND_SPECIFIED = (1 << 1);
  public static final int HEXTILE_FOREGROUND_SPECIFIED = (1 << 2);
  public static final int HEXTILE_ANY_SUBRECTS         = (1 << 3);
  public static final int HEXTILE_SUBRECTS_COLOURED    = (1 << 4);

  //***************************************************************************
  // Tight subencoding types
  //***************************************************************************

  // Compression control
  public static final int TIGHT_EXPLICIT_FILTER = 0x04;
  public static final int TIGHT_FILL            = 0x08;
  public static final int TIGHT_JPEG            = 0x09;
  public static final int TIGHT_NO_ZLIB         = 0x0A;
  public static final int TIGHT_MAX_SUBENCODING = 0x09;

  // Filters to improve compression efficiency
  public static final int TIGHT_FILTER_COPY     = 0x00;
  public static final int TIGHT_FILTER_PALETTE  = 0x01;
  public static final int TIGHT_FILTER_GRADIENT = 0x02;

  //***************************************************************************
  // Extended desktop size reason and result codes
  //***************************************************************************

  public static final int EDS_REASON_SERVER       = 0;
  public static final int EDS_REASON_CLIENT       = 1;
  public static final int EDS_REASON_OTHER_CLIENT = 2;

  public static final int EDS_RESULT_SUCCESS      = 0;
  public static final int EDS_RESULT_PROHIBITED   = 1;
  public static final int EDS_RESULT_NO_RESOURCES = 2;
  public static final int EDS_RESULT_INVALID      = 3;

  //***************************************************************************
  // LED states
  //***************************************************************************

  public static final int LED_SCROLL_LOCK = 1 << 0;
  public static final int LED_NUM_LOCK    = 1 << 1;
  public static final int LED_CAPS_LOCK   = 1 << 2;
  public static final int LED_UNKNOWN     = -1;

  //***************************************************************************
  // Button masks for PointerEvent
  //***************************************************************************

  public static final int BUTTON1_MASK = 1;
  public static final int BUTTON2_MASK = 2;
  public static final int BUTTON3_MASK = 4;
  public static final int BUTTON4_MASK = 8;
  public static final int BUTTON5_MASK = 16;

  //***************************************************************************
  // GII
  //***************************************************************************

  // Message subtypes
  public static final int GII_EVENT          = 0;
  public static final int GII_VERSION        = 1;
  public static final int GII_DEVICE_CREATE  = 2;
  public static final int GII_DEVICE_DESTROY = 3;

  public static final int GII_BE             = 128;

  // Event types
  public static final int GII_BUTTON_PRESS      = 10;
  public static final int GII_BUTTON_RELEASE    = 11;
  public static final int GII_VALUATOR_RELATIVE = 12;
  public static final int GII_VALUATOR_ABSOLUTE = 13;

  public static String giiEventName(int num) {
    switch (num) {
      case GII_BUTTON_PRESS:       return "Button Press (" + num + ")";
      case GII_BUTTON_RELEASE:     return "Button Release (" + num + ")";
      case GII_VALUATOR_RELATIVE:  return "Relative Valuator (" + num + ")";
      case GII_VALUATOR_ABSOLUTE:  return "Absolute Valuator (" + num + ")";
      default:                     return Integer.toString(num);
    }
  }

  // Event masks
  public static final int GII_BUTTON_PRESS_MASK      = 0x00000400;
  public static final int GII_BUTTON_RELEASE_MASK    = 0x00000800;
  public static final int GII_VALUATOR_RELATIVE_MASK = 0x00001000;
  public static final int GII_VALUATOR_ABSOLUTE_MASK = 0x00002000;

  // Unit types
  public static final int GII_UNIT_UNKNOWN          = 0;
  public static final int GII_UNIT_TIME             = 1;
  public static final int GII_UNIT_FREQ             = 2;
  public static final int GII_UNIT_LENGTH           = 3;
  public static final int GII_UNIT_VELOCITY         = 4;
  public static final int GII_UNIT_ACCEL            = 5;
  public static final int GII_UNIT_ANGLE            = 6;
  public static final int GII_UNIT_ANGULAR_VELOCITY = 7;
  public static final int GII_UNIT_ANGULAR_ACCEL    = 8;
  public static final int GII_UNIT_AREA             = 9;
  public static final int GII_UNIT_VOLUME           = 10;
  public static final int GII_UNIT_MASS             = 11;
  public static final int GII_UNIT_FORCE            = 12;
  public static final int GII_UNIT_PRESSURE         = 13;
  public static final int GII_UNIT_TORQUE           = 14;
  public static final int GII_UNIT_ENERGY           = 15;
  public static final int GII_UNIT_POWER            = 16;
  public static final int GII_UNIT_TEMP             = 17;
  public static final int GII_UNIT_CURRENT          = 18;
  public static final int GII_UNIT_VOLTAGE          = 19;
  public static final int GII_UNIT_RESISTANCE       = 20;
  public static final int GII_UNIT_CAPACITY         = 21;
  public static final int GII_UNIT_INDUCTIVITY      = 22;

  // Device types (TurboVNC-specific-- not part of the official GII spec)
  public static final int GII_DEVTYPE_NONE   = 0;
  public static final int GII_DEVTYPE_CURSOR = 1;
  public static final int GII_DEVTYPE_STYLUS = 2;
  public static final int GII_DEVTYPE_ERASER = 3;
  public static final int GII_DEVTYPE_TOUCH  = 4;
  public static final int GII_DEVTYPE_PAD    = 5;

  //***************************************************************************
  // QEMU
  //***************************************************************************

  // Message subtypes
  public static final int QEMU_EXTENDED_KEY_EVENT = 0;

  private RFB() {}
};
