#include <iostream>
#include <string.h>
#include <vpx/vpx_image.h>
#include <iomanip>
#include <assert.h>
#include <vpx/vpx_transpose.h>
#include <exception>
#include <sstream>
using namespace std;

ostream &output_packed_data(ostream &out, unsigned char *sp, int bit_depth,
                            int width) {
  unsigned short *ssp = (unsigned short *)sp;
  switch (bit_depth) {
    case 8:
      for (int j = 0; j < width; ++j) {
        out << sp[j];
        out << " ";
      }
      break;
    case 16:
      for (int j = 0; j < width; ++j) {
        out << (char)ssp[j] << " ";
      }
  }
  return out;
}

ostream &operator<<(ostream &out, const vpx_image_t &img) {
  unsigned char *sp;
  if (img.fmt == VPX_IMG_FMT_NONE) {
    return out << "None";
  }
  if (img.fmt & VPX_IMG_FMT_PLANAR) out << "PLANE Y :" << endl;
  for (int i = 0; i < img.d_h; ++i) {
    output_packed_data(out,
                       &img.planes[VPX_PLANE_Y][i * img.stride[VPX_PLANE_Y]],
                       img.bit_depth, img.d_w)
        << endl;
  }
  if (img.fmt & VPX_IMG_FMT_PLANAR) {
    for (short p = VPX_PLANE_U; p < VPX_PLANE_ALPHA; ++p) {
      unsigned char *plane = img.planes[p];
      out << "PLANE " << (unsigned char)(p - VPX_PLANE_U + 'U') << ":" << endl;
      for (int i = 0; i < ceiling_shift(img.d_h, img.y_chroma_shift); ++i) {
        output_packed_data(out, &img.planes[p][i * img.stride[p]],
                           img.bit_depth,
                           ceiling_shift(img.d_w, img.x_chroma_shift))
            << endl;
      }
    }
    if ((img.fmt & VPX_IMG_FMT_HAS_ALPHA) != 0 &&
        img.planes[VPX_PLANE_ALPHA] != NULL) {
      out << "PLANE ALPHA" << endl;
      for (int i = 0; i < img.d_h; ++i) {
        output_packed_data(
            out, &img.planes[VPX_PLANE_ALPHA][i * img.stride[VPX_PLANE_ALPHA]],
            img.bit_depth, img.d_w)
            << endl;
      }
    }
  }

  return out;
}

bool operator==(const vpx_image_t &a, const vpx_image_t &b) {
  if (a.fmt != b.fmt) return false;
  if (a.d_w != b.d_w || a.d_h != b.d_h) return false;
  if (a.y_chroma_shift != b.y_chroma_shift) return false;
  if (a.x_chroma_shift != b.x_chroma_shift) return false;

  for (int p = VPX_PLANE_Y; p <= VPX_PLANE_ALPHA; ++p) {
    if (!a.planes[p] ^ !b.planes[p]) return false;
    if (a.planes[p] == NULL) continue;

    int w = p == VPX_PLANE_Y ? a.d_w : ceiling_shift(a.d_w, a.x_chroma_shift);
    int h = p == VPX_PLANE_Y ? a.d_h : ceiling_shift(a.d_h, a.y_chroma_shift);

    for (int row = 0; row < h; ++row) {
      for (int col = 0; col < w; ++col) {
        if (a.planes[p][a.stride[p] * row + col] !=
            b.planes[p][b.stride[p] * row + col])
          return false;
      }
    }
  }
  return true;
}

bool operator!=(const vpx_image_t &a, const vpx_image_t &b) {
  return !(a == b);
}
bool set_pixels(vpx_image_t *iptr, int plane, int row, const char *str_p) {
  int len = strlen((const char *)str_p);
  const char *str((const char *)str_p);
  int stride = iptr->stride[plane];
  if (len > iptr->d_w) return false;
  unsigned char *dp = &iptr->planes[plane][stride * row];
  switch (iptr->bit_depth) {
    case 8:
      memcpy(dp, str, len);
      break;
    // This is all little endian, and for the testing purposes of transpose its
    // enough to
    // use one endianess throughout.
    case 16:
      for (int i = 0; i < len; ++i) {
        *dp = str[i];
        *++dp = ' ';
        ++dp;
      }
      break;
    case 24:
      for (int i = 0; i < len; ++i) {
        *dp = str[i];
        *++dp = ' ';
        *++dp = ' ';
        ++dp;
      }
      break;
    case 32:
      for (int i = 0; i < len; ++i) {
        *dp = str[i];
        *++dp = 0;
        *++dp = 0;
        *++dp = 0;
        ++dp;
      }
      break;
    case 48:
      for (int i = 0; i < len; ++i) {
        *dp = str[i];
        for (int j = 0; j < 5; ++j) {
          *++dp = 0;
        }
        ++dp;
      }
      break;
    default: return false;
  }
  return true;
}

int eq_pixels(vpx_image_t *iptr, int plane, int row, const char *str_p) {
  const bool unexpected_bitdepth(false);

  int len = strlen((const char *)str_p);
  const char *str((const char *)str_p);
  int stride = iptr->stride[plane];
  if (len > iptr->d_w) return false;
  unsigned char *dp = &iptr->planes[plane][stride * row];
  switch (iptr->bit_depth) {
    case 8:
      return memcmp(dp, str, len) == 0;
    // This is all little endian, and for the testing purposes of transpose its
    // enough to
    // use one endianess throughout.
    case 16:
      for (int i = 0; i < len; ++i) {
        if (*dp != str[i]) return false;
        ++dp;
        ++dp;
      }
      return true;
      break;
    case 24:
      for (int i = 0; i < len; ++i) {
        if (*dp != str[i]) return false;
        ++dp;
        ++dp;
        ++dp;
      }
      return true;
    case 32:
      for (int i = 0; i < len; ++i) {
        if (*dp != str[i]) return false;
        dp += 4;
      }
      return true;
    case 48:
      for (int i = 0; i < len; ++i) {
        if (*dp != str[i]) return false;
        dp += 6;
      }
      return true;
    default: { assert(unexpected_bitdepth); }
  }
  return true;
}

const struct num_string_pair {
  vpx_img_fmt_t id;
  const char *name;
} packed_format_names[] = { { VPX_IMG_FMT_RGB24, "VPX_IMG_FMT_RGB24" },
                            { VPX_IMG_FMT_RGB32, "VPX_IMG_FMT_RGB32" },
                            { VPX_IMG_FMT_RGB565, "VPX_IMG_FMT_RGB565" },
                            { VPX_IMG_FMT_RGB555, "VPX_IMG_FMT_RGB555" },
                            { VPX_IMG_FMT_UYVY, "VPX_IMG_FMT_UYVY" },
                            { VPX_IMG_FMT_YUY2, "VPX_IMG_FMT_YUY2" },
                            { VPX_IMG_FMT_YVYU, "VPX_IMG_FMT_YVYU" },
                            { VPX_IMG_FMT_BGR24, "VPX_IMG_FMT_BGR24" },
                            { VPX_IMG_FMT_RGB32_LE, "VPX_IMG_FMT_RGB32_LE" },
                            { VPX_IMG_FMT_ARGB, "VPX_IMG_FMT_ARGB" },
                            { VPX_IMG_FMT_ARGB_LE, "VPX_IMG_FMT_ARGB_LE" },
                            { VPX_IMG_FMT_RGB565_LE, "VPX_IMG_FMT_RGB565_LE" },
                            { VPX_IMG_FMT_RGB555_LE, "VPX_IMG_FMT_RGB555_LE" },
                            { VPX_IMG_FMT_YV12, "VPX_IMG_FMT_YV12" },
                            { VPX_IMG_FMT_I420, "VPX_IMG_FMT_I420" },
                            { VPX_IMG_FMT_VPXYV12, "VPX_IMG_FMT_VPXYV12" },
                            { VPX_IMG_FMT_VPXI420, "VPX_IMG_FMT_VPXI420" },
                            { VPX_IMG_FMT_I422, "VPX_IMG_FMT_I422" },
                            { VPX_IMG_FMT_I444, "VPX_IMG_FMT_I444" },
                            { VPX_IMG_FMT_I440, "VPX_IMG_FMT_I440" },
                            { VPX_IMG_FMT_444A, "VPX_IMG_FMT_444A" },
                            { VPX_IMG_FMT_I42016, "VPX_IMG_FMT_I42016" },
                            { VPX_IMG_FMT_I42216, "VPX_IMG_FMT_I42216" },
                            { VPX_IMG_FMT_I44416, "VPX_IMG_FMT_I44416" },
                            { VPX_IMG_FMT_I44016, "VPX_IMG_FMT_I44016" },
                            { VPX_IMG_FMT_NONE, "VPX_IMG_FMT_NONE" } };

const char *vpx_img_fmt_to_name(const enum vpx_img_fmt f) {
  for (int i = 0; packed_format_names[i].id != VPX_IMG_FMT_NONE; ++i) {
    if (f == packed_format_names[i].id) {
      return packed_format_names[i].name;
    }
  }
  return "VPX_IMG_FMT_UNKNOWN";
}
