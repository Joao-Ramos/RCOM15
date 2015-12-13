#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <netdb.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define SERVER_PORT "21"
#define BUF_SIZE 1024
#define Window_adjust 45

typedef struct
{
    char
        user[64],
        pass[64],
        host[128],
        path[512];
} ArgStruct;

void PrintBar (char symbol)
{
	int
        i;
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (i = 0; i < w.ws_col; i++)
    	printf ("%c", symbol);
}

void PrintCenter (char *string, int adjust)
{
	int
        i;

	struct winsize
	w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	for (i = 0; i < (w.ws_col / 2) - (strlen(string) / 2) + adjust ; i++)
    	printf (" ");
	printf ("%s", string);
}

/* --- HandlerArg ---

    Param:
        char *argv:         Recebe o argumento recebido pelo programa.

    Return:
        int confirmation:   Recebe 0 caso o argumento está bem estruturado ou -1 caso o argumento esteja errado.

--- HandlerArg --- */

int HandlerArg (char *argv, ArgStruct *ArgPacket)
{
    printf ("\n");
    PrintCenter("A analisar: ", 0 - strlen(argv)/2);
    printf ("%s\n\n", argv);

    if (sscanf(argv, "ftp://%[^:]:%[^@]@%[^/]/%s\n", (*ArgPacket).user, (*ArgPacket).pass, (*ArgPacket).host, (*ArgPacket).path) == 4)
    {
        PrintCenter ("Mensagem analisada com sucesso.", 0);
        printf ("\n\n");
        printf ("User: %s\n", (*ArgPacket).user);
        printf ("Pass: %s\n", (*ArgPacket).pass);
        printf ("Host: %s\n", (*ArgPacket).host);
        printf ("Path: %s\n\n", (*ArgPacket).path);
        PrintBar('*');
        printf ("\n");

        return 0;
    }
	if (sscanf(argv, "ftp://%[^/]/%s\n", (*ArgPacket).host, (*ArgPacket).path) == 2)
	{
		PrintCenter ("Mensagem analisada com sucesso.", 0);
		printf ("\n\n");
		strcpy ((*ArgPacket).user, "anonymous");
		printf ("User: %s\n", (*ArgPacket).user);
		strcpy ((*ArgPacket).pass, "123");
		printf ("Pass: %s\n", (*ArgPacket).pass);
		printf ("Host: %s\n", (*ArgPacket).host);
		printf ("Path: %s\n\n", (*ArgPacket).path);
		PrintBar('*');
		printf ("\n");

		return 0;
	}
	else
    {
        PrintCenter ("Mensagem analisada com erros.", 0);
        printf ("\n\n");
        PrintBar('*');
        printf ("\n");
        return (-1);
    }
}

/* --- GetAnswer ---

    Param:
        int socket_desc:    Socket de onde está a receber.
        char *buf:          Variável onde ficar a resposta recebida.

--- GetAnswer --- */

void GetAnswer(int socket_desc, char *buf)
{
	int
        code;

    sleep (1);
	memset(buf, 0, BUF_SIZE);
	recv(socket_desc, buf, BUF_SIZE, 0);
	printf ("\n%s\n", buf);

	if (sscanf(buf, "%d %*s", &code) == 1)
	{
		if (code >= 500)
		{
			printf("Error! Command failed!\n");
			exit (1);
		}
		else
            return;
	}

	PrintBar('*');
    printf ("\n");
}

/* --- SendAnswer ---

    Param:
        char *cmd:          O tipo de mensagem que está a enviar.
        char *arg:          O parâmetro que está a enviar.
        int socket_desc:    Socket de onde está a receber.

--- SendAnswer --- */

void SendAnswer(char *cmd, char *arg, int socket_desc)
{
	char command[517];

    printf ("\n");
	if (strcmp(cmd, "PASV") == 0)
	{
		strcpy(command, cmd);
		strcat(command, "\n");
		send(socket_desc, command, strlen(command), 0);
		PrintCenter(command, 0);
	}
	else
	{
		strcpy(command, cmd);
		strcat(command, arg);
		strcat(command, "\n");
		send(socket_desc, command, strlen(command), 0);
		if (strcmp(cmd, "PASS ") == 0)
        {
            PrintCenter("PASS ***", 0);
            printf ("\n");
        }
        else
            PrintCenter(command, 0);
	}

	memset(command, 0, sizeof(command));
	return;
}

void SaveFile(int socket_desc, char *filename)
{
	char
		buf[BUF_SIZE];
	int
		bytes_written = 1,
		bytes_read = 0;
	FILE
		*file_desc;

	file_desc = fopen(filename, "w");
	if (file_desc == NULL)
	{
		PrintCenter("Erro: Impossível criar ficheiro!", 0);
		printf ("\n");
		exit(1);
	}
	while (bytes_written != 0)
	{
		bytes_read = recv(socket_desc, buf, BUF_SIZE, 0);
		bytes_written = fwrite(buf, sizeof(char), bytes_read, file_desc);
	}
	if (fclose(file_desc) == EOF)
	{
		PrintCenter("Erro: Impossível guardar ficheiro!", 0);
		printf ("\n");
		exit(1);
	}

	PrintCenter("Sucesso: Ficheiro criado e guardado!", 0);
	printf ("\n\n");
	return;
}

/* --- Main ---

    Param:
        char *argv:         Recebe o argumento recebido pelo programa.

    Return:
        int confirmation:   Recebe 0 caso o argumento está bem estruturado ou -1 caso o argumento esteja errado.

--- Main --- */

void main(int argc, char *argv[])
{
    /* --- Variáveis --- */

    ArgStruct
        ArgPacket;

    struct addrinfo
        host_info,
        *serv_info,
        *aux_info,
        *serv;

    char
        buf[BUF_SIZE],
        ip_str[INET6_ADDRSTRLEN],
        newIP[256],
        newPort[10],
        *ip_v,
        filename[256];

    void
        *addr;

    int
        i,
        j,
        socket_desc,
        socket_desc_end,
        ip[4],
        port[2];

    /* --- Código --- */

	/* -- Cabeçalho -- */

    system ("clear");
    PrintBar('+');
    printf ("\n");
    PrintCenter("Lab 2 - Computer Networks.", 0);
    printf ("\n");
    PrintCenter("Ivo Silva • João Almeida • Anaís Dias", 0);
    printf ("\n");
    PrintCenter("Turma 6", 0);
    printf ("\n");
    PrintBar('*');
    printf ("\n");

    /* -- Verificação dos argumentos -- */

    if (argc != 2) // Caso em que não lê nenhum argumento
    {
        PrintCenter("Número de argumentos errado.", 0);
        printf ("\n");
        PrintCenter("A fechar...", 0);
        printf ("\n\n\n\n\n");
		exit (1);
	}

	/* -- Coloca no ArgPacket os dados lidos -- */

    if (HandlerArg(argv[1], &ArgPacket) == -1)
    {
        exit(1);
    }

    /* -- Informação do host -- */

    memset (&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_INET;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_protocol = 0;

    if (getaddrinfo(ArgPacket.host, SERVER_PORT, &host_info, &serv_info) != 0)
    {
		PrintCenter("Erro: Impossível obter informação do host!", 0);
		printf ("\n");
        exit(1);
    }

    printf ("\n");
    PrintCenter("Endereços para ", 0 - strlen(ArgPacket.host)/2);
    printf ("%s!\n", ArgPacket.host);
	for (aux_info = serv_info; aux_info != NULL; aux_info = aux_info->ai_next)
	{
		if (aux_info->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *) aux_info->ai_addr;
			addr = &(ipv4->sin_addr);
			ip_v = "IPv4";
		}
		else
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) aux_info->ai_addr;
			addr = &(ipv6->sin6_addr);
			ip_v = "IPv6";
		}
		inet_ntop(aux_info->ai_family, addr, ip_str, sizeof(ip_str));
//		strcpy (ip_str, inet_ntoa(*((struct in_addr *)aux_info->ai_addr)));

		printf ("%s: ", ip_v);
		printf ("%s\n\n", ip_str);

		PrintBar('*');
		printf ("\n");
	}

	/* -- Criar socket -- */

	socket_desc = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol);
	if (socket_desc == -1)
	{
		PrintCenter("Erro: Impossível criar socket!", 0);
		printf ("\n");
		exit(1);
	}

	/* -- Ligar host -- */

	if (connect(socket_desc, serv_info->ai_addr, serv_info->ai_addrlen) == -1)
	{
		PrintCenter("Erro: Impossível ligar ao host!", 0);
		printf ("\n");
		exit(1);
	}

    GetAnswer(socket_desc, buf); // Resposta do servidor

    PrintBar('*');
    printf ("\n");

    SendAnswer("USER ", ArgPacket.user, socket_desc); // Envia USER
	GetAnswer(socket_desc, buf);

    PrintBar('*');
    printf ("\n");

	SendAnswer("PASS ", ArgPacket.pass, socket_desc); // Envia Pass
	GetAnswer(socket_desc, buf);

    PrintBar('*');
    printf ("\n");

	SendAnswer("PASV", NULL, socket_desc);
	GetAnswer(socket_desc, buf);

    PrintBar('*');
    printf ("\n");

	if (sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d).", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]) != 6)
	{
		PrintCenter("Erro: Impossível ler resposta!", 0);
		printf ("\n");
        exit(1);
    }

    memset(newIP, 0, strlen(newIP));
	sprintf(newIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	printf("\nNew IP: %s\n", newIP);
	port[0] = 256 * port[0] + port[1];
	memset(newPort, 0, strlen(newPort));
	sprintf(newPort, "%d", port[0]);
	printf("New PORT: %s\n\n", newPort);

	if (getaddrinfo(newIP, newPort, &host_info, &serv) != 0)
	{
		PrintCenter("Erro: Impossível obter informação do host!", 0);
		printf ("\n");
		exit(1);
	}

	socket_desc_end = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol);
	if (socket_desc == -1)
	{
		PrintCenter("Erro: Impossível criar socket!", 0);
		printf ("\n");
		exit(1);
	}

	if (connect(socket_desc_end, serv->ai_addr, serv->ai_addrlen) == -1)
	{
		PrintCenter("Erro: Impossível ligar ao host!", 0);
		printf ("\n");
		exit(1);
	}

    PrintBar('*');
    printf ("\n");

	SendAnswer("RETR ", ArgPacket.path, socket_desc);
	GetAnswer(socket_desc, buf);

	for (i = strlen(ArgPacket.path) - 1; i > 0; i--)
		if (ArgPacket.path[i] == '/')
		{
			i++;
			break;
		}

	for (j = 0; i < strlen(ArgPacket.path); i++, j++)
		filename[j] = ArgPacket.path[i];

	filename[j] = '\0';

	SaveFile(socket_desc_end, filename);
    PrintBar('*');
    printf ("\n");

	/* Encerrar ligações */

	close(socket_desc);
	close(socket_desc_end);
	freeaddrinfo(serv);
	freeaddrinfo(serv_info);
    PrintBar('+');
}
