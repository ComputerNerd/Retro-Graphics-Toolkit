// wu.h
// See wu.c for details
#ifdef __cplusplus
extern "C" {
#endif
int wu_quant(unsigned char *inbuf, int width, int height, int quant_to, uint8_t pal[3][256]);
#ifdef __cplusplus
}
#endif
