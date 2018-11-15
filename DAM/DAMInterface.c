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
	DeserializedData* dest = malloc(sizeof(DeserializedData));
	Serialization_Deserialize(arriveData, dest);

	char* filePath = (char*)dest->parts[0];
	uint32_t direccionLogica = *((int*)dest->parts[1]);

	/*
	 * TODO: PEPE!
	 *(esto se hace en CPU)
	 * Verificará que el archivo solicitado esté abierto por el G.DT. Para esto se utilizarán la
		estructura administrativa del DTB. En caso que no se encuentre, se abortará el G.DT.

		(una vez verificado eso se manda la peticion al DAM)
	 */



	//Primero que nada tenemos que abrir una conexion con el FM9 y una conexion con el MDJ
	//(esto seria la opcion 3 de la arquitectura interna: hilos con paralelismo)

	//Esta opcion es la mas conveniente porque por cada hilo hay una conexion distinta con el MDJ y el FM9 por lo que
	//podemos hacer todos los send() y recv() que queramos sin la necesidad de que el SocketServer intervenga porque cada hilo es cliente propio.
	//de lo contrario si hubiera una sola conexion con estos servicios habria que sincronizar todos los hilos para que solo 1 de ellos pueda hacer send() y recv() a la vez
	//sino podria pasar que dos hilos hacen send() y recv() y reciban datos cruzados!

	int socketMDJ = SocketClient_ConnectToServer(config->IPMDJ, config->puertoMDJ);
	int socketFM9 = SocketClient_ConnectToServer(config->IPFM9, config->puertoFM9);

	char* tmpData;
	char* archivo;

	//Lo que deberiamos hacer es pedirle al FM9 informacion hasta que nos llegue vacio
	while(1)
	{
		//aca habria que armar un paquete serializado para el FM9 que contenga la direccion logica que queremos y tambien nuestro transfer size
		//le mandamos el paquete al FM9 y esperamos que nos envia los bytes solicitados

		//si recibimos vacio hacer break!


	}


	//una vez que recibimos todo por parte del FM9 se lo mandamos al mdj:

	int offset = 0;
	int len = strlen(archivo);
	int sizeToSend = 0;
	int msg_type, length, status;

	while(1)
	{
		if(len <= 0)
			break;

		if(len < config->transferSize)
			sizeToSend = len;
		else
			sizeToSend = config->transferSize;

		len -= sizeToSend;

		SocketCommons_SendData(socketMDJ, MESSAGETYPE_MDJ_PUTDATA, (void*)(archivo + offset), sizeToSend);
		offset += config->transferSize;
		void* data = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);

		if(msg_type == MESSAGETYPE_INT)
		{
			switch(((uint32_t)data))
			{
				case 0: //OPERATION_SUCCESSFUL
				break;
				case 10: //INSUFFICIENT_SPACE
				break;
				case 11: //METADATA_OPEN_ERROR
				break;
				case 12: //FILE_NOT_EXISTS
				break;
			}
		}
		free(data);
	}

	free(archivo);

	/*
	Se enviará una solicitud a El Diego indicando que se requiere hacer un Flush del archivo,
	enviando los parámetros necesarios para que pueda obtenerlo desde FM9 y guardarlo en MDJ.

	Se comunicará al proceso S-AFA que el G.DT se encuentra a la espera de una respuesta por parte
	de El Diego y S-AFA lo bloqueará.
	*/

	Serialization_CleanupDeserializationStruct(dest);

	free(arriveData);
}

