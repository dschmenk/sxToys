/***************************************************************************\

    Copyright (c) 2001-2020 David Schmenk

    All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, and/or sell copies of the Software, and to permit persons
    to whom the Software is furnished to do so, provided that the above
    copyright notice(s) and this permission notice appear in all copies of
    the Software and that both the above copyright notice(s) and this
    permission notice appear in supporting documentation.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
    OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
    INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
    FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
    NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
    WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

    Except as contained in this notice, the name of a copyright holder
    shall not be used in advertising or otherwise to promote the sale, use
    or other dealings in this Software without prior written authorization
    of the copyright holder.

\***************************************************************************/
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/choicdlg.h>
#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/filedlg.h>
#include <wx/cmdline.h>
#ifdef _MSC_VER
#include "wintime.h"
#else
#include <sys/time.h>
#endif
#include "sxtdi.h"
#define ALIGN_EXP       100
#define MAX_WHITE       65535
#define INC_BLACK       1024
#define MIN_BLACK       0
#define MAX_BLACK       32768
#define PIX_BITWIDTH    16
#define MIN_PIX         0
#define MAX_PIX         ((1<<PIX_BITWIDTH)-1)
#define LUT_BITWIDTH    10
#define LUT_SIZE        (1<<LUT_BITWIDTH)
#define LUT_INDEX(i)    ((i)>>(PIX_BITWIDTH-LUT_BITWIDTH))
#define max(a,b)        ((a)>=(b)?(a):(b))
int ccdModel = SXCCD_MX5;
/*
 * Camera Model Overrired for generic USB/USB2 interface
 */
wxString FixedChoices[] = {wxT("HX-5"),
                           wxT("HX-9"),
                           wxT("MX-5"),
                           wxT("MX-5C"),
                           wxT("MX-7"),
                           wxT("MX-7C"),
                           wxT("MX-9")};
int FixedModels[] = {SXCCD_HX5,
                     SXCCD_HX9,
                     SXCCD_MX5,
                     SXCCD_MX5C,
                     SXCCD_MX7,
                     SXCCD_MX7C,
                     SXCCD_MX9};
/*
 * Initial values
 */
int      camUSBType      = 0;
double   initialRate     = 0.0;
long     initialDuration = 0;
wxString initialFileName = wxT("Untitled.fits");
long     initialBinX     = 1;
long     initialBinY     = 1;
long     initialCamIndex = 0;
bool     autonomous      = false;
/*
 * 16 bit image sample to RGB pixel LUT
 */
uint8_t redLUT[LUT_SIZE];
uint8_t blugrnLUT[LUT_SIZE];
static void calcRamp(int black, int white, float gamma, bool filter)
{
    int pix, offset;
    float scale, recipg, pixClamp;

    offset = LUT_INDEX(black) - 1;
    scale  = (float)MAX_PIX/(white - black);
    recipg = 1.0/gamma;
    for (pix = 0; pix < LUT_SIZE; pix++)
    {
        pixClamp = ((float)(pix - offset)/(LUT_SIZE-1)) * scale;
        if (pixClamp > 1.0) pixClamp = 1.0;
        else if (pixClamp < 0.0) pixClamp = 0.0;
        redLUT[pix]    = 255.0 * pow(pixClamp, recipg);
        blugrnLUT[pix] = filter ? 0 : redLUT[pix];
    }
}
/*
 * Star registration
 */
static void calcCentroid(int width, int height, uint16_t *pixels, int x, int y, int x_radius, int y_radius, float *x_centroid, float *y_centroid, int min)
{
    int   i, j;
    float sum;

    *x_centroid = 0.0;
    *y_centroid = 0.0;
    sum         = 0.0;

    for (j = y - y_radius; j <= y + y_radius; j++)
        for (i = x - x_radius; i <= x + x_radius; i++)
            if (pixels[j * width + i] > min)
            {
                *x_centroid += i * pixels[j * width + i];
                *y_centroid += j * pixels[j * width + i];
                sum         +=     pixels[j * width + i];
            }
    if (sum != 0.0)
    {
        *x_centroid /= sum;
        *y_centroid /= sum;
    }
}
static bool findBestCentroid(int width, int height, uint16_t *pixels, float *x_centroid, float *y_centroid, int x_range, int y_range, int *x_max_radius, int *y_max_radius, float sigs)
{
    int   i, j, x, y, x_min, x_max, y_min, y_max, x_radius, y_radius;
    int   pixel, pixel_min, pixel_max;
    float pixel_ave, pixel_sig;

    x           = (int)*x_centroid;
    y           = (int)*y_centroid;
    x_min       = x - x_range;
    x_max       = x + x_range;
    y_min       = y - y_range;
    y_max       = y + y_range;
    x           = -1;
    y           = -1;
    x_radius    = 0;
    y_radius    = 0;
    pixel_ave   = 0.0;
    pixel_sig   = 0.0;
    if (x_min < *x_max_radius)        x_min = *x_max_radius;
    if (x_max > width-*x_max_radius)  x_max = width-*x_max_radius;
    if (y_min < *y_max_radius)        y_min = *y_max_radius;
    if (y_max > height-*y_max_radius) y_max = height-*y_max_radius;
    for (j = y_min; j < y_max; j++)
        for (i = x_min; i < x_max; i++)
            pixel_ave += pixels[j * width + i];
    pixel_ave /= (y_max - y_min) * (x_max - x_min);
    for (j = y_min; j < y_max; j++)
        for (i = x_min; i < x_max; i++)
            pixel_sig += (pixel_ave - pixels[j * width + i]) * (pixel_ave - pixels[j * width + i]);
    pixel_sig = sqrt(pixel_sig / ((y_max - y_min) * (x_max - x_min) - 1));
    pixel_max = pixel_min = pixel_ave + pixel_sig * sigs;
    for (j = y_min; j < y_max; j++)
    {
        for (i = x_min; i < x_max; i++)
        {
            if (pixels[j * width + i] > pixel_max)
            {
                pixel = pixels[j * width + i];
                /*
                 * Local maxima
                 */
                if ((pixel >= pixels[j * width + i + 1])
                 && (pixel >= pixels[j * width + i - 1])
                 && (pixel >= pixels[(j + 1) * width + i])
                 && (pixel >= pixels[(j - 1) * width + i]))
                {
                    /*
                     * Avoid hot pixels
                     */
                     if ((pixel_min < pixels[j * width + i + 1])
                      && (pixel_min < pixels[j * width + i - 1])
                      && (pixel_min < pixels[(j + 1) * width + i])
                      && (pixel_min < pixels[(j - 1) * width + i]))
                    {
                        /*
                         * Find radius of highlight
                         */
                        for (y_radius = 1; (y_radius <= *y_max_radius)
                                        && ((pixels[(j + y_radius) * width + i] > pixel_min)
                                         || (pixels[(j - y_radius) * width + i] > pixel_min)); y_radius++);
                        for (x_radius = 1; (x_radius <= *x_max_radius)
                                        && ((pixels[j * width + i + x_radius] > pixel_min)
                                         || (pixels[j * width + i - x_radius] > pixel_min)); x_radius++);
                        /*
                         * If its really big, skip it.  Something like the moon
                         */
                        if (x_radius < *x_max_radius && y_radius < *y_max_radius)
                        {
                            pixel_max = pixel;
                            calcCentroid(width, height, pixels, i, j, x_radius, y_radius, x_centroid, y_centroid, pixel_min);
                            x = (int)(*x_centroid + 0.5);
                            y = (int)(*y_centroid + 0.5);
                            calcCentroid(width, height, pixels, x, y, x_radius, y_radius, x_centroid, y_centroid, pixel_min);
                        }
                    }
                }
            }
        }
    }
    if  (x >= 0 && y >= 0)
    {
        *x_max_radius = x_radius;
        *y_max_radius = y_radius;
    }
    return x >= 0 && y >= 0;
}
/*
 * Bin choices
 */
wxString BinChoices[] = {wxT("1x"), wxT("2x"), wxT("4x")};
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
        if (sxGetCameraModel(hlist[i]) == 0)
        {
            printf("Setting camera model to %02X\n", defmodel);
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
/*
 * TDI Scan App class
 */
class ScanApp : public wxApp
{
public:
    virtual void OnInitCmdLine(wxCmdLineParser &parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
    virtual bool OnInit();
};
class ScanFrame : public wxFrame
{
public:
    ScanFrame();
    void StartTDI();
private:
    HANDLE         camHandles[SXCCD_MAX_CAMS];
    t_sxccd_params camParams[SXCCD_MAX_CAMS];
    int            camSelect, camCount;
    wxString       tdiFilePath;
    wxString       tdiFileName;
    bool           tdiFileSaved;
    int            tdiState;
    unsigned int   ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int   ccdBinWidth, ccdBinHeight, ccdBinX, ccdBinY, ccdPixelCount;
    float          ccdPixelWidth, ccdPixelHeight;
    uint16_t      *ccdFrame;
    uint16_t      *tdiFrame;
    float          pixelGamma;
    bool           pixelFilter;
    int            tdiLength, tdiRow, numFrames;
    int            tdiExposure, tdiMinutes;
    float          tdiScanRate;
    float          trackStarInitialX, trackStarInitialY, trackStarX, trackStarY;
    wxImage       *scanImage;
    wxTimer        tdiTimer;
    void DoAlign();
    void DoTDI();
    void GetDuration();
    bool ConnectCamera(int index);
    void OnConnect(wxCommandEvent& event);
    void OnOverride(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAlign(wxCommandEvent& event);
    void OnScan(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnDuration(wxCommandEvent& event);
    void OnRate(wxCommandEvent& event);
    void OnBinX(wxCommandEvent& event);
    void OnBinY(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    wxDECLARE_EVENT_TABLE();
};
enum
{
    ID_TIMER = 1,
    ID_CONNECT,
    ID_OVERRIDE,
    ID_ALIGN,
    ID_SCAN,
    ID_STOP,
    ID_FILTER,
    ID_DURATION,
    ID_RATE,
    ID_BINX,
    ID_BINY,
};
enum
{
    STATE_IDLE = 0,
    STATE_ALIGNING,
    STATE_SCANNING,
    STATE_STOPPING
};
wxBEGIN_EVENT_TABLE(ScanFrame, wxFrame)
    EVT_TIMER(ID_TIMER,     ScanFrame::OnTimer)
    EVT_MENU(ID_CONNECT,    ScanFrame::OnConnect)
    EVT_MENU(ID_OVERRIDE,   ScanFrame::OnOverride)
    EVT_MENU(ID_ALIGN,      ScanFrame::OnAlign)
    EVT_MENU(ID_SCAN,       ScanFrame::OnScan)
    EVT_MENU(ID_STOP,       ScanFrame::OnStop)
    EVT_MENU(ID_FILTER,     ScanFrame::OnFilter)
    EVT_MENU(ID_DURATION,   ScanFrame::OnDuration)
    EVT_MENU(ID_RATE,       ScanFrame::OnRate)
    EVT_MENU(ID_BINX,       ScanFrame::OnBinX)
    EVT_MENU(ID_BINY,       ScanFrame::OnBinY)
    EVT_MENU(wxID_NEW,      ScanFrame::OnNew)
    EVT_MENU(wxID_SAVE,     ScanFrame::OnSave)
    EVT_MENU(wxID_ABOUT,    ScanFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     ScanFrame::OnExit)
    EVT_ERASE_BACKGROUND(   ScanFrame::OnBackground)
    EVT_PAINT(              ScanFrame::OnPaint)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(ScanApp);
void ScanApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddOption(wxT("m"), wxT("model"), wxT("USB camera model override"), wxCMD_LINE_VAL_STRING);
    parser.AddOption(wxT("c"), wxT("camera"), wxT("camera index"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("r"), wxT("rate"), wxT("scan rate (rows/sec)"), wxCMD_LINE_VAL_DOUBLE);
    parser.AddOption(wxT("d"), wxT("duration"), wxT("scan duration in hours"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("x"), wxT("xbin"), wxT("x bin (1, 2, 4)"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("y"), wxT("ybin"), wxT("y bin (1, 2, 4)"), wxCMD_LINE_VAL_NUMBER);
    parser.AddSwitch(wxT("a"), wxT("auto"), wxT("autonomous mode"));
    parser.AddParam(wxT("FITS filename"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}
bool ScanApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
    wxString *modelString = new wxString(' ', 10);
    if (parser.Found(wxT("m"), modelString))
    {
        //
        // Validate ccdModel
        //
        if (toupper(modelString->GetChar(1)) == 'X')
        {
            switch (toupper(modelString->GetChar(0)))
            {
                case 'H':
                    camUSBType &= ~SXCCD_INTERLEAVE;
                    break;
                case 'M':
                    camUSBType |= SXCCD_INTERLEAVE;
                    break;
                default:
                    printf("Invalid model type designation.\n");
            }
            switch ((char)modelString->GetChar(2))
            {
                case '5':
                case '7':
                case '9':
                    camUSBType = (camUSBType & 0xC0) | (modelString->GetChar(2) - '0');
                    break;
                default:
                    printf("Invalid model number designation.\n");
            }
            if (toupper(modelString->GetChar(3)) == 'C')
                camUSBType |= SXCCD_COLOR;
            else
                camUSBType &= ~SXCCD_COLOR;
        }
        else
            printf("Invalid SX designation.\n");
        printf("USB SX model type: 0x%02X\n", camUSBType);
    }
    if (parser.Found(wxT("c"), &initialCamIndex))
    {}
    if (parser.Found(wxT("r"), &initialRate))
    {}
    if (parser.Found(wxT("d"), &initialDuration))
    {}
    if (parser.Found(wxT("x"), &initialBinX))
    {
        switch (initialBinX)
        {
            case 1:
            case 2:
            case 4:
                break;
            default:
                initialBinX = 1;
        }
    }
    if (parser.Found(wxT("y"), &initialBinY))
    {
        switch (initialBinY)
        {
            case 1:
            case 2:
            case 4:
                break;
            default:
                initialBinY = 1;
        }
    }
    if (parser.Found(wxT("a")))
        autonomous = (initialRate > 0.0 && initialDuration > 0);
    if (parser.GetParamCount() > 0)
        initialFileName = parser.GetParam(0);
    return wxApp::OnCmdLineParsed(parser);
}
bool ScanApp::OnInit()
{
    if (wxApp::OnInit())
    {
        ScanFrame *frame = new ScanFrame();
        if (autonomous && ccdModel)
            //
            // In autonomous mode, skip Show() to reduce processing overhead of image display
            //
            frame->StartTDI();
        else
            frame->Show(true);
        return true;
    }
    return false;
}
ScanFrame::ScanFrame() : wxFrame(NULL, wxID_ANY, wxT("SX TDI")), tdiTimer(this, ID_TIMER)
{
    CreateStatusBar(3);
    wxMenu *menuCamera = new wxMenu;
    menuCamera->Append(ID_CONNECT, "&Connnect Camera...");
#ifndef _MSC_VER
    menuCamera->Append(ID_OVERRIDE, "&Override Camera...");
#endif
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_NEW, "&New\tCtrl-N");
    menuCamera->Append(wxID_SAVE, "&Save...\tCtrl-S");
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_EXIT);
    wxMenu *menuScan = new wxMenu;
    menuScan->Append(ID_ALIGN, "&Align\tA");
    menuScan->Append(ID_SCAN, "&TDI Scan\tT");
    menuScan->Append(ID_STOP, "S&top\tCtrl-T");
    menuScan->AppendSeparator();
    menuScan->AppendCheckItem(ID_FILTER, wxT("&Red Filter\tR"));
    menuScan->Append(ID_DURATION, "Scan &Duration...\tD");
    menuScan->Append(ID_RATE, "Scan &Rate...");
    menuScan->Append(ID_BINX, "&X Binning...");
    menuScan->Append(ID_BINY, "&Y Binning...");
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuCamera, "&Camera");
    menuBar->Append(menuScan, "&Scan");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    tdiFilePath = wxGetCwd();
    tdiFileName = initialFileName;
    tdiFrame    = NULL;
    tdiState    = STATE_IDLE;
    tdiMinutes  = initialDuration * 60;
    ccdBinX     = initialBinX;
    ccdBinY     = initialBinY;
    tdiScanRate = initialRate;
    tdiExposure = tdiScanRate > 0.0 ? 1000.0 / tdiScanRate : 0;
    pixelGamma  = 1.0;
    pixelFilter = false;
    ccdFrame    = NULL;
    scanImage   = NULL;
    camCount    = sxProbe(camHandles, camParams, camUSBType);
    ConnectCamera(initialCamIndex);
}
bool ScanFrame::ConnectCamera(int index)
{
    char statusText[40];
    if (ccdFrame)
        free(ccdFrame);
    if (camCount)
    {
        if (index >= camCount)
            index = camCount - 1;
        camSelect      = index;
        ccdModel       = sxGetCameraModel(camHandles[camSelect]);
        ccdFrameWidth  = camParams[camSelect].width;
        ccdFrameHeight = camParams[camSelect].height;
        ccdPixelWidth  = camParams[camSelect].pix_width;
        ccdPixelHeight = camParams[camSelect].pix_height;
        ccdPixelCount  = FRAMEBUF_COUNT(ccdFrameWidth, ccdFrameHeight, 1, 1);
        ccdFrame = (uint16_t *)malloc(sizeof(uint16_t) * ccdPixelCount);
        sprintf(statusText, "Attached: %cX-%d[%d]", ccdModel & SXCCD_INTERLEAVE ? 'M' : 'H', ccdModel & 0x3F, camSelect);
    }
    else
    {
        camSelect       = -1;
        ccdModel        = 0;
        ccdFrameWidth   = ccdFrameHeight = 512;
        ccdFrameDepth   = 16;
        ccdPixelWidth   = ccdPixelHeight = 1;
        ccdPixelCount   = 0;
        ccdFrame        = NULL;
        autonomous      = false;
        strcpy(statusText, "Attached: None");
    }
    if (!IsMaximized())
    {
        int winHeight = ccdFrameWidth; // Swap width/height
        int winWidth  = ccdFrameHeight * ccdPixelHeight / ccdPixelWidth;
        while (winHeight > 720) // Constrain initial size to something reasonable
        {
            winWidth  >>= 1;
            winHeight >>= 1;
        }
        SetClientSize(winWidth, winHeight);
    }
    SetStatusText(statusText, 0);
    if (tdiExposure > 0)
        sprintf(statusText, "Rate: %2.3f row/s", tdiScanRate);
    else
        sprintf(statusText, "Rate: -.-- row/s");
    SetStatusText(statusText, 1);
    sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
    SetStatusText(statusText, 2);
    return camSelect >= 0;
}
void ScanFrame::OnConnect(wxCommandEvent& event)
{
 	if (tdiState == STATE_IDLE)
	{
		if (camCount)   sxRelease(camHandles, camCount);
		if ((camCount = sxProbe(camHandles, camParams, camUSBType)) == 0)
		{
			wxMessageBox("No Cameras Found", "Connect Error", wxOK | wxICON_INFORMATION);
			return;
		}
		wxString CamChoices[8/*camCount*/];
		for (int i = 0; i < camCount; i++)
		{
			int model     = sxGetCameraModel(camHandles[i]);
			CamChoices[i] = wxString::Format("%cX-%d", model & SXCCD_INTERLEAVE ? 'M' : 'H', model & 0x3F);
		}
		wxSingleChoiceDialog dlg(this,
							  wxT("Camera:"),
							  wxT("Connect Camera"),
							  camCount,
							  CamChoices);
		if (dlg.ShowModal() == wxID_OK )
		{
			tdiExposure = 0;
			tdiScanRate = 0.0;
			ConnectCamera(dlg.GetSelection());
		}
	}
}
void ScanFrame::OnOverride(wxCommandEvent& event)
{
#ifndef _MSC_VER
 	if (tdiState == STATE_IDLE)
	{
		if (tdiTimer.IsRunning())
			tdiTimer.Stop();
		if (camCount)   sxRelease(camHandles, camCount);
		if ((camCount = sxProbe(camHandles, camParams, camUSBType)) == 0)
		{
			wxMessageBox("No Cameras Found", "Connect Error", wxOK | wxICON_INFORMATION);
			return;
		}
		if (camSelect < 0)
			camSelect = camCount - 1;
		wxSingleChoiceDialog dlg(this,
							  wxT("Camera:"),
							  wxT("Override Camera Model"),
							  7,
							  FixedChoices);
		if (dlg.ShowModal() == wxID_OK )
		{
			camUSBType = FixedModels[dlg.GetSelection()];
			sxSetCameraModel(camHandles[camSelect], camUSBType);
			ConnectCamera(camSelect);
		}
	}
#endif
}
void ScanFrame::OnBackground(wxEraseEvent& event)
{
}
void ScanFrame::OnPaint(wxPaintEvent& event)
{
    int winWidth, winHeight;
    GetClientSize(&winWidth, &winHeight);
    if (winWidth > 0 && winHeight > 0)
    {
        wxClientDC dc(this);
        if (scanImage)
        {
            wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
            dc.DrawBitmap(bitmap, 0, 0);
        }
        else
        {
            dc.SetBrush(wxBrush(*wxWHITE));
            dc.DrawRectangle(0, 0, winWidth, winHeight);
        }
    }
}
void ScanFrame::DoAlign()
{
    static struct timeval trackInitialTime;
    struct timeval trackFrameTime;
    int trackTime;
    int xRadius, yRadius;
    gettimeofday(&trackFrameTime, NULL);
    sxLatchPixels(camHandles[camSelect], // cam handle
                  SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                  SXCCD_IMAGE_HEAD, // main ccd
                  0, // xoffset
                  0, // yoffset
                  ccdFrameWidth, // width
                  ccdFrameHeight, // height
                  1, // xbin
                  1); // ybin
    sxReadPixels(camHandles[camSelect], // cam handle
                 ccdFrame, //pixbuf
                 ccdPixelCount); // pix count
    sxClearPixels(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    tdiTimer.StartOnce(ALIGN_EXP);
    if (numFrames == 0)
    {
        //
        // If first frame, identify best candidate for measuring scan rate
        //
        trackStarInitialX = ccdFrameWidth/2;
        trackStarInitialY = 0.0;
        xRadius = yRadius = 15;
        if (findBestCentroid(ccdFrameWidth, ccdFrameHeight, ccdFrame, &trackStarInitialX, &trackStarInitialY, ccdFrameWidth/4, ccdFrameHeight - ccdFrameHeight/4, &xRadius, &yRadius, 1.0))
        {
            trackInitialTime = trackFrameTime;
            trackStarX = trackStarInitialX;
            trackStarY = trackStarInitialY;
            numFrames  = 1;
        }
    }
    else
    {
        //
        // Track star for rate measurement
        //
        xRadius = 45;
        yRadius = 15;
        if ((trackStarY > tdiScanRate * ALIGN_EXP/1000)
          && findBestCentroid(ccdFrameWidth, ccdFrameHeight, ccdFrame, &trackStarX, &trackStarY, 5, ccdFrameHeight, &xRadius, &yRadius, 1.0))
        {
            if (trackStarInitialY > trackStarY)
            {
                trackTime   = (trackFrameTime.tv_sec  - trackInitialTime.tv_sec)  * 1000;
                trackTime  += (trackFrameTime.tv_usec - trackInitialTime.tv_usec) / 1000;
                tdiExposure = trackTime / (trackStarInitialY - trackStarY);
                tdiScanRate = 1000.0 / tdiExposure;
                numFrames++;
            }
        }
        else
        {
            wxCommandEvent event; // Bogus event to make compiler happy
            OnStop(event);
            wxMessageBox("Tracking Star Lost", "SX TDI Alignment", wxOK | wxICON_INFORMATION);
        }
    }
    int winWidth, winHeight;
    GetClientSize(&winWidth, &winHeight);
    if (winWidth > 0 && winHeight > 0)
    {
        int pixelMax = MIN_PIX;
        int pixelMin = MAX_PIX;
        unsigned char *rgb = scanImage->GetData();
        for (int y = 0; y < ccdFrameWidth; y++) // Rotate image 90 degrees counterclockwise as it gets copied
        {
            uint16_t *m16 = &(ccdFrame[ccdPixelCount - y - 1]);
            for (int x = 0; x < ccdFrameHeight; x++)
            {
                if (*m16 > pixelMax) pixelMax = *m16;
                if (*m16 < pixelMin) pixelMin = *m16;
                rgb[0] = max(rgb[0], redLUT[LUT_INDEX(*m16)]);
                rgb[1] = max(rgb[1], blugrnLUT[LUT_INDEX(*m16)]);
                rgb[2] = max(rgb[2], blugrnLUT[LUT_INDEX(*m16)]);
                rgb   += 3;
                m16   -= ccdFrameWidth;
            }
        }
        calcRamp(pixelMin, pixelMax, pixelGamma, pixelFilter);
        wxClientDC dc(this);
        wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
        dc.DrawBitmap(bitmap, 0, 0);
        char statusText[40];
        sprintf(statusText, "Track: %4.3f,%4.3f", trackStarX, trackStarY);
        SetStatusText(statusText, 2);
        sprintf(statusText, "Rate: %2.3f row/s", tdiScanRate);
        SetStatusText(statusText, 1);
    }
}
void ScanFrame::DoTDI()
{
    uint16_t *ccdRow = &tdiFrame[tdiRow * ccdBinWidth];
    sxLatchPixels(camHandles[camSelect], // cam handle
                  SXCCD_EXP_FLAGS_TDI |
                  SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                  SXCCD_IMAGE_HEAD, // main ccd
                 0, // xoffset
                 0, // yoffset
                 ccdFrameWidth, // width
                 ccdBinY, // height
                 ccdBinX, // xbin
                 ccdBinY); // ybin
    sxReadPixels(camHandles[camSelect], // cam handle
                 ccdRow, //pixbuf
                 ccdBinWidth); // pix count
    int winWidth, winHeight;
    GetClientSize(&winWidth, &winHeight);
    if (winWidth > 0 && winHeight > 0)
    {
        int pixelMax       = MIN_PIX;
        int pixelMin       = MAX_PIX;
        unsigned char *rgb = scanImage->GetData();
        uint16_t *pixels   = (tdiRow < ccdBinHeight) ? &tdiFrame[ccdBinWidth * (ccdBinHeight - 1)] : ccdRow;
        for (int y = 0; y < ccdBinWidth; y++) // Rotate image 90 degrees counterclockwise as it gets copied
        {
            uint16_t *m16 = &pixels[ccdBinWidth - y - 1];
            for (int x = 0; x < ccdBinHeight; x++)
            {
                if (*m16 > pixelMax) pixelMax = *m16;
                if (*m16 < pixelMin) pixelMin = *m16;
                rgb[0] = redLUT[LUT_INDEX(*m16)];
                rgb[1] = blugrnLUT[LUT_INDEX(*m16)];
                rgb[2] = blugrnLUT[LUT_INDEX(*m16)];
                rgb   += 3;
                m16   -= ccdBinWidth;
            }
        }
        calcRamp(pixelMin, pixelMax, pixelGamma, pixelFilter);
        wxClientDC dc(this);
        wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
        dc.DrawBitmap(bitmap, 0, 0);
    }
    if (++tdiRow == tdiLength)
    {
        tdiTimer.Stop();
        tdiState = STATE_IDLE;
        SetTitle(wxT("SX TDI"));
        if (autonomous)
        {
            char filename[255];
            char creator[] = "sxTDI";
            char camera[]  = "StarLight Xpress Camera";
            strcpy(filename, tdiFileName.c_str());
            tdiFileSaved = fitsWrite(filename, (unsigned char *)tdiFrame, ccdBinWidth, tdiLength, tdiExposure, creator, camera) >= 0;
            delete scanImage;
            scanImage = NULL;
            Close(true);
        }
    }
}
void ScanFrame::OnTimer(wxTimerEvent& event)
{
    if (tdiState == STATE_SCANNING)
        DoTDI();
    else
        DoAlign();
}
void ScanFrame::GetDuration()
{
 	if (tdiState == STATE_IDLE)
	{
		wxNumberEntryDialog dlg(this,
								wxT(""),
								wxT("Hours:"),
								wxT("Scan Duration"),
								6, 1, 12);
		if (dlg.ShowModal() != wxID_OK)
			return;
		tdiMinutes = dlg.GetValue() * 60;
	}
}
void ScanFrame::OnDuration(wxCommandEvent& event)
{
    if (tdiState == STATE_IDLE)
        GetDuration();
}
void ScanFrame::OnRate(wxCommandEvent& event)
{
    char rateText[40] = "";
    if (tdiState == STATE_IDLE)
    {
        if (tdiScanRate != 0.0)
            sprintf(rateText, "%2.3f", tdiScanRate);
        wxTextEntryDialog dlg(this,
                              wxT("Rows/sec:"),
                              wxT("Scan Rate"),
                              rateText);
        dlg.SetTextValidator(wxFILTER_NUMERIC);
        dlg.SetMaxLength(6);
        if (dlg.ShowModal() == wxID_OK )
        {
            wxString value = dlg.GetValue();
            tdiScanRate    = atof(value);
            tdiExposure    = 1000.0 / tdiScanRate;
            sprintf(rateText, "Rate: %2.3f row/s", tdiScanRate);
            SetStatusText(rateText, 1);
        }
    }
}
void ScanFrame::OnBinX(wxCommandEvent& event)
{
    if (tdiState == STATE_IDLE)
    {
        wxSingleChoiceDialog dlg(this,
                              wxT("X Bin:"),
                              wxT("X Binning"),
                              3,
                              BinChoices);
        if (dlg.ShowModal() == wxID_OK )
        {
            char binText[10];
            ccdBinX =  1 << dlg.GetSelection();
            sprintf(binText, "Bin: %d:%d", ccdBinX, ccdBinY);
            SetStatusText(binText, 2);
        }
    }
}
void ScanFrame::OnBinY(wxCommandEvent& event)
{
    if (tdiState == STATE_IDLE)
    {
        wxSingleChoiceDialog dlg(this,
                              wxT("Y Bin:"),
                              wxT("Y Binning"),
                              3,
                              BinChoices);
        if (dlg.ShowModal() == wxID_OK )
        {
            char binText[10];
            ccdBinY =  1 << dlg.GetSelection();
            sprintf(binText, "Bin: %d:%d", ccdBinX, ccdBinY);
            SetStatusText(binText, 2);
        }
    }
}
void ScanFrame::StartTDI()
{
    int binExposure;
    if (tdiState == STATE_ALIGNING)
    {
        tdiTimer.Stop();
        tdiState = STATE_IDLE;
    }
    if (tdiState == STATE_IDLE && ccdModel)
    {
        if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Overwrite unsaved image?", "Scan Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
            return;
        if (tdiExposure == 0)
        {
            wxMessageBox("Align & Measure Rate first", "Start TDI Error", wxOK | wxICON_INFORMATION);
            return;
        }
        if (tdiMinutes == 0)
        {
            GetDuration();
            if (tdiMinutes == 0)
                return;
        }
        if (scanImage)
            delete scanImage;
        ccdBinWidth  = ccdFrameWidth  / ccdBinX;
        ccdBinHeight = ccdFrameHeight / ccdBinY;
        scanImage    = new wxImage(ccdBinHeight, ccdBinWidth);
        binExposure  = tdiExposure * ccdBinY;
        tdiLength    = tdiMinutes * 60000 / binExposure;
        if (tdiLength < ccdBinHeight)
            tdiLength = ccdBinHeight;
        tdiFrame  = (uint16_t *)malloc(sizeof(uint16_t) * tdiLength * ccdBinWidth);
        memset(tdiFrame, 0, sizeof(uint16_t) * tdiLength * ccdBinWidth);
        sxClearPixels(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        tdiTimer.Start(binExposure);
        tdiFileSaved = false;
        tdiRow       = 0;
        tdiState     = STATE_SCANNING;
    }
}
void ScanFrame::OnAlign(wxCommandEvent& event)
{
    if (tdiState == STATE_ALIGNING)
    {
        tdiTimer.Stop();
        tdiState = STATE_IDLE;
    }
    if (tdiState == STATE_IDLE && ccdModel)
    {
        sxClearPixels(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        tdiTimer.StartOnce(ALIGN_EXP);
        if (scanImage)
            delete scanImage;
        scanImage = new wxImage(ccdFrameHeight, ccdFrameWidth);
        memset(scanImage->GetData(), 0, ccdPixelCount * 3);
        for (int y = 0; y < ccdFrameWidth; y += ccdFrameWidth/32)
        {
            unsigned char *rgb = scanImage->GetData() + y * ccdFrameHeight * 3;
            for (int x = 0; x < ccdFrameHeight; x++)
            {
                rgb[1] = 128;
                rgb   += 3;
            }
        }
        int winWidth, winHeight;
        GetClientSize(&winWidth, &winHeight);
        if (winWidth > 0 && winHeight > 0)
        {
            wxClientDC dc(this);
            wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
            dc.DrawBitmap(bitmap, 0, 0);
        }
        tdiScanRate = 0.0;
        numFrames   = 0;
        tdiState    = STATE_ALIGNING;
        SetTitle(wxT("SX TDI [Aligning]"));
    }
}
void ScanFrame::OnScan(wxCommandEvent& event)
{
    if (tdiState == STATE_IDLE && ccdModel)
    {
        StartTDI();
        SetTitle(wxT("SX TDI [Scanning]"));
    }
}
void ScanFrame::OnStop(wxCommandEvent& event)
{
    if (tdiTimer.IsRunning())
    {
        tdiTimer.Stop();
        if (tdiState == STATE_SCANNING)
            tdiLength = tdiRow;
        else
        {
            char statusText[40];
            sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
            SetStatusText(statusText, 2);
        }
        tdiState = STATE_IDLE;
        SetTitle(wxT("SX TDI"));
    }
}
void ScanFrame::OnNew(wxCommandEvent& event)
{
    if (tdiState == STATE_IDLE)
    {
        if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Clear unsaved image?", "New Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
            return;
        free(tdiFrame);
        tdiFrame = NULL;
    }
}
void ScanFrame::OnSave(wxCommandEvent& event)
{
    char filename[255];
    char creator[] = "sxTDI";
    char camera[]  = "StarLight Xpress Camera";
    if (tdiState == STATE_IDLE)
    {
        wxFileDialog dlg(this, wxT("Save Image"), tdiFilePath, tdiFileName, wxT("*.fits"/*"FITS file (*.fits)"*/), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (tdiFrame != NULL)
        {
            if (dlg.ShowModal() == wxID_OK)
            {
                tdiFilePath  = dlg.GetPath();
                tdiFileName  = dlg.GetFilename();
                strcpy(filename, tdiFilePath.c_str());
                printf("Saving to file %s\n", filename);
                tdiFileSaved = fitsWrite(filename, (unsigned char *)tdiFrame, ccdBinWidth, tdiLength, tdiExposure, creator, camera) >= 0;
            }
        }
        else
            wxMessageBox("No image to save", "Save Error", wxOK | wxICON_INFORMATION);
    }
}
void ScanFrame::OnExit(wxCommandEvent& event)
{
    if (tdiState == STATE_SCANNING && wxMessageBox("Cancel scan in progress?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
        return;
    else if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Exit without saving image?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
        return;
    if (scanImage != NULL)
    {
        delete scanImage;
        scanImage = NULL;
    }
    Close(true);
	if (camCount)
	{
		sxRelease(camHandles, camCount);
		camCount = 0;
	}
}
void ScanFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
}
void ScanFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Starlight Xpress Time Delay Integrator\nVersion 0.1\nCopyright (c) 2003-2020, David Schmenk", "About SX TDI", wxOK | wxICON_INFORMATION);
}
