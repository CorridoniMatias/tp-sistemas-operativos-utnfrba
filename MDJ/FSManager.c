#include "headers/FSManager.h"
#include <string.h>
t_bitarray* FSBitMap = NULL;


static void FIFA_BlockPutContent(int blockNum, int offset, void* content, int len)
{
	Logger_Log(LOG_DEBUG, "FIFA -> Solicitud de escritura al bloque %d", blockNum);

	char* blockFile = StringUtils_Format("%s%d.bin", config->blocksPath, blockNum);

	FILE* fd = fopen(blockFile, "a+");

	fseek(fd, offset, SEEK_SET);

	fwrite(content, len, 1, fd);

	fclose(fd);

	free(blockFile);
}

/*
 * 	Comprueba si un bloque no esta usado y lo reserva en forma atomica con un mutex.
 */
static bool FIFA_ReserveBlockIfNotUsed(int blockNum)
{
	pthread_mutex_lock(&bitmapLock);
	bool val = bitarray_test_bit(FSBitMap, blockNum);

	if(!val)
		bitarray_set_bit(FSBitMap, blockNum);
	pthread_mutex_unlock(&bitmapLock);
	return !val;
}

void FIFA_PrintBitmap()
{
	for(int i = 0; i < config->cantidadBloques; i++)
	{
		printf("--------------------------------\n");
		printf("\t Bit : %d = %d \n", i, FIFA_IsBlockUsed(i));
		printf("--------------------------------\n");
	}
}

static int FIFA_ReserveNextFreeBlock()
{
	for(int i = 0 ; i < config->cantidadBloques;i++)
	{
		if(FIFA_ReserveBlockIfNotUsed(i))
			return i;
	}

	return -1;
}

static char* FIFA_GetFullPath(char* path)
{
	return StringUtils_Format("%s%s%s", config->filesPath, path, ".bin");
}

void FIFA_Init()
{
	mkdir(config->blocksPath, 0700);
	mkdir(config->filesPath, 0700);
	pthread_mutex_init(&bitmapLock, NULL);
}

static void FIFA_InitBitmapFile()
{
	int cantBytes = ceil(config->cantidadBloques / (float)8);

	Logger_Log(LOG_DEBUG, "FIFA -> Creando BITMAP con cantidad de bloque (bits): %d, cantidad de bytes: %d, cantidad de bits totales: %d",
							config->cantidadBloques, cantBytes, cantBytes*8);

	char* base = (char*)malloc(cantBytes);

	memset(base, 0b000000000, cantBytes);

	pthread_mutex_lock(&bitmapLock);
	FSBitMap = bitarray_create_with_mode(base, cantBytes, MSB_FIRST);
	pthread_mutex_unlock(&bitmapLock);

	Logger_Log(LOG_DEBUG, "FIFA -> BITMAP creado, cantidad total de bits en bitmap: %d", bitarray_get_max_bit(FSBitMap));
	FIFA_FlushBitmap();
}

void FIFA_ReadBitmap()
{
	FILE *fp;
	fp = fopen(config->bitmapFile, "rb");

	if(!fp)
	{
		Logger_Log(LOG_DEBUG, "FIFA -> BITMAP no encontrado, inicializando...");
		FIFA_InitBitmapFile();
		fp = fopen(config->bitmapFile, "rb");
		if(!fp)
		{
			Logger_Log(LOG_ERROR, "FIFA -> ERROR AL ABRIR BITMAP DESDE ARCHIVO!");
			return;
		}
	}

	char* buff = (char*)malloc(config->cantidadBloques);

	fread(buff, config->cantidadBloques, 1, fp);

	fclose(fp);

	FIFA_FreeBitmap();

	pthread_mutex_lock(&bitmapLock);
	FSBitMap = bitarray_create_with_mode(buff, ceil(config->cantidadBloques / (float)8), MSB_FIRST);
	pthread_mutex_unlock(&bitmapLock);
}

void FIFA_FreeBitmap()
{
	pthread_mutex_lock(&bitmapLock);
	if(FSBitMap != NULL)
	{
		if(FSBitMap->bitarray != NULL)
			free(FSBitMap->bitarray);

		bitarray_destroy(FSBitMap);
	}
	pthread_mutex_unlock(&bitmapLock);
	pthread_mutex_destroy(&bitmapLock);
}

void FIFA_SetUsedBlock(int blockNum)
{
	pthread_mutex_lock(&bitmapLock);
	bitarray_set_bit(FSBitMap, blockNum);
	pthread_mutex_unlock(&bitmapLock);
}

static void FIFA_FreeBlock(int blockNum)
{
	pthread_mutex_lock(&bitmapLock);
	bitarray_clean_bit(FSBitMap, blockNum);
	pthread_mutex_unlock(&bitmapLock);
}

bool FIFA_IsBlockUsed(int blockNum)
{
	pthread_mutex_lock(&bitmapLock);
	bool val = bitarray_test_bit(FSBitMap, blockNum);
	pthread_mutex_unlock(&bitmapLock);
	return val;
}

void FIFA_FlushBitmap()
{
	FILE *fp;
	fp = fopen(config->bitmapFile, "w+b");

	pthread_mutex_lock(&bitmapLock);
	fwrite(FSBitMap->bitarray, FSBitMap->size, 1, fp);
	pthread_mutex_unlock(&bitmapLock);
	fclose(fp);
}

void FIFA_MkDir(char* path)
{
	Logger_Log(LOG_DEBUG, "FIFA -> mkdir recursivo de %s", path);
	char** paths = string_split(path, "/");
	char* tmp = (char*)malloc(string_length(config->filesPath) + 1);
	strcpy(tmp, config->filesPath);

	int i = 0;
	while (paths[i] != NULL) {
		string_append(&tmp, "/");
		string_append(&tmp, paths[i]);

		Logger_Log(LOG_DEBUG, "FIFA -> mkdir : %s", tmp);

		mkdir(tmp, 0700);
		free(paths[i]);
		i++;
	}

	free(tmp);
	free(paths);
}

static t_config* FIFA_OpenFile(char* path)
{
	char* fullPath = FIFA_GetFullPath(path);
	t_config* metadata = config_create(fullPath);
	free(fullPath);

	if(metadata == NULL)
	{
		Logger_Log(LOG_DEBUG, "FIFA: Attempt to open '%s' failed. File may not exist.", fullPath);
		return NULL;
	}

	return metadata;
}

static int* FIFA_ReserveBlocks(int cantBloques)
{
	int* bloques = (int*)malloc(sizeof(int) * cantBloques);

	int tmp;
	for(int i = 0 ; i < cantBloques;i++)
	{
		//ya arreglado: Posible bug: si se intenta crear un archivo que necesita X bloques pero solo hay Y libres (Y < X) los Y bloques se van a reservar pero la creacion va a fallar poque no hay X.
		tmp = FIFA_ReserveNextFreeBlock();
		if(tmp == -1) // no hay mas bloques
		{
			Logger_Log(LOG_DEBUG, "FIFA -> Se intento crear un archivo de mas bloques que los dispnibles! La creacion fallo.");
			if(i > 0) //Ya reservamos al menos un bloque, debemos liberarlo/s porque no lo/s vamos a usar.
			{
				Logger_Log(LOG_DEBUG, "FIFA -> Haciendo rollback de intento de creacion... liberando bloques...");
				for(int j = 0 ; j < i ; j++)
				{
					FIFA_FreeBlock( bloques[j] );
				}
			}

			free(bloques);
			return NULL;
		}

		Logger_Log(LOG_DEBUG, "FIFA -> Bloque %d reservado.", tmp);

		bloques[i] = tmp;
	}

	return bloques;
}

static bool FIFA_FileExists(char* path)
{
	return access( path, F_OK ) != -1;
}

void FIFA_WriteFile(char* path, int offset, int size, void* data)
{

}

bool FIFA_CreateFile(char* path, int newLines)
{
	char* fullPath = FIFA_GetFullPath(path);

	Logger_Log(LOG_DEBUG, "FIFA -> Petition to create file at '%s'", fullPath);

	if( FIFA_FileExists(fullPath) ) {
		Logger_Log(LOG_DEBUG, "FIFA -> No se creara el archivo %s. El archivo ya existe.", fullPath);
	    free(fullPath);
	    return false;
	}

	int cantBloques = ceil(newLines / (float)config->tamanioBloque);

	Logger_Log(LOG_DEBUG, "FIFA -> Cantidad de bloques requererida para creacion: %d", cantBloques);

	int* bloques;

	bloques = FIFA_ReserveBlocks(cantBloques);

	if(bloques == NULL)
	{
		free(fullPath);
		return false;
	}

	char* blocksCharArray = StringUtils_ArrayFromInts(bloques, cantBloques, true, true);

	Logger_Log(LOG_DEBUG, "FIFA -> Bloques asignados a nuevo archivo: %s", blocksCharArray);

	//Creamos las rutas
	int last = StringUtils_LastIndexOf(path, '/');

	char* folders = string_substring_until(path, last + 1);

	FIFA_MkDir(folders);

	free(folders);

	void fallar(int* bloques, char* path, char* blocks, int cant)
	{
		for(int i = 0; i < cant;i++)
		{
			FIFA_FreeBlock( bloques[i] );
		}
		free(path);
		free(blocks);
		free(bloques);
	}

	FILE* file = fopen(fullPath, "w"); //Creamos el archivo para t_config ya que no puede crearlo solo.
	if(!file)
	{
		fallar(bloques, fullPath, blocksCharArray, cantBloques);
		return false;
	}
	fclose(file);

	t_config* metadata = FIFA_OpenFile(path);

	if(metadata == NULL)
	{
		Logger_Log(LOG_ERROR, "FIFA -> Error al abrir archivo de metadata para archivo %s", fullPath);
		fallar(bloques, fullPath, blocksCharArray, cantBloques);
		return false;
	}

	char* size = string_itoa(newLines);

	config_set_value(metadata, "TAMANIO", size);

	free(size);

	config_set_value(metadata, "BLOQUES", blocksCharArray);
	free(blocksCharArray);

	config_save(metadata);
	config_destroy(metadata);

	int cant = 0;
	char* content;
	for(int i = 0; i < cantBloques; i++)
	{
		if(config->tamanioBloque < newLines)
			cant = config->tamanioBloque;
		else
			cant = newLines;

		content = string_repeat('\n', cant);

		newLines -= cant;

		FIFA_BlockPutContent(bloques[i], 0, content, cant);

		free(content);
	}

	free(bloques);
	free(fullPath);

	return true;
}

bool FIFA_IsFileValid(char* path)
{
	t_config* metadata = FIFA_OpenFile(path);

	if(metadata == NULL)
		return false;

	config_destroy(metadata);
	return true;
}

char* FIFA_ReadFile(char* path, int offset, int size, int* amountCopied)
{
	t_config* metadata = FIFA_OpenFile(path);

	if(metadata == NULL)
		return NULL;

	/*
	 * +------------+-----------+
	 * |	0123	|	4567	|
	 * +------------+-----------+
	 */

	char** bloques = config_get_array_value(metadata, "BLOQUES");

	// Nos fijamos en que bloque empezar a buscar los bytes a copiar
	int primerBloqueIndex = ceil(offset/(float)config->tamanioBloque) - 1; //Nuestros bloques empiezan en 0

	//El offset que nos vamos a correr adentro de nuestro bloque.
	int primerByte = offset % config->tamanioBloque;


	//Por el offset que definimos si por ejemplo tenemos un tamaño 4 y offset 4 caeriamos en el blqoue 1° pero tendriamos que ir al 2°
	if(primerByte == 0)
		primerBloqueIndex ++;

	int cantBloques = StringUtils_ArraySize(bloques);

	Logger_Log(LOG_DEBUG, "\nFILE SYSTEM ACCESS \n\t FilesPath: '%s'\n\t path = '%s' \n\t primerBloqueIndex = %d \n\t primerByte = %d \n\t cantBloques = %d \n\t size = %d" ,
			config->filesPath, path, primerBloqueIndex, primerByte, cantBloques, size);

	char* buffer = malloc(1);
	int copiado = 0;
	int realCopiado = 0;
	int tam;
	FILE *fp;
	char* blockFilePath;

	for(int i = primerBloqueIndex; i < cantBloques;i++)
	{
		blockFilePath = StringUtils_Format("%s%s%s", config->blocksPath, bloques[i], ".bin");

		Logger_Log(LOG_DEBUG, "\nBLOCK ACCESS \n\t blockFilePath = '%s' \n\t" ,
					blockFilePath);


		fp = fopen(blockFilePath, "rb");

		if(primerByte != 0)
			fseek ( fp , primerByte , SEEK_SET );

		tam = config->tamanioBloque - primerByte;
		primerByte = 0; //Despues de la primera vuelta el primer byte es siempre 0 porque hay que leer el archivo desde el inicio.
		copiado += tam;
		if(copiado > size)
			tam = size - realCopiado;

		buffer = realloc(buffer, realCopiado + tam);
		fread(buffer + realCopiado, tam, 1, fp);

		realCopiado += tam;

		fclose(fp);
		free(blockFilePath);
		if(copiado >= size)
			break;
	}
	config_destroy(metadata);
	StringUtils_FreeArray(bloques);

	if(amountCopied != NULL)
		*amountCopied = realCopiado;

	return buffer;
}











