#include "bibliotecaCPU.h"

int main(int argc, char *argv[])
{
	//Loggers habituales
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	//Configuro bajo la variable settings
	configurar();
	CommandInterpreter_Init();
	Start_commands(); //Se incian los comandos

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

			int cant = *((uint32_t*) data->parts[6]);
			printf("cant=%d",cant);
			t_dictionary* dictionary = BuildDictionary(data->parts[7], cant);
			//Defino aca el struct para que se vaya actualizando el diccionario dependiendo cualquier cambio
			Operation extraData;
			extraData.dtb = *((uint32_t*) data->parts[0]);
			extraData.socketSAFA = safa;
			extraData.socketFM9 = fm9;
			extraData.socketDIEGO = diego;
			extraData.commandResult = 0;
			extraData.dictionary = dictionary;

			printf("\n\nprogram counter de data=%d\n\n",*((uint32_t*)data->parts[4]));
			extraData.programCounter = *((uint32_t*)data->parts[4]);
			printf("\n\nprogram counter recibido=%d\n\n",extraData.programCounter);
			extraData.quantum = *((int32_t*) data->parts[5]);

			while( extraData.quantum > 0 )
			{

					char* line = askLineToFM9(extraData.dtb,*((uint32_t*) data->parts[3]),extraData.programCounter, fm9); //Pido una linea
					if(strcmp(line,"error") != 0){
						usleep(settings->retardo*1000); //retardo por operacion
						extraData.programCounter++;
						extraData.quantum--;
						bool res = CommandInterpreter_Do(line, " ",&extraData);
						free(line);
						if(res == 1 && extraData.commandResult == 0){
							continue;
						}

						else if (extraData.commandResult == 2) {
							break;
						}

					// Terminar el command interpretar siempre ejecutando linea por linea y actualizando el PC de SAFA,
					}

					else if (strcmp(line,"") != 0){
						uint32_t idDtb = extraData.dtb;
						declare_and_init(id, uint32_t,idDtb);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));

						free(line);
						free(id);

						break;
					}
					else{
						uint32_t idDtb = extraData.dtb;
						declare_and_init(id, int32_t,idDtb);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, id, sizeof(uint32_t));
						free(line);
						free(id);

						break;
					}


				}
			if (extraData.quantum == 0) {
				declare_and_init(newQ, int32_t, extraData.quantum);
				SerializedPart fieldForSAFA1 = { .size = sizeof(uint32_t),.data = newQ };
				declare_and_init(updatedProgramCounter,uint32_t,extraData.programCounter)
				SerializedPart fieldForSAFA2 = { .size = sizeof(uint32_t), .data = updatedProgramCounter };
				uint32_t numberOfFiles = dictionary_size(dictionary);
				declare_and_init(newNumberOfFiles, uint32_t, numberOfFiles);

				// TODO TESTEAR BIEN SI ESTO QUEDARIA ACTUALIZADO CON EL ULTIMO VALOR O NO
				SerializedPart fieldForSAFA3 = { .size = sizeof(uint32_t),
						.data = newNumberOfFiles };

				SerializedPart fieldForSAFA4 = FlattenPathsAndAddresses(
						extraData.dictionary);

				SerializedPart* packetToSAFA = Serialization_Serialize(4,
						fieldForSAFA1, fieldForSAFA2, fieldForSAFA3,
						fieldForSAFA4);

				SocketCommons_SendData(safa, MESSAGETYPE_CPU_EOQUANTUM,
						packetToSAFA->data, packetToSAFA->size);

				free(newQ);
				free(newNumberOfFiles);
				Serialization_CleanupSerializedPacket(packetToSAFA);
			}
			Serialization_CleanupDeserializationStruct(data);
			dictionary_destroy_and_destroy_elements(extraData.dictionary, free);
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


