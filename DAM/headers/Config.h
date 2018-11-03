#ifndef HEADERS_CONFIG_H_
#define HEADERS_CONFIG_H_

#define RUTA_CONFIG "mdj.config"

struct Configuracion_s
{
	int puertoEscucha;
	char* IPSAFA;
	int puertoSAFA;
	char* IPMDJ;
	int puertoMDJ;
	char* IPFM9;
	int puertoFM9;
	int transferSize;

} typedef Configuracion;

Configuracion* config;


#endif /* HEADERS_CONFIG_H_ */
