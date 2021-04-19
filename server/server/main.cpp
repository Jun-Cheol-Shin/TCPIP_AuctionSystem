/*게플 3A 18032048 신준철
서버 중간고사*/

/* 경매 시스템은 3개의 경매품으로 이루어져 있으며, 각각의 경매품에는 최소금액이 정해져있다.
클라이언트들은 경매품에 입찰하기 위해서는 최소금액보다 많은 돈을 보유하고 있어야 하며 입찰금은 최소금액 이상으로 입찰을 해야한다. (최소금액으로 입찰 가능)
가장 입찰금이 높은 클라이언트에게 낙찰되며 경매품은 가장 입찰금을 높게 넣은 클라이언트에게 가게되고 경매참여 리스트에서 사라진다.
낙찰된 클라이언트 구매내역에 본인에게 낙찰된 경매품이 나오고 입찰내역은 본인이 입찰한 경매품 리스트를 보여준다.
만약 입찰금이 동일한 클라이언트가 나왔을 경우 가장 먼저 입찰한 클라이언트에게 낙찰이 된다.
이 경우는 물품 클래스에 아이디 배열을 만들어 입찰한 유저의 아이디를 매번 배열에 입력하게 된다.
그리고 배열의 0번째 자리 즉 첫번째로 입찰한 유저의 닉네임과 일치하게 되면 낙찰을 시키는 시스템으로 만들었다.*/
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT		9000
#define BUFSIZE			512
#define ITEMMAX			100							// 유저가 최대로 아이템을 가질 수 있는 개수
#define USER			100							// 회원가입 최대 회원 수
#define SIZE			16							// 이름 사이즈
#define ITEMNAMESIZE	50							// 아이템 이름 사이즈
#define INTROMSG "1. 회원가입\n2. 로그인\n3. 종료\n숫자를 입력하세요 >>"
#define SIGNMSG "\n[회원가입 페이지]\n"
#define LOGINMSG "\n[로그인 페이지]\n"
#define ID_ERROR_MSG "[ERROR] 없는 ID입니다.\n"
#define PW_ERROR_MSG "[ERROR] PW가 틀렸습니다.\n"
#define LOGIN_MSG "로그인에 성공했습니다.\n"
#define ID_OVERLAP_MSG "[ERROR] 중복된 아이디입니다.\n"
#define ALREADY_LOGIN_MSG "이미 아이디가 접속상태입니다.\n"
#define SIGN_UP_MSG "회원가입에 성공했습니다.\n"

#define JOIN_MSG "\n[경매 페이지]  (숫자0은 메뉴로 돌아가기)\n 번호\t\t이름\t\t\t최저가격\t\t참여 클라이언트 수\n"
#define VERIFY_MSG "\n[입찰 내역]\n 번호\t\t이름\t\t\t입찰 가격\t\t낙찰 여부\n"
#define LIST_MSG "\n[구매 내역]\n 번호\t\t이름\t\t\t구매 가격\n"
#define LOGOUT_MSG "로그아웃 하셨습니다.\n"
#define INTRO_ERROR_MSG "[ERROR] 올바른 번호가 아닙니다.\n"
#define LOGININTROMSG "1. 입금\n2. 경매참여\n3. 온라인통장\n4. 입찰내역\n5. 구매내역\n6. 낙찰내역삭제\n7. 로그아웃\n숫자를 입력하세요 >>"
#define DEPOSITMSG "입금하실 금액을 입력해주세요 >>"
#define DEPOSIT_ERROR_MSG "[ERROR] 입금만 가능합니다.\n"
#define JOIN_ERROR_MSG "[ERROR] 입력하신 번호에 맞는 경매품이 없습니다!\n"
#define MONEY_ERROR_MSG "돈이 부족하거나 최소 금액을 넘은 금액을 입력하셔야 합니다.\n"
#define AUCTION_SUCCESS_MSG "입찰 완료!!\n"
#define MENU_RETURN_MSG "메뉴로 돌아갑니다.\n"

#define VERIFY_DELETE_MSG "낙찰된 물건을 목록에서 지웁니다..\n"
#define AUCTION_END_MSG "[공지] 경매품이 모두 낙찰되어 경매가 종료되었습니다!!\n"

#define AUCTION_SELECT_MSG "입찰하실 번호를 입력해주세요 >>"
#define AUCTION_END_MSG2 "\t\t경 매 종 료 (돌아가기는 숫자0 입력)\n\t\t"

#define SUCCESS_RECEIVE 1
#define WAIT_RECEIVE 0
#define DISCONNECTED -1
#define CONNECTERROR -2

#define AUCTIONITEMSIZE 3							// 경매품 개수 현재 3개
#define BIDMAX 2									// 경매품 입찰 최대 인원 수

// 소켓 정보 저장을 위한 구조체와 변수


enum PROTOCOL {
	NODATA = -1,
	INTROSELECT = 1,
	INTROERROR,										// 메뉴 선택 에러
	SIGN,											// 회원가입
	LOGIN,											// 로그인
	DEPOSIT,										// 입금
	JOIN,											// 경매참여
	BANK,											// 온라인통장
	VERIFY,											// 경매내역
	LIST,											// 구매내역
	VERIFYDELETE,									// 경매내역지우기
	LOGOUT,											// 로그아웃
	EXIT											// 나가기
};

enum RESULT {
	NOTHING = -1,
	ID_ERROR = 1,									// 아이디 오류
	PW_ERROR,										// 비밀번호 오류
	LOGINSUCESS,									// 로그인 성공
	ALREADY_LOGIN,									// 이미 로그인
	JOIN_ERROR,										// 경매 입찰 실패 => 돈 부족 or 최소금액 미달
	JOIN_SUCCESS									// 경매 입찰 성공
};

enum STATE {
	NOTSTATE = -1,
	INTRO = 1,										// 초기 상태
	SELECTRESULT,									// 초기 상태에서 메뉴를 고른 상태
	INPUTRESULT,									// SELECTRESULT에서 입력값을 보내고 결과값을 받아올 때
	LOGININTRO,										// 로그인 직후 메뉴를 보여줄 때
	LOGINSELECTRESULT,								// 로그인 후 메뉴를 고른 상태
	LOGININPUTRESULT,								// LOGINSELECTRESULT에서 입력값을 보내고 결과값을 받아올 때
	JOIN_SUCCESS_STATE,								// 경매 참여의 결과값을 받아올 때
};

struct AuctionItem									// 경매품 클래스
{
	char clientsID[BIDMAX][SIZE];					// 경매에 참여한 클라이언트 아이디
	int bid_price[BIDMAX];							// 경매품에 입찰한 금액을 담는 배열
	char Itemname[ITEMNAMESIZE];					// 경매품 이름
	int price;										// 경매품의 최종가격
	int min_price;									// 경매품의 최소가격
	int client_count;								// 경매품에 입찰한 클라이언트 수
	bool sold_out;									// 경매품이 팔렸는지 안팔렸는지의 여부
};

struct Bid_item										// 클라이언트가 입찰한 경매품
{
	AuctionItem* item;								// 경매품
	int price;										// 클라이언트가 입찰한 가격
	bool bidflag;									// bid_check라는 함수를 거쳤을 경우 true
	bool win;										// 경매에서 자신이 낙찰 성공했을 경우 true
};

struct UserInfo										// 클라이언트의 정보 클래스
{
	char ID[SIZE];									// 아이디
	char PW[SIZE];									// 비밀번호
	bool state;										// 로그인 여부
	int Usermoney;									// 클라이언트의 현재 금액
	int temp_use_money;								// 클라이언트가 입찰한 금액

	AuctionItem* UserItem[ITEMMAX];			// 클라이언트가 입찰한 금액에 낙찰되어 구매하게된 경매품
	int UserItemCount;								// 구매한 경매품의 개수

	int bidnumber;									// 클라이언트가 입찰한 경매품의 번호
	Bid_item* bid[ITEMMAX];							// 클라이언트가 입찰한 경매품
	int UserBidCount;								// 클라이언트가 입찰한 경매품의 개수
};

struct SOCKETINFO									// 소켓 정보 클래스
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	STATE server_state;								// 소켓의 STATE
	UserInfo* user;									// 한 소켓 당 한 유저의 정보
	int successrecvbytes;							// 실제 패킷을 받고 난 뒤 받아 온 용량
	bool recvflag;									// 사이즈를 다 받았을 경우 실제 패킷을 받기위해 켜진다.
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[FD_SETSIZE];
UserInfo* userInfo[USER];							// 유저 정보
AuctionItem* ItemList[ITEMMAX];						// 경매품 정보
int User_Count = 0;									// 유저 수
int Item_Count = 0;									// 경매품 수

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

// 오류 출력 함수
void err_quit(char* msg);
void err_display(char* msg);
int packing(enum PROTOCOL protocol, const char* msg, char* buf);
int packing(enum PROTOCOL protocol, char* buf);
PROTOCOL GetProtocol(char* _buf);
char* msg_unpacking(const char* buf);
void unpacking(const char* _buf, char* ID, char* PW);
void unpacking(const char* _buf, char* ID, char* PW, int* money);
void unpacking(const char* _buf, int* money);
int NonBlock_recv(SOCKETINFO* ptr, int* size, int i);											// SELECT 모델 전용 recv 함수
int packing(enum PROTOCOL protocol, enum RESULT result, const char* msg, char* buf);
UserInfo* newUserRegist(const char* ID, const char* PW, const int money);						// 돈과 아이디, 비밀번호를 받아 새로운 유저 정보 구조체를 리턴
void AuctionSetting(int _i);																	// 경매품 구조체 세팅
Bid_item* User_Biditem_Set(AuctionItem* item, int money);										// 유저가 입찰할 경매품과 입찰 금액을 서버가 받아와 Bid_item이라는 구조체를 리턴 후 UserInfo에 넣는다.
char* Bid_Check(int i, SOCKETINFO* ptr);														// 유저가 입찰 후 경매품의 최대입찰 인원 수가 꽉차면 그에따른 결과값을 처리하기 위한 함수
bool bid_order(AuctionItem* _ptritem, const char* _id, int max);								// 입찰 순서를 확인하는 함수

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 넌블로킹 소켓으로 전환
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	FD_SET rset, wset;

	bool endflag = false;


	for (int i = 0; i < AUCTIONITEMSIZE; i++)											// 경매품 세팅
	{
		AuctionSetting(i);
	}

	while (1) {
		int number;
		int money;
		char msg[BUFSIZE + 1];
		char temp[BUFSIZE + 1];
		RESULT result = RESULT::NOTHING;
		int size = 0;
		bool flag = false;
		bool sign = true;

		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);

		for (int i = 0; i < nTotalSockets; i++) {
			if (SocketInfoArray[i]->recvbytes >= SocketInfoArray[i]->sendbytes) {
				FD_SET(SocketInfoArray[i]->sock, &wset);
			}
			else {
				FD_SET(SocketInfoArray[i]->sock, &rset);
			}
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);
		if (retval == SOCKET_ERROR) err_quit("select()");

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if (FD_ISSET(listen_sock, &rset)) {
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
			}
			else {
				printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				// 소켓 정보 추가
				AddSocketInfo(client_sock);
			}
		}

		char ID[SIZE];
		char PW[SIZE];

		//  서버 작업
		for (int i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO* ptr = SocketInfoArray[i];
			switch (ptr->server_state)
			{
				// 클라이언트와 연결 직후 초기 메뉴리스트를 클라이언트에게 보내는 상태
			case STATE::INTRO:
				if (FD_ISSET(ptr->sock, &wset)) {
					size = packing(PROTOCOL::INTROSELECT, INTROMSG, ptr->buf);
					retval = send(ptr->sock, ptr->buf,
						size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
					ptr->server_state = STATE::SELECTRESULT;			// 클라이언트에게 메뉴번호를 받아 그 메뉴에 대한 연산을 위해 state를 SELECTRESULT로 이동
					ptr->sendbytes = size;								// sendbytes에 값을 입력하여 wset이 작동하지 않도록 함.
				}
				break;
				// 클라이언트가 번호를 서버에게 전달 후의 상태
			case STATE::SELECTRESULT:
				if (FD_ISSET(ptr->sock, &rset))
				{
					printf("rset 통과...\n");
					// 받는 작업
					switch (NonBlock_recv(ptr, &size, i))				// define 해놓은 int값을 리턴하는 함수
					{
					case CONNECTERROR:
					case DISCONNECTED:									// 연결 에러 혹은 연결이 끊기면 endflag를 켜서 while문을 탈출한다.
						endflag = true;
						break;
					case WAIT_RECEIVE:									// 패킷을 덜 받은 상태이면 continue를 통해 while문의 초기상태로 돌아간다.
						continue;
						break;
					case SUCCESS_RECEIVE:								// 패킷을 완전히 받은 상태이면 작업 시작
						switch (GetProtocol(ptr->buf))
						{
							// 메뉴에서 번호를 선택 후 그것에 맞는 메세지를 보낸다
						case PROTOCOL::INTROSELECT:
							unpacking(ptr->buf, &number);
							switch (number)
							{
							case 1:										// 회원가입
								size = packing(PROTOCOL::SIGN, SIGNMSG, ptr->buf);
								break;
							case 2:										// 로그인
								size = packing(PROTOCOL::LOGIN, LOGINMSG, ptr->buf);
								break;
							case 3:										// 종료는 프로토콜 패킷만을 보낸다.
								size = packing(PROTOCOL::EXIT, ptr->buf);
								break;
							default:									// 1~3 이외의 것은 에러 메세지를 보내고 다시 INTRO로 이동
								size = packing(PROTOCOL::INTROERROR, INTRO_ERROR_MSG, ptr->buf);
								ptr->server_state = STATE::INTRO;
								ptr->sendbytes = 0;
								break;
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							break;
							// 회원가입 프로토콜
						case PROTOCOL::SIGN:
							unpacking(ptr->buf, ID, PW, &money);							// 아이디, 비밀번호, 초기입금액을 받았다
							for (int i = 0; i < User_Count; i++)
							{
								if (!strcmp(userInfo[i]->ID, ID))							// 아이디 중복이 있으면 중복 메세지를 보낸다.
								{
									sign = false;
									strcpy_s(msg, ID_OVERLAP_MSG);							// 아이디 중복 메세지
									break;
								}
							}
							if (sign)														// 중복이 없으면 회원가입을 진행.
							{
								strcpy_s(msg, SIGN_UP_MSG);									// 회원가입 성공메세지
								// 함수를 사용하여 유저 정보 입력
								userInfo[User_Count++] = newUserRegist(ID, PW, money);		// userInfo 전역변수에 등록
							}
							size = packing(PROTOCOL::SIGN, msg, ptr->buf);
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->sendbytes = 0;												// wset을 쓰기 위해 다시 0으로 초기화
							ptr->server_state = STATE::INTRO;								// 다시 INTRO 이동
							break;
							// 로그인 프로토콜
						case PROTOCOL::LOGIN:
							unpacking(ptr->buf, ID, PW);									// 클라이언트에게 아이디 비밀번호를 받는다.
							for (int i = 0; i < User_Count; i++)
							{
								if (!strcmp(ID, userInfo[i]->ID)) {							// 같은 아이디가 있으면 flag를 true
									flag = true;
									if (!strcmp(PW, userInfo[i]->PW)) {						// 패스워드도 같은게 있다면...
										if (userInfo[i]->state == true) {					// 이미 로그인 중이라면 이미 로그인 중이라는 메세지를 패킹.
											result = RESULT::ALREADY_LOGIN;
											strcpy_s(msg, ALREADY_LOGIN_MSG);
											ptr->server_state = STATE::INTRO;
										}
										else {												// 로그인 중이 아니라면 로그인 성공과 함께 성공 메세지 패킹 후 state 변경
											result = RESULT::LOGINSUCESS;
											// 함수를 사용하여 유저 정보 입력							
											ptr->user = userInfo[i];
											ptr->user->state = true;
											strcpy_s(msg, LOGIN_MSG);
											ptr->server_state = STATE::LOGININTRO;
										}
									}
									else {													// 패스워드가 다르면 패스워드 에러 메시지 패킹
										result = RESULT::PW_ERROR;
										strcpy_s(msg, PW_ERROR_MSG);
										ptr->server_state = STATE::INTRO;
									}
								}
							}
							if (!flag) {													// flag가 안켜져 있으면 같은 아이디가 없으므로 아이디 오류 메세지 패킹
								result = RESULT::ID_ERROR;
								strcpy_s(msg, ID_ERROR_MSG);
								ptr->server_state = STATE::INTRO;
							}
							size = packing(PROTOCOL::LOGIN, result, msg, ptr->buf);
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->sendbytes = 0;												// 로그인을 성공 하나 실패하나 메뉴 출력을 해야하기 때문에 wset을 쓰므로 sendbytes를 0으로 한다.
							break;
						case PROTOCOL::EXIT:												// 클라이언트가 종료 패킷을 보내면 그 클라이언트의 소켓을 제거한다.
							RemoveSocketInfo(i);
							break;
						}
						break;
					}
				}							// if문
				break;
				//	로그인 성공 직후 메뉴 출력 상태
			case STATE::LOGININTRO:
				strcpy(msg, "");
				for (int i = 0; i < Item_Count; i++)										// 경매품 갯수만큼 반복
				{
					if (ItemList[i]->client_count == BIDMAX)								// 경매품의 클라이언트 참여 인원 수가 꽉찬다면 함수 실행
					{
						strcat(msg, Bid_Check(i, ptr));										// 함수는 메세지를 리턴한다.
					}
				}
				if (FD_ISSET(ptr->sock, &wset)) {
					if (Item_Count == 0)													// 경매품이 모두 낙찰되면 경매 종료 메세지와 로그인 메뉴를 같이 패킹
					{
						strcat(msg, AUCTION_END_MSG);
						strcat(msg, LOGININTROMSG);
						size = packing(PROTOCOL::INTROSELECT, msg, ptr->buf);
					}
					else																	// 그 외의 경우에는 로그인 메뉴만 패킹
					{
						strcat(msg, LOGININTROMSG);
						size = packing(PROTOCOL::INTROSELECT, msg, ptr->buf);
					}
					retval = send(ptr->sock, ptr->buf, size, 0);
					if (retval == SOCKET_ERROR)
					{
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
					ptr->sendbytes = size;													// wset 막기
					ptr->server_state = STATE::LOGINSELECTRESULT;
				}
				break;
				// 로그인 메뉴에서 클라이언트가 고른 메뉴에 대해 처리하는 상태
			case STATE::LOGINSELECTRESULT:
				if (FD_ISSET(ptr->sock, &rset))
				{
					switch (NonBlock_recv(ptr, &size, i))
					{
					case CONNECTERROR:
					case DISCONNECTED:
						endflag = true;
						break;
					case WAIT_RECEIVE:
						continue;
						break;
					case SUCCESS_RECEIVE:
						switch (GetProtocol(ptr->buf))
						{
							// 로그인 메뉴 번호 선택
						case PROTOCOL::INTROSELECT:
							unpacking(ptr->buf, &number);
							switch (number)
							{
								// 입금은 입금 안내 메세지를 보낸다.
							case 1:
								size = packing(PROTOCOL::DEPOSIT, DEPOSITMSG, ptr->buf);
								break;
								// 경매 참여는 경매품 내역을 보낸다.
							case 2:
								strcpy(msg, JOIN_MSG);
								if (Item_Count > 0) {										// 경매품이 다 팔리지 않을 경우
									for (int i = 0; i < Item_Count; i++)
									{
										if (!ItemList[i]->sold_out) {
											if (ItemList[i]->min_price > 10000000) {
												sprintf(temp, "  %d    %s\t\t %d\t\t\t%d\n", i + 1, ItemList[i]->Itemname,
													ItemList[i]->min_price,
													ItemList[i]->client_count);
											}
											else {
												sprintf(temp, "  %d    %s\t\t %d\t\t\t\t%d\n", i + 1, ItemList[i]->Itemname,
													ItemList[i]->min_price,
													ItemList[i]->client_count);
											}
											strcat(msg, temp);
										}
									}
									strcat(msg, AUCTION_SELECT_MSG);
								}
								else														// 경매품이 다 팔렸을 경우
								{
									strcpy(msg, AUCTION_END_MSG2);
								}
								size = packing(PROTOCOL::JOIN, msg, ptr->buf);
								break;
								// 온라인통장을 골랐을 경우 금액을 보여주는 메세지를 패킹
							case 3:
								sprintf(msg, "현재 가지고 계신 금액 : %d(%d)원입니다. *괄호 안은 입찰 한 금액을 뺀 가격*\n", ptr->user->Usermoney,
									ptr->user->Usermoney - ptr->user->temp_use_money);
								size = packing(PROTOCOL::BANK, msg, ptr->buf);
								break;
								// 경매 내역은 여태까지 클라이언트가 입찰한 내역을 보여준다.
							case 4:
								strcpy(msg, VERIFY_MSG);
								for (int i = 0; i < ptr->user->UserBidCount; i++)								// 입찰한 수 만큼 
								{
									if (ptr->user->bid[i]->item->sold_out && ptr->user->bid[i]->bidflag == true)// 아이템이 팔렸다면 
									{
										if (ptr->user->bid[i]->win == true) {									// 자신이 낙찰에 성공했다면..
											if (ptr->user->bid[i]->price > 1000000) {
												sprintf(temp, "  %d   %s\t\t %d\t\t낙찰 성공\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
											else {
												sprintf(temp, "  %d   %s\t\t %d\t\t\t낙찰 성공\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
										}
										else {																	// 실패한 경우..
											if (ptr->user->bid[i]->price > 1000000) {
												sprintf(temp, "  %d   %s\t\t %d\t\t낙찰 실패\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
											else {
												sprintf(temp, "  %d   %s\t\t %d\t\t\t낙찰 실패\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
										}
									}
									else																		// 아이템이 팔리지 않았다면
									{
										if (ptr->user->bid[i]->price > 1000000) {
											sprintf(temp, "  %d   %s\t\t %d\t\t 경매 중\n", i + 1, ptr->user->bid[i]->item->Itemname,
												ptr->user->bid[i]->price);
										}
										else {
											sprintf(temp, "  %d   %s\t\t %d\t\t\t 경매 중\n", i + 1, ptr->user->bid[i]->item->Itemname,
												ptr->user->bid[i]->price);
										}
									}
									strcat(msg, temp);
								}
								size = packing(PROTOCOL::VERIFY, msg, ptr->buf);
								break;
								// 입찰에 성공해 성공한 경매품 내역을 보여준다.
							case 5:
								strcpy(msg, LIST_MSG);
								for (int i = 0; i < ptr->user->UserItemCount; i++)								// 클라이언트가 가진 아이템 수만큼
								{
									sprintf(temp, "  %d   %s\t\t %d\n", i + 1, ptr->user->UserItem[i]->Itemname, ptr->user->UserItem[i]->price);
									strcat(msg, temp);
								}
								size = packing(PROTOCOL::LIST, msg, ptr->buf);
								break;
								// 경매 내역을 지워주는 패킷을 보내준다.
							case 6:
								size = packing(PROTOCOL::VERIFYDELETE, VERIFY_DELETE_MSG, ptr->buf);
								break;
								// 로그아웃 패킷을 보낸다.
							case 7:
								size = packing(PROTOCOL::LOGOUT, LOGOUT_MSG, ptr->buf);
								break;
								// 그외의 번호는 에러 메세지를 보내고 다시 LOGININTRO로 이동
							default:
								size = packing(PROTOCOL::INTROERROR, INTRO_ERROR_MSG, ptr->buf);
								ptr->server_state = STATE::LOGININTRO;
								ptr->sendbytes = 0;
								break;
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							break;
							// 안내 메세지를 보내고 클라이언트가 금액을 보낸다면..
						case PROTOCOL::DEPOSIT:
							unpacking(ptr->buf, &money);
							if (money < 0)							// 0보다 작은 수는 즉 출금
							{
								size = packing(PROTOCOL::DEPOSIT, DEPOSIT_ERROR_MSG, ptr->buf);
							}
							else									// 양수일 경우는 입금 처리
							{
								ptr->user->Usermoney += money;
								sprintf(msg, "입력하신 %d원이 입금되었습니다!\n", money);
								size = packing(PROTOCOL::DEPOSIT, msg, ptr->buf);
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->server_state = STATE::LOGININTRO;	// 입금 후 다시 메뉴로..
							ptr->sendbytes = 0;
							break;
							// 경매 참여 패킷을 보내고 경매품의 숫자를 받았다면..
						case PROTOCOL::JOIN:
							unpacking(ptr->buf, &number);
							if (number > Item_Count)				// 아이템 수보다 번호가 클 경우는 에러 메세지
							{
								size = packing(PROTOCOL::JOIN, RESULT::JOIN_ERROR, JOIN_ERROR_MSG, ptr->buf);
								ptr->server_state = STATE::LOGININTRO;
								ptr->sendbytes = 0;
							}
							else if (number == 0)					// 0번은 돌아가기 메뉴
							{
								size = packing(PROTOCOL::JOIN, RESULT::JOIN_ERROR, MENU_RETURN_MSG, ptr->buf);
								ptr->server_state = STATE::LOGININTRO;
								ptr->sendbytes = 0;
							}
							else									// 그외의 경우는 입찰가를 받기위한 안내 메세지를 보내고 경매를 위한 state로 넘어간다. (JOIN_SUCCESS_STATE)
							{
								ptr->user->bidnumber = number;		// 유저가 입찰할 아이템 번호를  bidnumber에 저장
								sprintf(msg, "%d번 물품의 희망 입찰가를 적어주세요 >>", number);
								size = packing(PROTOCOL::JOIN, RESULT::JOIN_SUCCESS, msg, ptr->buf);
								ptr->server_state = STATE::JOIN_SUCCESS_STATE;
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							break;
							// 온라인통장, 경매내역, 구매내역은 모두 안내 메세지를 보내고 끝나므로 다시 LOGININTRO로 돌아가 메뉴를 보여주도록 한다.
						case PROTOCOL::BANK:
						case PROTOCOL::VERIFY:
						case PROTOCOL::LIST:
							ptr->server_state = STATE::LOGININTRO;
							ptr->sendbytes = 0;
							break;
							// 완료된 경매 내역을 지우는 프로토콜
						case PROTOCOL::VERIFYDELETE:
							for (int i = 0; i < ptr->user->UserBidCount; i++)								// 입찰한 수 만큼
							{
								if (ptr->user->bid[i]->item->sold_out)										// 입찰한 경매품이 팔렸다면..
								{
									if (i != ptr->user->UserBidCount - 1)									// 완전 끝이거나 1개 있을 경우가 아니면 삭제할 자리에 맨끝 자리의 아이템을 옮긴다. 
									{
										ptr->user->bid[i] = ptr->user->bid[ptr->user->UserBidCount - 1];
										i--;																// i를 감소시키고 바뀐 아이템을 검사
									}
									else																	// 완전 끝이거나 1개 있을 경우 그 자리를 그대로 제거
									{
										ptr->user->bid[i] = NULL;
									}
									--ptr->user->UserBidCount;												// 카운트 감소
								}
							}
							ptr->server_state = STATE::LOGININTRO;											// 다시 로그인 메뉴
							ptr->sendbytes = 0;
							break;
							// 로그아웃 프로토콜
						case PROTOCOL::LOGOUT:
							ptr->user->state = false;														// 로그인 상태 해제
							ptr->user = NULL;																// 소켓에 유저정보 지우기
							ptr->server_state = STATE::INTRO;												// 초기 메뉴로 STATE 이동
							ptr->sendbytes = 0;
							break;
						}
						break;
					}
				}
				break;
				// 경매 참여에서 경매품을 고르고 입찰가를 클라이언트에서 받아온 후 처리해야할 상태
			case STATE::JOIN_SUCCESS_STATE:
				if (FD_ISSET(ptr->sock, &rset))
				{
					switch (NonBlock_recv(ptr, &size, i))
					{
					case CONNECTERROR:
					case DISCONNECTED:
						endflag = true;
						break;
					case WAIT_RECEIVE:
						continue;
						break;
					case SUCCESS_RECEIVE:
						switch (GetProtocol(ptr->buf))
						{
						case PROTOCOL::JOIN:
							unpacking(ptr->buf, &money);
							if (money > (ptr->user->Usermoney - ptr->user->temp_use_money) ||													// 받아온 금액이 유저가 현재 가진 금액 - 경매품에 입찰한 금액 보다 크면 불가능.
								ItemList[ptr->user->bidnumber - 1]->min_price > money)
							{
								size = packing(PROTOCOL::JOIN, MONEY_ERROR_MSG, ptr->buf);
							}
							else																												// 가능하면 유저가 입찰할 아이템 번호를 bidnumber로 받아 bid_price에 입찰가를 저장
							{
								ItemList[ptr->user->bidnumber - 1]->
									bid_price[ItemList[ptr->user->bidnumber - 1]->client_count] = money;
								ptr->user->temp_use_money += money;																				// 입찰에 사용한 돈 변수에 입찰할 돈을 추가한다.
								strcpy(ItemList[ptr->user->bidnumber - 1]->
									clientsID[ItemList[ptr->user->bidnumber - 1]->client_count++], ptr->user->ID);								// 입찰한 유저 닉네임을 저장
								ptr->user->bid[ptr->user->UserBidCount++] = User_Biditem_Set(ItemList[ptr->user->bidnumber - 1], money);		// 입찰한 경매품 배열에 입찰품을 넣는다.
								ptr->user->bidnumber = 0;																						// 입찰 번호 초기화
								size = packing(PROTOCOL::JOIN, AUCTION_SUCCESS_MSG, ptr->buf);
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->server_state = STATE::LOGININTRO;																				// 다시 로그인 메뉴
							ptr->sendbytes = 0;
							break;
						}
						break;
					}
				}
				break;
			}	// STATE

			if (endflag) break;
		}

	}// while문 끝

	// 윈속 종료
	WSACleanup();
	return 0;
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}
	ptr->user = NULL;
	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	ptr->successrecvbytes = 0;
	ptr->recvflag = false;
	ptr->server_state = STATE::INTRO;
	SocketInfoArray[nTotalSockets++] = ptr;

	return TRUE;
}
// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];

	// 클라이언트 정보 얻기
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

	--nTotalSockets;
}
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
int packing(enum PROTOCOL protocol, const char* msg, char* buf)
{
	int strsize;
	int size = 0;
	char* ptr;
	strsize = strlen(msg);

	ptr = buf + sizeof(int);			// 총사이즈의 자리를 남겨둔다.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// 프로토콜 패킹
	size = size + sizeof(enum PROTOCOL);				// 사이즈 증가
	ptr = ptr + sizeof(enum PROTOCOL);					// 위치 이동

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, msg, strsize);
	size = size + strsize;

	ptr = buf;											// 모든 작업이 끝나면 처음 자리로 돌아온다.		
	memcpy(ptr, &size, sizeof(int));					// 총 사이즈를 입력한다.

	return size + sizeof(int);
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
PROTOCOL GetProtocol(char* _buf) {
	PROTOCOL protocol;
	memcpy(&protocol, _buf, sizeof(enum PROTOCOL));
	return protocol;
}
char* msg_unpacking(const char* buf) {
	int strsize;
	char msg[BUFSIZE];
	const char* ptr = buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&strsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);
	memcpy(msg, ptr, strsize);

	msg[strsize] = '\0';
	printf("%s\n", msg);

	return msg;
}
void unpacking(const char* _buf, char* ID, char* PW) {				// 클라이언트에게 받은 아이디 패스워드 패킷을 언패킹하는 작업함수
	int idsize;
	int pwsize;
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&idsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(ID, ptr, idsize);				// 새로만든 구조체의 아이디변수에 가져온 아이디를 입력
	ptr = ptr + idsize;

	ID[idsize] = '\0';

	memcpy(&pwsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(PW, ptr, pwsize);				// 새로만든 구조체의 비밀번호변수에 가져온 비밀번호를 입력

	PW[pwsize] = '\0';
}
void unpacking(const char* _buf, char* ID, char* PW, int* money) {				// 클라이언트에게 받은 아이디 패스워드 패킷을 언패킹하는 작업함수
	int idsize;
	int pwsize;
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&idsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(ID, ptr, idsize);				// 새로만든 구조체의 아이디변수에 가져온 아이디를 입력
	ptr = ptr + idsize;

	ID[idsize] = '\0';

	memcpy(&pwsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(PW, ptr, pwsize);				// 새로만든 구조체의 비밀번호변수에 가져온 비밀번호를 입력
	ptr = ptr + pwsize;

	PW[pwsize] = '\0';

	memcpy(money, ptr, sizeof(int));
}
void unpacking(const char* _buf, int* money) {
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(money, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

}
int NonBlock_recv(SOCKETINFO* ptr, int* size, int i)
{
	int retval;

	if (ptr->recvbytes != sizeof(int))
	{
		retval = recv(ptr->sock, (char*)size + ptr->recvbytes, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			RemoveSocketInfo(i);
			return CONNECTERROR;
		}
		else if (retval == 0) {
			RemoveSocketInfo(i);
			return DISCONNECTED;
		}

		ptr->recvbytes += retval;

		if (ptr->recvbytes == sizeof(int)) {
			ptr->recvflag = true;
		}
	}

	if (ptr->recvflag)
	{
		retval = recv(ptr->sock, ptr->buf + ptr->successrecvbytes, *size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			RemoveSocketInfo(i);
			return CONNECTERROR;
		}
		else if (retval == 0) {
			RemoveSocketInfo(i);
			return DISCONNECTED;
		}

		ptr->successrecvbytes += retval;
	}

	if (ptr->successrecvbytes == *size) {
		ptr->recvflag = false;
		ptr->successrecvbytes = 0;
		ptr->recvbytes = 0;
		return SUCCESS_RECEIVE;
	}
	else {
		return WAIT_RECEIVE;
	}

}
int packing(enum PROTOCOL protocol, enum RESULT result, const char* msg, char* buf)	// 클라이언트에 보낼 프로토콜 및 메세지를 패킹작업하는 함수
{
	int strsize;
	int size = 0;
	char* ptr;
	strsize = strlen(msg);

	ptr = buf + sizeof(int);			// 총사이즈의 자리를 남겨둔다.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// 프로토콜 패킹
	size = size + sizeof(enum PROTOCOL);				// 사이즈 증가
	ptr = ptr + sizeof(enum PROTOCOL);					// 위치 이동


	memcpy(ptr, &result, sizeof(enum RESULT));			// 결과 패킹
	size = size + sizeof(enum RESULT);					// 사이즈 증가
	ptr = ptr + sizeof(enum RESULT);					// 위치 이동

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, msg, strsize);
	size = size + strsize;

	ptr = buf;											// 모든 작업이 끝나면 처음 자리로 돌아온다.		
	memcpy(ptr, &size, sizeof(int));					// 총 사이즈를 입력한다.

	return size + sizeof(int);										// 총 사이즈를 리턴
}
UserInfo* newUserRegist(const char* ID, const char* PW, int money) 
{
	UserInfo* ptr = new UserInfo;

	if (ptr == NULL) {
		printf("유저 등록 오류!");
		return NULL;
	}
	ptr->UserItemCount = 0;
	ptr->UserBidCount = 0;
	ptr->state = false;
	strcpy(ptr->ID, ID);
	strcpy(ptr->PW, PW);
	ptr->Usermoney = money;
	ptr->temp_use_money = 0;

	return ptr;
}
void AuctionSetting(int _i) 
{
	AuctionItem* item = new AuctionItem;
	item->client_count = 0;
	item->sold_out = false;

	switch (_i) {
	case 0:
		strcpy(item->Itemname, "ROLEX SUBMARINER 18K");
		item->min_price = 17000000;
		break;
	case 1:
		strcpy(item->Itemname, "SUPREME HOOD BLACK");
		item->min_price = 500000;
		break;
	case 2:
		strcpy(item->Itemname, "PEACEMINUSONE AIR FORCE");
		item->min_price = 700000;
		break;
	}
	item->price = item->min_price;
	ItemList[Item_Count++] = item;
}
Bid_item* User_Biditem_Set(AuctionItem* item, int money) 
{
	Bid_item* ptr = new Bid_item;
	ptr->item = item;
	//memcpy(&ptr->item, &item, sizeof(AuctionItem));
	ptr->price = money;
	ptr->bidflag = false;
	ptr->win = false;
	return ptr;
}
char* Bid_Check(int i, SOCKETINFO* ptr)
{
	// 경매품의 최대 입찰 인원수가 찼다면 가장 큰 금액을 추려내 그 금액을 입찰한 고객을 찾아 낙찰 시키고 결과에 따라 클라이언트들에게 메세지를 다르게 보낸다.
	char msg[BUFSIZE + 1];
	int max = 0;
	bool itemdelete = true;
	AuctionItem* ptritem;
	bool flag = false;
	bool tiemax = false;

	strcpy(msg, "");
	ptritem = ItemList[i];																						// 아이템 for문을 돌려 최대 입찰 인원수가 찼다면 번호 i를 받아 포인터로 만든다. 즉, ptritem은 클라이언트가 꽉찬 낙찰을 해야 할 경매품
	for (int j = 0; j < ptritem->client_count; j++)																// 입찰한 클라이언트 수만큼 진행
	{
		if (max < ptritem->bid_price[j])																		// 가장 높은 금액을 추려낸다.
			max = ptritem->bid_price[j];
	}

	for (int i = 0; i < ptritem->client_count; i++)																// 가장 높은 금액이 여러개 있는 경우  tiemax를 true로 만든 뒤 다른 연산을 진행
	{
		if (flag && max == ptritem->bid_price[i]) 
		{
			tiemax = true;
		}
		if (max == ptritem->bid_price[i]) 
		{
			flag = true;
		}
	}

	for (int k = 0; k < ptr->user->UserBidCount; k++)															// 유저가 입찰한 수 만큼
	{
		if (ptr->user->bid[k]->item == ptritem &&																// 유저가 입찰한 아이템 중 ptritem과 일치하고.. 클라이언트가 bid_check를 하지 않았을 경우
			ptr->user->bid[k]->bidflag == false)
		{
			ptritem->sold_out = true;																			// 팔렸다는 bool 변수를 킨다.
			ptritem->price = max;																				// 경매품의 최종가를 가장 높은 금액으로 한다.
			if (!tiemax) {
				if (ptr->user->bid[k]->price == max)									// 클라이언트의 입찰가가 가장 높은 금액이고 팔리지 않았을 경우..
				{
					ptr->user->bid[k]->win = true;																	// 자신이 입찰에 성공했으므로 true
					ptr->user->Usermoney -= ptr->user->bid[k]->price;												// 금액만큼 돈을 제한다.
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// 이 경매품에 쓴 입찰금을 없앤다.
					//ptr->user->UserItem[ptr->user->UserItemCount++] = ptritem;
					memcpy(&ptr->user->UserItem[ptr->user->UserItemCount++], &ptritem, sizeof(AuctionItem));		// 클라이언트가 구매한 아이템 배열에 ptritem을 넣는다. (구매내역 표시)
					sprintf(msg, "[공지] 입찰하신 %s 물품이 당신에게 낙찰되었습니다!\n", ptritem->Itemname);			// 성공 메세지 패킹
				}
				else
				{
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// 이 경매품 입찰에 사용된 돈을 제거
					sprintf(msg, "[공지] 입찰하신 %s 물품이 낙찰에 실패하였습니다..\n", ptritem->Itemname);				// 실패 메세지 패킹
				}
			}
			else																									// tiemax가 켜져있으면 입찰순으로 가장 먼저 들어온 클라이언트에게 낙찰한다.
			{
				if (ptr->user->bid[k]->price == max)																// 클라이언트의 입찰가가 가장 높은 금액이고 팔리지 않았을 경우..
				{
					if (bid_order(ptritem, ptr->user->ID, max))															// TRUE인 경우 자신보다 빠른 입찰자가 없다는 뜻. FALSE 경우는 자신보다 빠른 입찰자가 있다는 뜻.
					{
						ptr->user->bid[k]->win = true;																// 자신의 입찰이 성공했으므로 true
						ptr->user->Usermoney -= ptr->user->bid[k]->price;											// 금액만큼 돈을 제한다.
						ptr->user->temp_use_money -= ptr->user->bid[k]->price;										// 이 경매품에 쓴 입찰금을 없앤다.
						//ptr->user->UserItem[ptr->user->UserItemCount++] = ptritem;
						memcpy(&ptr->user->UserItem[ptr->user->UserItemCount++], &ptritem, sizeof(AuctionItem));	// 클라이언트가 구매한 아이템 배열에 ptritem을 넣는다. (구매내역 표시)
						sprintf(msg, "[공지] 입찰하신 %s 물품이 당신에게 낙찰되었습니다!\n", ptritem->Itemname);
					}
					else 
					{
						ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// 이 경매품 입찰에 사용된 돈을 제거
						sprintf(msg, "[공지] 입찰하신 %s 물품이 낙찰에 실패하였습니다..\n", ptritem->Itemname);				// 실패 메세지 패킹
					}
				}
				else
				{
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// 이 경매품 입찰에 사용된 돈을 제거
					sprintf(msg, "[공지] 입찰하신 %s 물품이 낙찰에 실패하였습니다..\n", ptritem->Itemname);				// 실패 메세지 패킹
				}
			}
			ptr->user->bid[k]->bidflag = true;																	// 함수를 모두 돌았으므로 플래그 체킹

			// 입찰한 유저가 모두 이 함수를 거쳐야한다.
			for (int j = 0; j < nTotalSockets; j++)																// 이 아이템을 입찰한 유저들이 이 함수를 모두 돌아야 아이템 제거가 가능하다.
			{
				SOCKETINFO* temp = SocketInfoArray[j];
				for (int a = 0; a < temp->user->UserBidCount; a++)
				{
					if (temp->user->bid[a]->item == ptritem &&													// 한명이라도 이 함수를 돌지 않을 경우 그대로 리턴해서 함수 종료
						temp->user->bid[a]->bidflag == false)
					{
						return msg;
					}
					else
					{
						itemdelete = false;																		// 모두 돌았을 경우 itemdelete 변수 끈다.
					}
				}
			}
		}
	}

	if (!itemdelete) {																							// 모두 돌았을 경우 팔린 경매품을 리스트에서 제거
		if (i != Item_Count - 1)
		{
			ItemList[i]->client_count = 0;
			ItemList[i] = ItemList[Item_Count - 1];
		}
		else
		{
			ItemList[i]->client_count = 0;
			ItemList[i] = NULL;
		}
		--Item_Count;
	}

	return msg;
}


bool bid_order(AuctionItem* _ptritem, const char* _id, int max)
{
	int number = 0;

	// 현재 입찰한 유저의 순서를 알아낸다.
	for (int i = 0; i < _ptritem->client_count; i++) {
		if (!strcmp(_ptritem->clientsID[i], _id)) 
		{
			number = i;
			break;
		}
	}

	// 현재 입찰한 유저의 앞까지 같은 아이템을 입찰한 유저를 찾아 그 유저의 입찰금과 최대금액이 같으면 앞 사람이 이겨야하므로 FALSE
	for (int i = 0; i < number; i++)
	{
		for (int j = 0; j < nTotalSockets; j++)
		{
			SOCKETINFO* temp = SocketInfoArray[j];
			if (!strcmp(temp->user->ID, _ptritem->clientsID[i])) 
			{
				for (int k = 0; k < temp->user->UserBidCount; k++)
				{
					if (temp->user->bid[k]->item == _ptritem && 
						temp->user->bid[k]->price == max) 
					{
						return FALSE;
					}
				}
			}
		}
	}

	// 없으면 TRUE
	return TRUE;
}