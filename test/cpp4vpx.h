#include <iostream>
#include "../vpx/vpx_image.h"
#ifndef CPP4VPX_H
#define CPP4VPX_H
using namespace std;

std::ostream &output_packed_data(std::ostream &out, unsigned char *sp, int bps,
                                 int width);
std::ostream &operator<<(std::ostream &out, const vpx_image_t &img);
bool operator==(const vpx_image_t &a, const vpx_image_t &b);
bool operator!=(const vpx_image_t &a, const vpx_image_t &b);
bool set_pixels(vpx_image_t *iptr, int plane, int row, const char *str_p);
const char *vpx_img_fmt_to_name(const enum vpx_img_fmt f);
#endif
