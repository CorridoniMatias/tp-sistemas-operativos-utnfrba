#include "headers/FSManager.h"
#include <string.h>
t_bitarray* FSBitMap = NULL;

/*
 * 	Comprueba si un bloque no esta usado y lo reserva en forma atomica con un mutex.
 */
static bool FIFA_ReserveBlockIfNotUsed(int blockNum)
{
	pthread_mutex_lock(&bitmapLock);
	bool val = bitarray_test_bit(FSBitMap, blockNum);

	//printf("Max bit: %d - Block %d is %d\n", bitarray_get_max_bit(FSBitMap), blockNum, val);

	if(!val)
		bitarray_set_bit(FSBitMap, blockNum);
	pthread_mutex_unlock(&bitmapLock);
	FIFA_PrintBitmap();
	return !val;
}

void FIFA_PrintBitmap()
{
	for(int i = 0; i < FSBitMap->size; i++)
	{
		printf("--------------------------------\n");
		printf("\t Bit : %d = %d \n", i, FIFA_IsBlockUsed(i));
		printf("--------------------------------\n");
	}
}

static int FIFA_ReserveNextFreeBlock()
{
	for(int i = 0 ; i < FSBitMap->size;i++)
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
	char* base = (char*)malloc(config->cantidadBloques);

	memset(base, 0b1, config->cantidadBloques);

	printf("---%s---", base);

	pthread_mutex_lock(&bitmapLock);
	FSBitMap = bitarray_create(base, config->cantidadBloques);
	pthread_mutex_unlock(&bitmapLock);
	FIFA_FlushBitmap();

	free(base);
}

void FIFA_ReadBitmap()
{
	FILE *fp;
	fp = fopen(config->bitmapFile, "r");

	if(!fp)
	{
		printf("!!!!!!!!INIT!!!!!!!!");
		FIFA_InitBitmapFile();
		fp = fopen(config->bitmapFile, "r");
		if(!fp)
		{
			Logger_Log(LOG_ERROR, "FIFA -> ERROR AL ABRIR BITMAP DESDE ARCHIVO!");
			return;
		}
	}

	char* buff = (char*)malloc(config->cantidadBloques);

	//fgets(buff, config->cantidadBloques + 1, (FILE*)fp);

	fread(buff, config->cantidadBloques, 1, fp);

	fclose(fp);

	FIFA_FreeBitmap();

	printf("BITMAP BUFFER: '%s'\n", buff);

	pthread_mutex_lock(&bitmapLock);
	FSBitMap = bitarray_create(buff, config->cantidadBloques);
	pthread_mutex_unlock(&bitmapLock);
}

void FIFA_FreeBitmap()
{
	pthread_mutex_lock(&bitmapLock);
	if(FSBitMap != NULL)
	{
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
	fp = fopen(config->bitmapFile, "w+");

	pthread_mutex_lock(&bitmapLock);
	//fputs(FSBitMap->bitarray, fp);
	fwrite(FSBitMap->bitarray, config->cantidadBloques, 1, fp);
	pthread_mutex_unlock(&bitmapLock);
	fclose(fp);
}

void FIFA_WriteFile(char* path, int offset, int size, void* data)
{

}

bool FIFA_CreateFile(char* path, int newLines)
{
	char* fullPath = FIFA_GetFullPath(path);
	printf("Path: %s\n", fullPath);
	int cantBloques = ceil(newLines / (float)config->tamanioBloque);

	int bloques[cantBloques];
	char* blocksCharArray = (char*)malloc(1);
	strcpy(blocksCharArray, "[");

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
					printf("LIberando: %d/%d\n", j,i);
					//FIFA_FreeBlock( bloques[i] );
				}
			}

			free(fullPath);
			free(blocksCharArray);
			return false;
		}

		bloques[i] = tmp;
		char* temp = string_itoa(tmp);
		string_append(&blocksCharArray, temp);
		free(temp);

		if(i < cantBloques - 1)
			string_append(&blocksCharArray, ",");
	}

	string_append(&blocksCharArray, "]");

	printf("Bloques: %s\n", blocksCharArray);

	//TODO: Hacer que el array de ints se pase a array de chars y guardarlo en el archivo.bin
	//TODO: Crear los archivos de los bloques y llenarlos con \n

	free(fullPath);

	return true;
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


		fp = fopen(blockFilePath, "r");

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











