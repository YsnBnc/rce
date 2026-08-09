#include <filesystem>
#include <iostream>
#include <thread>
#include <wx/simplebook.h>
#include <wx/wx.h>
#include <streambuf>
#include "utilities/bridge.h"
using namespace std;

int PORT;
char TARGET_IP[16];
std::string COMMAND;
std::string PATH_TO_FILE;

class TerminalOutputCatch : public std::streambuf  {
public:
    explicit TerminalOutputCatch(wxTextCtrl* ctrl) : m_ctrl(ctrl){}
protected:
    //Single character output
    int_type overflow(int_type __c) override {
        if (__c != EOF) {
            char ch = static_cast<char>(__c);
            wxTheApp->CallAfter([this, ch]() {
               if (m_ctrl) {
                   m_ctrl->AppendText(wxString(ch));
                   m_ctrl->ShowPosition(m_ctrl->GetLastPosition());
               }
            });
        }
        return __c;
    }
    //String or buffer output
    std::streamsize xsputn(const char_type* __s, std::streamsize __n) override {
        wxString str(__s, __n);
        wxTheApp->CallAfter([this, str]() {
            if (m_ctrl) {
                m_ctrl->AppendText(str);
                m_ctrl->ShowPosition(m_ctrl->GetLastPosition());
            }
        });
        return __n;
    }
private:
    wxTextCtrl* m_ctrl;
};

class RCE_App : public wxApp
{
public:
    bool OnInit() override;
};

wxIMPLEMENT_APP(RCE_App);
class RCE_Frame : public wxFrame
{
public:
    enum
    {
        ID_SERVER_SIDE = 1001,
        ID_CLIENT_SIDE = 1002,
    };

    RCE_Frame():wxFrame(nullptr, wxID_ANY, "Remote Code Execution", wxDefaultPosition)
    {
        wxSize fixedSize(700, 400); //Lock the window size
        wxTopLevelWindowBase::SetMinSize(fixedSize);
        wxTopLevelWindowBase::SetMaxSize(fixedSize);

        m_book = new wxSimplebook(this, wxID_ANY); //Page container

        //Permanent panels
        initServerPanel();
        initClientPanel();

        wxMenu *menuView = new wxMenu();
        menuView->Append(ID_SERVER_SIDE, wxString("&Server Side"));
        menuView->Append(ID_CLIENT_SIDE, wxString("&Client Side"));

        wxMenuBar *menu_bar = new wxMenuBar();
        menu_bar->Append(menuView, wxString("&Sides"));
        wxFrameBase::SetMenuBar(menu_bar);

        wxFrameBase::CreateStatusBar();
        wxFrameBase::SetStatusText("Ready");

        Bind(wxEVT_MENU, &RCE_Frame::OnServerSide, this, ID_SERVER_SIDE);
        Bind(wxEVT_MENU, &RCE_Frame::OnClientSide, this, ID_CLIENT_SIDE);
    }

    ~RCE_Frame() {
        std::cout.rdbuf(oldCoutBuffer);
        std::cerr.rdbuf(oldCerrBuffer);
        delete streamBuffer;
    }

private:
    wxSimplebook* m_book = nullptr;
    wxPanel* serverPanel = nullptr;
    wxPanel* clientPanel = nullptr;
    wxTextCtrl* ipInput = nullptr;
    wxTextCtrl* portInput = nullptr;
    wxTextCtrl* terminalOutput = nullptr;
    TerminalOutputCatch* streamBuffer = nullptr;
    wxButton *browseBtn = nullptr;
    wxButton *srv_executeBtn = nullptr;
    wxButton *clt_executeBtn = nullptr;
    std::streambuf* oldCoutBuffer = nullptr;
    std::streambuf* oldCerrBuffer = nullptr;
    OPENFILENAME file_to_open;

    void initServerPanel() {
        serverPanel = new wxPanel(m_book);
        new wxStaticText(serverPanel, ID_SERVER_SIDE, "PORT:", wxPoint(20,20), wxSize(35,15));
        portInput = new wxTextCtrl(serverPanel, ID_SERVER_SIDE, "",wxPoint(60,20), wxSize(50,20));

        clt_executeBtn = new wxButton(serverPanel, ID_SERVER_SIDE, wxString("EXECUTE"), wxPoint(20,140), wxSize(80,20));
        clt_executeBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onServerExecuteClicked, this);
        terminalCatch(serverPanel, ID_SERVER_SIDE);
        m_book->AddPage(serverPanel,"Server Side", true);
    }

    void initClientPanel() {
        clientPanel = new wxPanel(m_book);
        new wxStaticText(clientPanel, ID_CLIENT_SIDE, "PORT:", wxPoint(20,20), wxSize(35,15));
        new wxStaticText(clientPanel, ID_CLIENT_SIDE, "SERVER IP:",wxPoint(20,60), wxSize(63,15));

        browseBtn = new wxButton(clientPanel, ID_CLIENT_SIDE, wxString("BROWSE"), wxPoint(20,100), wxSize(80,20));
        srv_executeBtn = new wxButton(clientPanel, ID_CLIENT_SIDE, wxString("EXECUTE"), wxPoint(20,140), wxSize(80,20));

        browseBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onBrowseClicked, this);
        srv_executeBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onClientExecuteClicked, this);

        //TODO: These must have clamp of some sort
        portInput = new wxTextCtrl(clientPanel, ID_CLIENT_SIDE, "",wxPoint(60,20), wxSize(50,20));
        ipInput = new wxTextCtrl(clientPanel, ID_CLIENT_SIDE, "",wxPoint(86,60), wxSize(90,20));
        terminalCatch(clientPanel, ID_CLIENT_SIDE);
        m_book->AddPage(clientPanel,"Client Side", true);
    }
    void OnServerSide(wxCommandEvent& event) {
        m_book->SetSelection(0);
        SetStatusText("Server Side View");
    }
    void OnClientSide(wxCommandEvent& event) {
        m_book->SetSelection(1);
        SetStatusText("Client Side View");
    }
    void onBrowseClicked(wxCommandEvent& event) {
        file_to_open = {0};
        WCHAR fileBuffer[MAX_PATH] = L"";
        std::wstring path;

        file_to_open.lStructSize = sizeof(file_to_open);
        file_to_open.lpstrFile = fileBuffer;
        file_to_open.nMaxFile = MAX_PATH;
        file_to_open.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        if (GetOpenFileNameW(&file_to_open)) {
            path.assign(fileBuffer);
            std::filesystem::path p(path);
            std::string narrowPath = p.string();
            PATH_TO_FILE = narrowPath;
            std::cout <<"Selected file: " + narrowPath << std::endl;
        }
    }
    void onClientExecuteClicked(wxCommandEvent& event) {
        long temp = 0;
        wxString ip = ipInput->GetValue();
        wxString port = portInput->GetValue();

        snprintf(TARGET_IP, sizeof(TARGET_IP), "%s", (const char*) ip.mb_str());
        if (port.ToLong(&temp)) {
            PORT = static_cast<int>(temp);
        }
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            client_side(PORT,TARGET_IP);
        }).detach();
    }
    void onServerExecuteClicked(wxCommandEvent& event) {
        long temp = 0;
        wxString port = portInput->GetValue();
        if (port.ToLong(&temp)) {
            PORT = static_cast<int>(temp);
        }
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            std::cout << "Server Side Started" << std::endl;
            server_side(PORT);
        }).detach();
    }
    void terminalCatch(wxPanel* panel, wxWindowID ID) {
        terminalOutput = new wxTextCtrl(panel, ID, "", wxPoint(180,15), wxSize(490,300),wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2);
        terminalOutput->SetForegroundColour(wxColour(54, 69, 79));
        terminalOutput->SetFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));

        //Redirect cout and cerr stream buffers
        streamBuffer = new TerminalOutputCatch(terminalOutput);
        oldCoutBuffer = std::cout.rdbuf(streamBuffer);
        oldCerrBuffer = std::cerr.rdbuf(streamBuffer);
    }
};

bool RCE_App::OnInit()
{
    RCE_Frame *frame = new RCE_Frame();
    frame->Show(true);
    return true;
}
