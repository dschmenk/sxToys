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
#include "sxfocus.h"
#define MIN_ZOOM        -4
#define MAX_ZOOM        4
#define MAX_WHITE       0xFFFF
#define MIN_BLACK       0
#define MAX_BLACK       (MAX_WHITE/2)
#define INC_BLACK       (MAX_BLACK/16)
#define MIN_GAMMA       0.5
#define MAX_GAMMA       2.5
#define INC_GAMMA       0.5
#define INC_EXPOSURE    100
#define MIN_EXPOSURE    10
#define MAX_EXPOSURE    (INC_EXPOSURE*21)
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
int     camUSBType      = 0;
long    initialCamIndex = 0;
int     ccdModel = SXCCD_MX5;
class FocusApp : public wxApp
{
public:
    virtual void OnInitCmdLine(wxCmdLineParser &parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser &parser);
    virtual bool OnInit();
};
class FocusFrame : public wxFrame
{
public:
    FocusFrame();
private:
	HANDLE         camHandles[SXCCD_MAX_CAMS];
    t_sxccd_params camParams[SXCCD_MAX_CAMS];
    int            camSelect, camCount;
    unsigned int   ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    float          ccdPixelWidth, ccdPixelHeight;
    uint16_t      *ccdFrame;
    float          xBestCentroid, yBestCentroid;
    int            xOffset, yOffset;
    int            focusZoom, focusExposure;
    int            zoomWidth, zoomHeight;
    int            pixelMax, pixelMin;
    int            pixelBlack, pixelWhite;
    float          pixelGamma;
    bool           pixelFilter, autoLevels;
    wxTimer        focusTimer;
    void InitLevels();
    bool ConnectCamera(int index);
    void CenterCentroid(float x, float y, int width, int height);
    void OnTimer(wxTimerEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnOverride(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnAutoLevels(wxCommandEvent& event);
    void OnResetLevels(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnContrastInc(wxCommandEvent& event);
    void OnContrastDec(wxCommandEvent& event);
    void OnBrightnessInc(wxCommandEvent& event);
    void OnBrightnessDec(wxCommandEvent& event);
    void OnGammaInc(wxCommandEvent& event);
    void OnGammaDec(wxCommandEvent& event);
    void OnExposureInc(wxCommandEvent& event);
    void OnExposureDec(wxCommandEvent& event);
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
    ID_FILTER,
    ID_LEVEL_AUTO,
    ID_LEVEL_RESET,
    ID_ZOOM_IN,
    ID_ZOOM_OUT,
    ID_CONT_INC,
    ID_CONT_DEC,
    ID_BRITE_INC,
    ID_BRITE_DEC,
    ID_GAMMA_INC,
    ID_GAMMA_DEC,
    ID_EXPOSE_INC,
    ID_EXPOSE_DEC,
};
wxBEGIN_EVENT_TABLE(FocusFrame, wxFrame)
    EVT_TIMER(ID_TIMER,      FocusFrame::OnTimer)
    EVT_MENU(ID_CONNECT,     FocusFrame::OnConnect)
    EVT_MENU(ID_OVERRIDE,    FocusFrame::OnOverride)
    EVT_MENU(ID_FILTER,      FocusFrame::OnFilter)
    EVT_MENU(ID_LEVEL_AUTO,  FocusFrame::OnAutoLevels)
    EVT_MENU(ID_LEVEL_RESET, FocusFrame::OnResetLevels)
    EVT_MENU(ID_ZOOM_IN,     FocusFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,    FocusFrame::OnZoomOut)
    EVT_MENU(ID_CONT_INC,    FocusFrame::OnContrastInc)
    EVT_MENU(ID_CONT_DEC,    FocusFrame::OnContrastDec)
    EVT_MENU(ID_BRITE_INC,   FocusFrame::OnBrightnessInc)
    EVT_MENU(ID_BRITE_DEC,   FocusFrame::OnBrightnessDec)
    EVT_MENU(ID_GAMMA_INC,   FocusFrame::OnGammaInc)
    EVT_MENU(ID_GAMMA_DEC,   FocusFrame::OnGammaDec)
    EVT_MENU(ID_EXPOSE_INC,  FocusFrame::OnExposureInc)
    EVT_MENU(ID_EXPOSE_DEC,  FocusFrame::OnExposureDec)
    EVT_MENU(wxID_ABOUT,     FocusFrame::OnAbout)
    EVT_MENU(wxID_EXIT,      FocusFrame::OnExit)
    EVT_CLOSE(               FocusFrame::OnClose)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(FocusApp);
void FocusApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddOption(wxT("c"), wxT("camera"), wxT("camera index"), wxCMD_LINE_VAL_NUMBER);
    parser.AddOption(wxT("m"), wxT("model"), wxT("USB camera model override"), wxCMD_LINE_VAL_STRING);
}
bool FocusApp::OnCmdLineParsed(wxCmdLineParser &parser)
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
        printf("USB SX model type: 0x%02X\n", ccdModel);
    }
    if (parser.Found(wxT("c"), &initialCamIndex))
    {}
    return wxApp::OnCmdLineParsed(parser);
}
bool FocusApp::OnInit()
{
    if (wxApp::OnInit())
    {
        FocusFrame *frame = new FocusFrame();
        frame->Show(true);
        return true;
    }
    return false;
}
FocusFrame::FocusFrame() : wxFrame(NULL, wxID_ANY, "SX Focus"), focusTimer(this, ID_TIMER)
{
    wxMenu *menuCamera = new wxMenu;
    menuCamera->Append(ID_CONNECT,    wxT("&Connect Camera..."));
#ifndef _MSC_VER
    menuCamera->Append(ID_OVERRIDE,   wxT("&Set Camera Model..."));
#endif
    menuCamera->AppendSeparator();
    menuCamera->Append(wxID_EXIT);
    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(ID_FILTER,     wxT("Red Filter\tR"));
    menuView->AppendCheckItem(ID_LEVEL_AUTO, wxT("Auto Levels\tA"));
    menuView->Append(ID_LEVEL_RESET,         wxT("Reset Levels\tSPACE"));
    menuView->Append(ID_ZOOM_IN,             wxT("Zoom In\tX"));
    menuView->Append(ID_ZOOM_OUT,            wxT("Zoom Out\tZ"));
    menuView->Append(ID_EXPOSE_INC,          wxT("Exposure Inc\tE"));
    menuView->Append(ID_EXPOSE_DEC,          wxT("Exposure Dec\tW"));
    menuView->Append(ID_CONT_INC,            wxT("Contrast Inc\tPGUP"));
    menuView->Append(ID_CONT_DEC,            wxT("Contrast Dec\tPGDN"));
    menuView->Append(ID_BRITE_INC,           wxT("Brightness Inc\tHOME"));
    menuView->Append(ID_BRITE_DEC,           wxT("Brightness Dec\tEND"));
    menuView->Append(ID_GAMMA_INC,           wxT("Gamma Inc\t]"));
    menuView->Append(ID_GAMMA_DEC,           wxT("Gamma Dec\t["));
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuCamera, wxT("&Camera"));
    menuBar->Append(menuView,   wxT("&View"));
    menuBar->Append(menuHelp,   wxT("&Help"));
    SetMenuBar(menuBar);
    CreateStatusBar(4);
    pixelFilter = false;
    ccdFrame    = NULL;
    camCount    = sxProbe(camHandles, camParams, camUSBType);
    InitLevels();
    ConnectCamera(initialCamIndex);
}
void FocusFrame::InitLevels()
{
    focusExposure = MIN_EXPOSURE + INC_EXPOSURE;
    pixelGamma    = 1.5;
    pixelMin      = MAX_WHITE;
    pixelMax      = MIN_BLACK;
    pixelBlack    = MIN_BLACK;
    pixelWhite    = MAX_WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
bool FocusFrame::ConnectCamera(int index)
{
    char statusText[40];
    int focusWinWidth, focusWinHeight;
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
        focusTimer.StartOnce(focusExposure);
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
        focusWinWidth  = ccdFrameWidth;
        focusWinHeight = ccdFrameHeight * ccdPixelHeight / ccdPixelWidth; // Keep aspect ratio
        while (focusWinHeight > 720) // Constrain initial size to something reasonable
        {
            focusWinWidth  >>= 1;
            focusWinHeight >>= 1;
        }
        SetClientSize(focusWinWidth, focusWinHeight);
    }
    focusZoom  = -1;
    zoomWidth  = ccdFrameWidth  >> -focusZoom;
    zoomHeight = ccdFrameHeight >> -focusZoom;
    SetStatusText(statusText, 0);
    SetStatusText("Bin: X2", 1);
    return camSelect >= 0;
}
void FocusFrame::OnConnect(wxCommandEvent& WXUNUSED(event))
{
    if (focusTimer.IsRunning())
        focusTimer.Stop();
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
        focusTimer.StartOnce(focusExposure);
    }
}
void FocusFrame::OnOverride(wxCommandEvent& WXUNUSED(event))
{
#ifndef _MSC_VER
    if (focusTimer.IsRunning())
        focusTimer.Stop();
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
    else
    {
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        focusTimer.StartOnce(focusExposure);
    }
#endif
}
void FocusFrame::CenterCentroid(float x, float y, int width, int height)
{
    //
    // Use centroid to center zoomed window
    //
    xOffset += x - width  / 2;
    yOffset += y - height / 2;
    if (xOffset < 0)
        xOffset = 0;
    else if (xOffset >= (ccdFrameWidth - width))
        xOffset = ccdFrameWidth - width - 1;
    if (yOffset < 0)
        yOffset = 0;
    else if (yOffset >= (ccdFrameHeight - height))
        yOffset = ccdFrameHeight - height - 1;
}
void FocusFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    int focusWinWidth, focusWinHeight;
    int pixCount;
    if (focusExposure == MIN_EXPOSURE)
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    if (focusZoom < 1)
    {
        sxLatchImage(camHandles[camSelect], // cam handle
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     SXCCD_IMAGE_HEAD, // main ccd
                     0, // xoffset
                     0, // yoffset
                     ccdFrameWidth, // width
                     ccdFrameHeight, // height
                     1 << -focusZoom, // xbin
                     1 << -focusZoom); // ybin
        pixCount = FRAMEBUF_COUNT(ccdFrameWidth, ccdFrameHeight, 1 << -focusZoom, 1 << -focusZoom);
    }
    else
    {
        sxLatchImage(camHandles[camSelect], // cam handle
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     SXCCD_IMAGE_HEAD, // main ccd
                     xOffset,    // xoffset
                     yOffset,    // yoffset
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
         wxMessageBox("Camera Error", "SX TDI Alignment", wxOK | wxICON_INFORMATION);
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
    GetClientSize(&focusWinWidth, &focusWinHeight);
    if (focusWinWidth > 0 && focusWinHeight > 0)
    {
        wxClientDC dc(this);
        wxBitmap bitmap(ccdImage.Scale(focusWinWidth, focusWinHeight, wxIMAGE_QUALITY_BILINEAR));
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
            float xScale = (float)focusWinWidth  / (float)zoomWidth;
            float yScale = (float)focusWinHeight / (float)zoomHeight;
            xRadius *= xScale * 4;
            yRadius *= yScale * 4;
            dc.SetPen(wxPen(*wxGREEN, 1, wxSOLID));
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawEllipse(xBestCentroid * xScale - xRadius, yBestCentroid * yScale - yRadius, xRadius * 2, yRadius * 2);
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
    if (focusExposure < MAX_EXPOSURE)
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
    focusTimer.StartOnce(focusExposure);
}
void FocusFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnAutoLevels(wxCommandEvent& event)
{
    autoLevels = event.IsChecked();
}
void FocusFrame::OnResetLevels(wxCommandEvent& WXUNUSED(event))
{
    InitLevels();
}
void FocusFrame::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
    char statusText[20];

    if (focusZoom < MAX_ZOOM)
    {
        focusZoom++;
        if (focusZoom < 1)
        {
            zoomWidth  = ccdFrameWidth  >> -focusZoom;
            zoomHeight = ccdFrameHeight >> -focusZoom;
            sprintf(statusText, "Bin: X%d", 1 << -focusZoom);
        }
        else
        {
            zoomWidth  = ccdFrameWidth >> focusZoom;
            zoomHeight = ccdFrameHeight >> focusZoom;
            xOffset += zoomWidth / 2;
            yOffset += zoomHeight / 2;
            CenterCentroid(xBestCentroid / 2, yBestCentroid / 2, zoomWidth, zoomHeight);
            sprintf(statusText, "Zoom: %dX", 1 << focusZoom);
        }
        SetStatusText(statusText, 1);
    }
}
void FocusFrame::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
    char statusText[20];

    if (focusZoom > MIN_ZOOM)
    {
        focusZoom--;
        if (focusZoom < 1)
        {
            zoomWidth  = ccdFrameWidth  >> -focusZoom;
            zoomHeight = ccdFrameHeight >> -focusZoom;
            xOffset = yOffset = 0;
            sprintf(statusText, "Bin: X%d", 1 << -focusZoom);
        }
        else
        {
            zoomWidth  = ccdFrameWidth >> focusZoom;
            zoomHeight = ccdFrameHeight >> focusZoom;
            xOffset -= zoomWidth / 4;
            yOffset -= zoomHeight / 4;
            CenterCentroid(xBestCentroid * 2, yBestCentroid * 2, zoomWidth, zoomHeight);
            sprintf(statusText, "Zoom: %dX", 1 << focusZoom);
        }
        SetStatusText(statusText, 1);
    }
}
void FocusFrame::OnContrastInc(wxCommandEvent& WXUNUSED(event))
{
    pixelWhite = (pixelWhite + pixelBlack) / 2 + 1;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnContrastDec(wxCommandEvent& WXUNUSED(event))
{
    pixelWhite += (pixelWhite - pixelBlack);
    if (pixelWhite > MAX_WHITE) pixelWhite = MAX_WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessInc(wxCommandEvent& WXUNUSED(event))
{
    pixelBlack -= INC_BLACK;
    if (pixelBlack < MIN_BLACK) pixelBlack = MIN_BLACK;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessDec(wxCommandEvent& WXUNUSED(event))
{
    pixelBlack += INC_BLACK;
    if (pixelBlack >= pixelWhite) pixelBlack = pixelWhite - 1;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaInc(wxCommandEvent& WXUNUSED(event))
{
    if (pixelGamma < MAX_GAMMA) pixelGamma += INC_GAMMA;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaDec(wxCommandEvent& WXUNUSED(event))
{
    if (pixelGamma > MIN_GAMMA) pixelGamma -= INC_GAMMA;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnExposureInc(wxCommandEvent& WXUNUSED(event))
{
    if (focusExposure < MAX_EXPOSURE) focusExposure += INC_EXPOSURE;
}
void FocusFrame::OnExposureDec(wxCommandEvent& WXUNUSED(event))
{
    if (focusExposure > MIN_EXPOSURE) focusExposure -= INC_EXPOSURE;
}
void FocusFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
    if (focusTimer.IsRunning())
        focusTimer.Stop();
	if (camCount)
	{
		sxRelease(camHandles, camCount);
		camCount = 0;
	}
    Destroy();
}
void FocusFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}
void FocusFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox("Starlight Xpress Focusser\nVersion 0.1 Alpha 2\nCopyright (c) 2003-2020, David Schmenk", "About SX Focus", wxOK | wxICON_INFORMATION);
}
