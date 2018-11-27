#include "headerFiles/Settings.h"


void Configurar()
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

	Logger_Log(LOG_INFO, "Configuracion leida de SAFA.config");

	archivoConfigDestruir(archivoConfig);
	free(algoritmo);

}

void UpdateSettings()
{

	char* campos[] = {
			"PUERTO",
			"ALGORITMO",
			"QUANTUM",
			"MULTIPROGRAMACION",
			"RETARDO_PLANIF",
			NULL
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	char* algoritmo = archivoConfigSacarStringDe(archivoConfig, "ALGORITMO");
	settings->algoritmo = malloc(strlen(algoritmo) + 1);
	strcpy(settings->algoritmo, algoritmo);
	settings->quantum = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	settings->multiprogramacion = archivoConfigSacarIntDe(archivoConfig, "MULTIPROGRAMACION");
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO_PLANIF");

	archivoConfigDestruir(archivoConfig);
	free(algoritmo);

}

void MonitorConfigFile()
{

	int monitorFD, watcherFD;
	char buffer[BUFFER_SIZE];

	//Creo la instancia de monitor de inotify, y le agrego un watcher ante modificaciones en cualquier
	//archivo en la carpeta Config (que solo tiene el archivo SAFA.config, el cual quiero observar)
	monitorFD = inotify_init();
	watcherFD = inotify_add_watch(monitorFD, RUTA_WATCHABLE, IN_MODIFY);

	while(1)
	{
		//NOTA: Por algun motivo, la modificacion se dispara dos veces al modificar con gedit
		read(monitorFD, buffer, BUFFER_SIZE);
		Logger_Log(LOG_INFO, "Se registro un cambio en el archivo de configuracion");
		UpdateSettings();
		Logger_Log(LOG_INFO, "Se actualizaron los parametros internos de configuracion con exito");
	}

	//Dejo el lugar ordenado antes de irme...
	inotify_rm_watch(monitorFD, watcherFD);
	close(monitorFD);

}
