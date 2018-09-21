#include "bibliotecaSAFA.h"


void configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));


	char* campos[] = {
			"PUERTO",
			"ALGORITMO",
			"QUANTUM",
			"MULTIPROGRAMACION",
			"RETARDO_PLANIF"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	settings->puerto = archivoConfigSacarIntDe(archivoConfig, "PUERTO");
	strcpy(settings->algortimo, archivoConfigSacarStringDe(archivoConfig, "ALGORITMO"));
	settings->quantum = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	settings->multiprogramacion = archivoConfigSacarIntDe(archivoConfig, "MULTIPROGRAMACION");
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO_PLANIF");


	archivoConfigDestruir(archivoConfig);

}
