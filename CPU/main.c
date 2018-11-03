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
				//TODO hacer verificacion de que verdaderamente me lleno una linea de codigo
				char* line = askLineToFM9(data, fm9);

			}

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

