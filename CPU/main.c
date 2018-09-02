#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	exit_gracefully(0);
}

