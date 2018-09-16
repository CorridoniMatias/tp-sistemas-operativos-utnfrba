#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "MDJlib.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	Logger_Log(LOG_INFO, "Proceso DMJ iniciado...");
	initGlobals();
	startServer();
	ThreadPool_FreeGracefully(threadPool);
	exit_gracefully_custom((void*)freeGlobals, 0);
}

