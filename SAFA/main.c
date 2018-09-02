#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./SAFA.log", "SAFA", true);
	Logger_Log(LOG_INFO, "Proceso SAFA iniciado...");
	exit_gracefully(0);
}

