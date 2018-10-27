#include "incs/CPUsManager.h"
#include "incs/ConsoleHandler.h"
#include "Headers/bibliotecaFM9.h"
#include "Headers/FM9_Server.h"

int main(int argc, char **argv)
{
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	StartServer();
	ThreadPool_FreeGracefully(threadPool);
	exit_gracefully(0);
}

