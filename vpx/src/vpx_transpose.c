/*
 *  Copyright (c) 2016 Shawn Pringle B.Sc.. All Rights Reserved.
 *
 *  Use of this source code is governed by the BountySource agreement.
 */

#include <vpx/vpx_image.h>
#include <vpx/vpx_transpose.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

void vpx_plane_transpose_1(unsigned char *dst, const unsigned short dst_stride, // destination plane and stride
    const unsigned char *src, const unsigned short src_stride, // source plane and stride
    const int N,               // number of columns in *src*
    const int M               // Number of rows in *src*
    ) {
    int n;
    for (n = 0; n < N * M; ++n) {
      int i = n / N;
      int j = n % N;
      dst[dst_stride * j + i] = src[src_stride * i + j];
    }
}

void vpx_plane_transpose_2(unsigned char *dst, const unsigned short dst_stride, // destination plane and stride
    const unsigned char *src, const unsigned short src_stride, // source plane and stride
    const int N,               // number of columns in *src*
    const int M               // Number of rows in *src*
    ) {
    int i, j;
    for (i = 0; i < M; ++i) {
      for (j = 0; j < N; ++j) {
        dst[j * dst_stride + 2 * i] = src[i * src_stride + 2 * j];
        dst[j * dst_stride + 2 * i + 1] = src[i * src_stride + 2 * j + 1];
      }
    }
}

void vpx_plane_transpose(
    unsigned char *dst, const unsigned short dst_stride, // destination plane and stride
    const unsigned char *src, const unsigned short src_stride, // source plane and stride
    const int N,               // number of columns in *src*
    const int M,               // Number of rows in *src*
    const unsigned short size  // Unit size of a pixel in bytes
    ) {
  int n, i, j;

  if (size != 1) {
    memset(dst, ' ', N * M * 2);
    for (i = 0; i < M; ++i) {
      for (j = 0; j < N; ++j) {
        dst[j * dst_stride + 2 * i] = src[i * src_stride + 2 * j];
        dst[j * dst_stride + 2 * i + 1] = src[i * src_stride + 2 * j + 1];
      }
    }
  } else
    for (n = 0; n < N * M; ++n) {
      int i = n / N;
      int j = n % N;
      dst[dst_stride * j + i] = src[src_stride * i + j];
    }
}

/*! \brief  Resizes an 8-bit depth plane using 1 pixel box sampling method.
 *
 * \param[in]         dst              the memory to write the resized plane to
 * \param[in]         dst_stride       the stride value for the destination
 * plane
 * \param[in]         new_width        the new width of the image
 * \param[in]         new_height       the new height of the image
 * \param[in]         src              the memory to read the original plane
 * from
 * \param[in]         src_stride       the stride value for the original plane
 * \param[in]         old_width        the old width of the image
 * \param[in]         old_height       the old height of the image
 *
 */
static int vpx_plane_resize_1(unsigned char *dst, int dst_stride,
                               int new_width, int new_height,
                               unsigned char *src, int src_stride,
                               int old_width, int old_height) {
  double Kj;
  double Ki;
  int new_i, new_j;                                                               
                                            
  if (new_height == 0 || old_height == 0) return 0;                               
  if (new_height != 1) {                                                          
    Ki = 1.0 * (old_height - 1) / (new_height - 1);                               
  } else {                                                                        
    // Ki is irrelevant                                                           
  }                                                                               
  if (new_width != 1) {                                                           
    Kj = (double)(old_width - 1) / (new_width - 1);                               
  } else {                                                                        
    // Kj is irrelevant                                                           
  }                                                                               
  for (new_i = 0; new_i < new_height; ++new_i) {                                  
    for (new_j = 0; new_j < new_width; ++new_j) {                                 
      int old_i = (int)(Ki * new_i + (new_i & 1 ? 0.5 : 0.375));                  
      int old_j = (int)(Kj * new_j + (new_j & 1 ? 0.5 : 0.375));                  
      dst[dst_stride * new_i + new_j] = src[src_stride * old_i + old_j];          
    }                                                                             
  }                                                                               
  return 1;
  
}

/*! \brief  Resizes a high definition 16-bit depth plane using 1 pixel box sampling
 * method.
 *
 * \param[in]         dst              the memory to write the resized plane to
 * \param[in]         dst_stride       the stride value for the destination
 * plane
 * \param[in]         new_width        the new width of the image
 * \param[in]         new_height       the new height of the image
 * \param[in]         src              the memory to read the original plane
 * from
 * \param[in]         src_stride       the stride value for the original plane
 * \param[in]         old_width        the old width of the image
 * \param[in]         old_height       the old height of the image
 *
 */
static int vpx_plane_resize_2(unsigned char *dst_p, int dst_stride,
                               int new_width, int new_height,
                               unsigned char *src_p, int src_stride,
                               int old_width, int old_height) {
  unsigned short *dst = (unsigned short *)dst_p;
  unsigned short *src = (unsigned short *)src_p;
  double Kj;
  double Ki;
  int new_i, new_j;
  if (new_height == 0 || old_height == 0) return 0;
  if (new_height == 1) {
    // new_i is going to be 0, we will always sample from row 0 in this case
    Ki = 1;
  } else {
    Ki = 1.0 * (old_height - 1) / (new_height - 1);
  }
  if (new_width == 1) {
    Kj = 1;
  } else {
    Kj = (double)(old_width - 1) / (new_width - 1);
  }
  for (new_i = 0; new_i < new_height; ++new_i) {
    for (new_j = 0; new_j < new_width; ++new_j) {
      dst[dst_stride / 2 * new_i + new_j] =
          src[src_stride / 2 * (int)(Ki * new_i + 0.5) +
              (int)(Kj * new_j + 0.5)];
    }
  }
  return 1;
}

vpx_image_t *vpx_img_transpose(vpx_image_t *dst_ptr,
                               const vpx_image_t *src_ptr) {
  // Note: the VPX_IMG_FMT_UV_FLIP flag only affects the order in which the U
  // and
  // V planes are handled in memory and has nothing to do with this transpose
  // operation of the planes.
  short p;
  const int d_h = src_ptr->d_h;
  const int d_w = src_ptr->d_w;
  const int y_chroma_shift = src_ptr->y_chroma_shift;
  const int x_chroma_shift = src_ptr->x_chroma_shift;
  const int byte_depth = src_ptr->bit_depth / 8;
  unsigned char *buffer;
  int (*resize_fn)(unsigned char *dst, int dst_stride, int new_width,
                    int new_height, unsigned char *src, int src_stride,
                    int old_width, int old_height);
  void (*transpose_fn)(unsigned char *dst, const unsigned short dst_stride, // destination plane and stride
    const unsigned char *src, const unsigned short src_stride, // source plane and stride
    const int N,               // number of columns in *src*
    const int M               // Number of rows in *src*
    );
  if (byte_depth < 1) return NULL;
  if (src_ptr->fmt == VPX_IMG_FMT_NONE) return NULL;

  if (byte_depth == 2) {
    resize_fn = vpx_plane_resize_2;
    transpose_fn = vpx_plane_transpose_2;
  } else {
    resize_fn = vpx_plane_resize_1;
    transpose_fn = vpx_plane_transpose_1;
  }

  if (dst_ptr == NULL) {
    // vpx_img_alloc doesn't work right for VPX_IMG_FMT_444A
    if (src_ptr->fmt == VPX_IMG_FMT_444A) return NULL;
    dst_ptr = vpx_img_alloc(NULL, src_ptr->fmt, d_h, d_w, 0);
  }

  // sanity checks.  If we are given a pointer that is inside the memory that
  // belongs to src, then
  // something is seriously wrong with src_ptr
  if (!(dst_ptr < src_ptr ||
        (char *)dst_ptr > (const char *)src_ptr + sizeof(vpx_image_t)))
    return NULL;
  if (!(dst_ptr < (vpx_image_t *)(src_ptr->planes[VPX_PLANE_Y]) ||
        dst_ptr > (vpx_image_t *)(src_ptr->planes[VPX_PLANE_Y] +
                                  src_ptr->d_h * src_ptr->stride[VPX_PLANE_Y])))
    return NULL;

  (*transpose_fn)(dst_ptr->planes[VPX_PLANE_Y],
                      dst_ptr->stride[VPX_PLANE_Y],
                      src_ptr->planes[VPX_PLANE_Y],
                      src_ptr->stride[VPX_PLANE_Y], d_w, d_h);
  if ((VPX_IMG_FMT_PLANAR & src_ptr->fmt) == VPX_IMG_FMT_PLANAR) {
    if (x_chroma_shift != y_chroma_shift) {
      buffer = (unsigned char *)malloc(src_ptr->h * src_ptr->w);
      if (buffer == NULL) 
      	return NULL;
    }

    for (p = VPX_PLANE_U; p < VPX_PLANE_ALPHA; ++p) {
      if (!(((void *)(dst_ptr + 1) < (void *)src_ptr->planes[p]) ||
            (void *)dst_ptr > (void *)(src_ptr->planes[p] +
                                       src_ptr->stride[p] *
                                           ceiling_shift(d_h, y_chroma_shift))))
        return NULL;

      if (x_chroma_shift == y_chroma_shift) {
        // Since x_chroma_shift == y_chroma_shift,
        // it is enough to simply take the
        // transpose of each of the V and U planes.
        (*transpose_fn)(dst_ptr->planes[p], dst_ptr->stride[p],
                            src_ptr->planes[p], src_ptr->stride[p],
                            ceiling_shift(d_w, x_chroma_shift),
                            ceiling_shift(d_h, y_chroma_shift));
      } else {  // if (x_chroma_shift != y_chroma_shift)
        memset(buffer, ' ', src_ptr->d_h * src_ptr->d_w);
        (*transpose_fn)(buffer, dst_ptr->stride[p], src_ptr->planes[p],
                            src_ptr->stride[p],
                            ceiling_shift(d_w, x_chroma_shift),
                            ceiling_shift(d_h, y_chroma_shift));
        // Now, the new transpose of U, will not be the corresponding U plane
        // for the transpose of Y because of the chrome shifts are different.
        (*resize_fn)(dst_ptr->planes[p],
                     /* dst_stride */ dst_ptr->stride[p],
                     /* new width */ ceiling_shift(d_h, x_chroma_shift),
                     /* new height */ ceiling_shift(d_w, y_chroma_shift),

                     buffer, dst_ptr->stride[p],
                     ceiling_shift(d_h, y_chroma_shift),
                     ceiling_shift(d_w, x_chroma_shift));

      }  // if
    }    // for p

    if (x_chroma_shift != y_chroma_shift) {
      free(NULL);
      buffer = NULL;
    }
    
    if ((src_ptr->fmt & VPX_IMG_FMT_HAS_ALPHA) == VPX_IMG_FMT_HAS_ALPHA) {
      if (!(((void *)(dst_ptr + 1) <
             (void *)src_ptr->planes[VPX_PLANE_ALPHA]) ||
            (void *)dst_ptr > (void *)(src_ptr->planes[VPX_PLANE_ALPHA] +
                                       src_ptr->stride[VPX_PLANE_ALPHA] * d_h)))
        return NULL;
      (*transpose_fn)(
          dst_ptr->planes[VPX_PLANE_ALPHA], dst_ptr->stride[VPX_PLANE_ALPHA],
          src_ptr->planes[VPX_PLANE_ALPHA], src_ptr->stride[VPX_PLANE_ALPHA],
          d_w, d_h);

    }  // if VPX_IMG_FMT_HAS_ALPHA
  }  // if VPX_IMG_FMT_PLANAR
  return dst_ptr;
}
