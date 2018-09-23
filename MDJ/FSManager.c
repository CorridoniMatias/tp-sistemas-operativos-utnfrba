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
	printf("\nFilesPath: '%s'\n path = '%s'\n", config->filesPath, path);
	char* fullPath = StringUtils_Format("%s%s%s", config->filesPath, path, ".bin");
	printf("\nFULLPATH: '%s'\n", fullPath);

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

	// Nos fijamos en que bloque empezar a buscar los bytes a copiar
	int primerBloqueIndex = ceil(offset/(float)config->tamanioBloque) - 1; //Nuestros bloques empiezan en 0

	//El offset que nos vamos a correr adentro de nuestro bloque.
	int primerByte = offset % config->tamanioBloque;


	//Por el offset que definimos si por ejemplo tenemos un tamaño 4 y offset 4 caeriamos en el blqoue 1° pero tendriamos que ir al 2°
	if(primerByte == 0)
		primerBloqueIndex ++;

	int cantBloques = StringUtils_ArraySize(bloques);

	printf("\nFILE ACCESS \n\t FilesPath: '%s'\n\t path = '%s' \n\t primerBloqueIndex = %d \n\t primerByte = %d \n\t cantBloques = %d \n\t size = %d" ,
			config->filesPath, path, primerBloqueIndex, primerByte, cantBloques, size);

	char* buffer = malloc(size);
	int copiado = 0;
	int tam;
	FILE *fp;
	char* blockFilePath;

	for(int i = primerBloqueIndex; i < cantBloques;i++)
	{
		blockFilePath = StringUtils_Format("%s%s%s", config->blocksPath, bloques[i], ".bin");

		printf("\nFILE ACCESS \n\t blockFilePath = '%s' \n\t" ,
					blockFilePath);


		fp = fopen(blockFilePath, "r");

		if(primerByte != 0)
			fseek ( fp , primerByte , SEEK_SET );

		tam = config->tamanioBloque - primerByte;
		primerByte = 0; //Despues de la primera vuelta el primer byte es siempre 0 porque hay que leer el archivo desde arriba.
		copiado += tam;
		if(copiado > size)
			tam = copiado - size;

		fread(buffer, tam, 1, fp);

		fclose(fp);
		free(blockFilePath);
		if(copiado > size)
			break;
	}
	config_destroy(metadata);

	return buffer;
}










