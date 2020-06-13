#include "sxccd.h"
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Camera utility functions
 */
int sxProbe(HANDLE hlist[], t_sxccd_params paramlist[], int defmodel)
{
    if (!defmodel) defmodel = SXCCD_MX5;
    int count = sxOpen(hlist);
    for (int i = 0 ; i < count; i++)
    {
#ifndef _MSC_VER
        if (sxGetCameraModel(hlist[i]) == 0xFFFF)
        {
            sxSetCameraModel(hlist[i], defmodel);
        }
#endif
        sxGetCameraParams(hlist[i], SXCCD_IMAGE_HEAD, &paramlist[i]);
    }
    return count;
}
void sxRelease(HANDLE hlist[], int count)
{
    while (count--)
        sxClose(hlist[count]);
}
#ifdef __cplusplus
}
#endif
