#include "headers/FM9lib.h"

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
	if (string_equals_ignore_case("SEG", archivoConfigSacarStringDe(archivoConfig, "MODO")))
		settings->modo = SEG;
	else if (string_equals_ignore_case("TPI", archivoConfigSacarStringDe(archivoConfig, "MODO")))
		settings->modo = TPI;
	else if (string_equals_ignore_case("SPA", archivoConfigSacarStringDe(archivoConfig, "MODO")))
		settings->modo = SPA;
	else
		archivoConfigEsInvalido();
	settings->tamanio = archivoConfigSacarIntDe(archivoConfig, "TAMANIO");
	settings->max_linea = archivoConfigSacarIntDe(archivoConfig, "MAX_LINEA");
	if(settings->modo!=SEG){
		if(!archivoConfigTieneCampo(archivoConfig,"TAM_PAGINA"))
		{
			archivoConfigEsInvalido();
		}
		else
		{

			settings->tam_pagina = archivoConfigSacarIntDe(archivoConfig, "TAM_PAGINA");
		}
	}
	memoryFunctions= malloc(sizeof(t_memoryFunctions));
	switch(settings->modo){
	case SEG:
		memoryFunctions->createStructures = createSegmentationStructures;
		memoryFunctions->freeStructures = freeSegmentationStructures;
		memoryFunctions->virtualAddressTranslation =segmentationAddressTranslation;
		memoryFunctions->writeData = writeData_SEG;
		memoryFunctions->closeFile = closeSegmentation;
		memoryFunctions->dump = dumpSegmentation;
	}
	archivoConfigDestruir(archivoConfig);

	Logger_Log(LOG_INFO, "FM9 -> Configurado correctamente.");

}
