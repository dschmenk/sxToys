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

int ccdModel = SXCCD_MX5;
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
    bool         tdiFilter;
    wxTimer      tdiTimer;
    void OnTimer(wxTimerEvent& event);
    void OnNew(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnFilter(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};
enum
{
    ID_TIMER = 1,
    ID_FILTER,
};
wxBEGIN_EVENT_TABLE(ScanFrame, wxFrame)
    EVT_TIMER(ID_TIMER,     ScanFrame::OnTimer)
    EVT_MENU(ID_FILTER,     ScanFrame::OnFilter)
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
ScanFrame::ScanFrame() : wxFrame(NULL, wxID_ANY, "SX TDI")
{
    char statusText[40];
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_NEW, "&New\tCtrl-N");
    menuFile->Append(wxID_SAVE, "&Save...\tCtrl-S");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu *menuScan = new wxMenu;
    menuScan->AppendCheckItem(ID_FILTER, wxT("Red Filter\tR"));
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuScan, "&Scan");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    if (sxOpen(ccdModel))
        sprintf(statusText, "%cX-%d Camera Attached", sxGetModel(0) & 0x40 ? 'M' : 'H', sxGetModel(0) & 0x3F);
    else
        strcpy(statusText, "No camera attached");
    SetStatusText(statusText);
}
void ScanFrame::OnTimer(wxTimerEvent& event)
{
}
void ScanFrame::OnNew(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}
void ScanFrame::OnSave(wxCommandEvent& event)
{
    wxLogMessage("Hello world from wxWidgets!");
}
void ScanFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}
void ScanFrame::OnFilter(wxCommandEvent& event)
{
    tdiFilter = event.IsChecked();
}
void ScanFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Starlight Xpress Time Delay Integration Scanner\nCopyright (c) 2020, David Schmenk", "About SX Focus", wxOK | wxICON_INFORMATION);
}
