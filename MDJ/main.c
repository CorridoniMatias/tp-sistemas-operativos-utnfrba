#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	Logger_Log(LOG_INFO, "Proceso DMJ iniciado...");
	//initGlobals();
	//startServer();
	//freeGlobals();
	exit_gracefully(0);
}

