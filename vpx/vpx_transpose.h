/*
 *  Copyright (c) 2016 Shawn Pringle B.Sc.. All Rights Reserved.
 *
 *  Use of this source code is governed by the BountySource agreement.
 */

#include <vpx/vpx_image.h>
#ifndef VPX_TRANSPOSE_H
#define VPX_TRANSPOSE_H
#ifdef __cplusplus
extern "C" {
#endif
/*! \brief shift left, take the ceiling.
 *         Find the smallest x, such that x << shift_amount >= size.
 *
 * \param[in]           size    the value to shift
 * \param[in]   shift_amount    the shift value
 *
 * \return The value returned is equivalent to ceil( size * 1.0 / pow(2,
 * shift_amount) )
 */
inline static unsigned int ceiling_shift(unsigned int size,
                                         unsigned int shift_amount) {
  return (size + (1 << shift_amount) - 1) >> shift_amount;
}

/*!\brief Returns the transpose of the second parameter
 *
 * \param[in]         dst_ptr     the memory to write the transpose to
 * \param[in]         src_ptr     the original image
 *
 * \return Takes a pointer to a vpx_image and returns a new vpx_image  which is
 *         the transpose of the passed image if dst_src is NULL.  The transpose
 *         is written into dst_src and dst_src is returned if it is not NULL.
 *
 *         In the case of the dst_src being NULL, the returned image must be
 * freed by
 *         the caller with vpx_img_free().
 *
 *         Whether the destination matrix is passed to the routine or not, the
 * routine returns
 *         NULL if the memory regions of these two images overlap.
 *
 *
 * NOTE: If vpx_img_flip() means the checks done here may not detect problems
 */
vpx_image_t *vpx_img_transpose(vpx_image_t *dst_ptr,
                               const vpx_image_t *src_ptr);
#ifdef __cplusplus
}
#endif
#endif
