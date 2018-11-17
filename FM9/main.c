#include <unistd.h>
#include "headers/CPUsManager.h"
#include "headers/ConsoleHandler.h"
#include "headers/FM9_Server.h"
#include "headers/FM9lib.h"
#include "headers/Storage.h"
#include "headers/Paging.h"
#include "headers/InvertedPageTable.h"
#include "headers/Segmentation.h"
#include <stdlib.h>
#include "Tests.c"

int main(int argc, char **argv) {
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	createStorage();




//	StartServer();
//	ThreadPool_FreeGracefully(threadPool);
	freeStorage();
	free(settings);
	exit_gracefully(0);
}
