#include "headerFiles/Settings.h"


void configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));


	char* campos[] = {
			"PUERTO",
			"ALGORITMO",
			"QUANTUM",
			"MULTIPROGRAMACION",
			"RETARDO_PLANIF",
			NULL
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	settings->puerto = archivoConfigSacarIntDe(archivoConfig, "PUERTO");
	char* algoritmo = archivoConfigSacarStringDe(archivoConfig, "ALGORITMO");
	settings->algoritmo = malloc(strlen(algoritmo) + 1);
	strcpy(settings->algoritmo, algoritmo);
	settings->quantum = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	settings->multiprogramacion = archivoConfigSacarIntDe(archivoConfig, "MULTIPROGRAMACION");
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO_PLANIF");

	archivoConfigDestruir(archivoConfig);
	free(algoritmo);

}
