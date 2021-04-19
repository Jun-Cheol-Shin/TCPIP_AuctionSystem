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
	INTROERROR,				// 메뉴 선택 에러
	SIGN,					// 회원가입
	LOGIN,					// 로그인
	DEPOSIT,				// 입금
	JOIN,					// 경매참여
	BANK,					// 온라인통장
	VERIFY,					// 경매내역
	LIST,					// 구매내역
	VERIFYDELETE,			// 경매내역지우기
	LOGOUT,					// 로그아웃
	EXIT					// 나가기
};

enum RESULT {
	NOTHING = -1,
	ID_ERROR = 1,			// 아이디 오류
	PW_ERROR,				// 비밀번호 오류
	LOGINSUCESS,			// 로그인 성공
	ALREADY_LOGIN,			// 이미 로그인
	JOIN_ERROR,				// 경매 입찰 실패 => 돈 부족 or 최소금액 미달
	JOIN_SUCCESS			// 경매 입찰 성공
};

enum STATE {
	NOTSTATE = -1,
	INTRO = 1,				// 초기 상태
	SELECTRESULT,			// 초기 상태에서 메뉴를 고른 상태
	INPUTRESULT,			// SELECTRESULT에서 입력값을 보내고 결과값을 받아올 때
	LOGININTRO,				// 로그인 직후 메뉴를 보여줄 때
	LOGINSELECTRESULT,		// 로그인 후 메뉴를 고른 상태
	LOGININPUTRESULT,		// LOGINSELECTRESULT에서 입력값을 보내고 결과값을 받아올 때
	JOIN_SUCCESS_STATE,		// 경매 참여의 결과값을 받아올 때
};

// 소켓 함수 오류 출력 후 종료
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
// 소켓 함수 오류 출력
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
// 사용자 정의 데이터 수신 함수
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

	ptr = buf + sizeof(int);			// 총사이즈의 자리를 남겨둔다.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// 프로토콜 패킹
	size = size + sizeof(enum PROTOCOL);				// 사이즈 증가
	ptr = ptr + sizeof(enum PROTOCOL);					// 위치 이동

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, number, strsize);
	size = size + strsize;

	ptr = buf;											// 모든 작업이 끝나면 처음 자리로 돌아온다.		
	memcpy(ptr, &size, sizeof(int));					// 총 사이즈를 입력한다.

	return size + sizeof(int);
}
int packing(enum PROTOCOL protocol, const char* ID, const char* PW, char* buf)
{
	int idsize = strlen(ID);
	int pwsize = strlen(PW);
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);							// 총사이즈의 자리를 남겨둔다.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// 프로토콜 패킹
	size = size + sizeof(enum PROTOCOL);				// 사이즈 증가
	ptr = ptr + sizeof(enum PROTOCOL);					// 위치 이동

	memcpy(ptr, &idsize, sizeof(int));					// 아이디의 사이즈
	size = size + sizeof(int);							// 사이즈 증가
	ptr = ptr + sizeof(int);							// 위치 이동

	memcpy(ptr, ID, idsize);						// 아이디
	size = size + idsize;								// 사이즈 증가
	ptr = ptr + idsize;									// 위치 이동

														// 패스워드도 마찬가지로 패킹
	memcpy(ptr, &pwsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, PW, pwsize);
	size = size + pwsize;

	ptr = buf;											// 처음 자리
	memcpy(ptr, &size, sizeof(int));					// 총사이즈를 ptr에 카피한다.

	return size + sizeof(int);										// 총 사이즈 리턴
}
int packing(enum PROTOCOL protocol, const char* ID, const char* PW, int money, char* buf)
{
	int idsize = strlen(ID);
	int pwsize = strlen(PW);
	char* ptr;
	int size = 0;
	ptr = buf + sizeof(int);							// 총사이즈의 자리를 남겨둔다.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// 프로토콜 패킹
	size = size + sizeof(enum PROTOCOL);				// 사이즈 증가
	ptr = ptr + sizeof(enum PROTOCOL);					// 위치 이동

	memcpy(ptr, &idsize, sizeof(int));					// 아이디의 사이즈
	size = size + sizeof(int);							// 사이즈 증가
	ptr = ptr + sizeof(int);							// 위치 이동

	memcpy(ptr, ID, idsize);						// 아이디
	size = size + idsize;								// 사이즈 증가
	ptr = ptr + idsize;									// 위치 이동

														// 패스워드도 마찬가지로 패킹
	memcpy(ptr, &pwsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, PW, pwsize);
	size = size + pwsize;
	ptr = ptr + pwsize;

	memcpy(ptr, &money, sizeof(int));
	size = size + sizeof(int);

	ptr = buf;											// 처음 자리
	memcpy(ptr, &size, sizeof(int));					// 총사이즈를 ptr에 카피한다.

	return size + sizeof(int);										// 총 사이즈 리턴
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
		printf("아이디를 입력하세요>>");
		scanf("%s", id);
		printf("비밀번호를 입력하세요>>");
		scanf("%s", pw);

		if (SIZE < strlen(id) || SIZE < strlen(pw)) 
		{
			printf("아이디 혹은 비밀번호가 너무 깁니다.\n");
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

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int size;
	STATE state = STATE::INTRO;
	bool endflag = false;
	RESULT result = RESULT::NOTHING;

	// 서버와 데이터 통신
	while (1) {
		int money;
		char ID[SIZE];
		char PW[SIZE];
		int num;

		// 데이터 받기
		if (!GetRetval(sock, buf))break;

		switch (state) {
			// 맨 처음 서버로부터 받아오는 메뉴 STATE
		case STATE::INTRO:
			switch (GetProtocol(buf))
			{
				// 메뉴에서 번호를 선택해 서버에게 전달.
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
			// state를 선택 후 상태로 변경
			state = SELECTRESULT;
			break;
			// SELECTRESULT 선택 후 상태에서는 로그인,회원가입,종료가 있다. 종료 이외의 프로토콜을 받으면 state를 INPUTRESULT로 변경
		case STATE::SELECTRESULT:
			system("cls");
			switch (GetProtocol(buf))
			{
				// 번호를 잘못 골랐을 경우 1~3 이외의 숫자
			case PROTOCOL::INTROERROR:
				msg_unpacking(buf);
				state = STATE::INTRO;
				break;
				// 회원 가입을 골랐을 경우
			case PROTOCOL::SIGN:
				msg_unpacking(buf);
				// ID와 PW를 입력받고
				InfoInput(ID, PW);
				// 최초 입금액을 받아 패킹 후 전달
				printf("최초 입금액을 설정하세요!>>");
				scanf("%d", &money);
				size = packing(PROTOCOL::SIGN, ID, PW, money, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::INPUTRESULT;
				break;
				// 로그인을 골랐을 경우
			case PROTOCOL::LOGIN:
				msg_unpacking(buf);
				// ID와 PW를 입력받아 패킹하여 보낸다.
				InfoInput(ID, PW);
				size = packing(PROTOCOL::LOGIN, ID, PW, buf);
				retval = send(sock, buf, size, 0);
				if (retval == SOCKET_ERROR) {
					err_display("send()");
					break;
				}
				state = STATE::INPUTRESULT;
				break;
				// 종료를 눌렀을 경우 flag를 변경하고 서버에게 무사히 종료되었음을 알리는 패킷 전달
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
			// INPUTRESULT는 로그인, 회원가입에서 아이디 패스워드를 서버에게 전달 후의 상태
		case STATE::INPUTRESULT:
			switch (GetProtocol(buf))
			{
				// 성공하면 state를 LOGININTRO로 실패하면 다시 state를 INTRO로 변경
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
				// 회원가입은 실패,성공 상관없이 INTRO로 변경 단 메세지로 실패 혹은 성공을 알린다.
			case PROTOCOL::SIGN:
				system("cls");
				msg_unpacking(buf);
				state = STATE::INTRO;
				break;
			}
			break;
			// LOGININTRO는 로그인 직후의 상태
		case STATE::LOGININTRO:
			switch (GetProtocol(buf))
			{
				// 번호를 입력받아 서버에게 전달 후 state를 LOGINSELECTRESULT로 변경
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
			// LOGINSELECTRESULT는 LOGININTRO에서 메뉴를 선택 한 후의 상태
		case STATE::LOGINSELECTRESULT:
			system("cls");
			switch (GetProtocol(buf))
			{
				// 로그인 메뉴는 입금,경매참여,통장확인,입찰내역,구매내역,입찰내역지우기,로그아웃이 있다.
				// 메뉴 선택이 잘못되었을 때
			case PROTOCOL::INTROERROR:
				msg_unpacking(buf);
				state = STATE::LOGININTRO;
				break;
				// 입금을 골랐을 때 입금액 메세지를 서버에게 받고 입금액을 서버에게 보낸 후 LOGININPUTRESULT로 state 이동
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
				// 경매 참여를 골랐을 때 경매 할 수 있는 물품 내역을 보여주고 LOGININPUTRESULT로 state 이동
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
				// 온라인통장 => 돈 확인을 골랐을 때 프로토콜 패킷만 받은 후 메세지만을 받고 LOGININTRO로 이동
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
				// 경매내역을 골랐을 때 내역 메세지를 보여주고 LOGININTRO로 다시 이동
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
				// 구매 내역을 골랐을 때 메세지를 보여주고 LOGININTRO로 다시 이동
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
				// 내역 삭제를 골랐을 경우 내역 삭제 확인 메세지만 받고 LOGININTRO이동
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
				// 로그아웃을 골랐을 경우 확인 메세지를 받고 INTRO로 이동
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
			// LOGININPUTRESULT는 메뉴에 대한 선택을 한 후 어떠한 입력값을 서버에게 보낸 후 출력을 받아야할 때를 나타냄.
		case STATE::LOGININPUTRESULT:
			switch (GetProtocol(buf))
			{
				// 입금액을 서버에게 보내고 확인 메세지를 받고 LOGININTRO로 이동
			case PROTOCOL::DEPOSIT:
				system("cls");
				msg_unpacking(buf);
				state = STATE::LOGININTRO;
				break;
				// 경매 참여를 고르고 경매내역에서 번호를 선택 후 결과값을 받아온다.
				// 그 후 RESULT값에 따라 성공이면 돈값을 실패면 다시 LOGININTRO로 이동
				// 성공은 돈 값을 서버에게 보내고 JOIN_SUCCESS_STATE로 state를 이동한다.
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
			// JOIN_SUCCESS는 경매품을 번호를 골라 보낸다  => 금액을 보낸다의 결과값을 출력하기위한 상태
			// 성공 실패 여부를 메세지로 출력 후 state를 LOGININTRO로 돌아간다.
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

	// 윈속 종료
	WSACleanup();
	return 0;
}