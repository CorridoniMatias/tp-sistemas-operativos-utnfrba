#include "bibliotecaFM9.h"

void Configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"PUERTO_ESCUCHA",
			"MODO",
			"TAMANIO",
			"MAX_LINEA",
			"TAM_PAGINA"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//strcpy(settings->puertoEscucha, archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	settings->puertoEscucha= atoi(archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	strcpy(settings->modo, archivoConfigSacarStringDe(archivoConfig, "MODO"));
	settings->tamanio = archivoConfigSacarIntDe(archivoConfig, "TAMANIO");
	settings->max_linea = archivoConfigSacarIntDe(archivoConfig, "MAX_LINEA");
	settings->tam_pagina = archivoConfigSacarIntDe(archivoConfig, "TAM_PAGINA");

	free(campos);
	archivoConfigDestruir(archivoConfig);

}
