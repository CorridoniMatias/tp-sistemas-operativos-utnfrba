#include "headers/DAMInterface.h"

// Static functions
void* DAM_ReadFile(char* filePath, int socketMDJ, int* received_content_length)
{
	int bufferSize = 0;
	void* buffer = malloc(1);

	while(1)
	{
		declare_and_init(poffset, uint32_t, bufferSize);
		SerializedPart p_offset = {.size = sizeof(uint32_t), .data = poffset};

		declare_and_init(psize, uint32_t, settings->transferSize);
		SerializedPart p_size = {.size = sizeof(uint32_t), .data = psize};
		SerializedPart p_path = {.size = strlen(filePath)+1, .data = filePath};
		SerializedPart* serializedContent = Serialization_Serialize(3, p_path, p_offset, p_size);

		SocketCommons_SendData(socketMDJ, MESSAGETYPE_MDJ_GETDATA, serializedContent->data, serializedContent->size);
		free(psize);
		free(poffset);
		Serialization_CleanupSerializedPacket(serializedContent);

		int message_type, error_status, message_length;
		void* recvData = SocketCommons_ReceiveData(socketMDJ, &message_type, &message_length, &error_status);

		if(message_length == 0) //recvData es NULL por definicion de las kemmens, si llegamos a length 0 es porque no hay mas nada para recibir, tenemos el archivo completo.
			break;

		switch(message_type)
		{
			case MESSAGETYPE_INT: //codigo de error.
			break;
			case MESSAGETYPE_STRING:
			{
				bufferSize += message_length;
				buffer = realloc(buffer, bufferSize);
				memcpy(buffer+(bufferSize-message_length), recvData, message_length);
				free(recvData);
				break;
			}
		}
	}

	if(bufferSize == 0)
		free(buffer);

	*received_content_length = bufferSize;

	return buffer;
}

static void* DAM_ReadFileFromFM9(uint32_t dtbID, uint32_t logicalAddress, int socketFM9, int* received_content_length)
{
	int bufferSize = 0;
	void* buffer = malloc(1);

	declare_and_init(id,uint32_t,dtbID)
	SerializedPart p_id = {.size = sizeof(uint32_t), .data = id};
	declare_and_init(address, uint32_t, logicalAddress)
	SerializedPart p_address = {.size = sizeof(uint32_t), .data = address};
	declare_and_init(psize, uint32_t, settings->transferSize);
	SerializedPart p_size = {.size = sizeof(uint32_t), .data = psize};
	SerializedPart* serializedContent = Serialization_Serialize(3, p_id, p_address,p_size);

	SocketCommons_SendData(socketFM9, MESSAGETYPE_FM9_FLUSH, serializedContent->data, serializedContent->size);
	free(id);
	free(address);
	free(psize);
	Serialization_CleanupSerializedPacket(serializedContent);

	while(1)
	{
		int message_type, error_status, message_length;

		void* recvData = SocketCommons_ReceiveData(socketFM9, &message_type, &message_length, &error_status);

		if(message_length == 0) //recvData es NULL por definicion de las kemmens, si llegamos a length 0 es porque no hay mas nada para recibir, tenemos el archivo completo.
			break;

		switch(message_type)
		{
			case MESSAGETYPE_INT: //codigo de error.
			break;
			case MESSAGETYPE_STRING:
			{
				bufferSize += message_length;
				buffer = realloc(buffer, bufferSize);
				memcpy(buffer+(bufferSize-message_length), recvData, message_length);
				free(recvData);
				break;
			}
		}

		SocketCommons_SendData(socketFM9, MESSAGETYPE_FM9_FLUSH, 0, 0);
	}

	if(bufferSize == 0)
		free(buffer);

	*received_content_length = bufferSize;

	return buffer;
}


static int DAM_SendToFM9(int socketFM9, void* content, int len, uint32_t iddtb)
{
	int offset = 0;
	int sizeToSend = 0;
	int msg_type, length, status;
	int error = 0;

	while(1)
	{
		if(len <= 0){
			SocketCommons_SendData(socketFM9,MESSAGETYPE_INT, NULL, 0);
			break;
		}

		if(len < settings->transferSize)
			sizeToSend = len;
		else
			sizeToSend = settings->transferSize;

		len -= sizeToSend;

		declare_and_init(piddtb, uint32_t, iddtb);
		SerializedPart p_iddtb = {.size = sizeof(uint32_t), .data = piddtb};

		declare_and_init(psize, uint32_t, sizeToSend);
		SerializedPart p_size = {.size = sizeof(uint32_t), .data = psize};

		void* buffer = malloc(sizeToSend);
		memcpy(buffer, ((void*)(content + offset)), sizeToSend);

		SerializedPart p_buffer = {.size = sizeToSend, .data = buffer};

		SerializedPart* serializedContent = Serialization_Serialize(3, p_iddtb, p_size, p_buffer);

		SocketCommons_SendData(socketFM9, MESSAGETYPE_FM9_OPEN, serializedContent->data, serializedContent->size);

		Serialization_CleanupSerializedPacket(serializedContent);
		free(piddtb);
		free(psize);
		free(buffer);
		offset += sizeToSend;

		void* data = SocketCommons_ReceiveData(socketFM9, &msg_type, &length, &status);

		if(msg_type == MESSAGETYPE_INT)
		{
			switch(*((uint32_t*)data))
			{
				case 1: //parte recibida OK
				break;
				case 400: //bad request
					error = 1;
				break;
			}
		}
		free(data);
		if(error == 1)
			break;
	}

	if(error == 0) //recibimos la direccion de donde esta el archivo
	{
		void* data = SocketCommons_ReceiveData(socketFM9, &msg_type, &length, &status);

		if(msg_type == MESSAGETYPE_ADDRESS)
		{
			uint32_t tmp = *((uint32_t*)data);
			free(data);
			return tmp;
		} else if(msg_type == MESSAGETYPE_INT)
		{
			return -1;
		}
	}

	return -1;
}

void DAM_ErrorOperacion(uint32_t idDTB)
{
	//le comunico del error al SAFA y le paso el id del DTB
	declare_and_init(pointer_iddtb, uint32_t, idDTB);

	Logger_Log(LOG_DEBUG, "Enviando error de operacion al SAFA para %d", idDTB);
	SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_ERR, (void*)pointer_iddtb, sizeof(uint32_t));
	free(pointer_iddtb);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////// FUNCIONES DE INTERFAZ //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DAM_Crear(void* arriveData)
{
	OnArrivedData* onArriveData = (OnArrivedData*) arriveData;

	DeserializedData* data = Serialization_Deserialize(onArriveData->receivedData);

	if(data->count < 3) {
		Logger_Log(LOG_ERROR, "DAM::DAM_Crear -> Recibido menos de 3 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		SocketServer_CleanOnArrivedData(onArriveData);
		return;
	}

	uint32_t idDTB = *((uint32_t*)data->parts[0]);
	char* filePath = (char*)data->parts[1];
	uint32_t cantNewLines = *((uint32_t*)data->parts[2]);

	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);

	declare_and_init(newlines, uint32_t, cantNewLines);
	SerializedPart p_newlines = {.size = sizeof(uint32_t), .data = newlines};
	SerializedPart p_filepath = {.size = strlen(filePath)+1, .data = filePath};
	SerializedPart* packet = Serialization_Serialize(2, p_filepath, p_newlines);

	SocketCommons_SendData(socketMDJ, MESSAGETYPE_MDJ_CREATEFILE, packet->data, packet->size);
	Serialization_CleanupSerializedPacket(packet);
	int msg_type, length, status;
	void* response = SocketCommons_ReceiveData(socketMDJ, &msg_type, &length, &status);

	if(msg_type == MESSAGETYPE_INT)
	{
		switch(*((uint32_t*)response))
		{
			case 0: //OK
				Logger_Log(LOG_INFO, "DAM::DAM_Crear -> Se realizo correctamente la operacion.");
				//Avisamos al SAFA que el crear termino
				SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_CREAR, data->parts[0], sizeof(uint32_t));
			break;
			case 1: //EXISTING_FILE
			case 2: //METADATA_CREATE_ERROR
			case 10: //INSUFFICIENT_SPACE
			case 11: //METADATA_OPEN_ERROR
				Logger_Log(LOG_ERROR, "DAM::DAM_Crear -> ERROR N: %d", *((uint32_t*)response));
				DAM_ErrorOperacion(idDTB);
			break;

		}
	}
	free(response);
	close(socketMDJ);

	Serialization_CleanupDeserializationStruct(data);
	free(newlines);
	SocketServer_CleanOnArrivedData(onArriveData);
}

void DAM_Abrir(void* arriveData)
{
	OnArrivedData* onArriveData = (OnArrivedData*) arriveData;

	DeserializedData* data = Serialization_Deserialize(onArriveData->receivedData);

	//verificamos que nos esten enviando los dos campos necesarios
	if(data->count < 2) {
		//no tenemos los datos necesarios para seguir adelante
		Logger_Log(LOG_ERROR, "DAM::DAM_Abrir -> Recibido menos de 2 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		SocketServer_CleanOnArrivedData(onArriveData);
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
			{
				//printf("El archivo solicitado con ubicacion %s no existe!\n", filePath);
				Logger_Log(LOG_ERROR, "DAM::DAM_Abrir -> El archivo solicitado con ubicacion %s no existe!\n", filePath);
				DAM_ErrorOperacion(idDTB);
				break;
			}
			case 1: //EL ARCHIVO EXISTE
			{
				//establecemos conexion con el FM9 y SAFA
				int socketFM9 = SocketClient_ConnectToServerIP(settings->ipFM9, settings->puertoFM9);

				int file_size;
				//Recibimos Archivo desde MDJ
				void* file_content = DAM_ReadFile(filePath, socketMDJ, &file_size);
				close(socketMDJ);

				int logicAddr = DAM_SendToFM9(socketFM9, file_content, file_size, idDTB);

				if(logicAddr >= 0)
				{
					SerializedPart p_filepath = {.size = strlen(filePath)+1, .data = data->parts[1]};
					declare_and_init(p_logic, uint32_t, logicAddr);
					SerializedPart p_direccion = {.size = sizeof(uint32_t), .data = p_logic};
					SerializedPart p_iddtb = {.size = sizeof(uint32_t), .data = data->parts[0]};

					SerializedPart* part = Serialization_Serialize(3, p_iddtb, p_filepath, p_direccion);

					//Verificamos si la operacion real fue abrir un archivo o hacer la operacion dummy
					if(onArriveData->message_type == MESSAGETYPE_CPU_EXECDUMMY)
						SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_DUMMY, part->data, part->size);
					else
						SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_ABRIR, part->data, part->size);

					Serialization_CleanupSerializedPacket(part);
					free(p_logic);
				} else
				{
					SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_ERR, data->parts[0], sizeof(uint32_t));
				}

				close(socketFM9);
				break;
			}
		}
	}

	free(response);
	Serialization_CleanupDeserializationStruct(data);
	SocketServer_CleanOnArrivedData(onArriveData);
}

void DAM_Flush(void* arriveData)
{

	OnArrivedData* data = arriveData;

	DeserializedData* dest = Serialization_Deserialize(data->receivedData);

	if(dest->count < 3) {
		//no tenemos los datos necesarios para seguir adelante
		Logger_Log(LOG_ERROR, "DAM::DAM_Borrar -> Recibido menos de 3 parametros.");
		Serialization_CleanupDeserializationStruct(dest);
		SocketServer_CleanOnArrivedData(data);
		return;
	}

	uint32_t dtbID = *((uint32_t*) dest->parts[0]);
	uint32_t direccionLogica = *((int*) dest->parts[1]);
	char* filePath = (char*) dest->parts[3];


	//Primero que nada tenemos que abrir una conexion con el FM9 y una conexion con el MDJ
	//(esto seria la opcion 3 de la arquitectura interna: hilos con paralelismo)

	//Esta opcion es la mas conveniente porque por cada hilo hay una conexion distinta con el MDJ y el FM9 por lo que
	//podemos hacer todos los send() y recv() que queramos sin la necesidad de que el SocketServer intervenga porque cada hilo es cliente propio.
	//de lo contrario si hubiera una sola conexion con estos servicios habria que sincronizar todos los hilos para que solo 1 de ellos pueda hacer send() y recv() a la vez
	//sino podria pasar que dos hilos hacen send() y recv() y reciban datos cruzados!

	int socketMDJ = SocketClient_ConnectToServerIP(settings->ipMDJ, settings->puertoMDJ);
	int socketFM9 = SocketClient_ConnectToServerIP(settings->ipFM9, settings->puertoFM9);

	int len;

	char* archivo = DAM_ReadFileFromFM9(dtbID, direccionLogica,socketFM9, &len);
	//Usado para test:
	//char* archivo = "MAKE c:/1/2/3.txt\nPULL d:/9/8/.bat\nULTIMA LINEA\nMAKE c:/1/2/3.txt\nPULL d:/9/8/.bat\nULTIMA LINEA";

	int offset = 0;
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
				{
					//Avisamos al SAFA que el flush termino
					SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_FLUSH, dest->parts[0], sizeof(uint32_t));
				break;
				}
				case 12: //FILE_NOT_EXISTS
				case 11: //METADATA_OPEN_ERROR
				case 10: //INSUFFICIENT_SPACE
				{
					Logger_Log(LOG_ERROR, "ERROR EN FLUSH POR PARTE DEL MDJ: %d", *((uint32_t*)data));
					DAM_ErrorOperacion(dtbID);
				break;
				}
			}
		}
		free(data);
	}

	free(archivo);
	close(socketFM9);
	close(socketMDJ);

	Serialization_CleanupDeserializationStruct(dest);
	SocketServer_CleanOnArrivedData(data);
}

void DAM_Borrar(void* arriveData)
{
	OnArrivedData* onArriveData = (OnArrivedData*) arriveData;

	DeserializedData* data = Serialization_Deserialize(onArriveData->receivedData);

	//verificamos que nos pasen los dos campos necesarios para la operacion
	if(data->count < 2) {
		//no tenemos los datos necesarios para seguir adelante
		Logger_Log(LOG_ERROR, "DAM::DAM_Borrar -> Recibido menos de 2 parametros.");
		Serialization_CleanupDeserializationStruct(data);
		SocketServer_CleanOnArrivedData(onArriveData);
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
			{
				//El DAM se conecta con el SAFA para avisarle que se elimino OK el archivo
				Logger_Log(LOG_INFO, "DAM::DAM_Borrar -> Se elimino correctamente el archivo de path %s.", filePath);
				//Avisamos al SAFA que el borrar termino
				SocketCommons_SendData(settings->socketSAFA, MESSAGETYPE_DAM_SAFA_BORRAR, (void*)data->parts[0], sizeof(uint32_t));
			break;
			}

			case 12: //FILE_NOT_EXISTS
			{
				Logger_Log(LOG_ERROR, "DAM::DAM_Borrar :: ERROR 60001 -> El archivo solicitado con ubicacion %s no existe!\n", filePath);
				DAM_ErrorOperacion(idDTB);
			break;
			}
		}
	}
	close(socketMDJ);
	free(response);

	Serialization_CleanupDeserializationStruct(data);
	SocketServer_CleanOnArrivedData(onArriveData);
}

