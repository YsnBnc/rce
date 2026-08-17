#include "utilities/bridge.h"
#include <filesystem>
#include <iostream>
#include <streambuf>
#include <thread>
#include <wx/simplebook.h>
#include <wx/wx.h>
using namespace std;

int PORT;
char TARGET_IP[16];
std::string PATH_TO_FILE;

class TerminalOutputCatch : public std::streambuf {
public:
  explicit TerminalOutputCatch(wxTextCtrl *initialTarget = nullptr) {
    if (initialTarget)
      m_targets.push_back(initialTarget);
  }
  void AddTarget(wxTextCtrl *target) {
    if (target)
      m_targets.push_back(target);
  }

protected:
  // Single character output
  int_type overflow(int_type _c) override {
    if (_c != EOF) {
      char ch = static_cast<char>(_c);
      for (auto *ctrl : m_targets) {
        wxTheApp->CallAfter([ctrl, ch]() { ctrl->AppendText(ch); });
      }
    }
    return _c;
  }

  // String/buffer output
  std::streamsize xsputn(const char *_s, std::streamsize _n) override {
    wxString text(_s, _n);
    for (auto *ctrl : m_targets) {
      wxTheApp->CallAfter([ctrl, text]() { ctrl->AppendText(text); });
    }
    return _n;
  }

private:
  std::vector<wxTextCtrl *> m_targets;
};

class RCE_App : public wxApp {
public:
  bool OnInit() override;
};

wxIMPLEMENT_APP(RCE_App);

class RCE_Frame : public wxFrame {
public:
  enum {
    ID_SERVER_SIDE = 1001,
    ID_CLIENT_SIDE = 1002,
  };
  RCE_Frame()
      : wxFrame(nullptr, wxID_ANY, "Remote Code Execution", wxDefaultPosition) {
    wxSize fixedSize(700, 400); // Lock the window size
    wxTopLevelWindowBase::SetMinSize(fixedSize);
    wxTopLevelWindowBase::SetMaxSize(fixedSize);

    m_book = new wxSimplebook(this, wxID_ANY); // Page container

    // Permanent panels
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

  ~RCE_Frame() override {
    std::cout.rdbuf(oldCoutBuffer);
    std::cerr.rdbuf(oldCerrBuffer);
    delete streamBuffer;
  }

private:
  wxSimplebook *m_book = nullptr;
  wxPanel *serverPanel = nullptr;
  wxPanel *clientPanel = nullptr;
  wxTextCtrl *ipInput = nullptr;
  wxTextCtrl *client_portInput = nullptr;
  wxTextCtrl *server_portInput = nullptr;
  wxTextCtrl *commandInput = nullptr;
  wxTextCtrl *terminalOutput = nullptr;
  TerminalOutputCatch *streamBuffer = nullptr;
  wxButton *browseBtn = nullptr;
  wxButton *srv_executeBtn = nullptr;
  wxButton *clt_executeBtn = nullptr;
  std::streambuf *oldCoutBuffer = nullptr;
  std::streambuf *oldCerrBuffer = nullptr;
  server_class server;
  client_class client;
#ifdef _WIN32
  OPENFILENAME file_to_open{};
#endif

  void initServerPanel() {
    serverPanel = new wxPanel(m_book);
    new wxStaticText(serverPanel, ID_SERVER_SIDE, "PORT:", wxPoint(20, 20),
                     wxSize(35, 15));
    server_portInput = new wxTextCtrl(serverPanel, ID_SERVER_SIDE, "",
                                      wxPoint(60, 20), wxSize(50, 20));
    srv_executeBtn =
        new wxButton(serverPanel, ID_SERVER_SIDE, wxString("EXECUTE"),
                     wxPoint(20, 70), wxSize(80, 20));
    srv_executeBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onServerExecuteClicked,
                         this);
    terminalCatch(serverPanel, ID_SERVER_SIDE);
    m_book->AddPage(serverPanel, "Server Side", true);
  }

  void initClientPanel() {
    clientPanel = new wxPanel(m_book);
    new wxStaticText(clientPanel, ID_CLIENT_SIDE, "PORT:", wxPoint(20, 20),
                     wxSize(35, 15));
    new wxStaticText(clientPanel, ID_CLIENT_SIDE, "SERVER IP:", wxPoint(20, 60),
                     wxSize(63, 15));
    new wxStaticText(clientPanel, ID_CLIENT_SIDE, "COMMAND", wxPoint(20, 90),
                     wxSize(70, 15));

    browseBtn = new wxButton(clientPanel, ID_CLIENT_SIDE, wxString("BROWSE"),
                             wxPoint(20, 170), wxSize(80, 20));
    clt_executeBtn =
        new wxButton(clientPanel, ID_CLIENT_SIDE, wxString("EXECUTE"),
                     wxPoint(20, 205), wxSize(80, 20));

    browseBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onBrowseClicked, this);
    clt_executeBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onClientExecuteClicked,
                         this);

    // TODO: These must have clamp of some sort
    commandInput = new wxTextCtrl(clientPanel, ID_CLIENT_SIDE, "",
                                  wxPoint(20, 115), wxSize(155, 40));

    client_portInput = new wxTextCtrl(clientPanel, ID_CLIENT_SIDE, "",
                                      wxPoint(60, 20), wxSize(50, 20));
    ipInput = new wxTextCtrl(clientPanel, ID_CLIENT_SIDE, "", wxPoint(86, 60),
                             wxSize(70, 20));
    terminalCatch(clientPanel, ID_CLIENT_SIDE);
    m_book->AddPage(clientPanel, "Client Side", true);
  }
  void OnServerSide(wxCommandEvent &event) {
    m_book->SetSelection(0);
    SetStatusText("Server Side View");
  }
  void OnClientSide(wxCommandEvent &event) {
    m_book->SetSelection(1);
    SetStatusText("Client Side View");
  }
  void onBrowseClicked(wxCommandEvent &event) {
#ifdef _WIN32
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
      // FILENAME = p.filename().string();
      std::string narrowPath = p.string();
      PATH_TO_FILE = narrowPath;
      std::cout << "Selected file: " + narrowPath << std::endl;
    }
#else
    wxFileDialog openFileDialog(this, "Select a file to open", "", "",
                                "All files (*.*)|*.*",
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_OK) {
      std::string narrowPath = openFileDialog.GetPath().ToStdString();
      FILE_NAME = openFileDialog.GetFilename().ToStdString();
      PATH_TO_FILE = narrowPath;
      std::cout << "Selected file: " + narrowPath << std::endl;
    }
#endif
  }
  void onClientExecuteClicked(wxCommandEvent &event) {
    long temp = 0;
    wxString ip = ipInput->GetValue();
    wxString port = client_portInput->GetValue();

    snprintf(TARGET_IP, sizeof(TARGET_IP), "%s", (const char *)ip.mb_str());
    if (port.ToLong(&temp)) {
      PORT = static_cast<int>(temp);
    }
    COMPILE_COMMAND = commandInput->GetValue();
    std::thread([this]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(800));
      std::cout << "Client Side started on " << TARGET_IP << ":" << PORT
                << std::endl;
      client.client_side(PORT, TARGET_IP, COMPILE_COMMAND);
    }).detach();
  }
  void onServerExecuteClicked(wxCommandEvent &event) {
    long temp = 0;
    wxString port = server_portInput->GetValue();
    if (port.ToLong(&temp)) {
      PORT = static_cast<int>(temp);
    }
    std::thread([this]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(800));
      std::cout << "Server Side Started on port " << PORT << std::endl;
      server.server_side(PORT);
    }).detach();
  }
  void terminalCatch(wxWindow *panel, wxWindowID ID) {
    terminalOutput =
        new wxTextCtrl(panel, ID, "", wxPoint(180, 15), wxSize(490, 300),
                       wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    terminalOutput->SetForegroundColour(wxColour(54, 69, 79));
    terminalOutput->SetFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));

    // Redirect cout and cerr stream buffers
    if (!streamBuffer) {
      streamBuffer = new TerminalOutputCatch(terminalOutput);
      oldCoutBuffer = std::cout.rdbuf(streamBuffer);
      oldCerrBuffer = std::cerr.rdbuf(streamBuffer);
    } else {
      streamBuffer->AddTarget(terminalOutput);
    }
  }
};

bool RCE_App::OnInit() {
  RCE_Frame *frame = new RCE_Frame();
  frame->Show(true);
  return true;
}
