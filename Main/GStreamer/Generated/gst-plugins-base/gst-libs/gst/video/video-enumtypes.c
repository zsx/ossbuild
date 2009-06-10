


#include "video-enumtypes.h"

#include "video.h"

/* enumerations from "video.h" */
GType
gst_video_format_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GST_VIDEO_FORMAT_UNKNOWN, "GST_VIDEO_FORMAT_UNKNOWN", "unknown" },
      { GST_VIDEO_FORMAT_I420, "GST_VIDEO_FORMAT_I420", "i420" },
      { GST_VIDEO_FORMAT_YV12, "GST_VIDEO_FORMAT_YV12", "yv12" },
      { GST_VIDEO_FORMAT_YUY2, "GST_VIDEO_FORMAT_YUY2", "yuy2" },
      { GST_VIDEO_FORMAT_UYVY, "GST_VIDEO_FORMAT_UYVY", "uyvy" },
      { GST_VIDEO_FORMAT_AYUV, "GST_VIDEO_FORMAT_AYUV", "ayuv" },
      { GST_VIDEO_FORMAT_RGBx, "GST_VIDEO_FORMAT_RGBx", "rgbx" },
      { GST_VIDEO_FORMAT_BGRx, "GST_VIDEO_FORMAT_BGRx", "bgrx" },
      { GST_VIDEO_FORMAT_xRGB, "GST_VIDEO_FORMAT_xRGB", "xrgb" },
      { GST_VIDEO_FORMAT_xBGR, "GST_VIDEO_FORMAT_xBGR", "xbgr" },
      { GST_VIDEO_FORMAT_RGBA, "GST_VIDEO_FORMAT_RGBA", "rgba" },
      { GST_VIDEO_FORMAT_BGRA, "GST_VIDEO_FORMAT_BGRA", "bgra" },
      { GST_VIDEO_FORMAT_ARGB, "GST_VIDEO_FORMAT_ARGB", "argb" },
      { GST_VIDEO_FORMAT_ABGR, "GST_VIDEO_FORMAT_ABGR", "abgr" },
      { GST_VIDEO_FORMAT_RGB, "GST_VIDEO_FORMAT_RGB", "rgb" },
      { GST_VIDEO_FORMAT_BGR, "GST_VIDEO_FORMAT_BGR", "bgr" },
      { GST_VIDEO_FORMAT_Y41B, "GST_VIDEO_FORMAT_Y41B", "y41b" },
      { GST_VIDEO_FORMAT_Y42B, "GST_VIDEO_FORMAT_Y42B", "y42b" },
      { GST_VIDEO_FORMAT_YVYU, "GST_VIDEO_FORMAT_YVYU", "yvyu" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GstVideoFormat", values);
  }
  return etype;
}



