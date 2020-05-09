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
#define MIN_GAMMA       0.6
#define MAX_GAMMA       4.0
#define INC_GAMMA       0.4
#define INC_EXPOSURE    100
#define MIN_EXPOSURE    INC_EXPOSURE
#define MAX_EXPOSURE    (INC_EXPOSURE*50)
#define PIX_BITWIDTH    16
#define MIN_PIX         0
#define MAX_PIX         ((1<<PIX_BITWIDTH)-1)
#define LUT_BITWIDTH    10
#define LUT_SIZE        (1<<LUT_BITWIDTH)
#define LUT_INDEX(i)    ((i)>>(PIX_BITWIDTH-LUT_BITWIDTH))
/*
 * Initial values
 */
int     camUSBType      = 0;
long    initialCamIndex = 0;
int     ccdModel = SXCCD_MX5;
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
    int          camIndex, camCount;
    unsigned int ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int ccdPixelWidth, ccdPixelHeight;
    uint16_t    *ccdFrame;
    int          focusZoom, focusExposure;
    int          pixelMax, pixelMin;
    int          pixelBlack, pixelWhite;
    float        pixelGamma;
    bool         pixelFilter;
    wxTimer      focusTimer;
    bool ConnectCamera(int index);
    void OnTimer(wxTimerEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);
    void OnZoomOut(wxCommandEvent& event);
    void OnContrastInc(wxCommandEvent& event);
    void OnContrastDec(wxCommandEvent& event);
    void OnContrastReset(wxCommandEvent& event);
    void OnBrightnessInc(wxCommandEvent& event);
    void OnBrightnessDec(wxCommandEvent& event);
    void OnBrightnessReset(wxCommandEvent& event);
    void OnAutoLevels(wxCommandEvent& event);
    void OnGammaInc(wxCommandEvent& event);
    void OnGammaDec(wxCommandEvent& event);
    void OnGammaReset(wxCommandEvent& event);
    void OnExposureInc(wxCommandEvent& event);
    void OnExposureDec(wxCommandEvent& event);
    void OnExposureReset(wxCommandEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};
enum
{
    ID_TIMER = 1,
    ID_FILTER,
    ID_ZOOM_IN,
    ID_ZOOM_OUT,
    ID_CONT_INC,
    ID_CONT_DEC,
    ID_CONT_RST,
    ID_BRITE_INC,
    ID_BRITE_DEC,
    ID_BRITE_RST,
    ID_LEVEL_AUTO,
    ID_GAMMA_INC,
    ID_GAMMA_DEC,
    ID_GAMMA_RST,
    ID_EXPOSE_INC,
    ID_EXPOSE_DEC,
    ID_EXPOSE_RST,
    ID_CONNECT,
};
wxBEGIN_EVENT_TABLE(FocusFrame, wxFrame)
    EVT_TIMER(ID_TIMER,     FocusFrame::OnTimer)
    EVT_MENU(ID_FILTER,     FocusFrame::OnFilter)
    EVT_MENU(ID_ZOOM_IN,    FocusFrame::OnZoomIn)
    EVT_MENU(ID_ZOOM_OUT,   FocusFrame::OnZoomOut)
    EVT_MENU(ID_CONT_INC,   FocusFrame::OnContrastInc)
    EVT_MENU(ID_CONT_DEC,   FocusFrame::OnContrastDec)
    EVT_MENU(ID_CONT_RST,   FocusFrame::OnContrastReset)
    EVT_MENU(ID_BRITE_INC,  FocusFrame::OnBrightnessInc)
    EVT_MENU(ID_BRITE_DEC,  FocusFrame::OnBrightnessDec)
    EVT_MENU(ID_BRITE_RST,  FocusFrame::OnBrightnessReset)
    EVT_MENU(ID_LEVEL_AUTO, FocusFrame::OnAutoLevels)
    EVT_MENU(ID_GAMMA_INC,  FocusFrame::OnGammaInc)
    EVT_MENU(ID_GAMMA_DEC,  FocusFrame::OnGammaDec)
    EVT_MENU(ID_GAMMA_RST,  FocusFrame::OnGammaReset)
    EVT_MENU(ID_EXPOSE_INC, FocusFrame::OnExposureInc)
    EVT_MENU(ID_EXPOSE_DEC, FocusFrame::OnExposureDec)
    EVT_MENU(ID_EXPOSE_RST, FocusFrame::OnExposureReset)
    EVT_MENU(ID_CONNECT,    FocusFrame::OnConnect)
    EVT_MENU(wxID_ABOUT,    FocusFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     FocusFrame::OnExit)
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
    wxMenu *menuFocus = new wxMenu;
    menuFocus->AppendCheckItem(ID_FILTER, wxT("Red Filter\tR"));
    menuFocus->Append(ID_ZOOM_IN,    wxT("Zoom In\t="));
    menuFocus->Append(ID_ZOOM_OUT,   wxT("Zoom Out\t-"));
    menuFocus->Append(ID_CONT_INC,   wxT("Contrast Inc\t]"));
    menuFocus->Append(ID_CONT_DEC,   wxT("Contrast Dec\t["));
    menuFocus->Append(ID_CONT_RST,   wxT("Contrast Reset\t\\"));
    menuFocus->Append(ID_BRITE_INC,  wxT("Brightness Inc\t}"));
    menuFocus->Append(ID_BRITE_DEC,  wxT("Brightness Dec\t{"));
    menuFocus->Append(ID_BRITE_RST,  wxT("Brightness Reset\t|"));
    menuFocus->Append(ID_LEVEL_AUTO, wxT("Auto Levels\tA"));
    menuFocus->Append(ID_GAMMA_INC,  wxT("Gamma Inc\t>"));
    menuFocus->Append(ID_GAMMA_DEC,  wxT("Gamma Dec\t<"));
    menuFocus->Append(ID_GAMMA_RST,  wxT("Gamma Reset\t?"));
    menuFocus->Append(ID_EXPOSE_INC, wxT("Exposure Inc\t."));
    menuFocus->Append(ID_EXPOSE_DEC, wxT("Exposure Dec\t,"));
    menuFocus->Append(ID_EXPOSE_RST, wxT("Exposure Reset\t/"));
    menuFocus->AppendSeparator();
    menuFocus->Append(ID_CONNECT,    wxT("&Connect Camera..."));
    menuFocus->AppendSeparator();
    menuFocus->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFocus, wxT("&Focus"));
    menuBar->Append(menuHelp, wxT("&Help"));
    SetMenuBar(menuBar);
    CreateStatusBar(4);
    pixelGamma  = 1.0;
    pixelFilter = false;
    camCount    = sxOpen(camUSBType);
    ConnectCamera(initialCamIndex);
}
bool FocusFrame::ConnectCamera(int index)
{
    char statusText[40];
    int focusWinWidth, focusWinHeight;
    calcRamp(MIN_PIX, MAX_PIX, 1.0, false);
    focusExposure = MIN_EXPOSURE;
    pixelMin      = MAX_WHITE;
    pixelMax      = MIN_BLACK;
    pixelBlack    = MIN_BLACK;
    pixelWhite    = MAX_WHITE;
    if (ccdFrame)
        free(ccdFrame);
    if (camCount)
    {
        if (index >= camCount)
            index = camCount - 1;
        camIndex = index;
        ccdModel = sxGetModel(camIndex);
        sxGetFrameDimensions(camIndex, &ccdFrameWidth, &ccdFrameHeight, &ccdFrameDepth);
        sxGetPixelDimensions(camIndex, &ccdPixelWidth, &ccdPixelHeight);
        sxClearFrame(camIndex, SXCCD_EXP_FLAGS_FIELD_BOTH);
        ccdFrame = (uint16_t *)malloc(sizeof(uint16_t) * ccdFrameWidth * ccdFrameHeight);
        focusTimer.StartOnce(focusExposure);
        sprintf(statusText, "Attached: %cX-%d[%d]", ccdModel & SXCCD_INTERLEAVE ? 'M' : 'H', ccdModel & 0x3F, camIndex);
    }
    else
    {
        camIndex      = -1;
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
    focusZoom = -1;
    SetStatusText(statusText, 0);
    SetStatusText("Bin: X2", 1);
    return camIndex >= 0;
}
void FocusFrame::OnTimer(wxTimerEvent& event)
{
    int zoomWidth, zoomHeight;
    int focusWinWidth, focusWinHeight;
    if (focusZoom < 1)
    {
        zoomWidth  = ccdFrameWidth  >> -focusZoom;
        zoomHeight = ccdFrameHeight >> -focusZoom;
        sxReadPixels(camIndex, // cam idx
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     0, // xoffset
                     0, // yoffset
                     ccdFrameWidth, // width
                     ccdFrameHeight, // height
                     1 << -focusZoom, // xbin
                     1 << -focusZoom, // ybin
                     (unsigned char *)ccdFrame); //pixbuf
    }
    else
    {
        zoomWidth  = ccdFrameWidth >> focusZoom;
        zoomHeight = ccdFrameHeight >> focusZoom;
        sxReadPixels(camIndex, // cam idx
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     (ccdFrameWidth  - zoomWidth)  / 2, // xoffset
                     (ccdFrameHeight - zoomHeight) / 2, // yoffset
                     zoomWidth, // width
                     zoomHeight, // height
                     1, // xbin
                     1, // ybin
                     (unsigned char *)ccdFrame); //pixbuf
    }
    /*
     * Convert 16 bit samples to 24 BPP image
     */
    wxImage ccdImage(zoomWidth, zoomHeight);
    unsigned char *rgb = ccdImage.GetData();
    uint16_t      *m16 = ccdFrame;
    pixelMin = MAX_PIX;
    pixelMax = MIN_PIX;
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
    }
    char minmax[20];
    sprintf(minmax, "Min: %d", pixelMin);
    SetStatusText(minmax, 2);
    sprintf(minmax, "Max: %d", pixelMax);
    SetStatusText(minmax, 3);
    /*
     * Prep next frame
     */
    if (focusExposure < 1000)
        sxClearFrame(camIndex, SXCCD_EXP_FLAGS_FIELD_BOTH);
    focusTimer.StartOnce(focusExposure);
}
void FocusFrame::OnConnect(wxCommandEvent& event)
{
    if (focusTimer.IsRunning())
        focusTimer.Stop();
    if ((camCount = sxOpen(camUSBType)) == 0)
    {
        wxMessageBox("No Cameras Found", "Connect Error", wxOK | wxICON_INFORMATION);
        return;
    }
    wxString CamChoices[camCount];
    for (int i = 0; i < camCount; i++)
    {
        int model     = sxGetModel(i);
        CamChoices[i] = wxString::Format("%cX-%d", model & SXCCD_INTERLEAVE ? 'M' : 'H', model & 0x3F);
    }
    wxSingleChoiceDialog dlg(this,
                          wxT("Camera:"),
                          wxT("Connect Camera"),
                          camCount,
                          CamChoices);
    if (dlg.ShowModal() == wxID_OK )
        ConnectCamera(dlg.GetSelection());
    else if (camIndex)
        focusTimer.StartOnce(focusExposure);
}
void FocusFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnZoomIn(wxCommandEvent& event)
{
    char statusText[20];

    focusZoom++;
    if (focusZoom > MAX_ZOOM)
        focusZoom = MAX_ZOOM;
    if (focusZoom < 1)
        sprintf(statusText, "Bin: X%d", 1 << -focusZoom);
    else
        sprintf(statusText, "Zoom: %dX", 1 << focusZoom);
    SetStatusText(statusText, 1);
}
void FocusFrame::OnZoomOut(wxCommandEvent& event)
{
    char statusText[20];

    focusZoom--;
    if (focusZoom < MIN_ZOOM)
        focusZoom = MIN_ZOOM;
    if (focusZoom < 1)
        sprintf(statusText, "Bin: X%d", 1 << -focusZoom);
    else
        sprintf(statusText, "Zoom: %dX", 1 << focusZoom);
    SetStatusText(statusText, 1);
}
void FocusFrame::OnContrastInc(wxCommandEvent& event)
{
    pixelWhite = (pixelWhite + pixelBlack) / 2 + 1;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnContrastDec(wxCommandEvent& event)
{
    pixelWhite += (pixelWhite - pixelBlack);
    if (pixelWhite > MAX_WHITE) pixelWhite = MAX_WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnContrastReset(wxCommandEvent& event)
{
    pixelWhite = MAX_WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessInc(wxCommandEvent& event)
{
    pixelBlack -= INC_BLACK;
    if (pixelBlack < MIN_BLACK) pixelBlack = MIN_BLACK;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessDec(wxCommandEvent& event)
{
    pixelBlack += INC_BLACK;
    if (pixelBlack >= pixelWhite) pixelBlack = pixelWhite - 1;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessReset(wxCommandEvent& event)
{
    pixelBlack = MIN_BLACK;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnAutoLevels(wxCommandEvent& event)
{
    pixelBlack = pixelMin;
    pixelWhite = pixelMax;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaInc(wxCommandEvent& event)
{
    if (pixelGamma < MAX_GAMMA) pixelGamma += INC_GAMMA;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaDec(wxCommandEvent& event)
{
    if (pixelGamma > MIN_GAMMA) pixelGamma -= INC_GAMMA;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaReset(wxCommandEvent& event)
{
    pixelGamma = 1.0;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void FocusFrame::OnExposureInc(wxCommandEvent& event)
{
    if (focusExposure < MAX_EXPOSURE) focusExposure += INC_EXPOSURE;
}
void FocusFrame::OnExposureDec(wxCommandEvent& event)
{
    if (focusExposure > MIN_EXPOSURE) focusExposure -= INC_EXPOSURE;
}
void FocusFrame::OnExposureReset(wxCommandEvent& event)
{
    focusExposure = MIN_EXPOSURE;
}
void FocusFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void FocusFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Starlight Xpress Focussing App\nCopyright (c) 2020, David Schmenk", "About SX Focus", wxOK | wxICON_INFORMATION);
}
