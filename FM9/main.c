#include "headers/CPUsManager.h"
#include "headers/ConsoleHandler.h"
#include "headers/bibliotecaFM9.h"
#include "headers/FM9_Server.h"
#include "headers/Storage.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	createStorage();
	char * datos="0123456789ABCDE";
	int resultado = writeStorage(datos,15,10);
	if(resultado == INVALID_WRITE)
		Logger_Log(LOG_DEBUG, "FM9 -> Espacio Insuficiente.");
	else
		Logger_Log(LOG_DEBUG, "FM9 -> Guardado Correctamente");
	char * buffer=malloc(15);
	resultado = readStorage(buffer,15,10);
	if(resultado == INVALID_READ)
		Logger_Log(LOG_DEBUG, "FM9 -> No existe el espacio de memoria a leer deseado.");
	else
	{
		Logger_Log(LOG_DEBUG, "FM9 -> Leido Correctamente");
		Logger_Log(LOG_DEBUG, "FM9 -> Lectura = %s",buffer);
	}
//	StartServer();
//	ThreadPool_FreeGracefully(threadPool);

	exit_gracefully(0);
}

