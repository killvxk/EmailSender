#include"emailTool.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include < winsock2.h >  //link ws2_32.lib
#include "conio.h" 
#pragma comment (lib, "Ws2_32.lib")

struct _sendmail {
	LPSTR host;
	LPSTR IP;
	LPSTR sender;
	LPSTR recip;
	LPSTR subject;
	LPSTR body;
	SOCKET hSocket;
} sendmail;
int sendEmail(char * from, char * to, char * subject, char * body); 
BOOL InitSock(LPSTR Server);
BOOL SendMail();

void emailTool(char* k, int size){
	char from[1024],to[1024],subject[1024],body[1024];
	//from = coande@e12e.com&to = e12e@qq.com&subject = 主题&body = 这是一封邮件!
	sscanf_s(k, "from=%[^&]&to=%[^&]&subject=%[^&]&body=%s", from,1024, to,1024, subject,1024,body,1024);
	printf("%s\n%s\n%s\n%s\n" , from, to, subject, body);
	sendEmail(from, to, subject, body);
}

int sendEmail(char * from, char * to, char * subject , char * body) {
		sendmail.host = "mx1.qq.com";
		sendmail.sender = from;
		sendmail.recip = to;
	
		sendmail.subject = subject;
		sendmail.body = body;
	
		if (InitSock(sendmail.host))
		{
			SendMail();
			WSACleanup();
		}
		printf("\nok\n");
	return 0;
}

BOOL InitSock(LPSTR Server)
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(2, 2), &wsd) > 0)
	{
		printf("Winsock init error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

LPSTR GetIP(LPSTR h)
{
	in_addr ia;
	hostent* hbn;
	unsigned int addr;

	if (isalpha(h[0])) hbn = gethostbyname(h);
	else
	{
		addr = inet_addr(h);
		hbn = gethostbyaddr((char *)&addr, 4, AF_INET);
	}

	if (hbn == 0) return "";
	ia.S_un.S_addr = *(DWORD *)hbn->h_addr_list[0];
	return inet_ntoa(ia);
}

BOOL ValidateEnvelope(LPSTR h, LPSTR r, LPSTR s, LPSTR *ip)
{
	if (h == "" || r == "" || s == "")
	{
		printf("Invalid host or sender/recipient email address\n");
		return false;
	}
	*ip = GetIP(h);
	if (*ip == "")
	{
		printf("Can not resolve host name %s to ip address\n", h);
		return false;
	}
	return true;
}

BOOL ConnectTo()
{
	const u_short SMTP_PORT = 25;
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = inet_addr(sendmail.IP);
	sa.sin_port = htons(SMTP_PORT);
	return (connect(sendmail.hSocket, (SOCKADDR *)&sa, sizeof(sa)) == 0);
}

BOOL snd(LPSTR txt, bool resp)
{
	char tx[4096];
	sprintf(tx, "%s\r\n", txt);

	if (send(sendmail.hSocket, tx, strlen(tx), 0) == SOCKET_ERROR)
	{
		printf("%s%s\n", tx, " - send operation failed.");
		return false;
	}
	if (!resp) return true;

	while (true)
	{
		char rcv[1024];
		char outp[255] = "";

		HANDLE hEvent = WSACreateEvent();
		WSAEventSelect(sendmail.hSocket, hEvent, FD_READ);
		DWORD nWait = WSAWaitForMultipleEvents(1, &hEvent, 0, 1000, 0);
		WSACloseEvent(hEvent);
		if (nWait != 0) return false;
		recv(sendmail.hSocket, rcv, sizeof(rcv), 0);

		strncpy(outp, rcv, strcspn(rcv, "\n") + 1);
		printf("%s\n", outp);
	}
	return true;
}

BOOL SendMail()
{
	if (!ValidateEnvelope(sendmail.host, sendmail.recip,
		sendmail.sender, &sendmail.IP)) return false;

	char tmp[1024];
	if (sendmail.sender == "" || sendmail.recip == "" ||
		sendmail.body == "" || sendmail.subject == "") return false;

	sendmail.hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sendmail.hSocket == SOCKET_ERROR) return false;

	if (!ConnectTo())
	{
		printf("Unable to connect to the server.\n");
		return false;
	}
	snd("ehlo localhost", true);

	sprintf(tmp, "%s%s%s", "MAIL FROM:<", "winsock@winexp.com", ">");
	snd(tmp, true);
	sprintf(tmp, "%s%s%s", "RCPT TO:<", sendmail.recip, ">");
	snd(tmp, true);

	snd("DATA", true);
	sprintf(tmp, "%s%s", "From: ", sendmail.sender);
	snd(tmp, false);
	sprintf(tmp, "%s%s", "To: ", sendmail.recip);
	snd(tmp, false);
	sprintf(tmp, "%s%s", "Subject: ", sendmail.subject);
	snd(tmp, false);
	snd("", false);
	snd(sendmail.body, false);
	snd(".", true);

	snd("NOOP", true);
	snd("QUIT", true);
	closesocket(sendmail.hSocket);

	return true;
}
