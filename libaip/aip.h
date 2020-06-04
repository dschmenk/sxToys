#ifdef __cplusplus
extern "C" {
#endif
#define PIX_BITWIDTH        16
#define MIN_PIX             0
#define MAX_PIX             ((1<<PIX_BITWIDTH)-1)
#define LUT_BITWIDTH        10
#define LUT_SIZE            (1<<LUT_BITWIDTH)
#define LUT_INDEX(i)        ((i)>>(PIX_BITWIDTH-LUT_BITWIDTH))
extern unsigned char redLUT[LUT_SIZE];
extern unsigned char blugrnLUT[LUT_SIZE];
void calcRamp(int black, int white, float gamma, int filter);
void calcCentroid(int width, int height, unsigned short *pixels, int x, int y, int x_radius, int y_radius, float *x_centroid, float *y_centroid, int min);
int findBestCentroid(int width, int height, unsigned short *pixels, float *x_centroid, float *y_centroid, int x_range, int y_range, int *x_max_radius, int *y_max_radius, float sigs);
#ifdef __cplusplus
}
#endif
