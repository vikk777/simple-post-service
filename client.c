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

	memset(&Hints, 0, sizeof Hints); //Очищаем структуру
	//Ставим флаги
	Hints.ai_family = AF_INET;
	Hints.ai_socktype = SOCK_STREAM;
	if ((Status = getaddrinfo("127.0.0.1" , "3490", &Hints, &Res)) != 0)
	{
		printf("getaddrinfo error:%s\n", gai_strerror(Status));
		return 2;
	}

	//Получаем дескриптор сокета
	if ((SockFd = socket(Res->ai_family, Res->ai_socktype, Res->ai_protocol)) == -1)
	{
		printf("socket error\n");
		return 3;
	}

	//Подключаемся к 127.0.0.1
	if (((connect(SockFd, Res->ai_addr, Res->ai_addrlen))) == -1)
	{
		printf("connect error\n");
		return 4;
	}

	//Войти
	while (1)
	{
		printf("Нажмите цифру для выбора действия\n\n");
		printf("1. Войти\n");
		printf("2. Зарегистрироваться\n\n");
		char Choice = getch();

		if ((Choice!='1')&&(Choice!='2'))
		{
			printf("Неизвестная команда\n");
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
	//Показать меню
		ShowMenu();
		Choice = getch();
		if ((Choice!='1')&&(Choice!='2')&&(Choice!='3')&&(Choice!='4'))
		{
			printf("Неизвестная команда\n");
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

//Мои функции
int ToEnterUS(SOCKET Sock, char* User)
{
	char Str[15]= {'\0'}, Name[15]= {'\0'};

	system("cls");
	send(Sock, ENTER, 2, 0);//s1
	recv(Sock, Str, sizeof Str, 0);//s5

	if (Str[0] == '0')
	{
		printf("Нет зарегистрированных пользователей\n\n");
		return 0;
	}

	printf("Имя: ");
	scanf("%s", Name);
	send(Sock, Name, strlen(Name), 0);//s6
	printf("Пароль: ");
	scanf("%s", Str);
	send(Sock, Str, strlen(Str), 0);//s7
	recv(Sock, Str, sizeof Str, 0);//s8

	if (Str[0] == '0')
	{
		printf("Неверное имя или пароль\n\n");
		return 0;
	}

	if (Str[0] == '1')
	{
		printf("Добро пожаловать, %s!\n\n", Name);
		strcpy(User, Name);
		return 1;
	}
}

int ToRegisterUS(SOCKET Sock, char* User)
{
	char Str[15] = {'\0'}, Name[15] = {'\0'};

	system("cls");
	printf("Введите Ваше имя без пробелов. Иначе оно будет урезано до разделителя\n");
	printf("Используйте латиницу. Длина имени - не более пятнадцати символов\nИмя: ");

	scanf("%s", Name);
	send(Sock, REG_NEW_USER, 2, 0);//s1
	send(Sock, Name, strlen(Name), 0);//s2
	recv(Sock, Str, sizeof(Str), 0);//s3

	if (Str[0] == '0')
	{
		printf("Пользователь с таким именем уже существует\n\n");
		return 0;
	}

	strcpy(User, Name);//Присваиваем пользователю имя
	printf("Введите пароль: ");
	scanf("%s", &Str);
	send(Sock, Str, strlen(Str), 0);//s4
	printf("Вы успешно зарегистрировались\n\n");
	return 1;
}

void ShowMenu()
{
	printf("1. Проверить почту\n");
	printf("2. Написать письмо\n");
	printf("3. Очистить экран\n");
	printf("4. Завершить программу\n\n");
}

void ToShowMailUS(SOCKET Sock, char* User)
{
	char Str[50] = {'\0'};
	int Eos;//Отделять строки

	send(Sock, SHOW_MAIL, 4, 0);//g1
	send(Sock, User, strlen(User), 0);//g1-1
	recv(Sock, Str, sizeof(Str), 0);//g2

	if (!(strcmp(Str, "0")))
	{
		printf("Такого пользователя не существует\n");
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
			printf("В вашем ящике нет писем");
			break;
		}
		printf("%s", Str);
		send(Sock, "1", 1, 0);//g4 подтверждение
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
	printf("Введите имя получателя: ");
	scanf("%s", Str);
	send(Sock, Str, strlen(Str), 0);//h2
	recv(Sock, Str, sizeof(Str), 0);//h3
	if (Str[0] == '0')
	{
		printf("Такого пользователя не существует\n\n");
		return;
	}

	printf("Введите сообщение: ");
	Ch = getchar();
	Ch = getchar();
	while (Ch != '\n')
	{
		Str[i] = Ch;
		Ch = getchar();
		i++;
		if (i >= 150)
		{
			printf("Размер сообщения не может быть больше 150 символов.\n\n");
			send(Sock, "-end", 4, 0);//h4
			return;
		}
	}
	Str[i] = '\0';
	send(Sock, Str, strlen(Str), 0);//h4
	printf("Сообщение отправлено\n\n");
	return;
}
