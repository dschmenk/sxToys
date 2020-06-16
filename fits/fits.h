#ifdef __cplusplus
extern "C" {
#endif
int fits_write_key_int(const char *key, int value, const char *comment);
int fits_write_key_float(const char *key, float value, const char *comment);
int fits_write_key_string(const char *key, const char * value, const char *comment);
int fits_write_image(unsigned short *pixels, int width, int height);
int fits_open(const char *filename);
int fits_close(void);
int fits_cleanup(void);
#ifdef __cplusplus
}
#endif
