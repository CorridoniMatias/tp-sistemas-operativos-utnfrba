#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "bibliotecaDAM.h"

int main(int argc, char **argv)
{
	//TODO: Terminar, en duda con funciones a threadear
	Configuracion* settings = (Configuracion*)malloc(sizeof(Configuracion));
	configurar(settings);
	Logger_CreateLog("./DAM.log", "DAM", true);
	Logger_Log(LOG_INFO, "Proceso DAM iniciado; escuchando en el puerto %d", settings->puertoEscucha);
	CommandInterpreter_Init();
	pozoDeHilos = ThreadPool_CreatePool(10, false);
	conectarAProceso(settings->ipSAFA, settings->puertoSAFA, "S-AFA");
	conectarAProceso(settings->ipFM9, settings->puertoFM9, "FM9");
	conectarAProceso(settings->ipMDJ, settings->puertoMDJ, "MDJ");
	free(settings);
	ThreadPool_FreeGracefully(pozoDeHilos);
	exit_gracefully(0);
}

