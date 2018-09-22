#include "headers/FSManager.h"
#include <string.h>
t_bitarray* FSBitMap = NULL;

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

void FSManager_WriteFile(char* path, int offset, int size, void* data)
{

}

char* FIFA_ReadFile(char* path, int offset, int size)
{

	char* fullPath = StringUtils_Format("%s%s", config->filesPath, path);

	t_config* metadata = config_create(fullPath);

	if(metadata == NULL)
	{
		free(fullPath);
		return NULL;
	}

	/*
	 * +------------+-----------+
	 * |	0123	|	4567	|
	 * +------------+-----------+
	 */

	char** bloques = config_get_array_value(metadata, "BLOQUES");

	int primerBloqueIndex = ceil(offset/(float)config->tamanioBloque);

	if(primerBloqueIndex % config->tamanioBloque == 0) //Por el offset que definimos si por ejemplo tenemos un tamaño 4 y offset 4 caeriamos en el blqoue 1 pero tendriamos que ir al 2
		primerBloqueIndex ++;

	int cantBloques = StringUtils_ArraySize(bloques);

	char* buffer = malloc(size);

	for(int i = primerBloqueIndex; i < cantBloques;i++)
	{

	}

	config_destroy(metadata);
}










