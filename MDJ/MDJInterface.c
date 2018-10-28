#include "headers/MDJInterface.h"

static OnArrivedData* GetArriveData(void* data)
{
	return (OnArrivedData*)data;
}

static void delay()
{
	usleep(config->delay);
}

void MDJ_CheckFile(void* arriveData)
{
	delay();
	OnArrivedData* data = GetArriveData(arriveData);

	//Obtener el path:
	char* path = (char*)data->receivedData;

	uint32_t valid = ((FIFA_IsFileValid(path)) ? 1 : 0);

	//del otro lado se deberia usar SocketCommons_ReceiveDataWithoutHeader por un size de uint32_t
	SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)valid, sizeof(uint32_t));

	free(path);
	free(arriveData);
}

void MDJ_CreateFile(void* arriveData)
{
	delay();
	OnArrivedData* data = GetArriveData(arriveData);

	DeserializedData* actualData = malloc(sizeof(DeserializedData));

	Serialization_Deserialize(data->receivedData, actualData);

	if(actualData->count < 2)
	{
		uint32_t status = FIFA_CreateFile(((char*)(actualData->parts[0])), *((uint32_t*)(actualData->parts[1])));

		SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)status, sizeof(uint32_t));
	} else
	{
		//En honor a HTTP : Bad Request
		SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)400, sizeof(uint32_t));
	}

	Serialization_CleanupDeserializationStruct(actualData);
	free(data->receivedData);
	free(arriveData);
}

void MDJ_GetData(void* arriveData)
{
	delay();
	OnArrivedData* data = GetArriveData(arriveData);

	DeserializedData* actualData = malloc(sizeof(DeserializedData));

	Serialization_Deserialize(data->receivedData, actualData);

	if(actualData->count < 3)
	{
		char* path = ((char*)(actualData->parts[0]));
		int offset = *((uint32_t*)(actualData->parts[1]));
		int size = *((uint32_t*)(actualData->parts[2]));
		int copied;

		char* content = FIFA_ReadFile(path, offset, size, &copied);

		if(content == NULL)
			SocketCommons_SendData(data->calling_SocketID, MESSAGETYPE_INT, (void*)404, sizeof(uint32_t));
		else
			SocketCommons_SendData(data->calling_SocketID, MESSAGETYPE_STRING, (void*)content, copied);
	} else
	{
		//En honor a HTTP : Bad Request
		SocketCommons_SendData(data->calling_SocketID, MESSAGETYPE_INT, (void*)400, sizeof(uint32_t));
	}

	Serialization_CleanupDeserializationStruct(actualData);
	free(data->receivedData);
	free(arriveData);
}

void MDJ_PutData(void* arriveData)
{
	delay();
	OnArrivedData* data = GetArriveData(arriveData);

	DeserializedData* actualData = malloc(sizeof(DeserializedData));

	Serialization_Deserialize(data->receivedData, actualData);

	if(actualData->count < 4)
	{
		char* path = ((char*)(actualData->parts[0]));
		int offset = *((uint32_t*)(actualData->parts[1]));
		int size = *((uint32_t*)(actualData->parts[2]));
		void* buffer = actualData->parts[3];

		uint32_t status = FIFA_WriteFile(path, offset, size, buffer);

		SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)status, sizeof(uint32_t));
	} else
	{
		//En honor a HTTP : Bad Request
		SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)400, sizeof(uint32_t));
	}

	Serialization_CleanupDeserializationStruct(actualData);
	free(data->receivedData);
	free(arriveData);
}

void MDJ_DeleteFile(void* arriveData)
{
	delay();
	OnArrivedData* data = GetArriveData(arriveData);

	//Obtener el path:
	char* path = (char*)data->receivedData;

	uint32_t deleteStatus = FIFA_DeleteFile(path);

	//del otro lado se deberia usar SocketCommons_ReceiveDataWithoutHeader por un size de uint32_t
	SocketCommons_SendDataWithoutHeader(data->calling_SocketID, (void*)deleteStatus, sizeof(uint32_t));

	free(path);
	free(arriveData);
}
