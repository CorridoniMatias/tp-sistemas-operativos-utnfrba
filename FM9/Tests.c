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

void testSEG(){
	createSegmentationStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1,12);
	char * datos2 ="ABCDEFGHIJKLMNO";

	int resultado;
	int dir;
	dir = writeData_SEG(datos,10,1);
	resultado = readDataSegmentation(buffer,dir,1);
	resultado = readDataSegmentation(buffer,dir+1,1);

	dir = writeData_SEG(datos2,15,1);
	resultado = readDataSegmentation(buffer,dir,1);
	resultado = readDataSegmentation(buffer,dir+1,1);
	free(buffer);
	freeSegmentationStructures();
}


void testPAG(){
		createPagingStructures();
		createIPTStructures();

		char * datos = "0123456789";
		char * buffer = calloc(1,12);
		char * datos2 ="ABCDEFGHIJKLMNO";

		Logger_Log(LOG_DEBUG, "FM9 -> Por usar write frame");
		int resultado;
		int dir;

		resultado = writeFrame(datos, 6);
		resultado = readFrame(buffer, 6);



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
		free(buffer);
		freePagingStructures();
		freeIPTStructures();
}

