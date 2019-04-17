#include <stdio.h>
#include <conio.h>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include "windows.h"

#include "../server/keys.h"

int  ToEnterUS();
int  ToRegisterUS();
void ShowMenu();
void ToShowMailUS();
void ToSendMailUS();

int main()
{	
	struct addrinfo Hints, *Res;
	int    Status, Choice;
	char   Name[15]= {'\0'};
	SOCKET SockFd;
	
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	
	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}

	memset(&Hints, 0, sizeof Hints); //������� ���������
	//������ �����
	Hints.ai_family = AF_INET;
	Hints.ai_socktype = SOCK_STREAM;
	if ((Status = getaddrinfo("127.0.0.1" , "3490", &Hints, &Res)) != 0)
	{
		printf("getaddrinfo error:%s\n", gai_strerror(Status));
		return 2;
	}

	//�������� ���������� ������
	if ((SockFd = socket(Res->ai_family, Res->ai_socktype, Res->ai_protocol)) == -1)
	{
		printf("socket error\n");
		return 3;
	}

	//������������ � 127.0.0.1
	if (((connect(SockFd, Res->ai_addr, Res->ai_addrlen))) == -1)
	{
		printf("connect error\n");
		return 4;
	}

	//�����
	while (1)
	{
		printf("������� ����� ��� ������ ��������\n\n");
		printf("1. �����\n");
		printf("2. ������������������\n\n");
		char Choice = getch();

		if ((Choice!='1')&&(Choice!='2'))
		{
			printf("����������� �������\n");
			continue;
		}
		if (Choice == '1')
		{
			if (ToEnterUS(SockFd, Name)) break;
		}
		if (Choice == '2')
		{
			if (ToRegisterUS(SockFd, Name)) break;
		}
	}

	while (1)
	{
	//�������� ����
		ShowMenu();
		Choice = getch();
		if ((Choice!='1')&&(Choice!='2')&&(Choice!='3')&&(Choice!='4'))
		{
			printf("����������� �������\n");
			continue;
		}
		if (Choice == '1')
		{
			ToShowMailUS(SockFd, Name);	
		}
		if (Choice == '2')
		{
			ToSendMailUS(SockFd, Name);
		}  
		if (Choice == '3')
		{
			system("cls");
		}
		if (Choice == '4')
		{
			return 0;
		} 
	}

	system("pause");
	freeaddrinfo(Res);
	closesocket(SockFd);

	WSACleanup();
	return 0;
}

//��� �������
int ToEnterUS(SOCKET Sock, char* User)
{
	char Str[15]= {'\0'}, Name[15]= {'\0'};

	system("cls");
	send(Sock, ENTER, 2, 0);//s1
	recv(Sock, Str, sizeof Str, 0);//s5

	if (Str[0] == '0')
	{
		printf("��� ������������������ �������������\n\n");
		return 0;
	}

	printf("���: ");
	scanf("%s", Name);
	send(Sock, Name, strlen(Name), 0);//s6
	printf("������: ");
	scanf("%s", Str);
	send(Sock, Str, strlen(Str), 0);//s7
	recv(Sock, Str, sizeof Str, 0);//s8

	if (Str[0] == '0')
	{
		printf("�������� ��� ��� ������\n\n");
		return 0;
	}

	if (Str[0] == '1')
	{
		printf("����� ����������, %s!\n\n", Name);
		strcpy(User, Name);
		return 1;
	}
}

int ToRegisterUS(SOCKET Sock, char* User)
{
	char Str[15] = {'\0'}, Name[15] = {'\0'};

	system("cls");
	printf("������� ���� ��� ��� ��������. ����� ��� ����� ������� �� �����������\n");
	printf("����������� ��������. ����� ����� - �� ����� ���������� ��������\n���: ");

	scanf("%s", Name);
	send(Sock, REG_NEW_USER, 2, 0);//s1
	send(Sock, Name, strlen(Name), 0);//s2
	recv(Sock, Str, sizeof(Str), 0);//s3

	if (Str[0] == '0')
	{
		printf("������������ � ����� ������ ��� ����������\n\n");
		return 0;
	}

	strcpy(User, Name);//����������� ������������ ���
	printf("������� ������: ");
	scanf("%s", &Str);
	send(Sock, Str, strlen(Str), 0);//s4
	printf("�� ������� ������������������\n\n");
	return 1;
}

void ShowMenu()
{
	printf("1. ��������� �����\n");
	printf("2. �������� ������\n");
	printf("3. �������� �����\n");
	printf("4. ��������� ���������\n\n");
}

void ToShowMailUS(SOCKET Sock, char* User)
{
	char Str[50] = {'\0'};
	int Eos;//�������� ������

	send(Sock, SHOW_MAIL, 4, 0);//g1
	send(Sock, User, strlen(User), 0);//g1-1
	recv(Sock, Str, sizeof(Str), 0);//g2

	if (!(strcmp(Str, "0")))
	{
		printf("������ ������������ �� ����������\n");
		return;
	}

	while(1)
	{
		Eos = recv(Sock, Str, sizeof(Str), 0);//g3
		Str[Eos] = '\0';
		if (!(strcmp(Str, "-end")))
		{
			break;
		}
		if (!(strcmp(Str, "-empty")))
		{
			printf("� ����� ����� ��� �����");
			break;
		}
		printf("%s", Str);
		send(Sock, "1", 1, 0);//g4 �������������
	}
	printf("\n\n");
	return;
}

void ToSendMailUS(SOCKET Sock, char* User)
{
	char Str[150] = {'\0'};
	char Ch;
	int i = 0;

	send(Sock, SEND_MAIL, 3, 0);//g1
	send(Sock, User, strlen(User), 0);//h1
	printf("������� ��� ����������: ");
	scanf("%s", Str);
	send(Sock, Str, strlen(Str), 0);//h2
	recv(Sock, Str, sizeof(Str), 0);//h3
	if (Str[0] == '0')
	{
		printf("������ ������������ �� ����������\n\n");
		return;
	}

	printf("������� ���������: ");
	Ch = getchar();
	Ch = getchar();
	while (Ch != '\n')
	{
		Str[i] = Ch;
		Ch = getchar();
		i++;
		if (i >= 150)
		{
			printf("������ ��������� �� ����� ���� ������ 150 ��������.\n\n");
			send(Sock, "-end", 4, 0);//h4
			return;
		}
	}
	Str[i] = '\0';
	send(Sock, Str, strlen(Str), 0);//h4
	printf("��������� ����������\n\n");
	return;
}
