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

int main(int argc, char **argv) {
	Logger_CreateLog("./FM9.log", "FM9", true);
	Logger_Log(LOG_INFO, "Proceso FM9 iniciado...");
	configurar();
	createStorage();
//	createPagingStructures();
//	createIPTStructures();
	createSegmentationStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1,12);
//	char * buffer = malloc(12);
	char * datos2 ="ABCDEFGHIJKLMNO";

	Logger_Log(LOG_DEBUG, "FM9 -> Por usar write frame");
	int resultado;
	int dir;
//
//	resultado = writeFrame(datos, 6);
//	resultado = readFrame(buffer, 6);

	dir = writeData_SEG(datos,10,1);
	resultado = readDataSegmentation(buffer,dir,1);
	resultado = readDataSegmentation(buffer,dir+1,1);

	dir = writeData_SEG(datos2,15,1);
	resultado = readDataSegmentation(buffer,dir,1);
	resultado = readDataSegmentation(buffer,dir+1,1);

	if (resultado == INVALID_LINE_NUMBER)
		Logger_Log(LOG_DEBUG, "FM9 -> Espacio Insuficiente.");
	else
		Logger_Log(LOG_DEBUG, "FM9 -> Guardado Correctamente");


	if(resultado == INVALID_FRAME_NUMBER)
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
	free(buffer);
	freeSegmentationStructures();
//	freePagingStructures();
//	freeIPTStructures();
	freeStorage();
	free(settings);
	exit_gracefully(0);
}

