/***************************************************************************\

    Copyright (c) 2001-2020 David Schmenk

    All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    copy of this software and associated documentation files (the
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
#include <wx/config.h>
#include "sxtdi.h"
#define TRACK_STAR_RADIUS   200 // Tracking star max radius in microns
#define TRACK_STAR_SIGMA    1.0 // Only track stars 1 sigma over the noise level
#define ALIGN_EXP           500
#define MIN_SCREEN_UPDATE   1000
#define SCAN_OK             ((wxThread::ExitCode)0)
#define SCAN_ERR_TIME       ((wxThread::ExitCode)-1)
#define SCAN_ERR_CAMERA     ((wxThread::ExitCode)-2)
#define MAX_WHITE           MAX_PIX
#define INC_BLACK           1024
#define MIN_BLACK           0
#define MAX_BLACK           MAX_WHITE/2
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
 * Bin choices
 */
wxString BinChoices[] = {wxT("1x"), wxT("2x"), wxT("4x")};
/*
 * Gamma choices
 */
wxString GammaChoices[] = {wxT("1.0"), wxT("1.5"), wxT("2.0")};
float GammaValues[] = {1.0, 1.5, 2.0};
/*
 * CCD Readout Thread
 */
class ScanFrame;
class ScanThread : public wxThread
{
public:
    ScanThread(ScanFrame *param);
protected:
    virtual ExitCode Entry();
private:
    ScanFrame *scan;
};
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
protected:
    friend class ScanThread;
    ScanThread    *tdiThread;
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
    volatile int   tdiLength, tdiRow;
    int            tdiMinutes, numFrames;
    float          tdiScanRate, tdiExposure, binExposure;
private:
    float          trackStarInitialX, trackStarInitialY, trackStarX, trackStarY;
    wxStopWatch   *trackWatch;
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
    void OnGamma(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnClose(wxCloseEvent& event);
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
    ID_GAMMA,
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
    EVT_MENU(ID_GAMMA,      ScanFrame::OnGamma)
    EVT_MENU(wxID_NEW,      ScanFrame::OnNew)
    EVT_MENU(wxID_SAVE,     ScanFrame::OnSave)
    EVT_MENU(wxID_ABOUT,    ScanFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     ScanFrame::OnExit)
    EVT_ERASE_BACKGROUND(   ScanFrame::OnBackground)
    EVT_PAINT(              ScanFrame::OnPaint)
    EVT_CLOSE(              ScanFrame::OnClose)
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
    wxConfig config(wxT("sxTDI"), wxT("sxToys"));
    config.Read(wxT("ScanRate"), &initialRate);
#ifndef _MSC_VER
    config.Read(wxT("USB1Camera"), &camUSBType);
#endif
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
    menuCamera->Append(ID_OVERRIDE, "&Set Camera Model...");
#endif
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_NEW, "&New\tCtrl-N");
    menuCamera->Append(wxID_SAVE, "&Save...\tCtrl-S");
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_EXIT);
    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(ID_FILTER, wxT("&Red Filter\tR"));
    menuView->Append(ID_GAMMA, "&Gamma...");
    wxMenu *menuScan = new wxMenu;
    menuScan->Append(ID_ALIGN, "&Align\tA");
    menuScan->Append(ID_SCAN, "&TDI Scan\tT");
    menuScan->Append(ID_STOP, "S&top\tCtrl-T");
    menuScan->AppendSeparator();
    menuScan->Append(ID_DURATION, "Scan &Duration...\tD");
    menuScan->Append(ID_RATE, "Scan &Rate...");
    menuScan->Append(ID_BINX, "&X Binning...");
    menuScan->Append(ID_BINY, "&Y Binning...");
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuCamera, "&Camera");
    menuBar->Append(menuView, "&View");
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
    tdiExposure = tdiScanRate > 0.0 ? 1000.0 / tdiScanRate : 0.0;
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
        ccdPixelCount  = FRAMEBUF_COUNT(ccdFrameWidth / 2, ccdFrameHeight, 1, 1);
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
        int winWidth  = ccdFrameHeight * 2 * ccdPixelHeight / ccdPixelWidth; // Make the width double the height
        while (winHeight > 720) // Constrain initial size to something reasonable
        {
            winWidth  >>= 1;
            winHeight >>= 1;
        }
        SetClientSize(winWidth, winHeight);
    }
    SetStatusText(statusText, 0);
    if (tdiExposure > 0.0)
        sprintf(statusText, "Rate: %2.3f row/s", tdiScanRate);
    else
        sprintf(statusText, "Rate: -.-- row/s");
    SetStatusText(statusText, 1);
    sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
    SetStatusText(statusText, 2);
    return camSelect >= 0;
}
void ScanFrame::OnConnect(wxCommandEvent& WXUNUSED(event))
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
			tdiExposure = 0.0;
			tdiScanRate = 0.0;
			ConnectCamera(dlg.GetSelection());
		}
	}
}
void ScanFrame::OnOverride(wxCommandEvent& WXUNUSED(event))
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
            wxConfig config(wxT("sxTDI"), wxT("sxToys"));
            config.Write(wxT("USB1Camera"), camUSBType);
		}
	}
#endif
}
void ScanFrame::OnBackground(wxEraseEvent& WXUNUSED(event))
{
}
void ScanFrame::OnPaint(wxPaintEvent& WXUNUSED(event))
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
    static long trackInitialTime;
    long trackTime = trackWatch->Time();
    sxLatchImage(camHandles[camSelect],      // cam handle
                 SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                 SXCCD_IMAGE_HEAD,           // main ccd
                 ccdFrameWidth / 4, // xoffset
                 0,                 // yoffset
                 ccdFrameWidth / 2, // width
                 ccdFrameHeight,    // height
                 1,  // xbin
                 1); // ybin
    if (!sxReadImage(camHandles[camSelect], // cam handle
                     ccdFrame, //pixbuf
                     ccdPixelCount)) // pix count
    {
        wxCommandEvent event; // Bogus event to make compiler happy
        OnStop(event);
        wxMessageBox("Camera Error", "SX TDI Alignment", wxOK | wxICON_INFORMATION);
        return;
    }
    sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    tdiTimer.StartOnce(ALIGN_EXP);
    int xRadius = TRACK_STAR_RADIUS / ccdPixelWidth;    // Centroid radius
    int yRadius = TRACK_STAR_RADIUS / ccdPixelHeight * 2; // Take into account star streaking for long focal lengths
    if (numFrames == 0)
    {
        //
        // If first frame, identify best candidate for measuring scan rate
        //
        trackStarX = ccdFrameWidth / 4;  // Start search in middle quarter of image for best centroid
        trackStarY = ccdFrameHeight - ccdFrameHeight / 4 - yRadius / 4; // Start search in left half of image for best centroid
        if (findBestCentroid(ccdFrameWidth / 2,
                             ccdFrameHeight,
                             ccdFrame,
                             &trackStarX, // centroid coordinate
                             &trackStarY,
                             ccdFrameWidth / 4, // search range in middle/left of frame
                             ccdFrameHeight / 4,
                             &xRadius,
                             &yRadius,
                             TRACK_STAR_SIGMA))
        {
            trackInitialTime  = trackTime;
            trackStarInitialX = trackStarX;
            trackStarInitialY = trackStarY;
            numFrames  = 1;
        }
    }
    else
    {
        //
        // Track star for rate measurement
        //
        if ((trackStarY > tdiScanRate)
          && findBestCentroid(ccdFrameWidth / 2,
                              ccdFrameHeight,
                              ccdFrame,
                              &trackStarX,  // centroid start and final search coordinate
                              &trackStarY,
                              xRadius,      // search X range
                              yRadius,      // search Y range
                              &xRadius,     // max X radius
                              &yRadius,     // max Y radius
                              TRACK_STAR_SIGMA))         // noise threshold
        {
            if (trackStarInitialY > trackStarY)
            {
                tdiExposure = (trackTime - trackInitialTime) / (trackStarInitialY - trackStarY);
                tdiScanRate = 1000.0 / tdiExposure;
                numFrames++;
            }
            else
            {
                //
                // Restart search for best centroid
                //
                tdiExposure = 0.0;
                tdiScanRate = 0.0;
                numFrames = 0;
            }
        }
        else
        {
            wxCommandEvent event; // Bogus event to make compiler happy
            OnStop(event);
            wxMessageBox("Tracking Star Lost", "SX TDI Alignment", wxOK | wxICON_INFORMATION);
            return;
        }
    }
    int winWidth, winHeight;
    GetClientSize(&winWidth, &winHeight);
    if (winWidth > 0 && winHeight > 0)
    {
        int pixelMax = MIN_PIX;
        int pixelMin = MAX_PIX;
        unsigned char *rgb = scanImage->GetData();
        uint16_t *m16      = ccdFrame;
        for (int l = 0; l < ccdPixelCount; l++)
        {
            if (*m16 < pixelMin) pixelMin = *m16;
            if (*m16 > pixelMax) pixelMax = *m16;
            m16++;
        }
        calcRamp(pixelMin, pixelMax, pixelGamma, pixelFilter);
        for (unsigned y = 0; y < ccdFrameWidth / 2; y++) // Rotate image 90 degrees counterclockwise as it gets copied
        {
            m16 = &(ccdFrame[ccdPixelCount - (ccdFrameWidth / 2) - 1 + y]);
            for (unsigned x = 0; x < ccdFrameHeight; x++)
            {
                rgb[0] = max(rgb[0], redLUT[LUT_INDEX(*m16)]);
                rgb[1] = max(rgb[1], blugrnLUT[LUT_INDEX(*m16)]);
                rgb[2] = max(rgb[2], blugrnLUT[LUT_INDEX(*m16)]);
                rgb   += 3;
                m16   -= ccdFrameWidth / 2;
            }
        }
        wxClientDC dc(this);
        wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
        dc.DrawBitmap(bitmap, 0, 0);
        if (numFrames)
        {
            //
            // Draw ellipse around best star depicting FWHM
            //
            float xScale = (float)winHeight / (float)(ccdFrameWidth / 2);
            float yScale = (float)winWidth  / (float)ccdFrameHeight;
            xRadius *= xScale;
            yRadius *= yScale;
            dc.SetPen(wxPen(*wxGREEN, 1, wxSOLID));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawEllipse(winWidth - 1.5 - trackStarY * yScale - yRadius, trackStarX * xScale + 1.5 - xRadius, yRadius * 2, xRadius * 2);
        }
        if (tdiScanRate > 0.0)
        {
            char statusText[40];
            sprintf(statusText, "Track: %4.3f", trackStarX);
            SetStatusText(statusText, 2);
            sprintf(statusText, "Rate: %2.3f row/s", tdiScanRate);
            SetStatusText(statusText, 1);
        }
        else
        {
            SetStatusText("Track: ----.---", 2);
            SetStatusText("Rate: --.--- row/s", 1);
        }
    }
}
ScanThread::ScanThread(ScanFrame *param) : wxThread(wxTHREAD_JOINABLE)
{
    scan = param;
    Create();
    SetPriority(wxPRIORITY_MAX); // Make it as real-time as possible
}
wxThread::ExitCode ScanThread::Entry()
{
    ExitCode scanErr = SCAN_OK;
    uint16_t *ccdRow = scan->tdiFrame;
    sxClearImage(scan->camHandles[scan->camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    wxStopWatch sw;
    double rowTime = scan->binExposure; // Keep high precision running row time
    do
    {
        memset(ccdRow, 0, sizeof(uint16_t) * scan->ccdBinWidth); // Touch row in case swapped out
        int deltaTime = rowTime - sw.Time();
        if (deltaTime < 1)
            sxLatchImage(scan->camHandles[scan->camSelect], // cam handle
                         SXCCD_EXP_FLAGS_TDI |
                         SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                         SXCCD_IMAGE_HEAD, // main ccd
                         0, // xoffset
                         0, // yoffset
                         scan->ccdFrameWidth, // width
                         scan->ccdBinY, // height
                         scan->ccdBinX, // xbin
                         scan->ccdBinY); // ybin
        else
            sxExposeImage(scan->camHandles[scan->camSelect], // cam handle
                         SXCCD_EXP_FLAGS_TDI |
                         SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                         SXCCD_IMAGE_HEAD, // main ccd
                         0, // xoffset
                         0, // yoffset
                         scan->ccdFrameWidth, // width
                         scan->ccdBinY, // height
                         scan->ccdBinX, // xbin
                         scan->ccdBinY, // ybin
                         deltaTime); // msec
        if ((deltaTime = rowTime - sw.Time() - 1) > 0)
            wxMilliSleep(deltaTime);
        if (!sxReadImage(scan->camHandles[scan->camSelect], // cam handle
                         ccdRow, //pixbuf
                         scan->ccdBinWidth)) // pix count
        {
            scanErr = SCAN_ERR_CAMERA;
            break;
        }
        rowTime += scan->binExposure;
        ccdRow  += scan->ccdBinWidth;
    } while (++(scan->tdiRow) < scan->tdiLength);
    scan->tdiLength = scan->tdiRow; // Signal complete if errored out, nop if ok
    return scanErr;
}
void ScanFrame::DoTDI()
{
    //
    // Playing fast and loose with tdiRow and tdiLength. Grab a copy in case
    // it gets updated in ScanThread causing fault in final row
    //
    int currentRow = tdiRow;
    if (currentRow < tdiLength)
    {
        int winWidth, winHeight;
        GetClientSize(&winWidth, &winHeight);
        if (winWidth > 0 && winHeight > 0)
        {
            int pixelMax       = MIN_PIX;
            int pixelMin       = MAX_PIX;
            unsigned char *rgb = scanImage->GetData();
            uint16_t *pixels   = &tdiFrame[ccdBinWidth * ((currentRow < ccdBinHeight * 2) ? ccdBinHeight * 2 - 1 : currentRow)];
            for (unsigned y = 0; y < ccdBinWidth; y++) // Rotate image 90 degrees counterclockwise as it gets copied
            {
                uint16_t *m16 = &pixels[y];
                for (unsigned x = 0; x < ccdBinHeight * 2; x++)
                {
                    if (*m16 < pixelMin && *m16 > 0) pixelMin = *m16;
                    if (*m16 > pixelMax) pixelMax = *m16;
                    rgb[0] = redLUT[LUT_INDEX(*m16)];
                    rgb[1] = blugrnLUT[LUT_INDEX(*m16)];
                    rgb[2] = blugrnLUT[LUT_INDEX(*m16)];
                    rgb   += 3;
                    m16   -= ccdBinWidth;
                }
            }
            calcRamp(pixelMin, pixelMax, pixelGamma, pixelFilter); // Behind a row in ramp updates. Oh well
            wxClientDC dc(this);
            wxBitmap bitmap(scanImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
            dc.DrawBitmap(bitmap, 0, 0);
        }
    }
    else
    {
        //
        // All done.
        //
        tdiTimer.Stop();
        tdiState = STATE_IDLE;
        SetTitle(wxT("SX TDI"));
        wxThread::ExitCode scanErr = tdiThread->Wait();
        delete tdiThread;
        DISABLE_HIGH_RES_TIMER();
        if (scanErr == SCAN_ERR_TIME)
            wxMessageBox("Miniumum Timing Error", "Scan Error", wxOK | wxICON_INFORMATION);
        if (scanErr == SCAN_ERR_CAMERA)
            wxMessageBox("Camera Error", "Scan Error", wxOK | wxICON_INFORMATION);
        if (tdiRow < ccdBinHeight)
        {
            //
            // Don't bother if less than a full frame Image
            //
            free(tdiFrame);
            tdiFrame     = NULL;
            tdiFileSaved = true;
        }
        if (autonomous)
        {
            if (tdiFrame)
            {
                char filename[255];
                char creator[] = "sxTDI";
                char camera[]  = "StarLight Xpress Camera";
                strcpy(filename, tdiFileName.c_str());
                tdiFileSaved = fitsWrite(filename,
                                         (unsigned char *)&tdiFrame[ccdBinWidth * ccdBinHeight],
                                         ccdBinWidth,
                                         tdiLength - ccdBinHeight,
                                         tdiExposure,
                                         creator,
                                         camera) >= 0;
            }
            Close(true);
        }
    }
}
void ScanFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    if (tdiState == STATE_SCANNING)
        DoTDI();
    else
        DoAlign();
}
void ScanFrame::GetDuration()
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
void ScanFrame::OnDuration(wxCommandEvent& WXUNUSED(event))
{
    if (tdiState == STATE_IDLE)
        GetDuration();
    else
        wxBell();
}
void ScanFrame::OnRate(wxCommandEvent& WXUNUSED(event))
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
    else
        wxBell();
}
void ScanFrame::OnBinX(wxCommandEvent& WXUNUSED(event))
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
    else
        wxBell();
}
void ScanFrame::OnBinY(wxCommandEvent& WXUNUSED(event))
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
    if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Overwrite unsaved image?", "Scan Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
        return;
    if (tdiExposure == 0.0)
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
    scanImage    = new wxImage(ccdBinHeight * 2, ccdBinWidth);
    binExposure  = tdiExposure * ccdBinY;
    tdiLength    = tdiMinutes * 60000 / binExposure;
    if (tdiLength < ccdBinHeight)
        tdiLength = ccdBinHeight;
    tdiFrame  = (uint16_t *)malloc(sizeof(uint16_t) * tdiLength * ccdBinWidth);
    memset(tdiFrame, 0, sizeof(uint16_t) * tdiLength * ccdBinWidth);
    tdiFileSaved = false;
    tdiRow       = 0;
    tdiState     = STATE_SCANNING;
    ENABLE_HIGH_RES_TIMER();
    tdiThread = new ScanThread(this);
    tdiThread->Run();
    tdiTimer.Start(max(binExposure, MIN_SCREEN_UPDATE)); // Bound screen update rate
}
void ScanFrame::OnAlign(wxCommandEvent& WXUNUSED(event))
{
    if (tdiState == STATE_ALIGNING)
    {
        tdiTimer.Stop();
        delete trackWatch;
        DISABLE_HIGH_RES_TIMER();
        trackWatch = NULL;
        tdiState   = STATE_IDLE;
    }
    if (tdiState == STATE_IDLE && ccdModel)
    {
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        ENABLE_HIGH_RES_TIMER();
        tdiTimer.StartOnce(ALIGN_EXP);
        trackWatch = new wxStopWatch();
        if (scanImage)
            delete scanImage;
        scanImage = new wxImage(ccdFrameHeight, ccdFrameWidth / 2);
        memset(scanImage->GetData(), 0, ccdPixelCount * 3);
        for (int y = 0; y < ccdFrameWidth / 2; y += ccdFrameWidth/32)
        {
            unsigned char *rgb = scanImage->GetData() + y * ccdFrameHeight * 3;
            for (int x = 0; x < ccdFrameHeight; x++)
            {
                rgb[1] = 64;
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
    else
        wxBell();
}
void ScanFrame::OnScan(wxCommandEvent& WXUNUSED(event))
{
    if (tdiState == STATE_ALIGNING)
    {
        char statusText[40];
        tdiTimer.Stop();
        DISABLE_HIGH_RES_TIMER();
        delete trackWatch;
        trackWatch = NULL;
        tdiState   = STATE_IDLE;
        sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
        SetStatusText(statusText, 2);
    }
    if (tdiState == STATE_IDLE && ccdModel)
    {
        StartTDI();
        SetTitle(wxT("SX TDI [Scanning]"));
    }
    else
        wxBell();
}
void ScanFrame::OnStop(wxCommandEvent& WXUNUSED(event))
{
    if (tdiTimer.IsRunning())
    {
        tdiTimer.Stop();
        if (tdiState == STATE_ALIGNING)
        {
            char statusText[40];
            sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
            SetStatusText(statusText, 2);
            delete trackWatch;
            trackWatch = NULL;
        }
        else if (tdiState == STATE_SCANNING)
        {
            tdiLength = tdiRow;
            tdiThread->Wait();
            delete tdiThread;
            tdiThread = NULL;
            if (tdiRow < ccdBinHeight)
            {
                //
                // Don't bother if less than a full frame Image
                //
                free(tdiFrame);
                tdiFrame     = NULL;
                tdiFileSaved = true;
            }
        }
        DISABLE_HIGH_RES_TIMER();
        tdiState = STATE_IDLE;
        SetTitle(wxT("SX TDI"));
    }
}
void ScanFrame::OnNew(wxCommandEvent& WXUNUSED(event))
{
    if (tdiState == STATE_IDLE)
    {
        if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Clear unsaved image?", "New Warning", wxYES_NO | wxICON_INFORMATION) == wxID_NO)
            return;
        free(tdiFrame);
        tdiFrame = NULL;
    }
}
void ScanFrame::OnSave(wxCommandEvent& WXUNUSED(event))
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
                tdiFileSaved = fitsWrite(filename,
                                         (unsigned char *)&tdiFrame[ccdBinWidth * ccdBinHeight],
                                         ccdBinWidth,
                                         tdiLength - ccdBinHeight,
                                         tdiExposure,
                                         creator,
                                         camera) >= 0;
            }
        }
        else
            wxMessageBox("No image to save", "Save Error", wxOK | wxICON_INFORMATION);
    }
    else
        wxBell();
}
void ScanFrame::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto() && tdiState == STATE_SCANNING && wxMessageBox("Cancel scan in progress?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxNO)
    {
        event.Veto();
        return;
    }
    if (tdiTimer.IsRunning())
    {
        tdiTimer.Stop();
        if (tdiState == STATE_ALIGNING)
        {
            delete trackWatch;
            trackWatch = NULL;
        }
        else if (tdiState == STATE_SCANNING)
        {
            tdiLength = tdiRow;
            tdiThread->Wait();
            delete tdiThread;
            tdiThread = NULL;
            if (tdiRow < ccdBinHeight)
            {
                //
                // Don't bother if less than a full frame Image
                //
                free(tdiFrame);
                tdiFrame     = NULL;
                tdiFileSaved = true;
            }
        }
        DISABLE_HIGH_RES_TIMER();
        tdiState = STATE_IDLE;
    }
    if (tdiFrame != NULL && !tdiFileSaved && wxMessageBox("Save image before exiting?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxYES)
    {
        wxCommandEvent eventSave;
        OnSave(eventSave);
    }
    if (scanImage != NULL)
    {
        delete scanImage;
        scanImage = NULL;
    }
	if (camCount)
	{
		sxRelease(camHandles, camCount);
		camCount = 0;
	}
    wxConfig config(wxT("sxTDI"), wxT("sxToys"));
    config.Write(wxT("ScanRate"), tdiScanRate);
    Destroy();
}
void ScanFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(false);
}
void ScanFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
}
void ScanFrame::OnGamma(wxCommandEvent& WXUNUSED(event))
{
    wxSingleChoiceDialog dlg(this,
                          wxT("Gamma:"),
                          wxT("Gamma Value"),
                          3,
                          GammaChoices);
    if (dlg.ShowModal() == wxID_OK )
    {
        pixelGamma =  GammaValues[dlg.GetSelection()];
    }
}
void ScanFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Starlight Xpress Time Delay Integrator\nVersion 0.1 Alpha 3\nCopyright (c) 2003-2020, David Schmenk", "About SX TDI", wxOK | wxICON_INFORMATION);
}
