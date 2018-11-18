#include "headers/DAMInterface.h"
#include "kemmens/Utils.h"


void DAM_Crear(void* arriveData)
{
	DeserializedData* data = Serialization_Deserialize(arriveData);

	if(data->count < 2) {
		Logger_Log(LOG_ERROR, "DAM::DAM_Crear -> Recibido menos de 2 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		free(arriveData);
		return;
	}

	//TODO: basicmanet estamos haciendo un middle man aca, ver de hacer una funcion para obtener el largo de un paquete serializado y forwardear el arrivedata.

	char* filePath = (char*)data->parts[0];
	uint32_t cantNewLines = *((uint32_t*)data->parts[1]);

	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);

	declare_and_init(newlines, uint32_t, cantNewLines);
	SerializedPart p_newlines = {.size = sizeof(uint32_t), .data = newlines};
	SerializedPart p_filepath = {.size = strlen(filePath)+1, .data = filePath};
	SerializedPart* packet = Serialization_Serialize(2, p_filepath, p_newlines);

	SocketCommons_SendData(socketMDJ, MESSAGETYPE_MDJ_CREATEFILE, packet->data, packet->size);

	int msg_type, length, status;
	void* response = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);

	if(msg_type == MESSAGETYPE_INT)
	{
		switch(*((uint32_t*)response))
		{
			case 0: //OK
			break;
			case 1: //EXISTING_FILE
			break;
			case 2: //METADATA_CREATE_ERROR
			break;
			case 10: //INSUFFICIENT_SPACE
			break;
			case 11: //METADATA_OPEN_ERROR
			break;
		}
	}
	free(response);

	Serialization_CleanupSerializedPacket(packet);
	Serialization_CleanupDeserializationStruct(data);
	free(newlines);
	free(arriveData);
}

void DAM_Abrir(void* arriveData)
{
	DeserializedData* data = Serialization_Deserialize(arriveData);

	//verificamos que nos esten enviando los dos campos necesarios
	if(data->count < 2) {
		//no tenemos los datos necesarios para seguir adelante
		Logger_Log(LOG_ERROR, "DAM::DAM_Abrir -> Recibido menos de 2 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		free(arriveData);
		return;
	}

	uint32_t idDTB = *((uint32_t*)data->parts[0]);
	char* filePath = (char*)data->parts[1];

	//abrimos conexion con el MDJ
	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);

	//verificamos que el archivo exista
	SocketCommons_SendStringAsContent(socketMDJ, filePath, MESSAGETYPE_MDJ_CHECKFILE);
	int msg_type, length, status;
	void* response = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);
	if(msg_type == MESSAGETYPE_INT)
	{
		switch(*((uint32_t*)response))
		{
			case 0: //EL ARCHIVO NO EXISTE
				//printf("El archivo solicitado con ubicacion %s no existe!\n", filePath);
				Logger_Log(LOG_ERROR, "DAM::DAM_Abrir -> El archivo solicitado con ubicacion %s no existe!\n", filePath);
				DAM_ErrorOperacion(idDTB);
				break;
			case 1: //EL ARCHIVO EXISTE
				//establecemos conexion con el FM9
				int socketFM9 = SocketClient_ConnectToServerIP(settings->ipFM9, settings->puertoFM9);
				//aca tengo que usar el getdata del MDJ y cuando tenga todo, lo cargo al FM9
				//y le aviso al SAFA
				break;
		}
	}

	free(response);
	free(data);
	Serialization_CleanupDeserializationStruct(data);
	free(arriveData);
}


void DAM_Flush(void* arriveData)
{
	DeserializedData* dest = Serialization_Deserialize(arriveData);

	//TODO: verificar que el paquete tenga al menos 2 elementos.
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

	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);
	int socketFM9 = SocketClient_ConnectToServerIP(settings->ipFM9, settings->puertoFM9);

	char* tmpData;
	char* archivo;

	//Lo que deberiamos hacer es pedirle al FM9 informacion hasta que nos llegue vacio
	while(1)
	{
		//aca habria que armar un paquete serializado para el FM9 que contenga la direccion logica que queremos y tambien nuestro transfer size
		//le mandamos el paquete al FM9 y esperamos que nos envia los bytes solicitados

		//si recibimos vacio hacer break!
	}

	//char* archivo = "MAKE c:/1/2/3.txt\nPULL d:/9/8/.bat\nULTIMA LINEA\nMAKE c:/1/2/3.txt\nPULL d:/9/8/.bat\nULTIMA LINEA";


	//una vez que recibimos todo por parte del FM9 se lo mandamos al mdj:

	int offset = 0;
	int len = strlen(archivo); //TODO: Verificar que tenga un \0 lo que recibimos al final sino el strlen() falla, si no tiene agregarlo
	int sizeToSend = 0;
	int msg_type, length, status;

	while(1)
	{
		if(len <= 0)
			break;

		if(len < settings->transferSize)
			sizeToSend = len;
		else
			sizeToSend = settings->transferSize;

		len -= sizeToSend;
		printf("ENVIANDO AL MDJ %d BYTES\n", sizeToSend );

		declare_and_init(poffset, uint32_t, offset);
		SerializedPart p_offset = {.size = sizeof(uint32_t), .data = poffset};

		declare_and_init(psize, uint32_t, sizeToSend);
		SerializedPart p_size = {.size = sizeof(uint32_t), .data = psize};

		SerializedPart p_path = {.size = strlen(filePath)+1, .data = filePath};

		void* buffer = malloc(sizeToSend);
		memcpy(buffer, ((void*)(archivo + offset)), sizeToSend);

		SerializedPart p_buffer = {.size = sizeToSend, .data = buffer};

		SerializedPart* serializedContent = Serialization_Serialize(4, p_path, p_offset, p_size, p_buffer);

		printf("ENVIANDO AL MDJ %d BYTES de buffer, tamaño del paquete: %d\n", sizeToSend , serializedContent->size);

		SocketCommons_SendData(socketMDJ, MESSAGETYPE_MDJ_PUTDATA, serializedContent->data, serializedContent->size);

		Serialization_CleanupSerializedPacket(serializedContent);
		free(poffset);
		free(psize);
		free(buffer);
		offset += sizeToSend;

		//Aguardamos validacion del MDJ
		void* data = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);
		printf("RESPUESTA DEL MDJ: %d \n", *((uint32_t*)data) );

		if(msg_type == MESSAGETYPE_INT)
		{
			switch(*((uint32_t*)data))
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

void DAM_Borrar(void* arriveData)
{
	DeserializedData* data = Serialization_Deserialize(arriveData);

	//verificamos que nos pasen los dos campos necesarios para la operacion
	if(data->count < 2) {
		//no tenemos los datos necesarios para seguir adelante
		Logger_Log(LOG_ERROR, "DAM::DAM_Borrar -> Recibido menos de 2 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		free(arriveData);
		return;
	}

	uint32_t idDTB = *((uint32_t*)data->parts[0]);
	char* filePath = (char*)data->parts[1];

	//abrimos conexion con el MDJ
	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);

	//le mandamos los datos al MDJ para que elimine el archivo
	SocketCommons_SendStringAsContent(socketMDJ, filePath, MESSAGETYPE_MDJ_DELETEFILE);
	int msg_type, length, status;
	void* response = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);
	if(msg_type == MESSAGETYPE_INT)
	{
		switch(*((uint32_t*)response))
		{
			case 0: //OK
				//El DAM se conecta con el SAFA para avisarle que se elimino OK el archivo
				//No tengo que conectarme al FM9 para pedir la direccion logica
				int socketSAFA = SocketClient_ConnectToServerIP(settings->ipSAFA, settings->puertoSAFA);
				SerializedPart idForSAFA = {.size = sizeof(uint32_t), .data = idDTB};
				SerializedPart* packetForSAFA = Serialization_Serialize(1, idForSAFA);
				Logger_Log(LOG_INFO, "DAM::DAM_Borrar -> Se elimino correctamente el archivo de path %s.", filePath);
				SocketCommons_SendData(socketSAFA, MESSAGETYPE_DAM_SAFA_BORRAR, (void*)packetForSAFA->data, packetForSAFA->size);
				free(packetForSAFA->data);
				free(packetForSAFA);
				break;
			case 10: //INSUFFICIENT_SPACE
			break;
			case 11: //METADATA_OPEN_ERROR
			break;
			case 12: //FILE_NOT_EXISTS
				Logger_Log(LOG_ERROR, "DAM::DAM_Borrar -> El archivo solicitado con ubicacion %s no existe!\n", filePath);
				DAM_ErrorOperacion(idDTB);
				break;
		}
	}
	free(response);
	free(data);
	Serialization_CleanupDeserializationStruct(data);
	free(arriveData);
}

void DAM_ErrorOperacion(uint32_t idDTB)
{
	//le comunico del error al SAFA y le paso el id del DTB
	int socketSAFA = SocketClient_ConnectToServerIP(settings->ipSAFA, settings->puertoSAFA);
	SerializedPart idForSAFA = {.size = sizeof(uint32_t), .data = idDTB};
	SerializedPart* packetForSAFA = Serialization_Serialize(1, idForSAFA);
	printf("Enviando error de operacion al SAFA\n");
	SocketCommons_SendData(socketSAFA, MESSAGETYPE_DAM_SAFA_ERR, (void*)packetForSAFA->data, packetForSAFA->size);
	free(packetForSAFA->data);
	free(packetForSAFA);
}
