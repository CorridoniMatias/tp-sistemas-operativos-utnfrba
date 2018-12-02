#include <unistd.h>
#include "headers/FM9_Server.h"
#include "headers/FM9lib.h"
#include "headers/Storage.h"
#include "headers/Paging.h"
#include "headers/InvertedPageTable.h"
#include "headers/Segmentation.h"
#include "headers/PagedSegmentation.h"
#include <stdlib.h>
#include "headers/Tests.h"

int main(int argc, char **argv) {
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	createStorage();
	memoryFunctions->createStructures();
//	testLineLength();

	StartServer();
	ThreadPool_FreeGracefully(threadPool);
	memoryFunctions->freeStructures();
	freeStorage();
	free(settings);
	exit_gracefully(0);
}
