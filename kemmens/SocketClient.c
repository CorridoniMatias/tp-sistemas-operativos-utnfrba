#include "kemmens/megekemmen.h"
#include "kemmens/SocketClient.h"

int SocketClient_ConnectToServerDefault(char* port)
{

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;    // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(SERVER_IP, port, &hints, &server_info);  // habria que poner una ip dinamica?

	// 2. Creemos el socket con el nombre "server_socket" usando la "server_info" que creamos anteriormente
	int server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(server_socket == -1)
	{
		return -1;
	}

	// 3. Conectemosnos al server a traves del socket! Para eso vamos a usar connect()
	int retorno = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return ((retorno < 0) ? -1 : server_socket);

}

int SocketClient_ConnectToServerIP(char* ip, char* puerto)
{

	struct addrinfo auxiliar;		//Temporal, para guardar transitoriamente datos de la direccion a la cual me quiero conectar
	struct addrinfo* infoServidor;	//La posta, ahi hare el getaddrinfo

	memset(&auxiliar, 0, sizeof(auxiliar));
	auxiliar.ai_family = AF_UNSPEC;    		//El sistema verifica que familia de direcciones usaremos (IPv4 o IPv6)
	auxiliar.ai_socktype = SOCK_STREAM;  	//Protocolo a usar es TCP/IP

	getaddrinfo(ip, puerto, &auxiliar, &infoServidor);	//Guardo info de la direccion aca

	int socketServidor;						//Descriptor del socket con la direccion del server
	//Preparamos el socket con el tipo de conexion deseado
	socketServidor = socket(infoServidor->ai_family, infoServidor->ai_socktype, infoServidor->ai_protocol);

	if(socketServidor == -1)				//Si al querer armarlo devolvio <0, algo salio mal
		return -1;

	// Ahora si, queremos conectarnos al servidor a traves de ese socket!
	int retorno = connect(socketServidor, infoServidor->ai_addr, infoServidor->ai_addrlen);

	freeaddrinfo(infoServidor);				//Es esto o memory leak

	if(retorno < 0)
	{
		return -1;
	}
	else
	{
		return socketServidor;				//Todo marcha bien Milhouse!
	}

}
