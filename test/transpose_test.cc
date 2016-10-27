/*
 *  Copyright (c) 2016 Shawn Pringle B.Sc.. All Rights Reserved.
 *
 *  Use of this source code is governed by the BountySource agreement.
 */

//
// Compile with:
//  g++ -std=c++11 -c -ggdb transpose_test.cc
//
//  g++ -c -ggdb cpp4vpx.cc
//
//	g++ -ggdb cpp4vpx.o vpx_transpose.o transpose_test.o -L .. -lvpx -o
// transpose_test

using namespace std;
#include <iostream>
#include <string.h>
#include <vpx/vpx_image.h>
#include <iomanip>
#include <assert.h>
#include <vpx/vpx_transpose.h>
#include "cpp4vpx.h"
#include <stdlib.h>
#include <vp9/encoder/vp9_resize.h>

using namespace std;
typedef unsigned char uint8_t;

// The following in the image would be displayed like this;
// plane Y: (3x6)
// a b c d e f
// g h i j k l
// m n o p q r
// plane U: (2x3)
// A A A
// B B B
// plane V:
// X X X
// Y Y Y

// The transpose gives us:
// plane Y: (6x3) but beause of alignment it is in (6x4) of memory
// a g m
// b h n
// c i o
// d j p
// e k q
// f l r
// Plane V: (3x2)
// X Y
// X Y
// X Y
// Plane U: (3x2)
// A B
// A B
// A B

const vpx_img_fmt_t img_formats[] = { VPX_IMG_FMT_YV12,   VPX_IMG_FMT_VPXI420,
                                      VPX_IMG_FMT_I440,   VPX_IMG_FMT_I444,
                                      VPX_IMG_FMT_444A,   VPX_IMG_FMT_I42016,
                                      VPX_IMG_FMT_I44016, VPX_IMG_FMT_RGB24,
                                      VPX_IMG_FMT_RGB32 };

void usage(char *progname) {
  cerr << "Unrecognized option." << endl
       << "Usage: " << progname
       << " [-v | --verbose] [-a | --all] [-b | --brief]" << endl;
}

int main(int argc, char **argv) {
  // To do:  correct for chroma shift being different between x, y in one of the
  // cases.
  //      transpose must swap chroma shifts
  bool verbose(false), all(false), breif(false);
  for (int argi = 1; argi < argc; ++argi) {
    char *arg = argv[argi];
    int arglen = strlen(arg);
    if (arg[0] == '-' && arg[1] != '-') {
      for (++arg; *arg; ++arg) {
        switch (*arg) {
          case 'v': verbose = true; break;
          case 'a': all = true; break;
          case 'b': breif = true; break;
          default: usage(argv[0]); return 1;
        }
      }
    } else if (strlen(arg) > 2) {
      if (strncmp(argv[argi], "--verbose", arglen) == 0) {
        verbose = true;
        continue;
      } else if (strncmp(arg, "--all", arglen) == 0) {
        all = true;
        continue;
      } else if (strncmp(arg, "--breif", arglen) == 0) {
        breif = true;
        continue;
      } else {
        usage(argv[0]);
        return 1;
      }
    } else {
      usage(argv[0]);
      return 1;
    }
  }

  vpx_image_t *i18t_control, *i18t_calculated, *i18;

  for (vpx_img_fmt_t f : img_formats) {
    /* vpx_img_alloc is broken for some formats.
           The most demanding fo
           rmat requres 48-bits per pixel, that's six
       bytes. */
    int h, w;
    unsigned char *i18t_control_data;
    vpx_image_t *i18t_control = vpx_img_alloc(NULL, f, 3, 6, 0);
    w = i18t_control->h + 1;
    h = i18t_control->w + 1;
    vpx_img_free(i18t_control);
    i18t_control =
        vpx_img_wrap(NULL, f, 3, 6, 0,
                     i18t_control_data = new unsigned char[(w * h) * 100 + 1]);

    set_pixels(i18t_control, VPX_PLANE_Y, 0, "agm");
    set_pixels(i18t_control, VPX_PLANE_Y, 1, "bhn");
    set_pixels(i18t_control, VPX_PLANE_Y, 2, "cio");
    set_pixels(i18t_control, VPX_PLANE_Y, 3, "djp");
    set_pixels(i18t_control, VPX_PLANE_Y, 4, "ekq");
    set_pixels(i18t_control, VPX_PLANE_Y, 5, "flr");

    if (f & VPX_IMG_FMT_PLANAR) {
      if (i18t_control->x_chroma_shift && i18t_control->y_chroma_shift) {
        /* 6x3 Y planes (Ys) have 3x2 U and V planes*/
        set_pixels(i18t_control, VPX_PLANE_U, 0, "AB");
        set_pixels(i18t_control, VPX_PLANE_U, 1, "AB");
        set_pixels(i18t_control, VPX_PLANE_U, 2, "AB");
        set_pixels(i18t_control, VPX_PLANE_V, 0, "XY");
        set_pixels(i18t_control, VPX_PLANE_V, 1, "XY");
        set_pixels(i18t_control, VPX_PLANE_V, 2, "XY");
      } else if (!i18t_control->x_chroma_shift &&
                 !i18t_control->y_chroma_shift) {
        /* 6x3 Ys have 6x3 U and V planes */
        set_pixels(i18t_control, VPX_PLANE_U, 0, "ABC");
        set_pixels(i18t_control, VPX_PLANE_U, 1, "ABC");
        set_pixels(i18t_control, VPX_PLANE_U, 2, "ABC");
        set_pixels(i18t_control, VPX_PLANE_U, 3, "ABC");
        set_pixels(i18t_control, VPX_PLANE_U, 4, "ABC");
        set_pixels(i18t_control, VPX_PLANE_U, 5, "ABC");

        set_pixels(i18t_control, VPX_PLANE_V, 0, "XYZ");
        set_pixels(i18t_control, VPX_PLANE_V, 1, "XYZ");
        set_pixels(i18t_control, VPX_PLANE_V, 2, "XYZ");
        set_pixels(i18t_control, VPX_PLANE_V, 3, "XYZ");
        set_pixels(i18t_control, VPX_PLANE_V, 4, "XYZ");
        set_pixels(i18t_control, VPX_PLANE_V, 5, "XYZ");
      } else if (i18t_control->y_chroma_shift) {
        /* 6x3 Ys have 3x3 U and V planes but because these planes
         * have an origin of a 3x6 image with 2x3 U and V planes,
         * the Cs and Zs were never in the original image and instead
         * there was only A and B.  The middle are artifacts of the resize_plane
         * routines. */
        for (int row = 0; row < (6 >> i18t_control->y_chroma_shift); ++row) {
          set_pixels(i18t_control, VPX_PLANE_U, row, "ABB");
          set_pixels(i18t_control, VPX_PLANE_V, row, "XYY");
        }
      } else {
        /* 6x3 Ys have 6x2 U and V planes and this image comes a an image with
         * U and V planes that are 3x3*/
        for (int row = 0; row < (6 >> i18t_control->y_chroma_shift); ++row) {
          set_pixels(i18t_control, VPX_PLANE_U, row, "AB");
          set_pixels(i18t_control, VPX_PLANE_V, row, "XY");
        }
      }
      if (!i18->x_chroma_shift && i18->y_chroma_shift) {
        set_pixels(i18t_control, VPX_PLANE_U, 0, "AB");
        set_pixels(i18t_control, VPX_PLANE_U, 1, "AB");
        set_pixels(i18t_control, VPX_PLANE_U, 2, "AB");

        set_pixels(i18t_control, VPX_PLANE_V, 0, "XY");
        set_pixels(i18t_control, VPX_PLANE_V, 1, "XY");
        set_pixels(i18t_control, VPX_PLANE_V, 2, "XY");
      } else if (i18->x_chroma_shift && !i18->y_chroma_shift) {
        set_pixels(i18t_control, VPX_PLANE_U, 0, "ABB");
        set_pixels(i18t_control, VPX_PLANE_U, 1, "ABB");
        set_pixels(i18t_control, VPX_PLANE_U, 2, "ABB");
        set_pixels(i18t_control, VPX_PLANE_V, 0, "XYY");
        set_pixels(i18t_control, VPX_PLANE_V, 1, "XYY");
        set_pixels(i18t_control, VPX_PLANE_V, 2, "XYY");
      }
      if (f & VPX_IMG_FMT_HAS_ALPHA) {
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 0, "* *");
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 1, "* *");
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 2, "* *");
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 3, "* *");
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 4, "* *");
        set_pixels(i18t_control, VPX_PLANE_ALPHA, 5, "***");
      }
    }

    vpx_image_t *i18 =
        vpx_img_wrap(NULL, f, 6, 3, 0, new unsigned char[(w * h) * 100 + 1]);
    if (i18 == NULL) {
      cout << "Unable to allocate 3x6 matrix in format "
           << vpx_img_fmt_to_name(f) << endl;
      continue;
    }
    set_pixels(i18, VPX_PLANE_Y, 0, "abcdef");
    set_pixels(i18, VPX_PLANE_Y, 1, "ghijkl");
    set_pixels(i18, VPX_PLANE_Y, 2, "mnopqr");
    if (f & VPX_IMG_FMT_PLANAR) {
      if (i18->x_chroma_shift && i18->y_chroma_shift) {
        set_pixels(i18, VPX_PLANE_U, 0, "AAA");
        set_pixels(i18, VPX_PLANE_U, 1, "BBB");
        set_pixels(i18, VPX_PLANE_V, 0, "XXX");
        set_pixels(i18, VPX_PLANE_V, 1, "YYY");
      } else if (!i18->x_chroma_shift && !i18->y_chroma_shift) {
        set_pixels(i18, VPX_PLANE_U, 0, "AAAAAA");
        set_pixels(i18, VPX_PLANE_U, 1, "BBBBBB");
        set_pixels(i18, VPX_PLANE_U, 2, "CCCCCC");
        set_pixels(i18, VPX_PLANE_V, 0, "XXXXXX");
        set_pixels(i18, VPX_PLANE_V, 1, "YYYYYY");
        set_pixels(i18, VPX_PLANE_V, 2, "ZZZZZZ");
      } else if (!i18->x_chroma_shift && i18->y_chroma_shift) {
        set_pixels(i18, VPX_PLANE_U, 0, "AAAAAA");
        set_pixels(i18, VPX_PLANE_U, 1, "BBBBBB");
        set_pixels(i18, VPX_PLANE_V, 0, "XXXXXX");
        set_pixels(i18, VPX_PLANE_V, 1, "YYYYYY");
      } else if (i18->x_chroma_shift && !i18->y_chroma_shift) {
        set_pixels(i18, VPX_PLANE_U, 0, "AAA");
        set_pixels(i18, VPX_PLANE_U, 1, "BBB");
        set_pixels(i18, VPX_PLANE_U, 2, "CCC");
        set_pixels(i18, VPX_PLANE_V, 0, "XXX");
        set_pixels(i18, VPX_PLANE_V, 1, "YYY");
        set_pixels(i18, VPX_PLANE_V, 2, "ZZZ");
      }
      if (f & VPX_IMG_FMT_HAS_ALPHA) {
        set_pixels(i18, VPX_PLANE_ALPHA, 0, "******");
        set_pixels(i18, VPX_PLANE_ALPHA, 1, "     *");
        set_pixels(i18, VPX_PLANE_ALPHA, 2, "******");
      }
    }
    vpx_image_t *i18t_calculated =
        vpx_img_wrap(NULL, f, 3, 6, 0, new unsigned char[(w * h) * 100 + 1]);
    i18t_calculated = vpx_img_transpose(i18t_calculated, i18);
    if (i18t_calculated == NULL) {
      // Failure to transpose 3x6 format
      // This means vpx_img_alloc() didn't allocate the amount of memory needed.
      // A lower bound for the size needed would be 4 * h * w of the image.
      cout << "!! Failure to transpose 3x6 format " << vpx_img_fmt_to_name(f)
           << endl;
      continue;
    }

    if (*i18t_calculated != *i18t_control) {
      cout << "Calculated and Control transposed 3x6 matrices for format "
           << vpx_img_fmt_to_name(f) << " do not match!" << endl;
      cout << "Original 3x6 matrix has chroma_shifts (x,y) = "
           << i18->x_chroma_shift << ", " << i18->y_chroma_shift << ":" << endl;
      cout << *i18 << endl;
      cout << "Transpose control" << endl;
      cout << *i18t_control << endl;
      cout << "Transpose calculated: " << endl << *i18t_calculated << endl;
    } else if (all || verbose) {
      cout << "Transpose 3x6 passes for format " << vpx_img_fmt_to_name(f)
           << "." << endl;
      if (verbose) {
        cout << "Original 3x6 matrix:" << endl;
        cout << *i18 << endl;
        cout << "Transpose: " << endl << *i18t_calculated << endl;
      }
    }

    vpx_img_free(i18);
    vpx_img_free(i18t_calculated);
    vpx_img_free(i18t_control);

    // the following line causes a memory corruption error and a crash.  vpx
    // images take ownership
    // of the pointers you give them.
    // delete[] i18t_control_data;
  }

  return 0;
}
