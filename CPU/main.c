#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/SocketClient.h"
#include "bibliotecaCPU.h"


int main(int argc, char **argv)
{
	//Configuracion* settings = (Configuracion*)malloc(sizeof(Configuracion));
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	//conectarAProceso(settings->ipSAFA,settings->puertoSAFA,"SAFA");
	//conectarAProceso(settings->ipDIEGO,settings->puertoDIEGO,"DIEGO");
    //TODO Arreglar las ip y los puertos de los procesos. Se tienen varios puertos o no?
	exit_gracefully(0);

}

