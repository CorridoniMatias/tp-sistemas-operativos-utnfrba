#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "headers/bibliotecaDAM.h"
#include "headers/DAMInterface.h"


int main(int argc, char **argv)
{
	Logger_CreateLog("./DAM.log", "DAM", true);
	inicializarVariablesGlobales();
	Logger_Log(LOG_INFO, "Proceso DAM iniciado; escuchando en el puerto %d", settings->puertoEscucha);


	levantarServidor();


	liberarVariablesGlobales();					//Libero la memoria de las variables globales
	exit_gracefully(0);
}

