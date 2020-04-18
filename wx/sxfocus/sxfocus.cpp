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
#define MIN_SCALE       1.0
#define MAX_SCALE       16.0
#define INC_OFFSET      0.05
#define MIN_OFFSET      0.0
#define MAX_OFFSET      0.5
#define INC_GAMMA       0.4
#define MIN_GAMMA       0.6
#define MAX_GAMMA       4.0
#define INC_EXPOSURE    100
#define MIN_EXPOSURE    INC_EXPOSURE
#define MAX_EXPOSURE    (INC_EXPOSURE*50)
#define LUT_BITWIDTH    10
#define LUT_SIZE        (1<<LUT_BITWIDTH)
int ccdModel = SXCCD_MX5;
uint8_t redLUT[LUT_SIZE];
uint8_t blugrnLUT[LUT_SIZE];
static void calcRamp(float offset, float scale, float gamma, bool filter)
{
    int pix;
    float pixClamp;

    for (pix = 0; pix < LUT_SIZE; pix++)
    {
        pixClamp = ((float)pix/(LUT_SIZE-1) - offset) * scale;
        if (pixClamp > 1.0) pixClamp = 1.0;
        else if (pixClamp < 0.0) pixClamp = 0.0;
        redLUT[pix]    = 255.0 * pow(pixClamp, 1.0/gamma);//log10(pow((pixClamp/65535.0), 1.0/gamma)*9.0 + 1.0);
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
    unsigned int ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int ccdPixelWidth, ccdPixelHeight;
    uint16_t    *ccdFrame;
    int          focusZoom, focusExposure;
    float        pixelOffset, pixelScale, pixelGamma;
    bool         pixelFilter;
    wxTimer      focusTimer;
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
    void OnGammaInc(wxCommandEvent& event);
    void OnGammaDec(wxCommandEvent& event);
    void OnGammaReset(wxCommandEvent& event);
    void OnExposureInc(wxCommandEvent& event);
    void OnExposureDec(wxCommandEvent& event);
    void OnExposureReset(wxCommandEvent& event);
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
    ID_GAMMA_INC,
    ID_GAMMA_DEC,
    ID_GAMMA_RST,
    ID_EXPOSE_INC,
    ID_EXPOSE_DEC,
    ID_EXPOSE_RST,
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
    EVT_MENU(ID_GAMMA_INC,  FocusFrame::OnGammaInc)
    EVT_MENU(ID_GAMMA_DEC,  FocusFrame::OnGammaDec)
    EVT_MENU(ID_GAMMA_RST,  FocusFrame::OnGammaReset)
    EVT_MENU(ID_EXPOSE_INC, FocusFrame::OnExposureInc)
    EVT_MENU(ID_EXPOSE_DEC, FocusFrame::OnExposureDec)
    EVT_MENU(ID_EXPOSE_RST, FocusFrame::OnExposureReset)
    EVT_MENU(wxID_ABOUT,    FocusFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     FocusFrame::OnExit)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(FocusApp);
void FocusApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddOption(wxT("m"), wxT("model"), wxT("camera model override"), wxCMD_LINE_VAL_STRING);
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
                    ccdModel &= ~SXCCD_INTERLEAVE;
                    break;
                case 'M':
                    ccdModel |= SXCCD_INTERLEAVE;
                    break;
                default:
                    printf("Invalid model type designation.\n");
            }
            switch ((char)modelString->GetChar(2))
            {
                case '5':
                case '7':
                case '9':
                    ccdModel = (ccdModel & 0xC0) | (modelString->GetChar(2) - '0');
                    break;
                default:
                    printf("Invalid model number designation.\n");
            }
            if (toupper(modelString->GetChar(3)) == 'C')
                ccdModel |= SXCCD_COLOR;
            else
                ccdModel &= ~SXCCD_COLOR;
        }
        else
            printf("Invalid SX designation.\n");
        printf("SX model now: 0x%02X\n", ccdModel);
    }
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
    char statusText[40];
    int focusWinWidth, focusWinHeight;
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
    menuFocus->Append(ID_GAMMA_INC,  wxT("Gamma Inc\t>"));
    menuFocus->Append(ID_GAMMA_DEC,  wxT("Gamma Dec\t<"));
    menuFocus->Append(ID_GAMMA_RST,  wxT("Gamma Reset\t?"));
    menuFocus->Append(ID_EXPOSE_INC, wxT("Exposure Inc\t."));
    menuFocus->Append(ID_EXPOSE_DEC, wxT("Exposure Dec\t,"));
    menuFocus->Append(ID_EXPOSE_RST, wxT("Exposure Reset\t/"));
    menuFocus->AppendSeparator();
    menuFocus->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFocus, wxT("&Focus"));
    menuBar->Append(menuHelp, wxT("&Help"));
    SetMenuBar(menuBar);
    if (sxOpen(ccdModel))
    {
        ccdModel = sxGetModel(0);
        sxGetFrameDimensions(0, &ccdFrameWidth, &ccdFrameHeight, &ccdFrameDepth);
        sxGetPixelDimensions(0, &ccdPixelWidth, &ccdPixelHeight);
        sxClearFrame(0, SXCCD_EXP_FLAGS_FIELD_BOTH);
        ccdFrame        = (uint16_t *)malloc(sizeof(uint16_t) * ccdFrameWidth * ccdFrameHeight);
        focusTimer.StartOnce(INC_EXPOSURE);
        sprintf(statusText, "Attached: %cX-%d", ccdModel & SXCCD_INTERLEAVE ? 'M' : 'H', ccdModel & 0x3F);
    }
    else
    {
        ccdModel        = 0;
        ccdFrameWidth   = ccdFrameHeight = 512;
        ccdFrameDepth   = 16;
        ccdPixelWidth   = ccdPixelHeight = 1;
        ccdFrame        = NULL;
        strcpy(statusText, "Attached: None");
    }
    focusWinWidth   = ccdFrameWidth;
    focusWinHeight  = ccdFrameHeight * ccdPixelHeight / ccdPixelWidth; // Keep aspect ratio
    while (focusWinHeight > 720) // Constrain initial size to something reasonable
    {
        focusWinWidth  <<= 1;
        focusWinHeight <<= 1;
    }
    focusZoom       = -2;
    focusExposure   = MIN_EXPOSURE;
    pixelOffset     = MIN_OFFSET;
    pixelScale      = MIN_SCALE;
    pixelGamma      = 1.0;
    pixelFilter     = false;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
    CreateStatusBar(4);
    SetStatusText(statusText, 0);
    SetStatusText("Bin: 2x2", 1);
    SetClientSize(focusWinWidth, focusWinHeight);
}
void FocusFrame::OnTimer(wxTimerEvent& event)
{
    int zoomWidth, zoomHeight;
    int focusWinWidth, focusWinHeight;
    if (focusZoom < 1)
    {
        zoomWidth  = ccdFrameWidth  >> -focusZoom;
        zoomHeight = ccdFrameHeight >> -focusZoom;
        sxReadPixels(0, // cam idx
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
        sxReadPixels(0, // cam idx
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
    unsigned char *rgb    = ccdImage.GetData();
    uint16_t      *m16    = ccdFrame;
    uint16_t       minPix = 0xFFFF;
    uint16_t       maxPix = 0;
    int x, y;
    for (y = 0; y < zoomHeight; y++)
        for (x = 0; x < zoomWidth; x++)
        {
            if (*m16 < minPix) minPix = *m16;
            if (*m16 > maxPix) maxPix = *m16;
            rgb[0] = redLUT[*m16>>(16-LUT_BITWIDTH)];
            rgb[1] =
            rgb[2] = blugrnLUT[*m16>>(16-LUT_BITWIDTH)];
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
    sprintf(minmax, "Min: %d", minPix);
    SetStatusText(minmax, 2);
    sprintf(minmax, "Max: %d", maxPix);
    SetStatusText(minmax, 3);
    /*
     * Prep next frame
     */
    if (focusExposure < 1000)
        sxClearFrame(0, SXCCD_EXP_FLAGS_FIELD_BOTH);
    focusTimer.StartOnce(focusExposure);
}
void FocusFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnZoomIn(wxCommandEvent& event)
{
    char statusText[20];

    focusZoom++;
    if (focusZoom > MAX_ZOOM)
        focusZoom = MAX_ZOOM;
    if (focusZoom < 1)
        sprintf(statusText, "Bin: %dX%d", 1 << -focusZoom, 1 << -focusZoom);
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
        sprintf(statusText, "Bin: %dX%d", 1 << -focusZoom, 1 << -focusZoom);
    else
        sprintf(statusText, "Zoom: %dX", 1 << focusZoom);
    SetStatusText(statusText, 1);
}
void FocusFrame::OnContrastInc(wxCommandEvent& event)
{
    if (pixelScale < MAX_SCALE) pixelScale *= 2.0;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnContrastDec(wxCommandEvent& event)
{
    if (pixelScale > MIN_SCALE) pixelScale /= 2.0;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnContrastReset(wxCommandEvent& event)
{
    pixelScale = MIN_SCALE;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessInc(wxCommandEvent& event)
{
    if (pixelOffset < MAX_OFFSET) pixelOffset += INC_OFFSET;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessDec(wxCommandEvent& event)
{
    if (pixelOffset > MIN_OFFSET) pixelOffset -= INC_OFFSET;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnBrightnessReset(wxCommandEvent& event)
{
    pixelOffset = MIN_OFFSET;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaInc(wxCommandEvent& event)
{
    if (pixelGamma < MAX_GAMMA) pixelGamma += INC_GAMMA;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaDec(wxCommandEvent& event)
{
    if (pixelGamma > MIN_GAMMA) pixelGamma -= INC_GAMMA;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
}
void FocusFrame::OnGammaReset(wxCommandEvent& event)
{
    pixelGamma = 1.0;
    calcRamp(pixelOffset, pixelScale, pixelGamma, pixelFilter);
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
