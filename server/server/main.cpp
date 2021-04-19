/*���� 3A 18032048 ����ö
���� �߰����*/

/* ��� �ý����� 3���� ���ǰ���� �̷���� ������, ������ ���ǰ���� �ּұݾ��� �������ִ�.
Ŭ���̾�Ʈ���� ���ǰ�� �����ϱ� ���ؼ��� �ּұݾ׺��� ���� ���� �����ϰ� �־�� �ϸ� �������� �ּұݾ� �̻����� ������ �ؾ��Ѵ�. (�ּұݾ����� ���� ����)
���� �������� ���� Ŭ���̾�Ʈ���� �����Ǹ� ���ǰ�� ���� �������� ���� ���� Ŭ���̾�Ʈ���� ���Եǰ� ������� ����Ʈ���� �������.
������ Ŭ���̾�Ʈ ���ų����� ���ο��� ������ ���ǰ�� ������ ���������� ������ ������ ���ǰ ����Ʈ�� �����ش�.
���� �������� ������ Ŭ���̾�Ʈ�� ������ ��� ���� ���� ������ Ŭ���̾�Ʈ���� ������ �ȴ�.
�� ���� ��ǰ Ŭ������ ���̵� �迭�� ����� ������ ������ ���̵� �Ź� �迭�� �Է��ϰ� �ȴ�.
�׸��� �迭�� 0��° �ڸ� �� ù��°�� ������ ������ �г��Ӱ� ��ġ�ϰ� �Ǹ� ������ ��Ű�� �ý������� �������.*/
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT		9000
#define BUFSIZE			512
#define ITEMMAX			100							// ������ �ִ�� �������� ���� �� �ִ� ����
#define USER			100							// ȸ������ �ִ� ȸ�� ��
#define SIZE			16							// �̸� ������
#define ITEMNAMESIZE	50							// ������ �̸� ������
#define INTROMSG "1. ȸ������\n2. �α���\n3. ����\n���ڸ� �Է��ϼ��� >>"
#define SIGNMSG "\n[ȸ������ ������]\n"
#define LOGINMSG "\n[�α��� ������]\n"
#define ID_ERROR_MSG "[ERROR] ���� ID�Դϴ�.\n"
#define PW_ERROR_MSG "[ERROR] PW�� Ʋ�Ƚ��ϴ�.\n"
#define LOGIN_MSG "�α��ο� �����߽��ϴ�.\n"
#define ID_OVERLAP_MSG "[ERROR] �ߺ��� ���̵��Դϴ�.\n"
#define ALREADY_LOGIN_MSG "�̹� ���̵� ���ӻ����Դϴ�.\n"
#define SIGN_UP_MSG "ȸ�����Կ� �����߽��ϴ�.\n"

#define JOIN_MSG "\n[��� ������]  (����0�� �޴��� ���ư���)\n ��ȣ\t\t�̸�\t\t\t��������\t\t���� Ŭ���̾�Ʈ ��\n"
#define VERIFY_MSG "\n[���� ����]\n ��ȣ\t\t�̸�\t\t\t���� ����\t\t���� ����\n"
#define LIST_MSG "\n[���� ����]\n ��ȣ\t\t�̸�\t\t\t���� ����\n"
#define LOGOUT_MSG "�α׾ƿ� �ϼ̽��ϴ�.\n"
#define INTRO_ERROR_MSG "[ERROR] �ùٸ� ��ȣ�� �ƴմϴ�.\n"
#define LOGININTROMSG "1. �Ա�\n2. �������\n3. �¶�������\n4. ��������\n5. ���ų���\n6. ������������\n7. �α׾ƿ�\n���ڸ� �Է��ϼ��� >>"
#define DEPOSITMSG "�Ա��Ͻ� �ݾ��� �Է����ּ��� >>"
#define DEPOSIT_ERROR_MSG "[ERROR] �Աݸ� �����մϴ�.\n"
#define JOIN_ERROR_MSG "[ERROR] �Է��Ͻ� ��ȣ�� �´� ���ǰ�� �����ϴ�!\n"
#define MONEY_ERROR_MSG "���� �����ϰų� �ּ� �ݾ��� ���� �ݾ��� �Է��ϼž� �մϴ�.\n"
#define AUCTION_SUCCESS_MSG "���� �Ϸ�!!\n"
#define MENU_RETURN_MSG "�޴��� ���ư��ϴ�.\n"

#define VERIFY_DELETE_MSG "������ ������ ��Ͽ��� ����ϴ�..\n"
#define AUCTION_END_MSG "[����] ���ǰ�� ��� �����Ǿ� ��Ű� ����Ǿ����ϴ�!!\n"

#define AUCTION_SELECT_MSG "�����Ͻ� ��ȣ�� �Է����ּ��� >>"
#define AUCTION_END_MSG2 "\t\t�� �� �� �� (���ư���� ����0 �Է�)\n\t\t"

#define SUCCESS_RECEIVE 1
#define WAIT_RECEIVE 0
#define DISCONNECTED -1
#define CONNECTERROR -2

#define AUCTIONITEMSIZE 3							// ���ǰ ���� ���� 3��
#define BIDMAX 2									// ���ǰ ���� �ִ� �ο� ��

// ���� ���� ������ ���� ����ü�� ����


enum PROTOCOL {
	NODATA = -1,
	INTROSELECT = 1,
	INTROERROR,										// �޴� ���� ����
	SIGN,											// ȸ������
	LOGIN,											// �α���
	DEPOSIT,										// �Ա�
	JOIN,											// �������
	BANK,											// �¶�������
	VERIFY,											// ��ų���
	LIST,											// ���ų���
	VERIFYDELETE,									// ��ų��������
	LOGOUT,											// �α׾ƿ�
	EXIT											// ������
};

enum RESULT {
	NOTHING = -1,
	ID_ERROR = 1,									// ���̵� ����
	PW_ERROR,										// ��й�ȣ ����
	LOGINSUCESS,									// �α��� ����
	ALREADY_LOGIN,									// �̹� �α���
	JOIN_ERROR,										// ��� ���� ���� => �� ���� or �ּұݾ� �̴�
	JOIN_SUCCESS									// ��� ���� ����
};

enum STATE {
	NOTSTATE = -1,
	INTRO = 1,										// �ʱ� ����
	SELECTRESULT,									// �ʱ� ���¿��� �޴��� �� ����
	INPUTRESULT,									// SELECTRESULT���� �Է°��� ������ ������� �޾ƿ� ��
	LOGININTRO,										// �α��� ���� �޴��� ������ ��
	LOGINSELECTRESULT,								// �α��� �� �޴��� �� ����
	LOGININPUTRESULT,								// LOGINSELECTRESULT���� �Է°��� ������ ������� �޾ƿ� ��
	JOIN_SUCCESS_STATE,								// ��� ������ ������� �޾ƿ� ��
};

struct AuctionItem									// ���ǰ Ŭ����
{
	char clientsID[BIDMAX][SIZE];					// ��ſ� ������ Ŭ���̾�Ʈ ���̵�
	int bid_price[BIDMAX];							// ���ǰ�� ������ �ݾ��� ��� �迭
	char Itemname[ITEMNAMESIZE];					// ���ǰ �̸�
	int price;										// ���ǰ�� ��������
	int min_price;									// ���ǰ�� �ּҰ���
	int client_count;								// ���ǰ�� ������ Ŭ���̾�Ʈ ��
	bool sold_out;									// ���ǰ�� �ȷȴ��� ���ȷȴ����� ����
};

struct Bid_item										// Ŭ���̾�Ʈ�� ������ ���ǰ
{
	AuctionItem* item;								// ���ǰ
	int price;										// Ŭ���̾�Ʈ�� ������ ����
	bool bidflag;									// bid_check��� �Լ��� ������ ��� true
	bool win;										// ��ſ��� �ڽ��� ���� �������� ��� true
};

struct UserInfo										// Ŭ���̾�Ʈ�� ���� Ŭ����
{
	char ID[SIZE];									// ���̵�
	char PW[SIZE];									// ��й�ȣ
	bool state;										// �α��� ����
	int Usermoney;									// Ŭ���̾�Ʈ�� ���� �ݾ�
	int temp_use_money;								// Ŭ���̾�Ʈ�� ������ �ݾ�

	AuctionItem* UserItem[ITEMMAX];			// Ŭ���̾�Ʈ�� ������ �ݾ׿� �����Ǿ� �����ϰԵ� ���ǰ
	int UserItemCount;								// ������ ���ǰ�� ����

	int bidnumber;									// Ŭ���̾�Ʈ�� ������ ���ǰ�� ��ȣ
	Bid_item* bid[ITEMMAX];							// Ŭ���̾�Ʈ�� ������ ���ǰ
	int UserBidCount;								// Ŭ���̾�Ʈ�� ������ ���ǰ�� ����
};

struct SOCKETINFO									// ���� ���� Ŭ����
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	STATE server_state;								// ������ STATE
	UserInfo* user;									// �� ���� �� �� ������ ����
	int successrecvbytes;							// ���� ��Ŷ�� �ް� �� �� �޾� �� �뷮
	bool recvflag;									// ����� �� �޾��� ��� ���� ��Ŷ�� �ޱ����� ������.
};

int nTotalSockets = 0;
SOCKETINFO* SocketInfoArray[FD_SETSIZE];
UserInfo* userInfo[USER];							// ���� ����
AuctionItem* ItemList[ITEMMAX];						// ���ǰ ����
int User_Count = 0;									// ���� ��
int Item_Count = 0;									// ���ǰ ��

// ���� ���� �Լ�
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

// ���� ��� �Լ�
void err_quit(char* msg);
void err_display(char* msg);
int packing(enum PROTOCOL protocol, const char* msg, char* buf);
int packing(enum PROTOCOL protocol, char* buf);
PROTOCOL GetProtocol(char* _buf);
char* msg_unpacking(const char* buf);
void unpacking(const char* _buf, char* ID, char* PW);
void unpacking(const char* _buf, char* ID, char* PW, int* money);
void unpacking(const char* _buf, int* money);
int NonBlock_recv(SOCKETINFO* ptr, int* size, int i);											// SELECT �� ���� recv �Լ�
int packing(enum PROTOCOL protocol, enum RESULT result, const char* msg, char* buf);
UserInfo* newUserRegist(const char* ID, const char* PW, const int money);						// ���� ���̵�, ��й�ȣ�� �޾� ���ο� ���� ���� ����ü�� ����
void AuctionSetting(int _i);																	// ���ǰ ����ü ����
Bid_item* User_Biditem_Set(AuctionItem* item, int money);										// ������ ������ ���ǰ�� ���� �ݾ��� ������ �޾ƿ� Bid_item�̶�� ����ü�� ���� �� UserInfo�� �ִ´�.
char* Bid_Check(int i, SOCKETINFO* ptr);														// ������ ���� �� ���ǰ�� �ִ����� �ο� ���� ������ �׿����� ������� ó���ϱ� ���� �Լ�
bool bid_order(AuctionItem* _ptritem, const char* _id, int max);								// ���� ������ Ȯ���ϴ� �Լ�

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
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

	// �ͺ��ŷ �������� ��ȯ
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	FD_SET rset, wset;

	bool endflag = false;


	for (int i = 0; i < AUCTIONITEMSIZE; i++)											// ���ǰ ����
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

		// ���� �� �ʱ�ȭ
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

		// ���� �� �˻�(1): Ŭ���̾�Ʈ ���� ����
		if (FD_ISSET(listen_sock, &rset)) {
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
			}
			else {
				printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				// ���� ���� �߰�
				AddSocketInfo(client_sock);
			}
		}

		char ID[SIZE];
		char PW[SIZE];

		//  ���� �۾�
		for (int i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO* ptr = SocketInfoArray[i];
			switch (ptr->server_state)
			{
				// Ŭ���̾�Ʈ�� ���� ���� �ʱ� �޴�����Ʈ�� Ŭ���̾�Ʈ���� ������ ����
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
					ptr->server_state = STATE::SELECTRESULT;			// Ŭ���̾�Ʈ���� �޴���ȣ�� �޾� �� �޴��� ���� ������ ���� state�� SELECTRESULT�� �̵�
					ptr->sendbytes = size;								// sendbytes�� ���� �Է��Ͽ� wset�� �۵����� �ʵ��� ��.
				}
				break;
				// Ŭ���̾�Ʈ�� ��ȣ�� �������� ���� ���� ����
			case STATE::SELECTRESULT:
				if (FD_ISSET(ptr->sock, &rset))
				{
					printf("rset ���...\n");
					// �޴� �۾�
					switch (NonBlock_recv(ptr, &size, i))				// define �س��� int���� �����ϴ� �Լ�
					{
					case CONNECTERROR:
					case DISCONNECTED:									// ���� ���� Ȥ�� ������ ����� endflag�� �Ѽ� while���� Ż���Ѵ�.
						endflag = true;
						break;
					case WAIT_RECEIVE:									// ��Ŷ�� �� ���� �����̸� continue�� ���� while���� �ʱ���·� ���ư���.
						continue;
						break;
					case SUCCESS_RECEIVE:								// ��Ŷ�� ������ ���� �����̸� �۾� ����
						switch (GetProtocol(ptr->buf))
						{
							// �޴����� ��ȣ�� ���� �� �װͿ� �´� �޼����� ������
						case PROTOCOL::INTROSELECT:
							unpacking(ptr->buf, &number);
							switch (number)
							{
							case 1:										// ȸ������
								size = packing(PROTOCOL::SIGN, SIGNMSG, ptr->buf);
								break;
							case 2:										// �α���
								size = packing(PROTOCOL::LOGIN, LOGINMSG, ptr->buf);
								break;
							case 3:										// ����� �������� ��Ŷ���� ������.
								size = packing(PROTOCOL::EXIT, ptr->buf);
								break;
							default:									// 1~3 �̿��� ���� ���� �޼����� ������ �ٽ� INTRO�� �̵�
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
							// ȸ������ ��������
						case PROTOCOL::SIGN:
							unpacking(ptr->buf, ID, PW, &money);							// ���̵�, ��й�ȣ, �ʱ��Աݾ��� �޾Ҵ�
							for (int i = 0; i < User_Count; i++)
							{
								if (!strcmp(userInfo[i]->ID, ID))							// ���̵� �ߺ��� ������ �ߺ� �޼����� ������.
								{
									sign = false;
									strcpy_s(msg, ID_OVERLAP_MSG);							// ���̵� �ߺ� �޼���
									break;
								}
							}
							if (sign)														// �ߺ��� ������ ȸ�������� ����.
							{
								strcpy_s(msg, SIGN_UP_MSG);									// ȸ������ �����޼���
								// �Լ��� ����Ͽ� ���� ���� �Է�
								userInfo[User_Count++] = newUserRegist(ID, PW, money);		// userInfo ���������� ���
							}
							size = packing(PROTOCOL::SIGN, msg, ptr->buf);
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->sendbytes = 0;												// wset�� ���� ���� �ٽ� 0���� �ʱ�ȭ
							ptr->server_state = STATE::INTRO;								// �ٽ� INTRO �̵�
							break;
							// �α��� ��������
						case PROTOCOL::LOGIN:
							unpacking(ptr->buf, ID, PW);									// Ŭ���̾�Ʈ���� ���̵� ��й�ȣ�� �޴´�.
							for (int i = 0; i < User_Count; i++)
							{
								if (!strcmp(ID, userInfo[i]->ID)) {							// ���� ���̵� ������ flag�� true
									flag = true;
									if (!strcmp(PW, userInfo[i]->PW)) {						// �н����嵵 ������ �ִٸ�...
										if (userInfo[i]->state == true) {					// �̹� �α��� ���̶�� �̹� �α��� ���̶�� �޼����� ��ŷ.
											result = RESULT::ALREADY_LOGIN;
											strcpy_s(msg, ALREADY_LOGIN_MSG);
											ptr->server_state = STATE::INTRO;
										}
										else {												// �α��� ���� �ƴ϶�� �α��� ������ �Բ� ���� �޼��� ��ŷ �� state ����
											result = RESULT::LOGINSUCESS;
											// �Լ��� ����Ͽ� ���� ���� �Է�							
											ptr->user = userInfo[i];
											ptr->user->state = true;
											strcpy_s(msg, LOGIN_MSG);
											ptr->server_state = STATE::LOGININTRO;
										}
									}
									else {													// �н����尡 �ٸ��� �н����� ���� �޽��� ��ŷ
										result = RESULT::PW_ERROR;
										strcpy_s(msg, PW_ERROR_MSG);
										ptr->server_state = STATE::INTRO;
									}
								}
							}
							if (!flag) {													// flag�� ������ ������ ���� ���̵� �����Ƿ� ���̵� ���� �޼��� ��ŷ
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
							ptr->sendbytes = 0;												// �α����� ���� �ϳ� �����ϳ� �޴� ����� �ؾ��ϱ� ������ wset�� ���Ƿ� sendbytes�� 0���� �Ѵ�.
							break;
						case PROTOCOL::EXIT:												// Ŭ���̾�Ʈ�� ���� ��Ŷ�� ������ �� Ŭ���̾�Ʈ�� ������ �����Ѵ�.
							RemoveSocketInfo(i);
							break;
						}
						break;
					}
				}							// if��
				break;
				//	�α��� ���� ���� �޴� ��� ����
			case STATE::LOGININTRO:
				strcpy(msg, "");
				for (int i = 0; i < Item_Count; i++)										// ���ǰ ������ŭ �ݺ�
				{
					if (ItemList[i]->client_count == BIDMAX)								// ���ǰ�� Ŭ���̾�Ʈ ���� �ο� ���� �����ٸ� �Լ� ����
					{
						strcat(msg, Bid_Check(i, ptr));										// �Լ��� �޼����� �����Ѵ�.
					}
				}
				if (FD_ISSET(ptr->sock, &wset)) {
					if (Item_Count == 0)													// ���ǰ�� ��� �����Ǹ� ��� ���� �޼����� �α��� �޴��� ���� ��ŷ
					{
						strcat(msg, AUCTION_END_MSG);
						strcat(msg, LOGININTROMSG);
						size = packing(PROTOCOL::INTROSELECT, msg, ptr->buf);
					}
					else																	// �� ���� ��쿡�� �α��� �޴��� ��ŷ
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
					ptr->sendbytes = size;													// wset ����
					ptr->server_state = STATE::LOGINSELECTRESULT;
				}
				break;
				// �α��� �޴����� Ŭ���̾�Ʈ�� �� �޴��� ���� ó���ϴ� ����
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
							// �α��� �޴� ��ȣ ����
						case PROTOCOL::INTROSELECT:
							unpacking(ptr->buf, &number);
							switch (number)
							{
								// �Ա��� �Ա� �ȳ� �޼����� ������.
							case 1:
								size = packing(PROTOCOL::DEPOSIT, DEPOSITMSG, ptr->buf);
								break;
								// ��� ������ ���ǰ ������ ������.
							case 2:
								strcpy(msg, JOIN_MSG);
								if (Item_Count > 0) {										// ���ǰ�� �� �ȸ��� ���� ���
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
								else														// ���ǰ�� �� �ȷ��� ���
								{
									strcpy(msg, AUCTION_END_MSG2);
								}
								size = packing(PROTOCOL::JOIN, msg, ptr->buf);
								break;
								// �¶��������� ����� ��� �ݾ��� �����ִ� �޼����� ��ŷ
							case 3:
								sprintf(msg, "���� ������ ��� �ݾ� : %d(%d)���Դϴ�. *��ȣ ���� ���� �� �ݾ��� �� ����*\n", ptr->user->Usermoney,
									ptr->user->Usermoney - ptr->user->temp_use_money);
								size = packing(PROTOCOL::BANK, msg, ptr->buf);
								break;
								// ��� ������ ���±��� Ŭ���̾�Ʈ�� ������ ������ �����ش�.
							case 4:
								strcpy(msg, VERIFY_MSG);
								for (int i = 0; i < ptr->user->UserBidCount; i++)								// ������ �� ��ŭ 
								{
									if (ptr->user->bid[i]->item->sold_out && ptr->user->bid[i]->bidflag == true)// �������� �ȷȴٸ� 
									{
										if (ptr->user->bid[i]->win == true) {									// �ڽ��� ������ �����ߴٸ�..
											if (ptr->user->bid[i]->price > 1000000) {
												sprintf(temp, "  %d   %s\t\t %d\t\t���� ����\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
											else {
												sprintf(temp, "  %d   %s\t\t %d\t\t\t���� ����\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
										}
										else {																	// ������ ���..
											if (ptr->user->bid[i]->price > 1000000) {
												sprintf(temp, "  %d   %s\t\t %d\t\t���� ����\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
											else {
												sprintf(temp, "  %d   %s\t\t %d\t\t\t���� ����\n", i + 1, ptr->user->bid[i]->item->Itemname,
													ptr->user->bid[i]->price);
											}
										}
									}
									else																		// �������� �ȸ��� �ʾҴٸ�
									{
										if (ptr->user->bid[i]->price > 1000000) {
											sprintf(temp, "  %d   %s\t\t %d\t\t ��� ��\n", i + 1, ptr->user->bid[i]->item->Itemname,
												ptr->user->bid[i]->price);
										}
										else {
											sprintf(temp, "  %d   %s\t\t %d\t\t\t ��� ��\n", i + 1, ptr->user->bid[i]->item->Itemname,
												ptr->user->bid[i]->price);
										}
									}
									strcat(msg, temp);
								}
								size = packing(PROTOCOL::VERIFY, msg, ptr->buf);
								break;
								// ������ ������ ������ ���ǰ ������ �����ش�.
							case 5:
								strcpy(msg, LIST_MSG);
								for (int i = 0; i < ptr->user->UserItemCount; i++)								// Ŭ���̾�Ʈ�� ���� ������ ����ŭ
								{
									sprintf(temp, "  %d   %s\t\t %d\n", i + 1, ptr->user->UserItem[i]->Itemname, ptr->user->UserItem[i]->price);
									strcat(msg, temp);
								}
								size = packing(PROTOCOL::LIST, msg, ptr->buf);
								break;
								// ��� ������ �����ִ� ��Ŷ�� �����ش�.
							case 6:
								size = packing(PROTOCOL::VERIFYDELETE, VERIFY_DELETE_MSG, ptr->buf);
								break;
								// �α׾ƿ� ��Ŷ�� ������.
							case 7:
								size = packing(PROTOCOL::LOGOUT, LOGOUT_MSG, ptr->buf);
								break;
								// �׿��� ��ȣ�� ���� �޼����� ������ �ٽ� LOGININTRO�� �̵�
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
							// �ȳ� �޼����� ������ Ŭ���̾�Ʈ�� �ݾ��� �����ٸ�..
						case PROTOCOL::DEPOSIT:
							unpacking(ptr->buf, &money);
							if (money < 0)							// 0���� ���� ���� �� ���
							{
								size = packing(PROTOCOL::DEPOSIT, DEPOSIT_ERROR_MSG, ptr->buf);
							}
							else									// ����� ���� �Ա� ó��
							{
								ptr->user->Usermoney += money;
								sprintf(msg, "�Է��Ͻ� %d���� �ԱݵǾ����ϴ�!\n", money);
								size = packing(PROTOCOL::DEPOSIT, msg, ptr->buf);
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->server_state = STATE::LOGININTRO;	// �Ա� �� �ٽ� �޴���..
							ptr->sendbytes = 0;
							break;
							// ��� ���� ��Ŷ�� ������ ���ǰ�� ���ڸ� �޾Ҵٸ�..
						case PROTOCOL::JOIN:
							unpacking(ptr->buf, &number);
							if (number > Item_Count)				// ������ ������ ��ȣ�� Ŭ ���� ���� �޼���
							{
								size = packing(PROTOCOL::JOIN, RESULT::JOIN_ERROR, JOIN_ERROR_MSG, ptr->buf);
								ptr->server_state = STATE::LOGININTRO;
								ptr->sendbytes = 0;
							}
							else if (number == 0)					// 0���� ���ư��� �޴�
							{
								size = packing(PROTOCOL::JOIN, RESULT::JOIN_ERROR, MENU_RETURN_MSG, ptr->buf);
								ptr->server_state = STATE::LOGININTRO;
								ptr->sendbytes = 0;
							}
							else									// �׿��� ���� �������� �ޱ����� �ȳ� �޼����� ������ ��Ÿ� ���� state�� �Ѿ��. (JOIN_SUCCESS_STATE)
							{
								ptr->user->bidnumber = number;		// ������ ������ ������ ��ȣ��  bidnumber�� ����
								sprintf(msg, "%d�� ��ǰ�� ��� �������� �����ּ��� >>", number);
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
							// �¶�������, ��ų���, ���ų����� ��� �ȳ� �޼����� ������ �����Ƿ� �ٽ� LOGININTRO�� ���ư� �޴��� �����ֵ��� �Ѵ�.
						case PROTOCOL::BANK:
						case PROTOCOL::VERIFY:
						case PROTOCOL::LIST:
							ptr->server_state = STATE::LOGININTRO;
							ptr->sendbytes = 0;
							break;
							// �Ϸ�� ��� ������ ����� ��������
						case PROTOCOL::VERIFYDELETE:
							for (int i = 0; i < ptr->user->UserBidCount; i++)								// ������ �� ��ŭ
							{
								if (ptr->user->bid[i]->item->sold_out)										// ������ ���ǰ�� �ȷȴٸ�..
								{
									if (i != ptr->user->UserBidCount - 1)									// ���� ���̰ų� 1�� ���� ��찡 �ƴϸ� ������ �ڸ��� �ǳ� �ڸ��� �������� �ű��. 
									{
										ptr->user->bid[i] = ptr->user->bid[ptr->user->UserBidCount - 1];
										i--;																// i�� ���ҽ�Ű�� �ٲ� �������� �˻�
									}
									else																	// ���� ���̰ų� 1�� ���� ��� �� �ڸ��� �״�� ����
									{
										ptr->user->bid[i] = NULL;
									}
									--ptr->user->UserBidCount;												// ī��Ʈ ����
								}
							}
							ptr->server_state = STATE::LOGININTRO;											// �ٽ� �α��� �޴�
							ptr->sendbytes = 0;
							break;
							// �α׾ƿ� ��������
						case PROTOCOL::LOGOUT:
							ptr->user->state = false;														// �α��� ���� ����
							ptr->user = NULL;																// ���Ͽ� �������� �����
							ptr->server_state = STATE::INTRO;												// �ʱ� �޴��� STATE �̵�
							ptr->sendbytes = 0;
							break;
						}
						break;
					}
				}
				break;
				// ��� �������� ���ǰ�� ���� �������� Ŭ���̾�Ʈ���� �޾ƿ� �� ó���ؾ��� ����
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
							if (money > (ptr->user->Usermoney - ptr->user->temp_use_money) ||													// �޾ƿ� �ݾ��� ������ ���� ���� �ݾ� - ���ǰ�� ������ �ݾ� ���� ũ�� �Ұ���.
								ItemList[ptr->user->bidnumber - 1]->min_price > money)
							{
								size = packing(PROTOCOL::JOIN, MONEY_ERROR_MSG, ptr->buf);
							}
							else																												// �����ϸ� ������ ������ ������ ��ȣ�� bidnumber�� �޾� bid_price�� �������� ����
							{
								ItemList[ptr->user->bidnumber - 1]->
									bid_price[ItemList[ptr->user->bidnumber - 1]->client_count] = money;
								ptr->user->temp_use_money += money;																				// ������ ����� �� ������ ������ ���� �߰��Ѵ�.
								strcpy(ItemList[ptr->user->bidnumber - 1]->
									clientsID[ItemList[ptr->user->bidnumber - 1]->client_count++], ptr->user->ID);								// ������ ���� �г����� ����
								ptr->user->bid[ptr->user->UserBidCount++] = User_Biditem_Set(ItemList[ptr->user->bidnumber - 1], money);		// ������ ���ǰ �迭�� ����ǰ�� �ִ´�.
								ptr->user->bidnumber = 0;																						// ���� ��ȣ �ʱ�ȭ
								size = packing(PROTOCOL::JOIN, AUCTION_SUCCESS_MSG, ptr->buf);
							}
							retval = send(ptr->sock, ptr->buf, size, 0);
							if (retval == SOCKET_ERROR) {
								err_display("send()");
								RemoveSocketInfo(i);
								continue;
							}
							ptr->server_state = STATE::LOGININTRO;																				// �ٽ� �α��� �޴�
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

	}// while�� ��

	// ���� ����
	WSACleanup();
	return 0;
}

// ���� ���� �߰�
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return FALSE;
	}

	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
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
// ���� ���� ����
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO* ptr = SocketInfoArray[nIndex];

	// Ŭ���̾�Ʈ ���� ���
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

	--nTotalSockets;
}
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
int packing(enum PROTOCOL protocol, const char* msg, char* buf)
{
	int strsize;
	int size = 0;
	char* ptr;
	strsize = strlen(msg);

	ptr = buf + sizeof(int);			// �ѻ������� �ڸ��� ���ܵд�.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// �������� ��ŷ
	size = size + sizeof(enum PROTOCOL);				// ������ ����
	ptr = ptr + sizeof(enum PROTOCOL);					// ��ġ �̵�

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, msg, strsize);
	size = size + strsize;

	ptr = buf;											// ��� �۾��� ������ ó�� �ڸ��� ���ƿ´�.		
	memcpy(ptr, &size, sizeof(int));					// �� ����� �Է��Ѵ�.

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
void unpacking(const char* _buf, char* ID, char* PW) {				// Ŭ���̾�Ʈ���� ���� ���̵� �н����� ��Ŷ�� ����ŷ�ϴ� �۾��Լ�
	int idsize;
	int pwsize;
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&idsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(ID, ptr, idsize);				// ���θ��� ����ü�� ���̵𺯼��� ������ ���̵� �Է�
	ptr = ptr + idsize;

	ID[idsize] = '\0';

	memcpy(&pwsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(PW, ptr, pwsize);				// ���θ��� ����ü�� ��й�ȣ������ ������ ��й�ȣ�� �Է�

	PW[pwsize] = '\0';
}
void unpacking(const char* _buf, char* ID, char* PW, int* money) {				// Ŭ���̾�Ʈ���� ���� ���̵� �н����� ��Ŷ�� ����ŷ�ϴ� �۾��Լ�
	int idsize;
	int pwsize;
	const char* ptr = _buf;
	ptr = ptr + sizeof(enum PROTOCOL);

	memcpy(&idsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(ID, ptr, idsize);				// ���θ��� ����ü�� ���̵𺯼��� ������ ���̵� �Է�
	ptr = ptr + idsize;

	ID[idsize] = '\0';

	memcpy(&pwsize, ptr, sizeof(int));
	ptr = ptr + sizeof(int);

	memcpy(PW, ptr, pwsize);				// ���θ��� ����ü�� ��й�ȣ������ ������ ��й�ȣ�� �Է�
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
int packing(enum PROTOCOL protocol, enum RESULT result, const char* msg, char* buf)	// Ŭ���̾�Ʈ�� ���� �������� �� �޼����� ��ŷ�۾��ϴ� �Լ�
{
	int strsize;
	int size = 0;
	char* ptr;
	strsize = strlen(msg);

	ptr = buf + sizeof(int);			// �ѻ������� �ڸ��� ���ܵд�.

	memcpy(ptr, &protocol, sizeof(enum PROTOCOL));		// �������� ��ŷ
	size = size + sizeof(enum PROTOCOL);				// ������ ����
	ptr = ptr + sizeof(enum PROTOCOL);					// ��ġ �̵�


	memcpy(ptr, &result, sizeof(enum RESULT));			// ��� ��ŷ
	size = size + sizeof(enum RESULT);					// ������ ����
	ptr = ptr + sizeof(enum RESULT);					// ��ġ �̵�

	memcpy(ptr, &strsize, sizeof(int));
	size = size + sizeof(int);
	ptr = ptr + sizeof(int);

	memcpy(ptr, msg, strsize);
	size = size + strsize;

	ptr = buf;											// ��� �۾��� ������ ó�� �ڸ��� ���ƿ´�.		
	memcpy(ptr, &size, sizeof(int));					// �� ����� �Է��Ѵ�.

	return size + sizeof(int);										// �� ����� ����
}
UserInfo* newUserRegist(const char* ID, const char* PW, int money) 
{
	UserInfo* ptr = new UserInfo;

	if (ptr == NULL) {
		printf("���� ��� ����!");
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
	// ���ǰ�� �ִ� ���� �ο����� á�ٸ� ���� ū �ݾ��� �߷��� �� �ݾ��� ������ ���� ã�� ���� ��Ű�� ����� ���� Ŭ���̾�Ʈ�鿡�� �޼����� �ٸ��� ������.
	char msg[BUFSIZE + 1];
	int max = 0;
	bool itemdelete = true;
	AuctionItem* ptritem;
	bool flag = false;
	bool tiemax = false;

	strcpy(msg, "");
	ptritem = ItemList[i];																						// ������ for���� ���� �ִ� ���� �ο����� á�ٸ� ��ȣ i�� �޾� �����ͷ� �����. ��, ptritem�� Ŭ���̾�Ʈ�� ���� ������ �ؾ� �� ���ǰ
	for (int j = 0; j < ptritem->client_count; j++)																// ������ Ŭ���̾�Ʈ ����ŭ ����
	{
		if (max < ptritem->bid_price[j])																		// ���� ���� �ݾ��� �߷�����.
			max = ptritem->bid_price[j];
	}

	for (int i = 0; i < ptritem->client_count; i++)																// ���� ���� �ݾ��� ������ �ִ� ���  tiemax�� true�� ���� �� �ٸ� ������ ����
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

	for (int k = 0; k < ptr->user->UserBidCount; k++)															// ������ ������ �� ��ŭ
	{
		if (ptr->user->bid[k]->item == ptritem &&																// ������ ������ ������ �� ptritem�� ��ġ�ϰ�.. Ŭ���̾�Ʈ�� bid_check�� ���� �ʾ��� ���
			ptr->user->bid[k]->bidflag == false)
		{
			ptritem->sold_out = true;																			// �ȷȴٴ� bool ������ Ų��.
			ptritem->price = max;																				// ���ǰ�� �������� ���� ���� �ݾ����� �Ѵ�.
			if (!tiemax) {
				if (ptr->user->bid[k]->price == max)									// Ŭ���̾�Ʈ�� �������� ���� ���� �ݾ��̰� �ȸ��� �ʾ��� ���..
				{
					ptr->user->bid[k]->win = true;																	// �ڽ��� ������ ���������Ƿ� true
					ptr->user->Usermoney -= ptr->user->bid[k]->price;												// �ݾ׸�ŭ ���� ���Ѵ�.
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// �� ���ǰ�� �� �������� ���ش�.
					//ptr->user->UserItem[ptr->user->UserItemCount++] = ptritem;
					memcpy(&ptr->user->UserItem[ptr->user->UserItemCount++], &ptritem, sizeof(AuctionItem));		// Ŭ���̾�Ʈ�� ������ ������ �迭�� ptritem�� �ִ´�. (���ų��� ǥ��)
					sprintf(msg, "[����] �����Ͻ� %s ��ǰ�� ��ſ��� �����Ǿ����ϴ�!\n", ptritem->Itemname);			// ���� �޼��� ��ŷ
				}
				else
				{
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// �� ���ǰ ������ ���� ���� ����
					sprintf(msg, "[����] �����Ͻ� %s ��ǰ�� ������ �����Ͽ����ϴ�..\n", ptritem->Itemname);				// ���� �޼��� ��ŷ
				}
			}
			else																									// tiemax�� ���������� ���������� ���� ���� ���� Ŭ���̾�Ʈ���� �����Ѵ�.
			{
				if (ptr->user->bid[k]->price == max)																// Ŭ���̾�Ʈ�� �������� ���� ���� �ݾ��̰� �ȸ��� �ʾ��� ���..
				{
					if (bid_order(ptritem, ptr->user->ID, max))															// TRUE�� ��� �ڽź��� ���� �����ڰ� ���ٴ� ��. FALSE ���� �ڽź��� ���� �����ڰ� �ִٴ� ��.
					{
						ptr->user->bid[k]->win = true;																// �ڽ��� ������ ���������Ƿ� true
						ptr->user->Usermoney -= ptr->user->bid[k]->price;											// �ݾ׸�ŭ ���� ���Ѵ�.
						ptr->user->temp_use_money -= ptr->user->bid[k]->price;										// �� ���ǰ�� �� �������� ���ش�.
						//ptr->user->UserItem[ptr->user->UserItemCount++] = ptritem;
						memcpy(&ptr->user->UserItem[ptr->user->UserItemCount++], &ptritem, sizeof(AuctionItem));	// Ŭ���̾�Ʈ�� ������ ������ �迭�� ptritem�� �ִ´�. (���ų��� ǥ��)
						sprintf(msg, "[����] �����Ͻ� %s ��ǰ�� ��ſ��� �����Ǿ����ϴ�!\n", ptritem->Itemname);
					}
					else 
					{
						ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// �� ���ǰ ������ ���� ���� ����
						sprintf(msg, "[����] �����Ͻ� %s ��ǰ�� ������ �����Ͽ����ϴ�..\n", ptritem->Itemname);				// ���� �޼��� ��ŷ
					}
				}
				else
				{
					ptr->user->temp_use_money -= ptr->user->bid[k]->price;											// �� ���ǰ ������ ���� ���� ����
					sprintf(msg, "[����] �����Ͻ� %s ��ǰ�� ������ �����Ͽ����ϴ�..\n", ptritem->Itemname);				// ���� �޼��� ��ŷ
				}
			}
			ptr->user->bid[k]->bidflag = true;																	// �Լ��� ��� �������Ƿ� �÷��� üŷ

			// ������ ������ ��� �� �Լ��� ���ľ��Ѵ�.
			for (int j = 0; j < nTotalSockets; j++)																// �� �������� ������ �������� �� �Լ��� ��� ���ƾ� ������ ���Ű� �����ϴ�.
			{
				SOCKETINFO* temp = SocketInfoArray[j];
				for (int a = 0; a < temp->user->UserBidCount; a++)
				{
					if (temp->user->bid[a]->item == ptritem &&													// �Ѹ��̶� �� �Լ��� ���� ���� ��� �״�� �����ؼ� �Լ� ����
						temp->user->bid[a]->bidflag == false)
					{
						return msg;
					}
					else
					{
						itemdelete = false;																		// ��� ������ ��� itemdelete ���� ����.
					}
				}
			}
		}
	}

	if (!itemdelete) {																							// ��� ������ ��� �ȸ� ���ǰ�� ����Ʈ���� ����
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

	// ���� ������ ������ ������ �˾Ƴ���.
	for (int i = 0; i < _ptritem->client_count; i++) {
		if (!strcmp(_ptritem->clientsID[i], _id)) 
		{
			number = i;
			break;
		}
	}

	// ���� ������ ������ �ձ��� ���� �������� ������ ������ ã�� �� ������ �����ݰ� �ִ�ݾ��� ������ �� ����� �̰ܾ��ϹǷ� FALSE
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

	// ������ TRUE
	return TRUE;
}