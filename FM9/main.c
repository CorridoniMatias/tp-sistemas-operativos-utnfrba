#include "headers/CPUsManager.h"
#include "headers/ConsoleHandler.h"
#include "headers/bibliotecaFM9.h"
#include "headers/FM9_Server.h"
#include "headers/Storage.h"

int main(int argc, char **argv)
{
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	Logger_Log(LOG_INFO, "Creando Storage");
	sleep(5);
	createStorage();
	Logger_Log(LOG_INFO, "Storage creado");
	sleep(5);

	sleep(10);
//	StartServer();
//	ThreadPool_FreeGracefully(threadPool);

	exit_gracefully(0);
}

