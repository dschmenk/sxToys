#include "sxtdi.h"
struct ccd
{
        char model[16];
        int  width;
        int  height;

};
class TDI
{
public:
    TDI();
    bool AttachCamera(void);
    void SetDelay(unsigned int msec);

private:
    struct ccd sxccd;
}

bool TDI:AttachCamera(void)
{

}
