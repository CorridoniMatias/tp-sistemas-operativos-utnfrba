#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/SocketServer.h"

int main(int argc, char **argv)
{
	Logger_CreateLog("./DAM.log", "DAM", true);
	Logger_Log(LOG_INFO, "Proceso DAM iniciado...");
	exit_gracefully(0);
}

