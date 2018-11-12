#include <unistd.h>
#include "headers/CPUsManager.h"
#include "headers/ConsoleHandler.h"
#include "headers/bibliotecaFM9.h"
#include "headers/FM9_Server.h"
#include "headers/Storage.h"
#include "headers/Paging.h"
#include "headers/InvertedPageTable.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	createStorage();
	createPagingStructures();
	createStructuresIPT();


	char * datos="0123";
	int resultado = writeLine(datos,15);
	if(resultado == INVALID_WRITE)
		Logger_Log(LOG_DEBUG, "FM9 -> Espacio Insuficiente.");
	else
		Logger_Log(LOG_DEBUG, "FM9 -> Guardado Correctamente");
	char * buffer=malloc(15);
	resultado = readLine(buffer,15);
	if(resultado == INVALID_READ)
		Logger_Log(LOG_DEBUG, "FM9 -> No existe el espacio de memoria a leer deseado.");
	else
	{
		Logger_Log(LOG_DEBUG, "FM9 -> Leido Correctamente");

		*(buffer+4)='\0';
		Logger_Log(LOG_DEBUG, "FM9 -> Lectura = %s",buffer);
	}

	int frame = getFreeFrame();
	if(frame != NO_FRAMES_AVAILABLE)
	{
		Logger_Log(LOG_DEBUG, "FM9 -> Frame %d libre.", frame);
		addFreeFrame(frame);
	}
	else
		Logger_Log(LOG_DEBUG, "FM9 -> No hay frames libres");

//	StartServer();
//	ThreadPool_FreeGracefully(threadPool);

	freePagingStructures();
	freeStructuresIPT();
	freeStorage();
	exit_gracefully(0);
}

