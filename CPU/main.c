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
		int err,messageType;
//		waitSafaOrders();
		void* msgFromSafa = SocketCommons_ReceiveData(safa,&messageType,&err);

		DeserializedData data;
		Serialization_Deserialize(msgFromSafa,&data);

		if ( *(int*)data.parts[0] == 0)
			{
			//TODO cambiar protocolo, hablar con pepe
				executeDummy(data, safa, diego);

				sleep(settings->retardo);

				continue;
			}

		else
			{
				int i = 0;
				int totalQuantum = *((int*)data.parts[3]);
				CommandInterpreter_Init();

				CommandInterpreter_RegisterCommand("abrir",(void*)CommandAbrir);
				CommandInterpreter_RegisterCommand("concentrar",(void*)CommandConcentrar);
				CommandInterpreter_RegisterCommand("asignar",(void*)CommandAsignar);
				CommandInterpreter_RegisterCommand("wait",(void*)CommandWait);
				CommandInterpreter_RegisterCommand("signal",(void*)CommandSignal);
				CommandInterpreter_RegisterCommand("flush",(void*)CommandFlush);
				CommandInterpreter_RegisterCommand("close",(void*)CommandClose);
				CommandInterpreter_RegisterCommand("crear",(void*)CommandCrear);
				CommandInterpreter_RegisterCommand("borrar",(void*)CommandBorrar);



			while( i < totalQuantum )
				{
				//TODO hacer verificacion de que verdaderamente me lleno una linea de codigo
				//TODO agregar direccion logica para mandar al FM9
					char* line = askLineToFM9(data, fm9); //Pido una linea
					Operation extraData = {};
					CommandInterpreter_Do(line, " ",NULL); //El null se tiene que cambiar por un struct extraData
														  //que me tengo que fijar bien que mandarle
					sleep(settings->retardo); //retardo por operacion

				// TODO terminar el command interpretar siempre ejecutando linea por linea y actualizando el PC de SAFA,
				// NO OLVIDAR RETARDO POR OPERACION

				}


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


void* CommandConcentrar(int argC, char** args, char* callingLine, void* extraData){
	sleep(settings->retardo);
	//StringUtils_FreeArray(args);
	}

