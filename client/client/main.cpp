#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define ITEMNAMESIZE 50
#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    512
#define SIZE		16

enum PROTOCOL {
	NODATA = -1,
	INTROSELECT = 1,
	INTROERROR,				// �޴� ���� ����
	SIGN,					// ȸ������
	LOGIN,					// �α���
	DEPOSIT,				// �Ա�
	JOIN,					// �������
	BANK,					// �¶�������
	VERIFY,					// ��ų���
	LIST,					// ���ų���
	VERIFYDELETE,			// ��ų��������
	LOGOUT,					// �α׾ƿ�
	EXIT					// ������
};

enum RESULT {
	NOTHING = -1,
	ID_ERROR = 1,			// ���̵� ����
	PW_ERROR,				// ��й�ȣ ����
	LOGINSUCESS,			// �α��� ����
	ALREADY_LOGIN,			// �̹� �α���
	JOIN_ERROR,				// ��� ���� ���� => �� ���� or �ּұݾ� �̴�
	JOIN_SUCCESS			// ��� ���� ����
};

enum STATE {
	NOTSTATE = -1,
	INTRO = 1,				// �ʱ� ����
	SELECTRESULT,			// �ʱ� ���¿��� �޴��� �� ����
	INPUTRESULT,			// SELECTRESULT���� �Է°��� ������ ������� �޾ƿ� ��
	LOGININTRO,				// �α��� ���� �޴��� ������ ��
	LOGINSELECTRESULT,		// �α��� �� �޴��� �� ����
	LOGININPUTRESULT,		// LOGINSELECTRESULT���� �Է°��� ������ ������� �޾ƿ� ��
	JOIN_SUCCESS_STATE,		// ��� ������ ������� �޾ƿ� ��
};

// ���� �Լ� ���� ��� �� ����
void err_quit(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
// ���� �Լ� ���� ���
void err_display(char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
bool GetRetval(SOCKET _client_sock, char* _buf) {
	int retval;
	int size;
	retval = recvn(_client_sock, (char*)&size, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		return false;
	}
	else if (retval == 0) return false;
	retval = recvn(_client_sock, _buf, size, 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv()");
		return false;
	}
	else if (retval == 0)return false;

	return true;
}
PROTOCOL GetProtocol(char* _buf) {
	PROTOCOL protocol;
	memcpy(&protocol, _buf, sizeof(enum PROTOCOL));
	return protocol;
}
RESULT GetResult(char* _buf) {
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	RESULT result = RESULT::NOTHING;
	memcpy(&result, _buf, sizeof(enum RESULT));

	return result;
}
void msg_unpacking(const char* buf) {
	int strsize;
	char msg[BUFSIZE];
	const char* ptr = buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);
	memcpy(msg, ptr, strsize);

	msg[strsize] = '\0';

	printf("%s", msg);
}
void msg_unpacking(const char* buf, enum RESULT* result) {
	int strsize;
	const char* ptr = buf;
	char msg[BUFSIZE + 1];
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(result, ptr, sizeof(enum RESULT));
	ptr = ptr + sizeof(enum RESULT);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);
	memcpy(msg, ptr, strsize);

	msg[strsize] = '\0';

	printf("%s", msg);

}
int packing(enum PROTOCOL protocol, const char* number, char* buf)
{
	int strsize;
	int size = 0;
	char* ptr;
	strsize = strlen(number);

	ptr = buf + sizeof(int);			// �ѻ������� �ڸ��� ���ܵд�.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// �������� ��ŷ
	size = size + sizeof(enum PROTOCOL);				// ������ ����
	ptr = ptr + sizeof(enum PROTOCOL);					// ��ġ �̵�

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, number, strsize);
	size = size + strsize;

	ptr = buf;											// ��� �۾��� ������ ó�� �ڸ��� ���ƿ´�.		
	memcpy(ptr, &size, sizeof(int));					// �� ����� �Է��Ѵ�.

	return size + sizeof(int);
}
int packing(enum PROTOCOL protocol, const char* ID, const char* PW, char* buf)
{
	int idsize = strlen(ID);
	int pwsize = strlen(PW);
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);							// �ѻ������� �ڸ��� ���ܵд�.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// �������� ��ŷ
	size = size + sizeof(enum PROTOCOL);				// ������ ����
	ptr = ptr + sizeof(enum PROTOCOL);					// ��ġ �̵�

	memcpy(ptr, &idsize, sizeof(int));					// ���̵��� ������
	size = size + sizeof(int);							// ������ ����
	ptr = ptr + sizeof(int);							// ��ġ �̵�

	memcpy(ptr, ID, idsize);						// ���̵�
	size = size + idsize;								// ������ ����
	ptr = ptr + idsize;									// ��ġ �̵�

														// �н����嵵 ���������� ��ŷ
	memcpy(ptr, &pwsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, PW, pwsize);
	size = size + pwsize;

	ptr = buf;											// ó�� �ڸ�
	memcpy(ptr, &size, sizeof(int));					// �ѻ���� ptr�� ī���Ѵ�.

	return size + sizeof(int);										// �� ������ ����
}
int packing(enum PROTOCOL protocol, const char* ID, const char* PW, int money, char* buf)
{
	int idsize = strlen(ID);
	int pwsize = strlen(PW);
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);							// �ѻ������� �ڸ��� ���ܵд�.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// �������� ��ŷ
	size = size + sizeof(enum PROTOCOL);				// ������ ����
	ptr = ptr + sizeof(enum PROTOCOL);					// ��ġ �̵�

	memcpy(ptr, &idsize, sizeof(int));					// ���̵��� ������
	size = size + sizeof(int);							// ������ ����
	ptr = ptr + sizeof(int);							// ��ġ �̵�

	memcpy(ptr, ID, idsize);						// ���̵�
	size = size + idsize;								// ������ ����
	ptr = ptr + idsize;									// ��ġ �̵�

														// �н����嵵 ���������� ��ŷ
	memcpy(ptr, &pwsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, PW, pwsize);
	size = size + pwsize;
	ptr = ptr + pwsize;

	memcpy(ptr, &money, sizeof(int));
	size = size + sizeof(int);

	ptr = buf;											// ó�� �ڸ�
	memcpy(ptr, &size, sizeof(int));					// �ѻ���� ptr�� ī���Ѵ�.

	return size + sizeof(int);										// �� ������ ����
}
int packing(enum PROTOCOL protocol, char* buf) {
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));
	size = size + sizeof(enum PROTOCOL);
	ptr = ptr + sizeof(enum PROTOCOL);

	ptr = buf;
	memcpy(ptr, &size, sizeof(int));

	return size + sizeof(int);
}
int packing(enum PROTOCOL protocol, int money, char* buf) {
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));
	size = size + sizeof(enum PROTOCOL);
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(ptr, &money, sizeof(int));
	size = size + sizeof(int);

	ptr = buf;
	memcpy(ptr, &size, sizeof(int));

	return size + sizeof(int);
}
void InfoInput(char* ID, char* PW)
{
	char id[SIZE];
	char pw[SIZE];

	while (1) 
	{
		printf("���̵� �Է��ϼ���>>");
		scanf("%s", id);
		printf("��й�ȣ�� �Է��ϼ���>>");
		scanf("%s", pw);

		if (SIZE < strlen(id) || SIZE < strlen(pw)) 
		{
			printf("���̵� Ȥ�� ��й�ȣ�� �ʹ� ��ϴ�.\n");
		}
		else
			break;
	}

	strcpy(ID, id);
	strcpy(PW, pw);
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE + 1];
	int size;
	STATE state = STATE::INTRO;
	bool endflag = false;
	RESULT result = RESULT::NOTHING;

	// ������ ������ ���
	while (1) {
		int money;
		char ID[SIZE];
		char PW[SIZE];
		int num;

		// ������ �ޱ�
		if (!GetRetval(sock, buf))break;

		switch (state) {
			// �� ó�� �����κ��� �޾ƿ��� �޴� STATE
		case STATE::INTRO:
			switch (GetProtocol(buf))
			{
				// �޴����� ��ȣ�� ������ �������� ����.
			case PROTOCOL::INTROSELECT:
				msg_unpacking(buf);
				scanf("%d", &num);
				size = packing(PROTOCOL::INTROSELECT, num, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				break;
			}
			// state�� ���� �� ���·� ����
			state = SELECTRESULT;
			break;
			// SELECTRESULT ���� �� ���¿����� �α���,ȸ������,���ᰡ �ִ�. ���� �̿��� ���������� ������ state�� INPUTRESULT�� ����
		case STATE::SELECTRESULT:
			system("cls");
			switch (GetProtocol(buf))
			{
				// ��ȣ�� �߸� ����� ��� 1~3 �̿��� ����
			case PROTOCOL::INTROERROR:
				msg_unpacking(buf);
				state = STATE::INTRO;
				break;
				// ȸ�� ������ ����� ���
			case PROTOCOL::SIGN:
				msg_unpacking(buf);
				// ID�� PW�� �Է¹ް�
				InfoInput(ID, PW);
				// ���� �Աݾ��� �޾� ��ŷ �� ����
				printf("���� �Աݾ��� �����ϼ���!>>");
				scanf("%d", &money);
				size = packing(PROTOCOL::SIGN, ID, PW, money, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::INPUTRESULT;
				break;
				// �α����� ����� ���
			case PROTOCOL::LOGIN:
				msg_unpacking(buf);
				// ID�� PW�� �Է¹޾� ��ŷ�Ͽ� ������.
				InfoInput(ID, PW);
				size = packing(PROTOCOL::LOGIN, ID, PW, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::INPUTRESULT;
				break;
				// ���Ḧ ������ ��� flag�� �����ϰ� �������� ������ ����Ǿ����� �˸��� ��Ŷ ����
			case PROTOCOL::EXIT:
				endflag = true;
				size = packing(PROTOCOL::EXIT, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				break;
			}
			break;
			// INPUTRESULT�� �α���, ȸ�����Կ��� ���̵� �н����带 �������� ���� ���� ����
		case STATE::INPUTRESULT:
			switch (GetProtocol(buf))
			{
				// �����ϸ� state�� LOGININTRO�� �����ϸ� �ٽ� state�� INTRO�� ����
			case PROTOCOL::LOGIN:
				system("cls");
				msg_unpacking(buf, &result);
				switch (result) {
				case RESULT::ID_ERROR:
				case RESULT::PW_ERROR:
				case RESULT::ALREADY_LOGIN:
					state = STATE::INTRO;
					break;
				case RESULT::LOGINSUCESS:
					state = STATE::LOGININTRO;
					break;
				}
				break;
				// ȸ�������� ����,���� ������� INTRO�� ���� �� �޼����� ���� Ȥ�� ������ �˸���.
			case PROTOCOL::SIGN:
				system("cls");
				msg_unpacking(buf);
				state = STATE::INTRO;
				break;
			}
			break;
			// LOGININTRO�� �α��� ������ ����
		case STATE::LOGININTRO:
			switch (GetProtocol(buf))
			{
				// ��ȣ�� �Է¹޾� �������� ���� �� state�� LOGINSELECTRESULT�� ����
			case PROTOCOL::INTROSELECT:
				msg_unpacking(buf);
				scanf("%d", &num);
				size = packing(PROTOCOL::INTROSELECT, num, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				break;
			}
			state = LOGINSELECTRESULT;
			break;
			// LOGINSELECTRESULT�� LOGININTRO���� �޴��� ���� �� ���� ����
		case STATE::LOGINSELECTRESULT:
			system("cls");
			switch (GetProtocol(buf))
			{
				// �α��� �޴��� �Ա�,�������,����Ȯ��,��������,���ų���,�������������,�α׾ƿ��� �ִ�.
				// �޴� ������ �߸��Ǿ��� ��
			case PROTOCOL::INTROERROR:
				msg_unpacking(buf);
				state = STATE::LOGININTRO;
				break;
				// �Ա��� ����� �� �Աݾ� �޼����� �������� �ް� �Աݾ��� �������� ���� �� LOGININPUTRESULT�� state �̵�
			case PROTOCOL::DEPOSIT:
				msg_unpacking(buf);
				scanf("%d", &money);
				size = packing(PROTOCOL::DEPOSIT, money, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININPUTRESULT;
				break;
				// ��� ������ ����� �� ��� �� �� �ִ� ��ǰ ������ �����ְ� LOGININPUTRESULT�� state �̵�
			case PROTOCOL::JOIN:
				msg_unpacking(buf);
				scanf("%d", &num);
				size = packing(PROTOCOL::JOIN, num, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININPUTRESULT;
				break;
				// �¶������� => �� Ȯ���� ����� �� �������� ��Ŷ�� ���� �� �޼������� �ް� LOGININTRO�� �̵�
			case PROTOCOL::BANK:
				msg_unpacking(buf);
				size = packing(PROTOCOL::BANK, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININTRO;
				break;
				// ��ų����� ����� �� ���� �޼����� �����ְ� LOGININTRO�� �ٽ� �̵�
			case PROTOCOL::VERIFY:
				msg_unpacking(buf);
				system("pause");
				size = packing(PROTOCOL::VERIFY, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININTRO;
				system("cls");
				break;
				// ���� ������ ����� �� �޼����� �����ְ� LOGININTRO�� �ٽ� �̵�
			case PROTOCOL::LIST:
				msg_unpacking(buf);
				system("pause");
				size = packing(PROTOCOL::LIST, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININTRO;
				system("cls");
				break;
				// ���� ������ ����� ��� ���� ���� Ȯ�� �޼����� �ް� LOGININTRO�̵�
			case PROTOCOL::VERIFYDELETE:
				msg_unpacking(buf);
				size = packing(PROTOCOL::VERIFYDELETE, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::LOGININTRO;
				break;
				// �α׾ƿ��� ����� ��� Ȯ�� �޼����� �ް� INTRO�� �̵�
			case PROTOCOL::LOGOUT:
				msg_unpacking(buf);
				size = packing(PROTOCOL::LOGOUT, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::INTRO;
				break;
			}
			break;
			// LOGININPUTRESULT�� �޴��� ���� ������ �� �� ��� �Է°��� �������� ���� �� ����� �޾ƾ��� ���� ��Ÿ��.
		case STATE::LOGININPUTRESULT:
			switch (GetProtocol(buf))
			{
				// �Աݾ��� �������� ������ Ȯ�� �޼����� �ް� LOGININTRO�� �̵�
			case PROTOCOL::DEPOSIT:
				system("cls");
				msg_unpacking(buf);
				state = STATE::LOGININTRO;
				break;
				// ��� ������ ���� ��ų������� ��ȣ�� ���� �� ������� �޾ƿ´�.
				// �� �� RESULT���� ���� �����̸� ������ ���и� �ٽ� LOGININTRO�� �̵�
				// ������ �� ���� �������� ������ JOIN_SUCCESS_STATE�� state�� �̵��Ѵ�.
			case PROTOCOL::JOIN:
				msg_unpacking(buf, &result);
				switch (result) 
				{
				case RESULT::JOIN_ERROR:
					system("cls");
					state = STATE::LOGININTRO;
					break;
				case RESULT::JOIN_SUCCESS:
					scanf("%d", &money);
					size = packing(PROTOCOL::JOIN, money, buf);
					retval = send(sock, buf, size, 0);
					if (retval == SOCKET_ERROR) {
						err_display("send()");
						break;
					}
					state = STATE::JOIN_SUCCESS_STATE;
					break;
				}
				break;
			}
			break;
			// JOIN_SUCCESS�� ���ǰ�� ��ȣ�� ��� ������  => �ݾ��� �������� ������� ����ϱ����� ����
			// ���� ���� ���θ� �޼����� ��� �� state�� LOGININTRO�� ���ư���.
		case STATE::JOIN_SUCCESS_STATE:
			switch (GetProtocol(buf))
			{
			case PROTOCOL::JOIN:
				system("cls");
				msg_unpacking(buf);
				state = STATE::LOGININTRO;
				break;
			}
			break;
		}

		if (endflag)break;
	}
	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}