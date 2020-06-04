#ifndef _SXCCD_H_
#include "sxccd.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
int sxProbe(HANDLE hlist[], t_sxccd_params paramlist[], int defmodel);
void sxRelease(HANDLE hlist[], int count);
#ifdef __cplusplus
}
#endif
