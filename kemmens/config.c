#include "kemmens/config.h"

t_config* archivoConfigCrear(char* path, char** campos) {

	t_config* archivoConfig = config_create(path);
	if(archivoConfigInvalido(archivoConfig, campos))
	{
		puts("Archivo de configuracion invalido\n");
		exit(EXIT_FAILURE);
	}
	return archivoConfig;

}

bool archivoConfigTieneCampo(t_config* archivoConfig, char* campo) {

	return config_has_property(archivoConfig, campo);

}

char* archivoConfigSacarStringDe(t_config* archivoConfig, char* campo) {

	return config_get_string_value(archivoConfig, campo);

}


int archivoConfigSacarIntDe(t_config* archivoConfig, char* campo) {

	return config_get_int_value(archivoConfig, campo);

}

long archivoConfigSacarLongDe(t_config* archivoConfig, char* campo) {

	return config_get_long_value(archivoConfig, campo);

}

double archivoConfigSacarDoubleDe(t_config* archivoConfig, char* campo) {

	return config_get_double_value(archivoConfig, campo);

}

char** archivoConfigSacarArrayDe(t_config* archivoConfig, char* campo) {

	return config_get_array_value(archivoConfig, campo);

}

int archivoConfigCantidadCampos(t_config* archivoConfig) {

	return config_keys_amount(archivoConfig);

}

void archivoConfigDestruir(t_config* archivoConfig) {

	config_destroy(archivoConfig);

}

void archivoConfigSetearCampo(t_config* archivoConfig, char* campo, char* valor) {

	config_set_value(archivoConfig, campo, valor);

}

bool archivoConfigInvalido(t_config* archivoConfig, char** campos) {

	return (archivoConfigIncompleto(archivoConfig, campos) || archivoConfigInexistente(archivoConfig)) ;
}

bool archivoConfigInexistente(t_config* archivoConfig) {

	return archivoConfig == NULL;

}

bool archivoConfigIncompleto(t_config* archivoConfig, char** campos) {

	int indice;
	for(indice = 0; indice < archivoConfigCantidadCampos(archivoConfig); indice++)
	{
		if(archivoConfigFaltaCampo(archivoConfig, campos[indice]))
		{
			return true;
		}
	}
	return false;
}

