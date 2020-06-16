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
#include <wx/cmdline.h>
#include <wx/config.h>
#include "sxsnap.h"
#define MAX_SNAPSHOTS   100
#define MAX_WHITE       MAX_PIX
#define MIN_BLACK       MIN_PIX
#define MAX_BLACK       MAX_WHITE/2
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
long     initialExposure = 30000;
long     initialCount    = 0.0;
long     initialDelay    = 0;
long     initialBinX     = 1;
long     initialBinY     = 1;
long     initialCamIndex = 0;
wxString initialBaseName = wxT("sxsnap");
bool     autonomous      = false;
int      ccdModel        = 0;
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
 * SnapShot App class
 */
class SnapApp : public wxApp
{
public:
    virtual void OnInitCmdLine(wxCmdLineParser &parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
    virtual bool OnInit();
};
class SnapFrame : public wxFrame
{
public:
    SnapFrame();
private:
	HANDLE         camHandles[SXCCD_MAX_CAMS];
    t_sxccd_params camParams[SXCCD_MAX_CAMS];
    int            camSelect, camCount;
    unsigned int   ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int   ccdBinWidth, ccdBinHeight, ccdBinX, ccdBinY, ccdPixelCount;
    double         ccdPixelWidth, ccdPixelHeight;
    uint16_t      *ccdFrame;
    int            snapExposure, snapCount, snapDelay,  snapView, snapMax;
    uint16_t      *snapShots[MAX_SNAPSHOTS];
    bool           snapSaved[MAX_SNAPSHOTS];
    int            pixelMax, pixelMin;
    int            pixelBlack, pixelWhite;
    float          pixelGamma;
    bool           pixelFilter, autoLevels;
    wxTimer        snapTimer;
    void InitLevels();
    bool ConnectCamera(int index);
    void CenterCentroid(float x, float y, int width, int height);
    void OnBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnOverride(wxCommandEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAll(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnAutoLevels(wxCommandEvent& event);
    void OnGamma(wxCommandEvent& event);
    void OnStart(wxCommandEvent& event);
    void OnNumber(wxCommandEvent& event);
    void OnExposure(wxCommandEvent& event);
    void OnDelay(wxCommandEvent& event);
    void OnBinX(wxCommandEvent& event);
    void OnBinY(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    wxDECLARE_EVENT_TABLE();
};
enum
{
    ID_TIMER = 1,
    ID_CONNECT,
    ID_OVERRIDE,
    ID_DELETE,
    ID_SAVE_ALL,
    ID_FILTER,
    ID_LEVEL_AUTO,
    ID_GAMMA,
    ID_START,
    ID_NUMBER,
    ID_EXPOSURE,
    ID_DELAY,
    ID_BINX,
    ID_BINY,
};
wxBEGIN_EVENT_TABLE(SnapFrame, wxFrame)
    EVT_TIMER(ID_TIMER,      SnapFrame::OnTimer)
    EVT_MENU(ID_CONNECT,     SnapFrame::OnConnect)
    EVT_MENU(ID_OVERRIDE,    SnapFrame::OnOverride)
    EVT_MENU(ID_DELETE,      SnapFrame::OnDelete)
    EVT_MENU(ID_SAVE_ALL,    SnapFrame::OnSaveAll)
    EVT_MENU(ID_FILTER,      SnapFrame::OnFilter)
    EVT_MENU(ID_LEVEL_AUTO,  SnapFrame::OnAutoLevels)
    EVT_MENU(ID_GAMMA,       SnapFrame::OnGamma)
    EVT_MENU(ID_START,       SnapFrame::OnStart)
    EVT_MENU(ID_NUMBER,      SnapFrame::OnNumber)
    EVT_MENU(ID_EXPOSURE,    SnapFrame::OnExposure)
    EVT_MENU(ID_DELAY,       SnapFrame::OnDelay)
    EVT_MENU(ID_BINX,        SnapFrame::OnBinX)
    EVT_MENU(ID_BINY,        SnapFrame::OnBinY)
    EVT_MENU(wxID_ABOUT,     SnapFrame::OnAbout)
    EVT_MENU(wxID_EXIT,      SnapFrame::OnExit)
    EVT_CLOSE(               SnapFrame::OnClose)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(SnapApp);
void SnapApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddOption(wxT("c"), wxT("camera"),   wxT("camera index"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("m"), wxT("model"),    wxT("USB camera model override"), wxCMD_LINE_VAL_STRING);
    parser.AddOption(wxT("e"), wxT("exposure"), wxT("exposure in msec"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("d"), wxT("delay"),    wxT("inter-exposure delay in msec"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("n"), wxT("number"),   wxT("number of exposures"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("x"), wxT("xbin"),     wxT("x bin (1, 2, 4)"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("y"), wxT("ybin"),     wxT("y bin (1, 2, 4)"), wxCMD_LINE_VAL_NUMBER);
    parser.AddSwitch(wxT("a"), wxT("auto"),     wxT("autonomous mode"));
}
bool SnapApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
    wxString *modelString = new wxString(' ', 10);
    if (parser.Found(wxT("m"), modelString))
    {
        //wxPrintf("Overriding SX camera model with: %s\n", modelString->c_str());
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
    }
    if (parser.Found(wxT("c"), &initialCamIndex))
    {}
    if (parser.Found(wxT("n"), &initialCount))
    {}
    if (parser.Found(wxT("d"), &initialDelay))
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
    autonomous = parser.Found(wxT("a"));
    if (parser.GetParamCount() > 0)
        initialBaseName = parser.GetParam(0);
    return wxApp::OnCmdLineParsed(parser);
}
bool SnapApp::OnInit()
{
    #ifndef _MSC_VER
    wxConfig config(wxT("sxSnap"), wxT("sxToys"));
    config.Read(wxT("USB1Camera"), &camUSBType);
#endif
    if (wxApp::OnInit())
    {
        SnapFrame *frame = new SnapFrame();
        frame->Show(true);
        return true;
    }
    return false;
}
SnapFrame::SnapFrame() : wxFrame(NULL, wxID_ANY, "SX SnapShot"), snapTimer(this, ID_TIMER)
{
    CreateStatusBar(3);
    wxMenu *menuCamera = new wxMenu;
    menuCamera->Append(ID_CONNECT,    wxT("&Connect Camera..."));
#ifndef _MSC_VER
    menuCamera->Append(ID_OVERRIDE,   wxT("Set Camera &Model..."));
#endif
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_NEW,      wxT("&New\tCtrl-N"));
    menuCamera->Append(ID_DELETE,     wxT("&Delete\tCtrl-D"));
    menuCamera->Append(wxID_SAVE,     wxT("&Save...\tCtrl-S"));
    menuCamera->Append(ID_SAVE_ALL,   wxT("Save &All...\tCtrl-A"));
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_EXIT);
    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(ID_FILTER,     wxT("Red Filter\tR"));
    menuView->AppendCheckItem(ID_LEVEL_AUTO, wxT("Auto Levels\tA"));
    menuView->Append(ID_GAMMA,               wxT("&Gamma..."));
    wxMenu *menuImage = new wxMenu;
    menuImage->Append(ID_START,    wxT("Start...\tENTER"));
    menuCamera->AppendSeparator();
    menuImage->Append(ID_NUMBER,   wxT("Number..\tN"));
    menuImage->Append(ID_EXPOSURE, wxT("Exposure..\tE"));
    menuImage->Append(ID_DELAY,    wxT("Delay..\tD"));
    menuImage->Append(ID_BINX,     wxT("&X Binning..."));
    menuImage->Append(ID_BINY,     wxT("&Y Binning..."));
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuCamera, wxT("&Camera"));
    menuBar->Append(menuView,   wxT("&View"));
    menuBar->Append(menuImage,  wxT("&Image"));
    menuBar->Append(menuHelp,   wxT("&Help"));
    SetMenuBar(menuBar);
    snapFilePath = wxGetCwd();
    snapBaseName = initialBaseName;
    snapCount    = 1;
    pixelFilter  = false;
    pixelGamma   = 1.5;
    ccdFrame     = NULL;
    snapImage    = NULL;
    snapView     = 0;
    snapMax      = 0;
    memset(snapShots, sizeof(unit16_t) * MAX_SNAPSHOTS);
    snapExposure = initialExposure;
    binX         = initialBinX;
    binY         = initialBinY;
    camCount     = sxProbe(camHandles, camParams, camUSBType);
    InitLevels();
    ConnectCamera(initialCamIndex);
}
void SnapFrame::InitLevels()
{
    pixelMin     = WHITE;
    pixelMax     = BLACK;
    pixelBlack   = BLACK;
    pixelWhite   = WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
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
        if (snapImage)
        {
            wxBitmap bitmap(snapImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
            dc.DrawBitmap(bitmap, 0, 0);
        }
        else
        {
            dc.SetBrush(wxBrush(*wxWHITE));
            dc.DrawRectangle(0, 0, winWidth, winHeight);
        }
    }
}
bool SnapFrame::ConnectCamera(int index)
{
    char statusText[40];
    int snapWinWidth, snapWinHeight;
    xOffset = yOffset = 0;
    InitLevels();
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
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        ccdFrame = (uint16_t *)malloc(sizeof(uint16_t) * ccdFrameWidth * ccdFrameHeight);
        snapTimer.StartOnce(snapExposure);
        sprintf(statusText, "Attached: %cX-%d[%d]", ccdModel & SXCCD_INTERLEAVE ? 'M' : 'H', ccdModel & 0x3F, camSelect);
    }
    else
    {
        camSelect     = -1;
        ccdModel      = 0;
        ccdFrameWidth = ccdFrameHeight = 512;
        ccdFrameDepth = 16;
        ccdPixelWidth = ccdPixelHeight = 1;
        ccdFrame      = NULL;
        strcpy(statusText, "Attached: None");
    }
    if (!IsMaximized())
    {
        snapWinWidth  = ccdFrameWidth;
        snapWinHeight = ccdFrameHeight * ccdPixelHeight / ccdPixelWidth; // Keep aspect ratio
        while (snapWinHeight > 720) // Constrain initial size to something reasonable
        {
            snapWinWidth  >>= 1;
            snapWinHeight >>= 1;
        }
        SetClientSize(snapWinWidth, snapWinHeight);
    }
    snapZoom   = -1;
    zoomWidth  = ccdFrameWidth  >> -snapZoom;
    zoomHeight = ccdFrameHeight >> -snapZoom;
    SetStatusText(statusText, 0);
    SetStatusText("Bin: X2", 1);
    return camSelect >= 0;
}
void SnapFrame::OnConnect(wxCommandEvent& WXUNUSED(event))
{
    if (snapTimer.IsRunning())
        snapTimer.Stop();
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
        ConnectCamera(dlg.GetSelection());
    else if (camSelect)
    {
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        snapTimer.StartOnce(snapExposure);
    }
}
void SnapFrame::OnOverride(wxCommandEvent& WXUNUSED(event))
{
#ifndef _MSC_VER
    if (snapTimer.IsRunning())
        snapTimer.Stop();
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
        wxConfig config(wxT("sxSnap"), wxT("sxToys"));
        config.Write(wxT("USB1Camera"), camUSBType);
    }
    else
    {
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        snapTimer.StartOnce(snapExposure);
    }
#endif
}
void OnNew(wxCommandEvent& event)
{}
void OnDelete(wxCommandEvent& event)
{}
void SnapFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
    {
        fitsfile *fptr;
        char filename[30];
        char creator[]  = "sxSnapShot";
        char camera[]   = "StarLight Xpress Camera";
        int  status     = 0;
        long exposure   = snapExposure;
        long naxes[2]   = {zoomWidth, zoomHeight};   // image size
        sprintf(filename, "sxsnap-%03d.fits", snapCount++);
        remove(filename); // Delete old file if it already exists
        if (fits_create_file(&fptr, filename, &status))                                            return;
        if (fits_create_img(fptr, USHORT_IMG, 2, naxes, &status))                                  return;
        if (fits_write_img(fptr, TUSHORT, 1, naxes[0] * naxes[1], ccdFrame, &status))              return;
        if (fits_write_date(fptr, &status))                                                        return;
        if (fits_update_key(fptr, TLONG,   "EXPOSURE", &exposure, "Total Exposure Time", &status)) return;
        if (fits_update_key(fptr, TSTRING, "CREATOR",   creator,  "Imaging Application", &status)) return;
        if (fits_update_key(fptr, TSTRING, "CAMERA",    camera,   "Imaging Device",      &status)) return;
        if (fits_close_file(fptr, &status))                                                        return;
        snapped = true;
        wxBell();
    }
}
void OnSaveAll(wxCommandEvent& event)
{}
void SnapFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void SnapFrame::OnAutoLevels(wxCommandEvent& event)
{
    autoLevels = event.IsChecked();
}
void SnapFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    int snapWinWidth, snapWinHeight;
    int pixCount;
    if (snapExposure == MIN_EXPOSURE)
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    if (snapZoom < 1)
    {
        sxLatchImage(camHandles[camSelect], // cam handle
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     SXCCD_IMAGE_HEAD, // main ccd
                     0, // xoffset
                     0, // yoffset
                     ccdFrameWidth, // width
                     ccdFrameHeight, // height
                     1 << -snapZoom, // xbin
                     1 << -snapZoom); // ybin
        pixCount = FRAMEBUF_COUNT(ccdFrameWidth, ccdFrameHeight, 1 << -snapZoom, 1 << -snapZoom);
    }
    else
    {
        sxLatchImage(camHandles[camSelect], // cam handle
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     SXCCD_IMAGE_HEAD, // main ccd
                     0,          // xoffset
                     0,          // yoffset
                     zoomWidth,  // width
                     zoomHeight, // height
                     1, // xbin
                     1); // ybin
        pixCount = FRAMEBUF_COUNT(zoomWidth, zoomHeight, 1, 1);
    }
    if (!sxReadImage(camHandles[camSelect], // cam handle
                     ccdFrame, //pixbuf
                     pixCount)) // pix count
    {
        wxMessageBox("Camera Error", "SX Snap", wxOK | wxICON_INFORMATION);
        return;
    }
    /*
     * Convert 16 bit samples to 24 BPP image
     */
    wxImage ccdImage(zoomWidth, zoomHeight);
    unsigned char *rgb = ccdImage.GetData();
    uint16_t      *m16 = ccdFrame;
    pixelMin = MAX_PIX;
    pixelMax = MIN_PIX;
    if (autoLevels)
    {
        for (int l = 0; l < zoomHeight*zoomWidth; l++)
        {
            if (*m16 < pixelMin) pixelMin = *m16;
            if (*m16 > pixelMax) pixelMax = *m16;
            m16++;
        }
        m16        = ccdFrame; // Reset CCD image pointer
        pixelBlack = pixelMin;
        pixelWhite = pixelMax;
        calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
    }
    for (int y = 0; y < zoomHeight; y++)
        for (int x = 0; x < zoomWidth; x++)
        {
            if (*m16 < pixelMin) pixelMin = *m16;
            if (*m16 > pixelMax) pixelMax = *m16;
            rgb[0] = redLUT[LUT_INDEX(*m16)];
            rgb[1] =
            rgb[2] = blugrnLUT[LUT_INDEX(*m16)];
            rgb   += 3;
            m16++;
        }
    GetClientSize(&snapWinWidth, &snapWinHeight);
    if (snapWinWidth > 0 && snapWinHeight > 0)
    {
        wxClientDC dc(this);
        wxBitmap bitmap(ccdImage.Scale(snapWinWidth, snapWinHeight, wxIMAGE_QUALITY_BILINEAR));
        dc.DrawBitmap(bitmap, 0, 0);
        xBestCentroid  = zoomWidth  / 2;
        yBestCentroid  = zoomHeight / 2;
        int xRadius = 100 / ccdPixelWidth;  // Max centroid radius
        int yRadius = 100 / ccdPixelHeight;
        if (findBestCentroid(zoomWidth,
                             zoomHeight,
                             ccdFrame,
                             &xBestCentroid, // centroid coordinate
                             &yBestCentroid,
                             zoomWidth  / 2, // search entire frame
                             zoomHeight / 2,
                             &xRadius,
                             &yRadius,
                             1.0))
        {
            CenterCentroid(xBestCentroid, yBestCentroid, zoomWidth, zoomHeight);
            //
            // Draw ellipse around best star depicting FWHM
            //
            float xScale = (float)snapWinWidth  / (float)zoomWidth;
            float yScale = (float)snapWinHeight / (float)zoomHeight;
            xRadius *= 2 * xScale;
            yRadius *= 2 * yScale;
            dc.SetPen(wxPen(*wxGREEN, 1, wxSOLID));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawEllipse((xBestCentroid + 0.5) * xScale - xRadius, (yBestCentroid + 0.5) * yScale - yRadius, xRadius * 2, yRadius * 2);
        }
    }
    char minmax[20];
    sprintf(minmax, "Min: %d", pixelMin);
    SetStatusText(minmax, 2);
    sprintf(minmax, "Max: %d", pixelMax);
    SetStatusText(minmax, 3);
    /*
     * Prep next frame
     */
    if (snapExposure < MAX_EXPOSURE)
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    snapTimer.StartOnce(snapExposure);
}
void ScanFrame::Start()
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
    if (snapState == STATE_IDLE)
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
    if (snapState == STATE_IDLE)
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
void SnapFrame::OnClose(wxCloseEvent& event)
{
    int i;
    //
    // Any unsaved images?
    //
    for (i = 0; i < snapMax; i++)
    {
        if (snapShots[i] && !snapSaved[i])
        {
            if (wxMessageBox("Save images before exiting?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxYES)
            {
                wxCommandEvent eventSaveAll;
                OnSaveAll(eventSaveAll);
            }
            break;
        }
    }
    //
    // Free up the images
    //
    for (i = 0; i < snapMax; i++)
    {
        if (snapShots[i])
        {
            free(snapShots[i]);
            snapShots[i] = NULL;
        }
        snapMax = 0;
    }
    if (snapImage != NULL)
    {
        delete snapImage;
        snapImage = NULL;
    }
    //
    // Release the cameras
    //
	if (camCount)
	{
		sxRelease(camHandles, camCount);
		camCount = 0;
	}
    Destroy();
}
void SnapFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}
void SnapFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Starlight Xpress SnapShot\nVersion 0.1 Alpha 3\nCopyright (c) 2003-2020, David Schmenk", "About SX SnapShot", wxOK | wxICON_INFORMATION);
}
