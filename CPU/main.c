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

	while(1)
	{


		int err,messageType,msglength;
		void* msgFromSafa = SocketCommons_ReceiveData(safa,&messageType,&msglength,&err);
		DeserializedData* data = Serialization_Deserialize(msgFromSafa);
		 int flagg = *(int*)data->parts[1];
		if (flagg == 0)
			{

				executeDummy(data, diego, safa);

				usleep(settings->retardo*1000);

				continue;

			}

		else
			{
				int i = 0;
				uint32_t totalQuantum = *((int32_t*)data->parts[5]);
				uint32_t updatedProgramCounter = *((int32_t*)data->parts[4]);
				CommandInterpreter_Init();

				t_dictionary* dictionary= BuildDictionary(data->parts[7],*((int*)data->parts[6]));
				//Defino aca el struct para que se vaya actualizando el diccionario dependiendo cualquier cambio
				Operation extraData;
				extraData.dictionary = dictionary;

				Start_commands(); //Se incian los comandos



			while( i < totalQuantum )
			{
					char* line = askLineToFM9(data, fm9); //Pido una linea
					if(strcmp(line,"error") != 0){

						extraData.dtb =*((int32_t*)data->parts[0]);
		 				extraData.programCounter = updatedProgramCounter;
						extraData.quantum = totalQuantum;
						extraData.dictionary = dictionary;
						extraData.socketSAFA = safa;
						extraData.socketFM9 = fm9;
						extraData.socketDIEGO = diego;
						extraData.commandResult = 0;

						bool res = CommandInterpreter_Do(line, " ",&extraData);

						if(res == 1 && extraData.commandResult == 0){
							sleep(settings->retardo); //retardo por operacion

							updatedProgramCounter ++;

							continue;
						}


						else if (extraData.commandResult == 2) {

							break;


						}

					// Terminar el command interpretar siempre ejecutando linea por linea y actualizando el PC de SAFA,
					}

					else if (strcmp(line,"") != 0){
						int32_t idDtb = extraData.dtb;
						declare_and_init(id, int32_t,idDtb);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));


						free(id);

						break;
					}
					else{
						int32_t idDtb = extraData.dtb;
						declare_and_init(id, int32_t,idDtb);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));

						free(id);

						break;
					}


				}
			declare_and_init(newQ, int32_t,totalQuantum);
			declare_and_init(newUpdatedProgramCounter, int32_t,updatedProgramCounter);
			SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data = newQ};
			SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newUpdatedProgramCounter};
			uint32_t numberOfFiles = dictionary_size(dictionary);
			declare_and_init(newNumberOfFiles, int32_t,numberOfFiles);

			// TODO TESTEAR BIEN SI ESTO QUEDARIA ACTUALIZADO CON EL ULTIMO VALOR O NO
			SerializedPart fieldForSAFA3 = {.size = sizeof(uint32_t) , .data = newNumberOfFiles};
			SerializedPart fieldForSAFA4 = {.size = strlen(FlattenPathsAndAddresses(dictionary)) + 1 , .data = FlattenPathsAndAddresses(dictionary)};
			SerializedPart* packetToSAFA = Serialization_Serialize(4, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4);

			SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOQUANTUM,packetToSAFA->data, packetToSAFA->size);

			free(newQ);
			free(newUpdatedProgramCounter);
			free(newNumberOfFiles);
			Serialization_CleanupSerializedPacket(packetToSAFA);
			free(extraData.dictionary);
			}

	}





//		 * EXPLICACION VILLERA TODO DOCUMENTAR BIEN
//		ACORDARME DE MANDARME EL QUANTUM QUE SOBRO A SAFA
//							bloqueado esperando recv del SAFA
						 //Deserializo todó el mensaje
						// Me fijo el id del DTb -> si es 0 es DUMMY, le hablo al DAM y al DAM le paso el path(pensar la estructura)
//						 * apenas le hablo al diego, le hablo al safa para que lo desaloje (le paso la estructura del id del dtb, codigo de error
//						 y )

//						---- usar continue ---- SALTEA CICLICAS
//						SI NO ES -> 0. Le hablo al FM9, me pasa el archivo o las lineas y me voy fijando que ejecutar
//										y voy haciendo las acciones requeridas



	free(settings);
	exit_gracefully(0);

}


