//  Copyright (C) 2015-2017 D. R. Commander.  All Rights Reserved.
//
//  This is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this software; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
//  USA.

#include "vncviewer.h"
#include "Exception.h"
#include <math.h>
#include <rfb/turbovnc_devtypes.h>


#define _throw(m) {  \
  vnclog.Print(-1, m"\n");  \
  goto bailout;  \
}


char *stristr(const char *s1, const char *s2)
{
  char *str1, *str2, *ret;
  int i;

  if (!s1 || !s2 || strlen(s1) < 1 || strlen(s2) < 1)
    return NULL;

  str1 = _strdup(s1);
  for (i = 0; i < (int)strlen(str1); i++)
    str1[i] = tolower(str1[i]);
  str2 = _strdup(s2);
  for (i = 0; i < (int)strlen(str2); i++)
    str2[i] = tolower(str2[i]);

  ret = strstr(str1, str2);
  free(str1);  free(str2);
  return ret;
}


void ClientConnection::ReadGII(void)
{
  CARD8 endian, subType;
  rfbServerToClientMsg msg;

  ReadExact((char *)&msg, sz_rfbGIIServerVersionMsg);
  endian = msg.giisv.endianAndSubType & rfbGIIBE;
  subType = msg.giisv.endianAndSubType & ~rfbGIIBE;

  if (endian & rfbGIIBE)
    msg.giisv.length = Swap16IfLE(msg.giisv.length);
  if (msg.giisv.length != sz_rfbGIIServerVersionMsg - 4)
    _throw("ERROR: improperly formatted GII server message");

  switch (subType) {

    case rfbGIIVersion:
      if (endian & rfbGIIBE) {
        msg.giisv.maximumVersion = Swap16IfLE(msg.giisv.maximumVersion);
        msg.giisv.minimumVersion = Swap16IfLE(msg.giisv.minimumVersion);
      }
      if (msg.giisv.maximumVersion < 1 || msg.giisv.minimumVersion > 1)
        _throw("ERROR: GII version mismatch");
      if (msg.giisv.minimumVersion != msg.giisv.maximumVersion)
        vnclog.Print(100, "Server supports GII versions %d - %d\n",
                     msg.giisv.minimumVersion + msg.giisv.maximumVersion);
      else
        vnclog.Print(100, "Server supports GII version %d\n",
                     msg.giisv.minimumVersion);
      supportsGII = true;
      if (!m_opts.m_benchFile) {
        vnclog.Print(-1, "Enabling GII\n");
        SendGIIVersion();
      }
      if (m_pApp->m_wacom)
        CreateWacomGIIDevices();
      break;

    case rfbGIIDeviceCreate:
      if (endian & rfbGIIBE)
        msg.giidc.deviceOrigin = Swap32IfLE(msg.giidc.deviceOrigin);
      if (msg.giidc.deviceOrigin == 0)
        _throw("ERROR: Could not create GII device");
      if (!m_opts.m_benchFile)
        AssignInputDevice(msg.giidc.deviceOrigin);
  }

  bailout:
  return;
}


void ClientConnection::SendGIIVersion(void)
{
  rfbGIIClientVersionMsg giicv;

  if (!supportsGII)
    throw ErrorException("Attempted to send GII version message, but the server does not support the extension.");

  giicv.type = rfbGIIClient;
  giicv.endianAndSubType = rfbGIIVersion | rfbGIIBE;
  giicv.length = Swap16IfLE(sz_rfbGIIClientVersionMsg - 4);
  giicv.version = Swap16IfLE(1);

  WriteExact((char *)&giicv, sz_rfbGIIClientVersionMsg);
}


void ClientConnection::SendGIIDeviceCreate(ExtInputDevice &dev)
{
  rfbGIIDeviceCreateMsg giidc;

  if (!supportsGII)
    throw ErrorException("Server does not support GII");

  giidc.type = rfbGIIClient;
  giidc.endianAndSubType = rfbGIIDeviceCreate | rfbGIIBE;
  giidc.length = Swap16IfLE(sz_rfbGIIDeviceCreateMsg - 4 +
                            dev.valuators.size() * sz_rfbGIIValuator);
  STRNCPY((char *)giidc.deviceName, dev.name, 32);
  giidc.vendorID = Swap32IfLE(dev.vendorID);
  giidc.productID = Swap32IfLE(dev.productID);
  giidc.canGenerate = Swap32IfLE(dev.canGenerate);
  giidc.numRegisters = Swap32IfLE(dev.numRegisters);
  giidc.numValuators = Swap32IfLE(dev.valuators.size());
  giidc.numButtons = Swap32IfLE(dev.numButtons);

  omni_mutex_lock l(m_writeMutex);  // Ensure back-to-back writes are grouped
  WriteExact((char *)&giidc, sz_rfbGIIDeviceCreateMsg);

  std::list<rfbGIIValuator>::const_iterator v;
  for (v = dev.valuators.begin(); v != dev.valuators.end(); ++v) {
    rfbGIIValuator val = *v;

    val.index = Swap32IfLE(val.index);
    val.rangeMin = Swap32IfLE(val.rangeMin);
    val.rangeCenter = Swap32IfLE(val.rangeCenter);
    val.rangeMax = Swap32IfLE(val.rangeMax);
    val.siUnit = Swap32IfLE(val.siUnit);
    val.siAdd = Swap32IfLE(val.siAdd);
    val.siMul = Swap32IfLE(val.siMul);
    val.siDiv = Swap32IfLE(val.siDiv);
    val.siShift = Swap32IfLE(val.siShift);

    WriteExact((char *)&val, sz_rfbGIIValuator);
  }
}


void ClientConnection::SendGIIEvent(UINT deviceID, ExtInputEvent &e)
{
  rfbGIIEventMsg giie;
  ExtInputDevice dev;
  bool found = false;

  if (!supportsGII)
    throw ErrorException("Server does not support GII");

  std::list<ExtInputDevice>::const_iterator d;
  for (d = devices.begin(); d != devices.end(); ++d)
    if (deviceID == d->id && d->remoteID != 0) {
      dev = *d;
      found = true;
    }
  if (!found) return;

  if ((e.type == rfbGIIButtonPress || e.type == rfbGIIButtonRelease) &&
      (e.buttonNumber > dev.numButtons || e.buttonNumber < 1)) {
    vnclog.Print(-1, "Button %d event ignored.\n", e.buttonNumber);
    vnclog.Print(-1, "  Device %s has buttons 1-%d.\n", dev.name);
    return;
  }

  if ((e.type == rfbGIIValuatorRelative || e.type == rfbGIIValuatorAbsolute) &&
      (e.firstValuator + e.numValuators > (int)dev.valuators.size())) {
    vnclog.Print(-1, "Valuator %d-%d event ignored.\n", e.firstValuator,
                 e.firstValuator + e.numValuators - 1);
    vnclog.Print(-1, "  Device %s has valuators 0-%d.", dev.name,
                 dev.valuators.size() - 1);
    return;
  }

  e.print();
  omni_mutex_lock l(m_writeMutex);

  giie.type = rfbGIIClient;
  giie.endianAndSubType = rfbGIIEvent | rfbGIIBE;

  switch (e.type) {

    case rfbGIIButtonPress:
    case rfbGIIButtonRelease:

      giie.length = Swap16IfLE(sz_rfbGIIButtonEvent);

      WriteExact((char *)&giie, sz_rfbGIIEventMsg);

      rfbGIIButtonEvent be;
      be.eventSize = sz_rfbGIIButtonEvent;
      be.eventType = e.type;
      be.pad = 0;
      be.deviceOrigin = Swap32IfLE(dev.remoteID);
      be.buttonNumber = Swap32IfLE(e.buttonNumber);

      WriteExact((char *)&be, sz_rfbGIIButtonEvent);
      break;

    case rfbGIIValuatorRelative:
    case rfbGIIValuatorAbsolute:

      giie.length = Swap16IfLE(sz_rfbGIIValuatorEvent +
                               e.numValuators * sizeof(int));

      WriteExact((char *)&giie, sz_rfbGIIEventMsg);

      rfbGIIValuatorEvent ve;
      ve.eventSize =
        (CARD8)(sz_rfbGIIValuatorEvent + e.numValuators * sizeof(int));
      ve.eventType = e.type;
      ve.pad = 0;
      ve.deviceOrigin = Swap32IfLE(dev.remoteID);
      ve.first = Swap32IfLE(e.firstValuator);
      ve.count = Swap32IfLE(e.numValuators);

      WriteExact((char *)&ve, sz_rfbGIIValuatorEvent);

      for (int i = 0; i < e.numValuators; i++) {
        int valuator = e.valuators[i];

        valuator = Swap32IfLE(valuator);
        WriteExact((char *)&valuator, sizeof(int));
      }
      break;
  }
}


void ClientConnection::AddInputDevice(ExtInputDevice &dev)
{
  devices.push_back(dev);
  dev.print();
  if (!m_opts.m_benchFile)
    SendGIIDeviceCreate(dev);
}


void ClientConnection::AssignInputDevice(int deviceOrigin)
{
  std::list<ExtInputDevice>::iterator dev;
  for (dev = devices.begin(); dev != devices.end(); ++dev) {
    if (dev->remoteID == 0) {
      dev->remoteID = deviceOrigin;
      vnclog.Print(-1, "Successfully created device %d (%s)\n", deviceOrigin,
                   dev->name);
      break;
    }
  }
}


void ClientConnection::CreateWacomGIIDevices(void)
{
  UINT firstCursor, nCursors;
  AXIS x, y, pressure;
  int nStyluses = 0, nErasers = 0;
  HCTX hctx = NULL;

  if (!gpWTInfoA(WTI_DEVICES, DVC_FIRSTCSR, &firstCursor) ||
      !gpWTInfoA(WTI_DEVICES, DVC_NCSRTYPES, &nCursors) || nCursors < 1)
    _throw("Could not list Wacom cursors");

  if (!gpWTInfoA(WTI_DEVICES, DVC_X, &x) ||
      !gpWTInfoA(WTI_DEVICES, DVC_Y, &y) ||
      !gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &pressure))
    _throw("Could not obtain valuator ranges");

  for (UINT i = firstCursor; i < firstCursor + nCursors; i++) {
    DWORD physID;
    ExtInputDevice dev;
    UINT type;
    BOOL active;
    char curName[256] = { 0 };
    WTPKT pktData;
    rfbGIIValuator v;

    if (!gpWTInfoA(WTI_CURSORS + i, CSR_PHYSID, &physID) || physID == 0 ||
        !gpWTInfoA(WTI_CURSORS + i, CSR_ACTIVE, &active) || !active ||
        !gpWTInfoA(WTI_CURSORS + i, CSR_TYPE, &type) ||
        (type & 0x0F06) != 0x0802)
      continue;

    if (!gpWTInfoA(WTI_CURSORS + i, CSR_NAME, curName))
      _throw("Could not get cursor name");
    if (!gpWTInfoA(WTI_CURSORS + i, CSR_PKTDATA, &pktData))
      _throw("Could not get cursor packet data mask");

    /* All X Input devices must provide X and Y coordinates, and there isn't
       much point to adding a tablet device that doesn't provide pressure
       information. */
    if ((pktData & PK_X) == 0 || (pktData & PK_Y) == 0 ||
        (pktData & PK_NORMAL_PRESSURE) == 0)
      continue;

    while (strlen(curName) > 0 && curName[strlen(curName) - 1] == ' ')
      curName[strlen(curName) - 1] = 0;
    /* TurboVNC-specific:  we use productID to represent the device type, so
       we can recreate it on the server */
    if (stristr(curName, "Stylus")) {
      dev.productID = rfbGIIDevTypeStylus;
      snprintf(&curName[strlen(curName)], 256 - strlen(curName), " %d",
               ++nStyluses);
    } else if (stristr(curName, "Eraser")) {
      dev.productID = rfbGIIDevTypeEraser;
      snprintf(&curName[strlen(curName)], 256 - strlen(curName), " %d",
               ++nErasers);
    } else continue;

    STRCPY(dev.name, curName);
    dev.vendorID = 4242;
    dev.id = i;

    if (!gpWTInfoA(WTI_CURSORS + i, CSR_BUTTONS, &dev.numButtons))
      dev.numButtons = 0;

    dev.canGenerate |= rfbGIIValuatorAbsoluteMask;
    if (dev.numButtons && (pktData & PK_BUTTONS))
      dev.canGenerate |= rfbGIIButtonPressMask | rfbGIIButtonReleaseMask;

    dev.absolute = true;

    memset(&v, 0, sizeof(rfbGIIValuator));
    v.index = 0;
    snprintf((char *)v.longName, 75, "Abs X");
    snprintf((char *)v.shortName, 5, "0");
    v.rangeMin = x.axMin;
    v.rangeCenter = (x.axMin + x.axMax) / 2;
    v.rangeMax = x.axMax;
    if (x.axUnits == TU_CENTIMETERS)
      v.siDiv = ROUND(x.axResolution) * 100;
    else if (x.axUnits == TU_INCHES)
      v.siDiv = (int)(ROUND(x.axResolution) * 39.37);
    else
      _throw("Improper units for X axis");
    v.siUnit = rfbGIIUnitLength;
    dev.addValuator(v);

    v.index = 1;
    snprintf((char *)v.longName, 75, "Abs Y");
    snprintf((char *)v.shortName, 5, "1");
    v.rangeMin = y.axMin;
    v.rangeCenter = (y.axMin + y.axMax) / 2;
    v.rangeMax = y.axMax;
    if (x.axUnits == TU_CENTIMETERS)
      v.siDiv = ROUND(y.axResolution) * 100;
    else if (x.axUnits == TU_INCHES)
      v.siDiv = (int)(ROUND(y.axResolution) * 39.37);
    else
      _throw("Improper units for Y axis");
    v.siUnit = rfbGIIUnitLength;
    dev.addValuator(v);

    v.index = 2;
    snprintf((char *)v.longName, 75, "Abs Pressure");
    snprintf((char *)v.shortName, 5, "2");
    v.rangeMin = pressure.axMin;
    v.rangeCenter = (pressure.axMin + pressure.axMax) / 2;
    v.rangeMax = pressure.axMax;
    v.siDiv = 1;
    v.siUnit = rfbGIIUnitLength;
    dev.addValuator(v);

    if (pktData & PK_ORIENTATION) {
      v.index = 3;
      snprintf((char *)v.longName, 75, "Abs Tilt X");
      snprintf((char *)v.shortName, 5, "3");
      v.rangeMin = -64;
      v.rangeCenter = 0;
      v.rangeMax = 63;
      v.siDiv = 57;
      v.siUnit = rfbGIIUnitLength;
      dev.addValuator(v);

      v.index = 4;
      snprintf((char *)v.longName, 75, "Abs Tilt Y");
      snprintf((char *)v.shortName, 5, "4");
      v.rangeMin = -64;
      v.rangeCenter = 0;
      v.rangeMax = 63;
      v.siDiv = 57;
      v.siUnit = rfbGIIUnitLength;
      dev.addValuator(v);
    }

    AddInputDevice(dev);
  }

  if (nStyluses == 0 && nErasers == 0)
    _throw("No extended input devices.");

  bailout:
  return;
}


#define round(x) ((x >= 0) ? (int)(x + 0.5) : (int)(x - 0.5))

void ClientConnection::TranslateWacomCoords(HWND hwnd, int localX, int localY,
                                            int &remoteX, int &remoteY)
{
  // Translate absolute tablet coordinates (which span over all available
  // client-side screens) into tablet coordinates that are relative to the
  // remote desktop.

  RECT winRect, workArea, screenArea;
  GetWindowRect(hwnd, &winRect);
  GetFullScreenMetrics(screenArea, workArea, SPAN_ALL, false);

  AXIS axisX, axisY;
  if (!gpWTInfoA(WTI_DEVICES, DVC_X, &axisX) ||
      !gpWTInfoA(WTI_DEVICES, DVC_Y, &axisY)) {
    vnclog.Print(-1, "ERROR: Could not obtain tablet X, Y axis limits\n");
    remoteX = remoteY = 0;
    return;
  }

  double x = (double)(localX - axisX.axMin) /
             (double)(axisX.axMax - axisX.axMin) *
             (double)(WidthOf(screenArea) - 1) - (double)winRect.left;
  x += m_hScrollPos;
  x *= (double)m_opts.m_scale_den / (double)m_opts.m_scale_num;
  remoteX = (int)(x / (double)(m_si.framebufferWidth - 1) *
                  (double)(axisX.axMax - axisX.axMin) +
                  (double)axisX.axMin + 0.5);
  if (remoteX < axisX.axMin)
    remoteX = axisX.axMin;
  if (remoteX > axisX.axMax)
    remoteX = axisX.axMax;

  double y = (double)(axisY.axMax - localY) /
             (double)(axisY.axMax - axisY.axMin) *
             (double)(HeightOf(screenArea) - 1) - (double)winRect.top;
  y += m_vScrollPos;
  y *= (double)m_opts.m_scale_den / (double)m_opts.m_scale_num;
  remoteY = (int)(y / (double)(m_si.framebufferHeight - 1) *
                  (double)(axisY.axMax - axisY.axMin) +
                  (double)axisY.axMin + 0.5);
  if (remoteY < axisY.axMin)
    remoteY = axisY.axMin;
  if (remoteY > axisY.axMax)
    remoteY = axisY.axMax;
}


#define PI 3.141592653589793238462643

void ClientConnection::ConvertWacomTilt(ORIENTATION orient, int &tiltX,
                                        int &tiltY)
{
  AXIS devOrient[3];

  if (!gpWTInfoA(WTI_DEVICES, DVC_ORIENTATION, devOrient)) {
    vnclog.Print(-1, "ERROR: Could not obtain tablet orientation limits\n");
    tiltX = tiltY = 0;
    return;
  }

  double azimuth = (double)orient.orAzimuth / (double)devOrient[0].axMax *
                   2.0 * PI;
  double altitude;
  if (orient.orAltitude >= 0)
    altitude = 1.0 - (double)orient.orAltitude / (double)devOrient[1].axMax;
  else
    altitude = 1.0 - (double)orient.orAltitude / (double)devOrient[1].axMin;
  altitude = altitude * 1.5;

  tiltX = round(altitude * sin(azimuth) * 63.0);
  if (tiltX < -63)
    tiltX = -63;
  if (tiltX > 63)
    tiltX = 63;
  tiltY = -round(altitude * cos(azimuth) * 63.0);
  if (tiltY < -63)
    tiltY = -63;
  if (tiltY > 63)
    tiltY = 63;
}
