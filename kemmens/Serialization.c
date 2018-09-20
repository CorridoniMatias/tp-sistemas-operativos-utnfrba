#include "kemmens/Serialization.h"

void SocketCommons_DeserializeContent(char* serializedContent, void** dataArray)
{

	int bytesToRead;		//Offset de los bytes que voy a leer; es el entero recibido antes de cada campo
	int base = 0;			//Indice de la cadena serializada en el cual estoy leyendo actualmente; arranca en 0
	int fieldNumber = 0;	//Indice del array donde quiero copiar el campo actual; arranca en 0

	bytesToRead = serializedContent[base++]; //Leo la longitud del proximo campo a leer; adelanto en uno el indice para leer lo siguiente
	if(isdigit(serializedContent[base]))	 //Hago esto en el caso de que el tamanio del proximo campo fueran dos digitos
	{
		bytesToRead *= 10;					      //Multiplico por 10 el valor actual, le sumo el siguiente (en el que estoy
		bytesToRead += serializedContent[base++]; //parado ahora) y adelanto de nuevo el puntero, asi no queda rezagado
	}

	while(bytesToRead != 0)				//Si 0 fuese la cantidad de bytes a leer, es porque ya no hay mas campos
	{
		//Copio la data al elemento correspondiente de datArray
		memcpy(&(dataArray[fieldNumber++]), &(serializedContent[base]), bytesToRead);
		base += bytesToRead;	//Adelanto el puntero de la cadena serializada el offset de lo que recien lei

		bytesToRead = serializedContent[base++];	//De nuevo, leo el proximo tamanio y verifico la cantidad de digitos
		if(isdigit(serializedContent[base]))
		{
			bytesToRead *= 10;
			bytesToRead += serializedContent[base++];
		}
	}

}

void* Serialization_Serialize(int fieldCount, ...)
{
	if(fieldCount < 1)
		return 0;

	va_list arguments;
	va_start(arguments, fieldCount);

	SerializedPart tmp;

	uint32_t totalsize = 0;
	uint32_t offset = 0;

	void* packet = malloc(1);

	for(int i = 0; i < fieldCount;i++)
	{
		tmp = va_arg(arguments, SerializedPart);

		totalsize += tmp.size + sizeof(uint32_t);

		packet = realloc(packet, totalsize);

		memcpy(packet + offset, &tmp.size, sizeof(uint32_t));

		offset += sizeof(uint32_t);

		memcpy(packet + offset, tmp.data, tmp.size);

		offset += tmp.size;
	}

	uint32_t packetEnd = 0;

	totalsize += sizeof(uint32_t);

	packet = realloc(packet, totalsize);

	memcpy(packet + offset, &packetEnd, sizeof(uint32_t));

	va_end(arguments);

	return packet;
}
