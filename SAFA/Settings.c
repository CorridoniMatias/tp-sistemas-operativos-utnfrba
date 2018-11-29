#include "headerFiles/Settings.h"

void Configurar()
{

	settings = (Configuracion*)malloc(sizeof(Configuracion));

	pthread_mutex_init(&mutexSettings, NULL);

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

	//Inicializo la variable de cambio de algoritmos en sin cambio (por ahora)
	algorithmChange = ALGORITHM_CHANGE_UNALTERED;

	Logger_Log(LOG_INFO, "Configuracion leida de SAFA.config");

	archivoConfigDestruir(archivoConfig);
	//No hace falta hacer un free de algoritmo, el archivoConfigDestruir ya se ocupa

}

void RegisterAlgorithmChange(char* prior, char* latter)
{

	if((strcmp(prior, latter)) == 0)
	{
		algorithmChange = ALGORITHM_CHANGE_UNALTERED;
	}

	else
	{

		if((strcmp(prior, "RR")) == 0)
		{
			if((strcmp(latter, "VRR")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_RR_TO_VRR;
			}
			else if((strcmp(latter, "PROPIO")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_RR_TO_OWN;
			}
		}

		else if((strcmp(prior, "VRR")) == 0)
		{
			if((strcmp(latter, "RR")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_VRR_TO_RR;
			}
			else if((strcmp(latter, "PROPIO")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_VRR_TO_OWN;
			}
		}

		else if((strcmp(prior, "PROPIO")) == 0)
		{
			if((strcmp(latter, "RR")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_OWN_TO_RR;
			}
			else if((strcmp(latter, "VRR")) == 0)
			{
				algorithmChange = ALGORITHM_CHANGE_OWN_TO_VRR;
			}
		}

	}

}

void UpdateSettings()
{

	char* campos[] = {
			"ALGORITMO",
			"QUANTUM",
			"MULTIPROGRAMACION",
			"RETARDO_PLANIF",
			NULL
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	char* antiguo = (char*) malloc(strlen(settings->algoritmo) + 1);
	strcpy(antiguo, settings->algoritmo);

	char* algoritmo = config_get_string_value(archivoConfig, "ALGORITMO");
	free(settings->algoritmo);
	settings->algoritmo = malloc(strlen(algoritmo) + 1);
	strncpy(settings->algoritmo, algoritmo, strlen(algoritmo));
	settings->algoritmo[strlen(algoritmo)] = '\0';
	settings->quantum = archivoConfigSacarIntDe(archivoConfig, "QUANTUM");
	settings->multiprogramacion = archivoConfigSacarIntDe(archivoConfig, "MULTIPROGRAMACION");
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO_PLANIF");

	//En base al anterior algoritmo y al recien seteado (pueden coincidir) actualizo la variable de cambio
	RegisterAlgorithmChange(antiguo, algoritmo);

	archivoConfigDestruir(archivoConfig);
	//Libero la memoria del antiguo, ya no me sirve
	free(antiguo);

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
		//Me aseguro de ser el unico que esta tocando o viendo estos parametros de configuracion
		pthread_mutex_lock(&mutexSettings);
		UpdateSettings();
		pthread_mutex_unlock(&mutexSettings);
		Logger_Log(LOG_INFO, "Se actualizaron los parametros internos de configuracion con exito");
	}

	//Dejo el lugar ordenado antes de irme...
	inotify_rm_watch(monitorFD, watcherFD);
	close(monitorFD);

}

void DeleteSettingsVariables()
{

	free(settings->algoritmo);
	free(settings);
	pthread_mutex_destroy(&mutexSettings);

}
