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
#include "sxtdi.h"
#define ALIGN_EXP       1000
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
uint8_t redLUT[LUT_SIZE];
uint8_t blugrnLUT[LUT_SIZE];
static void calcRamp(int black, int white, float gamma, bool filter)
{
    int pix, offset;
    float scale, recipg, pixClamp;

    offset = LUT_INDEX(black);
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
private:
    unsigned int ccdFrameX, ccdFrameY, ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int ccdPixelWidth, ccdPixelHeight;
    uint16_t    *ccdFrame;
    int          pixelMax, pixelMin;
    int          pixelBlack, pixelWhite;
    float        pixelGamma;
    bool         pixelFilter;
    int          tdiWinWidth, tdiWinHeight, tdiZoom;
    int          tdiExposure;
    wxImage     *alignImage;
    wxTimer      tdiTimer;
    void OnTimer(wxTimerEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnAlign(wxCommandEvent& event);
    void OnScan(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};
enum
{
    ID_TIMER = 1,
    ID_FILTER,
    ID_ALIGN,
    ID_SCAN,
    ID_STOP,
};
wxBEGIN_EVENT_TABLE(ScanFrame, wxFrame)
    EVT_TIMER(ID_TIMER,     ScanFrame::OnTimer)
    EVT_MENU(ID_FILTER,     ScanFrame::OnFilter)
    EVT_MENU(ID_ALIGN,      ScanFrame::OnAlign)
    EVT_MENU(ID_SCAN,       ScanFrame::OnScan)
    EVT_MENU(ID_STOP,       ScanFrame::OnStop)
    EVT_MENU(wxID_NEW,      ScanFrame::OnNew)
    EVT_MENU(wxID_SAVE,     ScanFrame::OnSave)
    EVT_MENU(wxID_ABOUT,    ScanFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     ScanFrame::OnExit)
wxEND_EVENT_TABLE()
wxIMPLEMENT_APP(ScanApp);
void ScanApp::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddOption(wxT("m"), wxT("model"), wxT("camera model override"), wxCMD_LINE_VAL_STRING);
}
bool ScanApp::OnCmdLineParsed(wxCmdLineParser &parser)
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
bool ScanApp::OnInit()
{
    if (wxApp::OnInit())
    {
        ScanFrame *frame = new ScanFrame();
        frame->Show(true);
        return true;
    }
    return false;
}
ScanFrame::ScanFrame() : wxFrame(NULL, wxID_ANY, "SX TDI"), tdiTimer(this, ID_TIMER)
{
    char statusText[40];
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, "&New\tCtrl-N");
    menuFile->Append(wxID_SAVE, "&Save...\tCtrl-S");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuScan = new wxMenu;
    menuScan->AppendCheckItem(ID_FILTER, wxT("&Red Filter\tR"));
    menuScan->Append(ID_ALIGN, "&Align\tA");
    menuScan->Append(ID_SCAN, "&TDI Scan\tT");
    menuScan->Append(ID_STOP, "S&top\tCtrl-T");
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuScan, "&Scan");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    tdiExposure = 100; // 0.1 sec
    pixelMin    = MAX_WHITE;
    pixelMax    = MIN_BLACK;
    pixelBlack  = MIN_BLACK;
    pixelWhite  = MAX_WHITE;
    pixelGamma  = 1.0;
    pixelFilter = false;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
    if (sxOpen(ccdModel))
    {
        ccdModel = sxGetModel(0);
        sxGetFrameDimensions(0, &ccdFrameWidth, &ccdFrameHeight, &ccdFrameDepth);
        sxGetPixelDimensions(0, &ccdPixelWidth, &ccdPixelHeight);
        ccdFrame        = (uint16_t *)malloc(sizeof(uint16_t) * ccdFrameWidth * ccdFrameHeight);
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
    tdiWinHeight = ccdFrameWidth; // Swap width/height
    tdiWinWidth  = ccdFrameHeight;
    tdiZoom      = 1;
    while (tdiWinHeight > 720) // Constrain initial size to something reasonable
    {
        tdiWinWidth  <<= 1;
        tdiWinHeight <<= 1;
        tdiZoom++;
    }
    CreateStatusBar(2);
    SetStatusText(statusText, 0);
    SetStatusText("Rate: 0.1 row/s", 1);
    SetClientSize(tdiWinWidth, tdiWinHeight);
}
void ScanFrame::OnTimer(wxTimerEvent& event)
{
    sxReadPixels(0, // cam idx
                 SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                 0, // xoffset
                 0, // yoffset
                 ccdFrameWidth, // width
                 ccdFrameHeight, // height
                 1, // xbin
                 1, // ybin
                 (unsigned char *)ccdFrame); //pixbuf
    calcRamp(pixelMin, pixelMax, pixelGamma, pixelFilter);
    pixelMin = MAX_PIX;
    pixelMax = MIN_PIX;
    unsigned char *rgb    = alignImage->GetData();
    uint16_t      *m16;
    for (int y = 0; y < ccdFrameWidth; y++) // Rotate image 90 degrees counterclockwise as it gets copied
    {
        m16 = &ccdFrame[ccdFrameWidth - y - 1];
        for (int x = 0; x < ccdFrameHeight; x++)
        {
            if (*m16 < pixelMin) pixelMin = *m16;
            if (*m16 > pixelMax) pixelMax = *m16;
            rgb[0] = max(rgb[0], redLUT[LUT_INDEX(*m16)]);
            rgb[1] = max(rgb[1], blugrnLUT[LUT_INDEX(*m16)]);
            rgb[2] = max(rgb[2], blugrnLUT[LUT_INDEX(*m16)]);
            rgb   += 3;
            m16   += ccdFrameWidth;
        }
    }
    GetClientSize(&tdiWinWidth, &tdiWinHeight);
    if (tdiWinWidth > 0 && tdiWinHeight > 0)
    {
        wxClientDC dc(this);
        wxBitmap bitmap(alignImage->Scale(tdiWinWidth, tdiWinHeight, wxIMAGE_QUALITY_BILINEAR));
        dc.DrawBitmap(bitmap, 0, 0);
    }
}
void ScanFrame::OnNew(wxCommandEvent& event)
{
    wxLogMessage("New scan");
}
void ScanFrame::OnSave(wxCommandEvent& event)
{
    wxLogMessage("Save Scan");
}
void ScanFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void ScanFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
}
void ScanFrame::OnAlign(wxCommandEvent& event)
{
    if (ccdModel)
    {
        sxClearFrame(0, SXCCD_EXP_FLAGS_FIELD_BOTH);
        tdiTimer.Start(ALIGN_EXP);
        alignImage = new wxImage(ccdFrameHeight, ccdFrameWidth, true);
    }
}
void ScanFrame::OnScan(wxCommandEvent& event)
{
    tdiTimer.Start(tdiExposure);
}
void ScanFrame::OnStop(wxCommandEvent& event)
{
    if (tdiTimer.IsRunning())
    {
        printf("Stopping timer\n");
        tdiTimer.Stop();
        delete alignImage;
    }
    else
        printf("Timer not running\n");

}
void ScanFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Starlight Xpress Time Delay Integration Scanner\nCopyright (c) 2020, David Schmenk", "About SX Focus", wxOK | wxICON_INFORMATION);
}
