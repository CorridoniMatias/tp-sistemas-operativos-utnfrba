#include "headers/Tests.h"
void testSEG() {
	createSegmentationStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCDEFGHIJKLMNO";

	int resultado;
	int dir;
	dir = writeData_SEG(datos, 10, 1);
	resultado = readDataSegmentation(buffer, dir, 1);
	resultado = readDataSegmentation(buffer, dir + 1, 1);

	dir = writeData_SEG(datos2, 15, 1);
	resultado = readDataSegmentation(buffer, dir, 1);
	resultado = readDataSegmentation(buffer, dir + 1, 1);
	free(buffer);
	freeSegmentationStructures();
}

void testPAG()
{
	createPagingStructures();
	createIPTStructures();

	char * datos = "0123456789";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCDEFGHIJKLMNO";

	Logger_Log(LOG_DEBUG, "FM9 -> Por usar write frame");
	int resultado;
	int dir;

	resultado = writeFrame(datos, 6);
	resultado = readFrame(buffer, 6);

	if (resultado == INVALID_LINE_NUMBER)
		Logger_Log(LOG_DEBUG, "FM9 -> Espacio Insuficiente.");
	else
		Logger_Log(LOG_DEBUG, "FM9 -> Guardado Correctamente");

	if (resultado == INVALID_FRAME_NUMBER)
		Logger_Log(LOG_DEBUG,
				"FM9 -> No existe el espacio de memoria a leer deseado.");
	else {
		Logger_Log(LOG_DEBUG, "FM9 -> Leido Correctamente");

		*(buffer + 4) = '\0';
		Logger_Log(LOG_DEBUG, "FM9 -> Lectura = %s", buffer);
	}

	int frame = getFreeFrame();
	if (frame != NO_FRAMES_AVAILABLE) {
		Logger_Log(LOG_DEBUG, "FM9 -> Frame %d libre.", frame);
		addFreeFrame(frame);
	} else
		Logger_Log(LOG_DEBUG, "FM9 -> No hay frames libres");
	free(buffer);
	freePagingStructures();
	freeIPTStructures();
}

void testAsignar()
{

	createSegmentationStructures();

	char * datos = "01\n";
	char * buffer = calloc(1, 12);
	char * datos2 = "ABCD";

	int resultado;
	int dir;
	dir = writeData_SEG(datos, 10, 1);

	FM9_AsignLine(dir, 1, datos2);
	readLine(buffer, dir);
	free(buffer);
	freeSegmentationStructures();
}
