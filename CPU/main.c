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

		if ( *(int*)data->parts[1] == 0)
			{

				executeDummy(data, safa, diego);

				sleep(settings->retardo);

				continue;
			}

		else
			{
				int i = 0;
				uint32_t totalQuantum = *((int32_t*)data->parts[4]);
				uint32_t updatedProgramCounter = *((int32_t*)data->parts[4]);
				CommandInterpreter_Init();

				t_dictionary* dictionary= BuildDictionary(data->parts[5],*((int*)data->parts[6]));
				//Defino aca el struct para que se vaya actualizando el diccionario dependiendo cualquier cambio
				Operation extraData;
				extraData.dictionary = dictionary;

				CommandInterpreter_RegisterCommand("abrir",(void*)CommandAbrir);
				CommandInterpreter_RegisterCommand("concentrar",(void*)CommandConcentrar);
				CommandInterpreter_RegisterCommand("asignar",(void*)CommandAsignar);
				CommandInterpreter_RegisterCommand("wait",(void*)CommandWait);
				CommandInterpreter_RegisterCommand("signal",(void*)CommandSignal);
				CommandInterpreter_RegisterCommand("flush",(void*)CommandFlush);
				CommandInterpreter_RegisterCommand("close",(void*)CommandClose);
				CommandInterpreter_RegisterCommand("crear",(void*)CommandCrear);
				CommandInterpreter_RegisterCommand("borrar",(void*)CommandBorrar);
				//pensar como ignorar cuando empiece con #



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
							//TODO hacer todos los free
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

						SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};

						SerializedPart* packetToSafa = Serialization_Serialize(1,fieldForSAFA1);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, packetToSafa->data, packetToSafa->size);

						free(id);
						Serialization_CleanupSerializedPacket(packetToSafa);
						break;
					}
					else{
						int32_t idDtb = extraData.dtb;
						declare_and_init(id, int32_t,idDtb);

						SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};

						SerializedPart* packetToSafa = Serialization_Serialize(1,fieldForSAFA1);

						SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOFORABORT, packetToSafa->data, packetToSafa->size);

						free(id);
						Serialization_CleanupSerializedPacket(packetToSafa);
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
			SerializedPart fieldForSAFA4 = {.size = sizeof(FlattenPathsAndAddresses(dictionary)) + 1 , .data = FlattenPathsAndAddresses(dictionary)};
			SerializedPart* packetToSAFA = Serialization_Serialize(4, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4);

			SocketCommons_SendData(safa,MESSAGETYPE_CPU_EOQUANTUM,packetToSAFA->data, packetToSAFA->size);

			free(newQ);
			free(newUpdatedProgramCounter);
			free(newNumberOfFiles);
			Serialization_CleanupSerializedPacket(packetToSAFA);
			free(extraData.dictionary);
			}

	}




/*
		 * EXPLICACION VILLERA TODO DOCUMENTAR BIEN
		ACORDARME DE MANDARME EL QUANTUM QUE SOBRO A SAFA
							bloqueado esperando recv del SAFA
						 //Deserializo todó el mensaje
						// Me fijo el id del DTb -> si es 0 es DUMMY, le hablo al DAM y al DAM le paso el path(pensar la estructura)
						 * apenas le hablo al diego, le hablo al safa para que lo desaloje (le paso la estructura del id del dtb, codigo de error
						 y )

						---- usar continue ---- SALTEA CICLICAS
						SI NO ES -> 0. Le hablo al FM9, me pasa el archivo o las lineas y me voy fijando que ejecutar
										y voy haciendo las acciones requeridas


*/

	exit_gracefully(0);

}


