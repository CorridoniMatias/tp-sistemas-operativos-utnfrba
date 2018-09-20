#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/SocketClient.h"
#include "bibliotecaCPU.h"


int main(int argc, char **argv)
{
	//Loggers habituales
	Logger_CreateLog("./CPU.log", "CPU", true);
	Logger_Log(LOG_INFO, "Proceso CPU iniciado...");
	//Configuro bajo la variable settings
	configurar();
	conectarAProceso(settings->ipSAFA,settings->puertoSAFA,"SAFA");
	conectarAProceso(settings->ipDIEGO,settings->puertoDIEGO,"DIEGO");
	exit_gracefully(0);

}

