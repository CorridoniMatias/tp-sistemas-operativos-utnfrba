#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#define SERVER_IP "127.0.0.1" //IP por defecto de la maquina
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include "SocketCommons.h"
#include <string.h>

/**
 * 		ACCION: Establece una conexion como cliente a un cierto puerto de la IP
 * 		PARAMETROS:
 * 			port: Puerto de la IP a la cual quiero conectarme, en forma de cadena
 * 		RETORNO: El numero de fd del socket a traves del cual se conecto, o bien -1 en caso de error
 */
int SocketClient_ConnectToServer(char* port);

#endif /* SOCKETCLIENT_H_ */
