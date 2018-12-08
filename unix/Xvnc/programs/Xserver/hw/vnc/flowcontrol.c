/*
 * flowcontrol.c - implement RFB flow control extensions
 */

/*
 *  Copyright (C) 2012, 2014, 2017-2018 D. R. Commander.  All Rights Reserved.
 *  Copyright (C) 2011 Pierre Ossman for Cendio AB.  All Rights Reserved.
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 *  USA.
 */

/* #define CONGESTION_DEBUG */

#include "rfb.h"
#include <netinet/tcp.h>
#include <sys/time.h>


/* This window should get us going fairly quickly on a network with decent
   bandwidth.  If it's too high, then it will rapidly be reduced and stay
   low. */
static const unsigned INITIAL_WINDOW = 16384;

/* TCP's minimal window is 3 * MSS, but since we don't know the MSS, we
   make a guess at 4 KB (it's probaly a bit higher). */
static const unsigned MINIMUM_WINDOW = 4096;

/* The current default maximum window for Linux (4 MB).  Should be a good
   limit for now... */
static const unsigned MAXIMUM_WINDOW = 4194304;


typedef struct {
  struct timeval tv;
  int offset;
  unsigned inFlight;
} RTTInfo;


static void HandleRTTPong(rfbClientPtr, RTTInfo *);
static void UpdateCongestion(rfbClientPtr);


static CARD32 congestionCallback(OsTimerPtr timer, CARD32 time, pointer arg)
{
  rfbClientPtr cl = (rfbClientPtr)arg;

  UpdateCongestion(cl);
  cl->congestionTimerRunning = FALSE;
  return 0;
}


static time_t msSince(const struct timeval *then)
{
  struct timeval now;
  time_t diff;

  gettimeofday(&now, NULL);

  diff = (now.tv_sec - then->tv_sec) * 1000;

  diff += now.tv_usec / 1000;
  diff -= then->tv_usec / 1000;

  return diff;
}


void rfbInitFlowControl(rfbClientPtr cl)
{
  cl->ackedOffset = cl->sockOffset;
  cl->congWindow = INITIAL_WINDOW;
}


/*
 * This is called whenever a client fence message is received.
 */
void HandleFence(rfbClientPtr cl, CARD32 flags, unsigned len, const char *data)
{
  RTTInfo rttInfo;

  if (flags & rfbFenceFlagRequest) {

    if (flags & rfbFenceFlagSyncNext) {
      cl->pendingSyncFence = TRUE;
      cl->fenceFlags = flags & (rfbFenceFlagBlockBefore |
                                rfbFenceFlagBlockAfter |
                                rfbFenceFlagSyncNext);
      cl->fenceDataLen = len;
      if (len > 0)
        memcpy(cl->fenceData, data, len);
      return;
    }

    /* We handle everything synchronously, so we trivially honor these
       modes */
    flags = flags & (rfbFenceFlagBlockBefore | rfbFenceFlagBlockAfter);

    rfbSendFence(cl, flags, len, data);
    return;
  }

  switch (len) {
    case 0:
      /* Initial dummy fence */
      break;

    case sizeof(RTTInfo):
      memcpy(&rttInfo, data, sizeof(RTTInfo));
      HandleRTTPong(cl, &rttInfo);
      break;

    default:
      rfbLog("Fence of unusual size received\n");
  }
}


static void HandleRTTPong(rfbClientPtr cl, RTTInfo *rttInfo)
{
  unsigned rtt, delay;

  cl->pingCounter--;

  rtt = msSince(&rttInfo->tv);
  if (rtt < 1)
    rtt = 1;

  cl->ackedOffset = rttInfo->offset;

  /* Try to estimate wire latency by tracking lowest latency seen */
  if (rtt < cl->baseRTT)
    cl->baseRTT = rtt;

  if (rttInfo->inFlight > cl->congWindow) {
    cl->seenCongestion = TRUE;

    /* Estimate added delay because of overtaxed buffers */
    delay = (rttInfo->inFlight - cl->congWindow) *
            cl->baseRTT / cl->congWindow;

    if (delay < rtt)
      rtt -= delay;
    else
      rtt = 1;

    /* If we underestimate the congestion window, then we'll get a latency
       that's less than the wire latency, which will confuse other portions
       of the code. */
    if (rtt < cl->baseRTT)
      rtt = cl->baseRTT;
  }

  /* We only keep track of the minimum latency seen (for a given interval)
     because we want to avoid issuing buffers continously, but we don't mind
     (and even approve of) bursts. */
  if (rtt < cl->minRTT)
    cl->minRTT = rtt;
}


Bool rfbSendRTTPing(rfbClientPtr cl)
{
  RTTInfo rttInfo;

  if (!cl->enableFence)
    return TRUE;

  memset(&rttInfo, 0, sizeof(RTTInfo));

  gettimeofday(&rttInfo.tv, NULL);
  rttInfo.offset = cl->sockOffset;
  rttInfo.inFlight = rttInfo.offset - cl->ackedOffset;

  /* We need to make sure that any old updates are already processed by the
     time we get the response back.  This allows us to reliably throttle
     back if the client or the network overloads. */
  if (!rfbSendFence(cl, rfbFenceFlagRequest | rfbFenceFlagBlockBefore,
                    sizeof(RTTInfo), (const char *)&rttInfo))
    return FALSE;

  cl->pingCounter++;

  cl->sentOffset = rttInfo.offset;

  /* Let some data flow before we adjust the settings */
  if (!cl->congestionTimerRunning) {
    cl->congestionTimer = TimerSet(cl->congestionTimer, 0,
                                   min(cl->baseRTT * 2, 100),
                                   congestionCallback, cl);
    cl->congestionTimerRunning = TRUE;
  }
  return TRUE;
}


static void UpdateCongestion(rfbClientPtr cl)
{
  unsigned diff;

  if (!cl->seenCongestion)
    return;

  diff = cl->minRTT - cl->baseRTT;

  if (diff > min(100, cl->baseRTT)) {
    /* Way too fast */
    cl->congWindow = cl->congWindow * cl->baseRTT / cl->minRTT;
  } else if (diff > min(50, cl->baseRTT / 2)) {
    /* Slightly too fast */
    cl->congWindow -= 4096;
  } else if (diff < 5) {
    /* Way too slow */
    cl->congWindow += 8192;
  } else if (diff < 25) {
    /* Too slow */
    cl->congWindow += 4096;
  }

  if (cl->congWindow < MINIMUM_WINDOW)
    cl->congWindow = MINIMUM_WINDOW;
  if (cl->congWindow > MAXIMUM_WINDOW)
    cl->congWindow = MAXIMUM_WINDOW;

#ifdef CONGESTION_DEBUG
  rfbLog("RTT: %d ms (%d ms), Window: %d KB, Offset: %d KB, Bandwidth: %g Mbps\n",
         cl->minRTT, cl->baseRTT, cl->congWindow / 1024, cl->sockOffset / 1024,
         cl->congWindow * 8.0 / cl->baseRTT / 1000.0);

#ifdef TCP_INFO
  struct tcp_info tcp_info;
  socklen_t tcp_info_length;

  tcp_info_length = sizeof(tcp_info);
  if (getsockopt(cl->sock, SOL_TCP, TCP_INFO, (void *)&tcp_info,
                 &tcp_info_length) == 0) {
    rfbLog("Socket: RTT: %d ms (+/- %d ms) Window %d KB\n",
           tcp_info.tcpi_rtt / 1000, tcp_info.tcpi_rttvar / 1000,
           tcp_info.tcpi_snd_mss * tcp_info.tcpi_snd_cwnd / 1024);
  }
#endif

#endif

  cl->minRTT = -1;
  cl->seenCongestion = FALSE;
}


/*
 * rfbSendFence sends a fence message to a specific client
 */
Bool rfbSendFence(rfbClientPtr cl, CARD32 flags, unsigned len,
                  const char *data)
{
  rfbFenceMsg f;

  if (!cl->enableFence) {
    rfbLog("ERROR in rfbSendFence: Client does not support fence extension\n");
    return FALSE;
  }
  if (len > 64) {
    rfbLog("ERROR in rfbSendFence: Fence payload is too large\n");
    return FALSE;
  }
  if ((flags & ~rfbFenceFlagsSupported) != 0) {
    rfbLog("ERROR in rfbSendFence: Unknown fence flags\n");
    return FALSE;
  }

  memset(&f, 0, sz_rfbFenceMsg);
  f.type = rfbFence;
  f.flags = Swap32IfLE(flags);
  f.length = len;

  if (WriteExact(cl, (char *)&f, sz_rfbFenceMsg) < 0) {
    rfbLogPerror("rfbSendFence: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  if (WriteExact(cl, (char *)data, len) < 0) {
    rfbLogPerror("rfbSendFence: write");
    rfbCloseClient(cl);
    return FALSE;
  }
  return TRUE;
}


Bool rfbIsCongested(rfbClientPtr cl)
{
  int offset;  time_t sockIdleTime;

  if (!cl->enableFence)
    return FALSE;

  sockIdleTime = msSince(&cl->lastWrite);

  /* Idle for too long? (and no data on the wire)

     FIXME: This should really just be one baseRTT, but we're getting
            problems with triggering the idle timeout on each update.
            Maybe we need to use a moving average for the wire latency
            instead of baseRTT. */
  if ((cl->sentOffset == cl->ackedOffset) &&
      (sockIdleTime > 2 * cl->baseRTT)) {

#ifdef CONGESTION_DEBUG
    if (cl->congWindow > INITIAL_WINDOW)
      rfbLog("Reverting to initial window (%d KB) after %d ms\n",
             INITIAL_WINDOW / 1024, sockIdleTime);
#endif

    /* Close congestion window and allow a transfer
       FIXME: Reset baseRTT like Linux Vegas? */
    cl->congWindow = min(INITIAL_WINDOW, cl->congWindow);
    return FALSE;
  }

  offset = cl->sockOffset;

  /* FIXME: Should we compensate for non-update data?
            (i.e. use sentOffset instead of offset) */
  if ((offset - cl->ackedOffset) < cl->congWindow)
    return FALSE;

  /* If we just have one outstanding "ping", then that means the client has
     started receiving our update.  In order to provide backward
     compatibility with viewers that don't support the flow control
     extensions, we allow another update here.  This could further clog up
     the tubes, but if we reach this point, congestion control isn't really
     working properly anyhow, as the wire would otherwise be idle for at
     least RTT/2. */
  if (cl->pingCounter == 1)
    return FALSE;

  return TRUE;
}


/*
 * rfbSendEndOfCU sends an end of Continuous Updates message to a specific
 * client
 */
Bool rfbSendEndOfCU(rfbClientPtr cl)
{
  CARD8 type = rfbEndOfContinuousUpdates;

  if (!cl->enableCU) {
    rfbLog("ERROR in rfbSendEndOfCU: Client does not support Continuous Updates\n");
    return FALSE;
  }

  if (WriteExact(cl, (char *)&type, 1) < 0) {
    rfbLogPerror("rfbSendEndOfCU: write");
    rfbCloseClient(cl);
    return FALSE;
  }

  return TRUE;
}
