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
#include <wx/choicdlg.h>
#include <wx/numdlg.h>
#include <wx/progdlg.h>
#include <wx/filedlg.h>
#include "sxsnap.h"
#define MAX_SNAPSHOTS   100
#define MIN_EXPOSURE    1
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
long     initialCount    = 1;
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
    unsigned int   ccdFrameWidth, ccdFrameHeight, ccdFrameDepth;
    unsigned int   ccdBinWidth, ccdBinHeight, ccdBinX, ccdBinY, ccdPixelCount;
    double         ccdPixelWidth, ccdPixelHeight;
    wxString       snapFilePath;
    wxString       snapBaseName;
    int            snapExposure, snapCount, snapDelay,  snapView, snapMax;
    wxImage       *snapImage;
    uint16_t      *snapShots[MAX_SNAPSHOTS];
    bool           snapSaved[MAX_SNAPSHOTS];
    int            pixelMax, pixelMin;
    int            pixelBlack, pixelWhite;
    float          pixelGamma;
    bool           pixelFilter, autoLevels;
    wxStopWatch   *snapWatch;
    void InitLevels();
    bool ConnectCamera(int index);
    void CenterCentroid(float x, float y, int width, int height);
    void OnBackground(wxEraseEvent& event);
    void OnPaint(wxPaintEvent& event);
    void SnapStatus();
    void OnConnect(wxCommandEvent& event);
    void OnOverride(wxCommandEvent& event);
    bool AreSaved();
    void FreeShots();
    void OnNew(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAll(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnAutoLevels(wxCommandEvent& event);
    void OnGamma(wxCommandEvent& event);
    void UpdateView(int view);
    void OnForward(wxCommandEvent& event);
    void OnBackward(wxCommandEvent& event);
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
    ID_CONNECT = 1,
    ID_OVERRIDE,
    ID_DELETE,
    ID_SAVE_ALL,
    ID_FILTER,
    ID_LEVEL_AUTO,
    ID_GAMMA,
    ID_FORWARD,
    ID_BACKWARD,
    ID_START,
    ID_NUMBER,
    ID_EXPOSURE,
    ID_DELAY,
    ID_BINX,
    ID_BINY,
};
wxBEGIN_EVENT_TABLE(SnapFrame, wxFrame)
    EVT_MENU(ID_CONNECT,    SnapFrame::OnConnect)
    EVT_MENU(ID_OVERRIDE,   SnapFrame::OnOverride)
    EVT_MENU(ID_DELETE,     SnapFrame::OnDelete)
    EVT_MENU(ID_SAVE_ALL,   SnapFrame::OnSaveAll)
    EVT_MENU(ID_FILTER,     SnapFrame::OnFilter)
    EVT_MENU(ID_LEVEL_AUTO, SnapFrame::OnAutoLevels)
    EVT_MENU(ID_GAMMA,      SnapFrame::OnGamma)
    EVT_MENU(ID_FORWARD,    SnapFrame::OnForward)
    EVT_MENU(ID_BACKWARD,   SnapFrame::OnBackward)
    EVT_MENU(ID_START,      SnapFrame::OnStart)
    EVT_MENU(ID_NUMBER,     SnapFrame::OnNumber)
    EVT_MENU(ID_EXPOSURE,   SnapFrame::OnExposure)
    EVT_MENU(ID_DELAY,      SnapFrame::OnDelay)
    EVT_MENU(ID_BINX,       SnapFrame::OnBinX)
    EVT_MENU(ID_BINY,       SnapFrame::OnBinY)
    EVT_MENU(wxID_NEW,      SnapFrame::OnNew)
    EVT_MENU(wxID_SAVE,     SnapFrame::OnSave)
    EVT_MENU(wxID_ABOUT,    SnapFrame::OnAbout)
    EVT_MENU(wxID_EXIT,     SnapFrame::OnExit)
    EVT_ERASE_BACKGROUND(   SnapFrame::OnBackground)
    EVT_PAINT(              SnapFrame::OnPaint)
    EVT_CLOSE(              SnapFrame::OnClose)
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
SnapFrame::SnapFrame() : wxFrame(NULL, wxID_ANY, "SX SnapShot")
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
    menuView->Append(ID_FORWARD,             wxT("&Next Image\tRIGHT"));
    menuView->Append(ID_BACKWARD,            wxT("&Previous Image\tLEFT"));
    wxMenu *menuImage = new wxMenu;
    menuImage->Append(ID_START,    wxT("Start...\tSPACE"));
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
    snapCount    = initialCount;
    pixelFilter  = false;
    pixelGamma   = 1.5;
    snapImage    = NULL;
    snapView     = 0;
    snapMax      = 0;
    memset(snapShots, 0, sizeof(uint16_t) * MAX_SNAPSHOTS);
    snapExposure = initialExposure;
    snapDelay    = initialDelay;
    ccdBinX      = initialBinX;
    ccdBinY      = initialBinY;
    camCount     = sxProbe(camHandles, camParams, camUSBType);
    InitLevels();
    ConnectCamera(initialCamIndex);
}
void SnapFrame::InitLevels()
{
    pixelGamma = 1.5;
    pixelMin   = MAX_WHITE;
    pixelMax   = MIN_BLACK;
    pixelBlack = MIN_BLACK;
    pixelWhite = MAX_WHITE;
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
}
void SnapFrame::OnBackground(wxEraseEvent& WXUNUSED(event))
{
}
void SnapFrame::OnPaint(wxPaintEvent& WXUNUSED(event))
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
void SnapFrame::SnapStatus()
{
    char statusText[40];
    if (camCount)
        sprintf(statusText, "Attached: %cX-%d[%d]", ccdModel & SXCCD_INTERLEAVE ? 'M' : 'H', ccdModel & 0x3F, camSelect);
    else
        strcpy(statusText, "Attached: None");
    SetStatusText(statusText, 0);
    if (snapMax)
        sprintf(statusText, "%d/%d@%d,%d", snapView + 1, snapMax, snapExposure, snapDelay);
    else
        sprintf(statusText, "-/-@%d,%d", snapExposure, snapDelay);
    SetStatusText(statusText, 1);
    sprintf(statusText, "Bin: %d:%d", ccdBinX, ccdBinY);
    SetStatusText(statusText, 2);
}
bool SnapFrame::ConnectCamera(int index)
{
    int snapWinWidth, snapWinHeight;
    InitLevels();
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
    }
    else
    {
        camSelect     = -1;
        ccdModel      = 0;
        ccdFrameWidth = ccdFrameHeight = 512;
        ccdFrameDepth = 16;
        ccdPixelWidth = ccdPixelHeight = 1;
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
    SnapStatus();
    return camSelect >= 0;
}
void SnapFrame::OnConnect(wxCommandEvent& WXUNUSED(event))
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
        ConnectCamera(dlg.GetSelection());
    else if (camSelect)
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
}
void SnapFrame::OnOverride(wxCommandEvent& WXUNUSED(event))
{
#ifndef _MSC_VER
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
    }
#endif
}
bool SnapFrame::AreSaved()
{
    for (int i = 0; i < snapMax; i++)
        if (snapShots[i] && !snapSaved[i])
            return false;
    return true;
}
void SnapFrame::FreeShots()
{
    for (int i = 0; i < snapMax; i++)
    {
        if (snapShots[i])
        {
            free(snapShots[i]);
            snapShots[i] = NULL;
        }
        snapView = snapMax = 0;
    }
    if (snapImage != NULL)
    {
        delete snapImage;
        snapImage = NULL;
    }
}
void SnapFrame::OnNew(wxCommandEvent& event)
{
    /*
     * Any unsaved images?
     */
    if (!AreSaved())
        if (wxMessageBox("Save images before clearing?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxYES)
        {
            wxCommandEvent eventSaveAll;
            OnSaveAll(eventSaveAll);
        }
    /*
     * Free up the images
     */
    FreeShots();
    UpdateView(0);
    SnapStatus();
}
void SnapFrame::OnDelete(wxCommandEvent& event)
{
    if (snapShots[snapView] != NULL)
        free(snapShots[snapView]);
    for (int i = snapView; i < snapMax - 1; i++)
    {
        snapShots[i] = snapShots[i + 1];
        snapSaved[i] = snapSaved[i + 1];
    }
    snapMax--;
    snapShots[snapMax] = NULL;
    snapSaved[snapMax] = true;
    UpdateView(snapView);
    SnapStatus();
}
void SnapFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
    if (snapShots[snapView] != NULL)
    {
        char filename[255];
        wxFileDialog dlg(this, wxT("Save Image"), snapFilePath, snapBaseName, wxT("*.fits"/*"FITS file (*.fits)"*/), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() == wxID_OK)
        {
            snapFilePath  = dlg.GetPath();
            snapBaseName  = dlg.GetFilename();
            strcpy(filename, snapFilePath.c_str());
            if (fits_open(filename))                                                          {fits_cleanup(); return;}
            if (fits_write_image(snapShots[snapView], ccdBinWidth, ccdBinHeight))             {fits_cleanup(); return;}
            if (fits_write_key_int("EXPOSURE", snapExposure, "Total Exposure Time"))          {fits_cleanup(); return;}
            if (fits_write_key_string("CREATOR", "sxSnapShot", "Imaging Application"))        {fits_cleanup(); return;}
            if (fits_write_key_string("CAMERA", "StarLight Xpress Camera", "Imaging Device")) {fits_cleanup(); return;}
            if (fits_close())                                                                 {fits_cleanup(); return;}
            snapSaved[snapView] = true;
        }
    }
    else
        wxMessageBox("No image to save", "Save Error", wxOK | wxICON_INFORMATION);
    SnapStatus();
}
void SnapFrame::OnSaveAll(wxCommandEvent& event)
{
    if (snapMax)
    {
        char basename[255];
        wxFileDialog dlg(this, wxT("Save Images"), snapFilePath, snapBaseName, wxT("*"/*"FITS file (*.fits)"*/), wxFD_SAVE);
        if (dlg.ShowModal() == wxID_OK)
        {
            snapFilePath  = dlg.GetPath();
            snapBaseName  = dlg.GetFilename();
            strcpy(basename, snapFilePath.c_str());
            for (int i = 0; i < snapMax; i++)
            {
                if (!snapSaved[i])
                {
                    char filename[255];
                    sprintf(filename, "%s-%02d.fits", basename, i);
                    if (fits_open(filename))                                                          {fits_cleanup(); return;}
                    if (fits_write_image(snapShots[i], ccdBinWidth, ccdBinHeight))                    {fits_cleanup(); return;}
                    if (fits_write_key_int("EXPOSURE", snapExposure, "Total Exposure Time"))          {fits_cleanup(); return;}
                    if (fits_write_key_string("CREATOR", "sxSnapShot", "Imaging Application"))        {fits_cleanup(); return;}
                    if (fits_write_key_string("CAMERA", "StarLight Xpress Camera", "Imaging Device")) {fits_cleanup(); return;}
                    if (fits_close())                                                                 {fits_cleanup(); return;}
                    snapSaved[i] = true;
                }
            }
        }
        SnapStatus();
    }
    else
        wxMessageBox("No images to save", "Save Error", wxOK | wxICON_INFORMATION);
}
void SnapFrame::UpdateView(int view)
{
    if (view >= snapMax)
        view = snapMax - 1;
    if (view < 0)
        view = 0;
    snapView = view;
    if (!snapImage)
        return;
    /*
     * Convert 16 bit samples to 24 BPP image
     */
    unsigned char *rgb = snapImage->GetData();
    uint16_t      *m16 = snapShots[snapView];
    pixelMin = MAX_PIX;
    pixelMax = MIN_PIX;
    for (int l = 0; l < ccdBinWidth * ccdBinHeight; l++)
    {
        if (*m16 < pixelMin) pixelMin = *m16;
        if (*m16 > pixelMax) pixelMax = *m16;
        rgb[0] = redLUT[LUT_INDEX(*m16)];
        rgb[1] =
        rgb[2] = blugrnLUT[LUT_INDEX(*m16)];
        rgb   += 3;
        m16++;
    }
    if (autoLevels)
    {
        pixelBlack = pixelMin;
        pixelWhite = pixelMax;
        calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
    }
    int winWidth, winHeight;
    GetClientSize(&winWidth, &winHeight);
    if (winWidth > 0 && winHeight > 0)
    {
        wxClientDC dc(this);
        wxBitmap bitmap(snapImage->Scale(winWidth, winHeight, wxIMAGE_QUALITY_BILINEAR));
        Refresh();
    }
}
void SnapFrame::OnFilter(wxCommandEvent& event)
{
    pixelFilter = event.IsChecked();
    calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
    UpdateView(snapView);
}
void SnapFrame::OnAutoLevels(wxCommandEvent& event)
{
    autoLevels = event.IsChecked();
    UpdateView(snapView);
}
void SnapFrame::OnGamma(wxCommandEvent& WXUNUSED(event))
{
    wxSingleChoiceDialog dlg(this,
                          wxT("Gamma:"),
                          wxT("Gamma Value"),
                          3,
                          GammaChoices);
    if (dlg.ShowModal() == wxID_OK )
    {
        pixelGamma =  GammaValues[dlg.GetSelection()];
        calcRamp(pixelBlack, pixelWhite, pixelGamma, pixelFilter);
        UpdateView(snapView);
    }
}
void SnapFrame::OnForward(wxCommandEvent& WXUNUSED(event))
{
    UpdateView(snapView + 1);
    SnapStatus();
}
void SnapFrame::OnBackward(wxCommandEvent& WXUNUSED(event))
{
    UpdateView(snapView - 1);
    SnapStatus();
}
void SnapFrame::OnStart(wxCommandEvent& WXUNUSED(event))
{
    wxString progress;
    int i;
    if (!AreSaved())
        if (wxMessageBox("Save images before overwriting?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxYES)
        {
            wxCommandEvent eventSaveAll;
            OnSaveAll(eventSaveAll);
        }
    FreeShots();
    ccdBinWidth  = ccdFrameWidth  / ccdBinX;
    ccdBinHeight = ccdFrameHeight / ccdBinY;
    snapImage    = new wxImage(ccdBinWidth, ccdBinHeight);
    int pixCount = ccdBinWidth * ccdBinHeight;
    long timeDelta, timeElapsed, timeTotal = snapCount * (snapDelay + snapExposure) - snapDelay;
    progress = wxString::Format(wxT("Integrating Image 0 of %d..."), snapCount);
    wxProgressDialog dlg(wxT("SnapShot Progress"),
                         progress,
                         timeTotal,
                         this,
                         wxPD_CAN_ABORT
                      // | wxPD_APP_MODAL
                       | wxPD_ELAPSED_TIME
                       | wxPD_REMAINING_TIME);
    for (i = 0; i < snapMax; i++)
    {
        if (snapShots[i])
            free(snapShots[i]);
        snapShots[i]  = NULL;
        snapSaved[i] = true;
    }
    timeElapsed = 0;
    dlg.Update(0);
    ENABLE_HIGH_RES_TIMER();
    for (i = 0; i < snapCount; i++)
    {
        wxStopWatch watch;
        /*
         * Init frame.
         */
        snapShots[i] = (uint16_t *)malloc(sizeof(uint16_t) * pixCount);
        if (snapDelay && i)
        {
            watch.Start();
            while ((timeDelta = snapDelay - watch.Time()) > 1000)
            {
                wxMilliSleep(1000);
                timeElapsed += 1000;
                if (!dlg.Update(timeElapsed))
                    goto cancelled;
            }
            if (timeDelta > 0)
            {
                wxMilliSleep(timeDelta);
                timeElapsed += timeDelta;
            }
        }
        progress = wxString::Format(wxT("Integrating Image %d of %d..."), i + 1, snapCount);
        if (!dlg.Update(timeElapsed, progress))
            goto cancelled;
        sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_FIELD_BOTH, SXCCD_IMAGE_HEAD);
        watch.Start();
        while ((timeDelta = snapExposure - watch.Time()) > 1500)
        {
            /*
             * Clear registers every second.
             */
            wxMilliSleep(1000);
            sxClearImage(camHandles[camSelect], SXCCD_EXP_FLAGS_NOCLEAR_FRAME, SXCCD_IMAGE_HEAD);
            /*
             * Allow dialog feedback.
             */
            timeElapsed += 1000;
            if (!dlg.Update(timeElapsed))
                goto cancelled;
        }
        if (timeDelta > 0)
        {
            wxMilliSleep(timeDelta);
            timeElapsed += timeDelta;
        }
        sxLatchImage(camHandles[camSelect], // cam handle
                     SXCCD_EXP_FLAGS_FIELD_BOTH, // options
                     SXCCD_IMAGE_HEAD, // main ccd
                     0, // xoffset
                     0, // yoffset
                     ccdFrameWidth, // width
                     ccdFrameHeight, // height
                     ccdBinX, // xbin
                     ccdBinY); // ybin
        if (!sxReadImage(camHandles[camSelect], // cam handle
                         snapShots[i], //pixbuf
                         pixCount)) // pix count
        {
            wxMessageBox("Camera Error", "SX SnapShot", wxOK | wxICON_INFORMATION);
            goto cancelled;
        }
        snapMax = i + 1;
        UpdateView(i);
        SnapStatus();
        snapSaved[i] = false;
        if (!dlg.Update(timeElapsed))
            goto cancelled;
    }
cancelled:
    DISABLE_HIGH_RES_TIMER();
    //dlg.Close();
}
void SnapFrame::OnNumber(wxCommandEvent& WXUNUSED(event))
{
    wxNumberEntryDialog dlg(this,
							wxT(""),
							wxT("Images:"),
							wxT("Number of Snap Shots"),
							1, 1, 100);
	if (dlg.ShowModal() != wxID_OK)
		return;
	snapCount = dlg.GetValue();
}
void SnapFrame::OnExposure(wxCommandEvent& WXUNUSED(event))
{
    wxNumberEntryDialog dlg(this,
							wxT(""),
							wxT("MSecs:"),
							wxT("Exposure"),
							snapExposure, 1, 3600000*12);
	if (dlg.ShowModal() != wxID_OK)
		return;
	snapExposure = dlg.GetValue();
    SnapStatus();
}
void SnapFrame::OnDelay(wxCommandEvent& WXUNUSED(event))
{
    wxNumberEntryDialog dlg(this,
							wxT(""),
							wxT("MSecs:"),
							wxT("Exposure"),
							0, 0, 60000);
	if (dlg.ShowModal() != wxID_OK)
		return;
	snapDelay = dlg.GetValue();
    SnapStatus();
}
void SnapFrame::OnBinX(wxCommandEvent& WXUNUSED(event))
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
    SnapStatus();
}
void SnapFrame::OnBinY(wxCommandEvent& WXUNUSED(event))
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
    SnapStatus();
}
void SnapFrame::OnClose(wxCloseEvent& event)
{
    if (!AreSaved())
        if (wxMessageBox("Save images before exiting?", "Exit Warning", wxYES_NO | wxICON_INFORMATION) == wxYES)
        {
            wxCommandEvent eventSaveAll;
            OnSaveAll(eventSaveAll);
        }
    FreeShots();
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
