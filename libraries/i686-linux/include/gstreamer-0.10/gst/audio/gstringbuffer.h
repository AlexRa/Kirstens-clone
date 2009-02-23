/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2005 Wim Taymans <wim@fluendo.com>
 *
 * gstringbuffer.h:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_RING_BUFFER_H__
#define __GST_RING_BUFFER_H__

#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_RING_BUFFER             (gst_ring_buffer_get_type())
#define GST_RING_BUFFER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RING_BUFFER,GstRingBuffer))
#define GST_RING_BUFFER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RING_BUFFER,GstRingBufferClass))
#define GST_RING_BUFFER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GST_TYPE_RING_BUFFER, GstRingBufferClass))
#define GST_IS_RING_BUFFER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RING_BUFFER))
#define GST_IS_RING_BUFFER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RING_BUFFER))

typedef struct _GstRingBuffer GstRingBuffer;
typedef struct _GstRingBufferClass GstRingBufferClass;
typedef struct _GstRingBufferSpec GstRingBufferSpec;

/* called to fill data with len bytes of samples */
typedef void (*GstRingBufferCallback) (GstRingBuffer *rbuf, guint8* data, guint len, gpointer user_data);

/**
 * GstRingBufferState:
 * @GST_RING_BUFFER_STATE_STOPPED: The ringbuffer is stopped
 * @GST_RING_BUFFER_STATE_PAUSED: The ringbuffer is paused
 * @GST_RING_BUFFER_STATE_STARTED: The ringbuffer is started
 *
 * The state of the ringbuffer.
 */
typedef enum {
  GST_RING_BUFFER_STATE_STOPPED,
  GST_RING_BUFFER_STATE_PAUSED,
  GST_RING_BUFFER_STATE_STARTED,
} GstRingBufferState;

/**
 * GstRingBufferSegState:
 * @GST_SEGSTATE_INVALID: The content of the segment is invalid
 * @GST_SEGSTATE_EMPTY: The segment is empty
 * @GST_SEGSTATE_FILLED: The segment contains valid data
 * @GST_SEGSTATE_PARTIAL: The segment partially contains valid data
 *
 * The state of a segment in the ringbuffer.
 */
typedef enum {
  GST_SEGSTATE_INVALID,
  GST_SEGSTATE_EMPTY,
  GST_SEGSTATE_FILLED,
  GST_SEGSTATE_PARTIAL,
} GstRingBufferSegState;

/**
 * GstBufferFormatType:
 * @GST_BUFTYPE_LINEAR: samples in linear PCM
 * @GST_BUFTYPE_FLOAT: samples in float 
 * @GST_BUFTYPE_MU_LAW: samples in mulaw
 * @GST_BUFTYPE_A_LAW: samples in alaw
 * @GST_BUFTYPE_IMA_ADPCM: samples in ima adpcm
 * @GST_BUFTYPE_MPEG: samples in mpeg audio format
 * @GST_BUFTYPE_GSM: samples in gsm format
 *
 * The format of the samples in the ringbuffer.
 */
typedef enum
{
  GST_BUFTYPE_LINEAR,
  GST_BUFTYPE_FLOAT,
  GST_BUFTYPE_MU_LAW,
  GST_BUFTYPE_A_LAW,
  GST_BUFTYPE_IMA_ADPCM,
  GST_BUFTYPE_MPEG,
  GST_BUFTYPE_GSM,
} GstBufferFormatType;

typedef enum
{
  GST_UNKNOWN,

  GST_S8,
  GST_U8,

  GST_S16_LE,
  GST_S16_BE,
  GST_U16_LE,
  GST_U16_BE,

  GST_S24_LE,
  GST_S24_BE,
  GST_U24_LE,
  GST_U24_BE,

  GST_S32_LE,
  GST_S32_BE,
  GST_U32_LE,
  GST_U32_BE,

  GST_S24_3LE,
  GST_S24_3BE,
  GST_U24_3LE,
  GST_U24_3BE,
  GST_S20_3LE,
  GST_S20_3BE,
  GST_U20_3LE,
  GST_U20_3BE,
  GST_S18_3LE,
  GST_S18_3BE,
  GST_U18_3LE,
  GST_U18_3BE,

  GST_FLOAT32_LE,
  GST_FLOAT32_BE,

  GST_FLOAT64_LE,
  GST_FLOAT64_BE,

  GST_MU_LAW,
  GST_A_LAW,
  GST_IMA_ADPCM,
  GST_MPEG,
  GST_GSM,

  /* fill me */

} GstBufferFormat;

/**
 * GstRingBufferSpec:
 * @caps: The caps that generated the Spec.
 * @type: the sample type
 * @format: the sample format
 * @sign: the sample sign
 * @bigend: the endianness of the samples
 * @width: the width of the samples
 * @depth: th depth of the samples
 * @rate: the samplerate
 * @channels: the number of channels
 * @latency_time: the latency in time units
 * @buffer_time: the total buffer size in time units
 * @segsize: the size of one segment in bytes
 * @segtotal: the total number of segments
 * @bytes_per_sample: number of bytes in one sample
 * @silence_sample: bytes representing one sample of silence
 *
 * The structure containing the format specification of the ringbuffer.
 */
struct _GstRingBufferSpec
{
  /*< public >*/
  /* in */
  GstCaps  *caps;               /* the caps of the buffer */

  /* in/out */
  GstBufferFormatType   type;
  GstBufferFormat format;
  gboolean  sign;
  gboolean  bigend;
  gint      width;
  gint      depth;
  gint      rate;
  gint      channels;
  
  GstClockTime latency_time;    /* the required/actual latency time */
  GstClockTime buffer_time;     /* the required/actual time of the buffer */
  gint     segsize;             /* size of one buffer segement */
  gint     segtotal;            /* total number of segments */

  /* out */
  gint     bytes_per_sample;    /* number of bytes of one sample */
  guint8   silence_sample[32];  /* bytes representing silence */

  /*< private >*/
  gpointer _gst_reserved[GST_PADDING];
};

#define GST_RING_BUFFER_GET_COND(buf) (((GstRingBuffer *)buf)->cond)
#define GST_RING_BUFFER_WAIT(buf)     (g_cond_wait (GST_RING_BUFFER_GET_COND (buf), GST_OBJECT_GET_LOCK (buf)))
#define GST_RING_BUFFER_SIGNAL(buf)   (g_cond_signal (GST_RING_BUFFER_GET_COND (buf)))
#define GST_RING_BUFFER_BROADCAST(buf)(g_cond_broadcast (GST_RING_BUFFER_GET_COND (buf)))

/**
 * GstRingBuffer:
 * @cond: used to signal start/stop/pause/resume actions
 * @open: boolean indicating that the ringbuffer is open
 * @acquired: boolean indicating that the ringbuffer is acquired
 * @data: data in the ringbuffer
 * @spec: format and layout of the ringbuffer data
 * @segstate: status of each segment in the ringbuffer (unused)
 * @samples_per_seg: number of samples in one segment
 * @empty_seg: pointer to memory holding one segment of silence samples
 * @state: state of the buffer
 * @segdone: readpointer in the ringbuffer
 * @segbase: segment corresponding to segment 0 (unused)
 * @waiting: is a reader or writer waiting for a free segment
 *
 * The ringbuffer base class structure.
 */
struct _GstRingBuffer {
  GstObject              object;

  /*< public >*/ /* with LOCK */
  GCond                 *cond;
  gboolean               open;
  gboolean               acquired;
  GstBuffer             *data;
  GstRingBufferSpec      spec;
  GstRingBufferSegState *segstate;
  gint                   samples_per_seg;
  guint8                *empty_seg;

  /*< public >*/ /* ATOMIC */
  gint                   state;
  gint                   segdone;
  gint                   segbase;
  gint                   waiting;

  /*< private >*/
  GstRingBufferCallback  callback;
  gpointer               cb_data;

  /*< private >*/
  union {
    struct {
      gboolean           flushing;
      /* ATOMIC */
      gint               may_start;
    } ABI;
    /* adding + 0 to mark ABI change to be undone later */
    gpointer _gst_reserved[GST_PADDING + 0];
  } abidata;
};

/**
 * GstRingBufferClass:
 * @open_device:  open the device, don't set any params or allocate anything
 * @acquire: allocate the resources for the ringbuffer using the given spec
 * @release: free resources of the ringbuffer
 * @close_device: close the device
 * @start: start processing of samples
 * @pause: pause processing of samples
 * @resume: resume processing of samples after pause
 * @stop: stop processing of samples
 * @delay: get number of samples queued in device
 *
 * The vmethods that subclasses can override to implement the ringbuffer.
 */
struct _GstRingBufferClass {
  GstObjectClass parent_class;

  /*< public >*/
  gboolean     (*open_device)  (GstRingBuffer *buf);
  gboolean     (*acquire)      (GstRingBuffer *buf, GstRingBufferSpec *spec);
  gboolean     (*release)      (GstRingBuffer *buf);
  gboolean     (*close_device) (GstRingBuffer *buf);

  gboolean     (*start)        (GstRingBuffer *buf);
  gboolean     (*pause)        (GstRingBuffer *buf);
  gboolean     (*resume)       (GstRingBuffer *buf);
  gboolean     (*stop)         (GstRingBuffer *buf);

  guint        (*delay)        (GstRingBuffer *buf);

  /*< private >*/
  gpointer _gst_reserved[GST_PADDING];
};

GType gst_ring_buffer_get_type(void);

/* callback stuff */
void            gst_ring_buffer_set_callback    (GstRingBuffer *buf, GstRingBufferCallback cb, 
                                                 gpointer user_data);

gboolean        gst_ring_buffer_parse_caps      (GstRingBufferSpec *spec, GstCaps *caps);
void            gst_ring_buffer_debug_spec_caps (GstRingBufferSpec *spec);
void            gst_ring_buffer_debug_spec_buff (GstRingBufferSpec *spec);

/* device state */
gboolean        gst_ring_buffer_open_device     (GstRingBuffer *buf);
gboolean        gst_ring_buffer_close_device    (GstRingBuffer *buf);

gboolean        gst_ring_buffer_device_is_open  (GstRingBuffer *buf);

/* allocate resources */
gboolean        gst_ring_buffer_acquire         (GstRingBuffer *buf, GstRingBufferSpec *spec);
gboolean        gst_ring_buffer_release         (GstRingBuffer *buf);

gboolean        gst_ring_buffer_is_acquired     (GstRingBuffer *buf);

/* flushing */
void            gst_ring_buffer_set_flushing    (GstRingBuffer *buf, gboolean flushing);

/* playback/pause */
gboolean        gst_ring_buffer_start           (GstRingBuffer *buf);
gboolean        gst_ring_buffer_pause           (GstRingBuffer *buf);
gboolean        gst_ring_buffer_stop            (GstRingBuffer *buf);

/* get status */
guint           gst_ring_buffer_delay           (GstRingBuffer *buf);
guint64         gst_ring_buffer_samples_done    (GstRingBuffer *buf);

void            gst_ring_buffer_set_sample      (GstRingBuffer *buf, guint64 sample);

/* clear all segments */
void            gst_ring_buffer_clear_all       (GstRingBuffer *buf);

/* commit samples */
guint           gst_ring_buffer_commit          (GstRingBuffer *buf, guint64 sample, 
                                                 guchar *data, guint len);
/* read samples */
guint           gst_ring_buffer_read            (GstRingBuffer *buf, guint64 sample, 
                                                 guchar *data, guint len);

/* mostly protected */
gboolean        gst_ring_buffer_prepare_write   (GstRingBuffer *buf, gint *segment, guint8 **writeptr, gint *len);
gboolean        gst_ring_buffer_prepare_read    (GstRingBuffer *buf, gint *segment, guint8 **readptr, gint *len);
void            gst_ring_buffer_clear           (GstRingBuffer *buf, gint segment);
void            gst_ring_buffer_advance         (GstRingBuffer *buf, guint advance);

void            gst_ring_buffer_may_start       (GstRingBuffer *buf, gboolean allowed);


G_END_DECLS

#endif /* __GST_RING_BUFFER_H__ */
