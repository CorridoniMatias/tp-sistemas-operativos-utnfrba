#include "bibliotecaFM9.h"

void configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"PUERTO",
			"MODO",
			"TAMANIO",
			"MAX_LINEA",
			"TAM_PAGINA"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	//strcpy(settings->puertoEscucha, archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	settings->puerto= archivoConfigSacarIntDe(archivoConfig, "PUERTO");
	strcpy(settings->modo, archivoConfigSacarStringDe(archivoConfig, "MODO"));
	settings->tamanio = archivoConfigSacarIntDe(archivoConfig, "TAMANIO");
	settings->max_linea = archivoConfigSacarIntDe(archivoConfig, "MAX_LINEA");
	settings->tam_pagina = archivoConfigSacarIntDe(archivoConfig, "TAM_PAGINA");

	//free(campos);
	archivoConfigDestruir(archivoConfig);

}
