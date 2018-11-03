#include "bibliotecaCPU.h"


int main(int argc, char *argv[])
{
	//Loggers habituales
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	//Configuro bajo la variable settings
	configurar();
	int safa = conectarAProceso(settings->ipSAFA,settings->puertoSAFA,"SAFA");
	int diego = conectarAProceso(settings->ipDIEGO,settings->puertoDIEGO,"DIEGO");
	int fm9 = conectarAProceso(settings->ipFM9,settings->puertoFM9,"FM9");


	while(1){
		int err,messageType;
//		waitSafaOrders();
		void* msgFromSafa = SocketCommons_ReceiveData(safa,&messageType,&err);

		DeserializedData data;
		Serialization_Deserialize(msgFromSafa,&data);

		if ( *(int*)data.parts[0] == 0){

			executeDummy(data, safa, diego);

			sleep(settings->retardo);

			continue;
		}
		else {
			int i = 0;
			while( i < *((int*)data.parts[3])){

				char* line = askLineToFM9(data);

			}

		}


		char* askLineToFM9(DeserializedData dtb){

			int* idDtb = (int*)malloc(4);
			char* path = (char*)malloc(sizeof(dtb.parts[1]));
			int* pc = (int*)malloc(4);
			int* code = (int*)malloc(4);

			*code = 0;

			SerializedPart fieldForFM91 = {.size = 4, .data = idDtb};
			SerializedPart fieldForFM92 = {.size = sizeof(dtb.parts[1]), .data = path};
			SerializedPart fieldForFM93 = {.size = 4, .data = pc};
			SerializedPart fieldForFM94 = {.size = 4, .data = code};

			void* packetToFM9 = Serialization_Serialize(4, fieldForFM91, fieldForFM92, fieldForFM93, fieldForFM94);

			SocketCommons_SendData(fm9,MESSAGETYPE_VOIDPOINTER, packetToFM9, sizeof(packetToFM9));

		}

		/*
		ACORDARME DE MANDARME EL QUANTUM QUE SOBRO A SAFA
							bloqueado esperando recv del SAFA
						 //Deserializo todo el mensaje
						// Me fijo el id del DTb -> si es 0 es DUMMY, le hablo al DAM y al DAM le paso el path(pensar la estructura)
						 * apenas le hablo al diego, le hablo al safa para que lo desaloje (le paso la estructura del id del dtb, codigo de error
						 y )

						---- usar continue ---- SALTEA CICLICAS
						SI NO ES -> 0. Le hablo al FM9, me pasa el archivo o las lineas y me voy fijando que ejecutar
										y voy haciendo las acciones requeridas




						 */
	}
	exit_gracefully(0);

}

