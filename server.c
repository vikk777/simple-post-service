#include <stdio.h>

#include <winsock2.h>
#include <Ws2tcpip.h>
#include "windows.h"

#include "uinfo.h"
#include "Keys.h"

//Предполагается, что пользователь не введет сообщение, длиной более 150 символов
//И файл (ящик) не будет редактироваться вручную

void CreateList();
int  ToRegisterSS();
int  ToEnterSS();
void ToShowMailSS();
void ToSendMailSS();

int main()
{
	struct    addrinfo Hints, *Res;
	int       Status;
	int       Dblen;
	SOCKET 	  SockFd, NewFd;
	struct    sockaddr_storage TheirAddr;
	socklen_t AddrSize;
	char      Key[5]= {'\0'};
	Uinfo*    Array;

	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	WSADATA wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}
	
	CreateList(&Array, &Dblen);

	memset(&Hints, 0, sizeof Hints); //Очистка структуры

//Ставим флаги
	Hints.ai_family = AF_INET;
	Hints.ai_socktype = SOCK_STREAM;
	Hints.ai_flags = AI_PASSIVE;
	
	if ((Status = getaddrinfo(NULL, "3490", &Hints, &Res)) != 0)
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

//Соединяем сокет с портом своего хоста
	if ((bind(SockFd, Res->ai_addr, Res->ai_addrlen)) == -1)
	{
		printf("bind error\n");
		return 4;
	}

//Слушаем
	if ((listen(SockFd, 1)) == -1)
	{
		printf("listen error\n");
		return 5;
	}

//Принимаем
	AddrSize = sizeof TheirAddr;
	if ((NewFd = accept(SockFd, (struct sockaddr*)&TheirAddr, &AddrSize)) == -1)
	{
		printf("accept error\n");
		return 6;
	}
	printf("Выполнено подключнение\n");

	while(1)
	{
		if (recv(NewFd, Key, sizeof Key, 0) == -1)
		{
			break;   //s1
		}
		if (!(strcmp(Key, REG_NEW_USER)))
		{
			if (ToRegisterSS(NewFd, &Array, &Dblen))
			{
				break;
			}
		}
		if (!(strcmp(Key, ENTER)))
		{
			if (ToEnterSS(NewFd, Array, Dblen))
			{
				break;
			}
		}
	}

	while(1)
	{
		Status = recv(NewFd, Key, sizeof Key, 0);//g1
		if (Status == -1)
		{
			break;
		}
		Key[Status] = '\0';//Отрезаем мусор

		if (!(strcmp(Key, SHOW_MAIL)))
		{
			ToShowMailSS(NewFd);
		}

		if (!(strcmp(Key, SEND_MAIL)))
		{
			ToSendMailSS(NewFd);
		}

		if (!(strcmp(Key, CLOSE_PROG)))
		{
			break;
		}
	}

	system("pause");

	freeaddrinfo(Res);
	free(Array);
	closesocket(NewFd);
	closesocket(SockFd);
	WSACleanup();	
	return 0;
}

//Мои функции
void CreateList(Uinfo** Array, int* Len)//проходит по файлу и записывает в массив структур
{
	FILE* Db;
	
	if (!(Db = fopen("database.txt", "r")))
	{
		printf("Ошибка открытия файла БД\n\n");
		return;
	}

	fscanf(Db, "%d", Len);
	if (*Len==0) return;
	if (((*Array = malloc(sizeof(Uinfo) * (*Len)))==NULL))
	{
		printf("Ошибка выделения памяти\n");
		exit(1);
	}

	for (int i=0; i<(*Len); i++)
	{
		fscanf(Db, "%d", &((*Array)[i].Id));
		fscanf(Db, "%s", (*Array)[i].Name);
		fscanf(Db, "%s", (*Array)[i].Pass);
	}
	fclose(Db);
	return;
}

int ToRegisterSS(SOCKET Sock, Uinfo** Array, int* Dblen)
{
	FILE* Db;
	if (!(Db = fopen("database.txt", "r+")))
	{
		printf("Ошибка открытия файла БД\n\n");
		return 0;
	}
	char Str[15]= {'\0'};
	int Eos;

	//Проверяем, нет ли таких юзеров
	recv(Sock, Str, sizeof Str, 0);//s2
	if (*Dblen!=0)
	{
		for (int i=0; i<*Dblen; i++)
		{
			if (!(strcmp(Str, (*Array)[i].Name)))
			{
				send(Sock, "0", 1, 0);//s3 Посылаем сообщение, что такой юзер существует
				return 0;
			}
		}
	}
	send(Sock, "1", 1, 0);//s3 Посылаем сообщение, что таких юзеров не существует

	(*Dblen)++;//Изменяем общее кол-во
	fprintf(Db, "%d\n", *Dblen);
	if (*Dblen == 1)
	{
		if ((*Array = malloc(sizeof(Uinfo))) == NULL)
		{
			printf("Ошибка выделения памяти\n");
			return 0;
		}
	}
//Расширяем массив структур
	else
	{
		if ((*Array = realloc(*Array, (sizeof(Uinfo)*(*Dblen)))) == NULL)
		{
			printf("Ошибка выделения памяти\n");
			return 0;
		}
	}

	(*Array)[*Dblen-1].Id = *Dblen;//Добавляем Id новому эл-ту
	fseek(Db, 0, SEEK_END);
	fprintf(Db, "%d %s", *Dblen, Str);
	strcpy((*Array)[*Dblen-1].Name, Str);

	strcat(Str, ".txt");
	FILE* Mail = fopen(Str, "w+");
	fclose(Mail);

	Eos = recv(Sock, Str, sizeof Str, 0);//s4
	Str[Eos] = '\0';
	fprintf(Db, " %s\n", Str);
	strcpy(((*Array)[*Dblen-1]).Pass, Str);
	fclose(Db);
	return 1;
}

int ToEnterSS(SOCKET Sock, Uinfo* Array, int Len)
{
	char Name[15]= {'\0'};
	char Pass[15]= {'\0'};

	if (Len == 0)
	{
		send(Sock, "0", 1, 0);    //s5
		return 0;
	}
	send(Sock, "1", 1, 0);//s5

	recv(Sock, Name, sizeof Name, 0);//s6
	recv(Sock, Pass, sizeof Pass, 0);//s7
//Проверяем имя и пароль
	for (int i=0; i<Len; i++)
	{
		if (!(strcmp(Name, Array[i].Name)))
			if (!(strcmp(Pass, Array[i].Pass)))
			{
				send(Sock, "1", 1, 0);    //s8
				return 1;
			}
	}
	send(Sock, "0", 1, 0);//s8
	return 0;
}

void ToShowMailSS(SOCKET Sock)
{
	FILE* Mail;
	char  Str[20], Name[15];
	char  Flag=0;
	char  Ch;
	int   Tmp;
	int   Status = recv(Sock, Name, sizeof Name, 0);//g1-1

	Name[Status] = '\0';//Чтобы в имя не попало мусора
	strcpy(Str, Name);
	strcat(Str, ".txt");

	if (!(Mail = fopen(Str, "r")))
	{
		send(Sock, "0", 1, 0);    //g2
		return;
	}
	else
	{
		send(Sock, "1", 1, 0);   //g2
	}

	while (1)
	{
		Tmp = fgetc(Mail);
		if ((Tmp == EOF) && (Flag==1))
		{
			send(Sock, "-end", 4, 0);    //g3
			break;
		}
		if ((Tmp == EOF) && (Flag==0))
		{
			send(Sock, "-empty", 6, 0);    //g3
			break;
		}
		Ch = Tmp;
		Flag = 1;
		send(Sock, &Ch, sizeof(char), 0);//g3
		recv(Sock, &Ch, sizeof(char), 0);//g4 подтверждение
	}
	fclose(Mail);
	return;
}

void ToSendMailSS(SOCKET Sock)
{
	FILE* Mail;
	char  NameFrom[15], NameTo[15];
	char  Message[150];

	recv(Sock, NameFrom, sizeof NameFrom, 0);//h1
	recv(Sock, NameTo, sizeof NameTo, 0);//h2
	strcat(NameTo, ".txt");
//Проверяем, есть ли такой юзер
	if (!(Mail = fopen(NameTo, "r")))
	{
		send(Sock, "0", 1, 0);//h3
		return;
	}
	else
	{
		send(Sock, "1", 1, 0);//h3
	}
	fclose(Mail);
	Mail = fopen(NameTo, "a");

	recv(Sock, Message, sizeof Message, 0);//h4
	if (!strcmp(Message, "-end"))
	{
		return;
	}
	fprintf(Mail, "Сообщение от %s:\n", NameFrom);
	fputs(Message, Mail);
	fprintf(Mail, "\n***************************\n\n");

	fclose(Mail);
	return;
}
