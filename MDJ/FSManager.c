#include "headers/FSManager.h"
#include <string.h>
t_bitarray* FSBitMap = NULL;

static int FIFA_GetNextFreeBlock()
{
	for(int i = 0 ; i < FSBitMap->size;i++)
	{
		if(!FIFA_IsBlockUsed(i))
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
}

void FIFA_ReadBitmap()
{
	FILE *fp;
	char* buff = (char*)malloc(config->cantidadBloques + 1);

	fp = fopen(config->bitmapFile, "r");

	fgets(buff, config->cantidadBloques + 1, (FILE*)fp);

	fclose(fp);

	FIFA_FreeBitmap();

	FSBitMap = bitarray_create(buff, config->cantidadBloques);
}

void FIFA_FreeBitmap()
{
	if(FSBitMap != NULL)
	{
		free(FSBitMap->bitarray);
		bitarray_destroy(FSBitMap);
	}
}

void FIFA_SetUsedBlock(int blockNum)
{
	bitarray_set_bit(FSBitMap, blockNum);
}

bool FIFA_IsBlockUsed(int blockNum)
{
	return bitarray_test_bit(FSBitMap, blockNum);
}

void FIFA_FlushBitmap()
{
	FILE *fp;
	fp = fopen(config->bitmapFile, "w+");

	fputs(FSBitMap->bitarray, fp);
	fclose(fp);
}

void FIFA_WriteFile(char* path, int offset, int size, void* data)
{

}

bool FIFA_CreateFile(char* path, int newLines)
{
	char* fullPath = FIFA_GetFullPath(path);

	int cantBloques = ceil(newLines / (float)config->tamanioBloque);

	int bloques[cantBloques];

	int tmp;
	for(int i = 0 ; i < cantBloques;i++)
	{
		//TODO: Hacer un mutex para el acceso a bitmap.
		//TODO: Posible bug: si se intenta crear un archivo que necesita X bloques pero solo hay Y libres (Y < X) los Y bloques se van a reservar pero la creacion va a fallar poque no hay X.
		tmp = FIFA_GetNextFreeBlock();
		if(tmp == -1) // no hay mas bloques
		{
			free(fullPath);
			return false;
		}

		FIFA_SetUsedBlock(tmp);
		bloques[i] = tmp;
	}

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











