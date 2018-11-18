#include "headers/Tests.h"
#include "kemmens/Utils.h"
#include "headers/DAMInterface.h"


void Test_CreateFile()
{
	declare_and_init(newlines, uint32_t, 80);
	char* filePath = "/nuevoArchivoRePiola.algo";
	SerializedPart p_newlines = {.size = sizeof(uint32_t), .data = newlines};
	SerializedPart p_filepath = {.size = strlen(filePath)+1, .data = filePath};
	SerializedPart* packet = Serialization_Serialize(2, p_filepath, p_newlines);
	DAM_Crear((void*)packet->data);
	free(newlines);
	Serialization_CleanupSerializedPacket(packet);
}
