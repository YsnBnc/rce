#include <iostream>
#include <wx/simplebook.h>
#include <wx/wx.h>
#include "utilities/bridge.h"
using namespace std;

int PORT;
char TARGET_IP[16];
std::string COMMAND;
std::string PATH_TO_FILE;


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
        SetMinSize(fixedSize);
        SetMaxSize(fixedSize);

        m_book = new wxSimplebook(this, wxID_ANY); //Page container

        //Permanent panels
        initServerPanel();
        initClientPanel();

        wxMenu *menuView = new wxMenu();
        menuView->Append(ID_SERVER_SIDE, wxString("&Server Side"));
        menuView->Append(ID_CLIENT_SIDE, wxString("&Client Side"));

        wxMenuBar *menu_bar = new wxMenuBar();
        menu_bar->Append(menuView, wxString("&Sides"));
        SetMenuBar(menu_bar);

        CreateStatusBar();
        SetStatusText("Ready");

        Bind(wxEVT_MENU, &RCE_Frame::OnServerSide, this, ID_SERVER_SIDE);
        Bind(wxEVT_MENU, &RCE_Frame::OnClientSide, this, ID_CLIENT_SIDE);
    }

private:
    wxSimplebook* m_book = nullptr;
    wxPanel* serverPanel = nullptr;
    wxPanel* clientPanel = nullptr;
    wxTextCtrl* ipInput = nullptr;
    wxTextCtrl* portInput = nullptr;
    wxButton *browseBtn = nullptr;
    wxButton *executeBtn = nullptr;

    void onButtonClicked(wxCommandEvent& event) {
        wxMessageBox("Hello world from wxWidgets!", "", wxOK | wxICON_INFORMATION);
    }

    void initServerPanel() {
        serverPanel = new wxPanel(m_book);

        new wxStaticText(serverPanel, ID_SERVER_SIDE, "PORT:", wxPoint(20,20), wxSize(35,15));
        new wxStaticText(serverPanel, ID_SERVER_SIDE, "SERVER IP:",wxPoint(20,60), wxSize(63,15));

        browseBtn = new wxButton(serverPanel, ID_SERVER_SIDE, wxString("BROWSE"), wxPoint(20,100), wxSize(80,20));
        executeBtn = new wxButton(serverPanel, ID_SERVER_SIDE, wxString("EXECUTE"), wxPoint(20,140), wxSize(80,20));

        browseBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onBrowseClicked, this);
        executeBtn->Bind(wxEVT_BUTTON, &RCE_Frame::onServerExecuteClicked, this);

        //TODO: These must have clamp of some sort
        portInput = new wxTextCtrl(serverPanel, ID_SERVER_SIDE, "",wxPoint(60,20), wxSize(50,20));
        ipInput = new wxTextCtrl(serverPanel, ID_SERVER_SIDE, "",wxPoint(86,60), wxSize(90,20));

        m_book->AddPage(serverPanel,"Server Side", true);
    }
    void onBrowseClicked(wxCommandEvent& event) {
        wxMessageBox("Server Button clicked", "Info", wxOK | wxICON_INFORMATION);
    }
    void onServerExecuteClicked(wxCommandEvent& event) {
        long temp = 0;
        wxString ip = ipInput->GetValue();
        wxString port = portInput->GetValue();

        snprintf(TARGET_IP, sizeof(TARGET_IP), "%s", (const char*) ip.mb_str());
        if (port.ToLong(&temp)) {
            PORT = static_cast<int>(temp);
        }
        wxLogMessage("Set PORT: %d, TARGET_IP: %s", PORT, TARGET_IP);
        server_side();
        SetStatusText("Server is ready");
    }
    void initClientPanel() {
        clientPanel = new wxPanel(m_book);
        new wxStaticText(clientPanel, wxID_ANY, "Client Side",wxPoint(100, 70));
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
};

bool RCE_App::OnInit()
{
    RCE_Frame *frame = new RCE_Frame();
    frame->Show(true);
    return true;
}


/*
int PORT;
char TARGET_IP[16];
std::string COMMAND;
std::string PATH_TO_FILE;

int client() {
    cout << "Port: ";
    cin >> PORT;
    cout <<"Server IP: ";
    cin >> TARGET_IP;
    cout <<"File: ";
    cin >> PATH_TO_FILE;
    cout <<"Client is executing...\n";
    client_side();
    return 0;
}
int server() {
    cout << "Port: ";
    cin >> PORT;
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    cout << "Command you want to execute: ";
    std::getline(cin, COMMAND);
    cout <<"Server is executing...\n";
    server_side();
    return 0;
}

int main() {

    int side = 0;
    cout << "Remote Code Execution\n" << "Specify a side\n 1. Server\n 2. Client\n";
    cin >> side;
    switch (side) {
        case 1:
            server();
            return 0;
        case 2:
            client();
            return 0;
        default:
            cout << "Invalid command\n";
            return 1;
    }


}
*/
