#pragma once
int scolorq_wrapper(uint8_t*in255, uint8_t*out, uint8_t user_pal[3][256], uint32_t width, uint32_t height, unsigned num_colors, double dithering_level = -1.0, uint8_t filter_size = 3);
