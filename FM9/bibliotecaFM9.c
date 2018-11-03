#include "headers/bibliotecaFM9.h"

void configurar()
{
	Logger_Log(LOG_INFO, "FM9 -> Configurando.");

	settings = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"PUERTO",
			"MODO",
			"TAMANIO",
			"MAX_LINEA",
			NULL
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	settings->puerto= archivoConfigSacarIntDe(archivoConfig, "PUERTO");
	strcpy(settings->modo, archivoConfigSacarStringDe(archivoConfig, "MODO"));
	settings->tamanio = archivoConfigSacarIntDe(archivoConfig, "TAMANIO");
	settings->max_linea = archivoConfigSacarIntDe(archivoConfig, "MAX_LINEA");
	if(!string_equals_ignore_case(settings->modo,"SEG")){
		if(!archivoConfigTieneCampo(archivoConfig,"TAM_PAGINA"))
		{
			archivoConfigEsInvalido();
		}
		else
		{

			settings->tam_pagina = archivoConfigSacarIntDe(archivoConfig, "TAM_PAGINA");
		}
	}
	archivoConfigDestruir(archivoConfig);

	Logger_Log(LOG_INFO, "FM9 -> Configurado correctamente.");

}
