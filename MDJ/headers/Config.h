#ifndef HEADERS_CONFIG_H_
#define HEADERS_CONFIG_H_

#define RUTA_CONFIG "mdj.config"
#define RUTA_METADATA "/Metadata/Metadata.bin"
#define RUTA_BITMAP "/Metadata/Bitmap.bin"
#define RUTA_BLOQUES "/Bloques/"
#define RUTA_ARCHIVOS "/Archivos/"
#define MAXPATHLENGTH 50				//se cuenta el /0

struct Configuracion_s
{
	int tamanioBloque;
	int cantidadBloques;
	char* puntoMontaje;
	int puertoEscucha;
	int delay;

	char* metadataFile;
	char* bitmapFile;
	char* blocksPath;
	char* filesPath;
} typedef Configuracion;

Configuracion* config;


#endif /* HEADERS_CONFIG_H_ */
