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
        wxSize fixedSize(800, 600); //Lock the window size
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

    void onButtonClicked(wxCommandEvent& event) {
        wxMessageBox("Hello world from wxWidgets!", "", wxOK | wxICON_INFORMATION);
    }

    void initServerPanel() {
        serverPanel = new wxPanel(m_book);
        new wxStaticText(serverPanel, wxID_ANY, "Server Side", wxPoint(100,70), wxSize(150,100));
        m_book->AddPage(serverPanel,"Server Side", true);
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
