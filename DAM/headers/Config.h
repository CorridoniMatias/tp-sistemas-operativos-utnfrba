#ifndef HEADERS_CONFIG_H_
#define HEADERS_CONFIG_H_

/*struct Configuracion_s
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


struct Modulos_s
{
	int socketMDJ;

} typedef Modulos;

Modulos* modulos;*/


//-------------CONSTANTES PREDEFINIDAS-------------//

#define RUTA_CONFIG "DAM.config"				//Ruta de acceso al archivo de configuracion
#define TAMMAXPUERTO 6							//Tamanio maximo de un nombre de puerto (contando \0)
#define TAMMAXIP 20								//Tamanio maximo de un nombre de ip (contando \0)

/*
 * Estructura para almacenar los datos de configuracion
 * CAMPOS:
 * 		puertoEscucha: Puerto a traves del cual se escucharan las conexiones entrantes de CPUs
 * 		ipSAFA:	IP del proceso S-AFA, al cual me conectare como cliente
 * 		puertoSAFA: Puerto de la IP del S-AFA al cual me conectare
 * 		ipMDJ: IP del proceso MDJ, al cual me conectare como cliente
 * 		puertoMDJ: Puerto de la IP del MDJ al cual me conectare
 * 		ipFM9: IP del proceso FM9, al cual me conectare como cliente
 * 		puertoFM9: Puerto de la IP del FM9 al cual me conectare
 * 		transferSize: Tamanio maximo de transferencia al interactuar con el FM9 y el MDJ
 */

struct Configuracion_s
{
	int puertoEscucha;
	char ipSAFA[TAMMAXIP];
	char puertoSAFA[TAMMAXPUERTO];
	char ipMDJ[TAMMAXIP];
	char puertoMDJ[TAMMAXPUERTO];
	char ipFM9[TAMMAXIP];
	char puertoFM9[TAMMAXPUERTO];
	int transferSize;
} typedef Configuracion;


Configuracion* settings;		//Variable que representa los datos de configuracion de la ejecucion actual


#endif /* HEADERS_CONFIG_H_ */
