#include "headers/bibliotecaFM9.h"

void configurar()
{
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
			Logger_Log(LOG_DEBUG, "pOR SACAR TAMANIO PAGINA");

			settings->tam_pagina = archivoConfigSacarIntDe(archivoConfig, "TAM_PAGINA");
		}
	}
	int page;
	if(settings->tam_pagina!=0)
	{
		page = settings->tam_pagina;
	}
	else
		page = -1;
	Logger_Log(LOG_DEBUG, "FM9 -> Valor tamaño pagina %d.",page);

	archivoConfigDestruir(archivoConfig);

}
