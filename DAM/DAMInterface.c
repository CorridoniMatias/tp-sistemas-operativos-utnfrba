#include "headers/DAMInterface.h"


//para el comando abrir
void DAM_Abrir(void* arriveData)
{
	free(arriveData);
}



void DAM_Abrir(void* arriveData)
{
	free(arriveData);
}

void DAM_Abrir(void* arriveData)
{
	free(arriveData);
}

void DAM_Abrir(void* arriveData)
{
	free(arriveData);
}

void DAM_Flush(void* arriveData)
{

	char* filePath = (char*)arriveData;

	/*
	 * TODO
	 *
	 * Verificará que el archivo solicitado esté abierto por el G.DT. Para esto se utilizarán la
		estructura administrativa del DTB. En caso que no se encuentre, se abortará el G.DT.
	 */

	char* tmpData;
	for(int i = 0; i < config->transferSize;i++)
	{
		//tmpData = Pedirle datos al FM9, usar las funciones del socketcommons.

		SocketCommons_SendData(modulos->socketMDJ, MESSAGETYPE_MDJ_PUTDATA, (void*)tmpData, config->transferSize);
	}

	Se enviará una solicitud a El Diego indicando que se requiere hacer un Flush del archivo,
	enviando los parámetros necesarios para que pueda obtenerlo desde FM9 y guardarlo en MDJ.

	Se comunicará al proceso S-AFA que el G.DT se encuentra a la espera de una respuesta por parte
	de El Diego y S-AFA lo bloqueará.




	free(arriveData);
}

