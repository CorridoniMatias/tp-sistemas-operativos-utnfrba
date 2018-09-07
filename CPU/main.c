#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/SocketClient.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	int socket = SocketClient_ConnectToServer("192.168.1.1","8000");
	SocketClient_ConnectToServer("192.168.1.2","8001");

	if(socket < 0){
		printf("no me pude conectar /0");
	}
	exit_gracefully(0);

}

