#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "headers/MDJlib.h"
#include "headers/FSManager.h"
#include "commons/string.h"

#include "kemmens/StringUtils.h"

void TestBitMapCreateAssignAndFree()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();
	FIFA_Init();
	FIFA_ReadBitmap();

	//FIFA_PrintBitmap();

	FIFA_CreateFile("/testfile", 1);

	FIFA_FlushBitmap();

	FIFA_FreeBitmap();

	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFAReadFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();

	int tam = 15;
	int cop = 0;

	char* cont = FIFA_ReadFile("/scripts/2.script", 0, 5, &cop);

	cont = realloc(cont, cop + 1);

	cont[cop] = '\0';

	printf("\nCONTENIDO: '%s' tamanio = %d\n", cont, cop);

	free(cont);

	exit_gracefully_custom((void*)freeGlobals, 0);
}

void TestFIFAWriteFile()
{
	Logger_CreateLog("./DMJ.log", "DMJ", true);
	initGlobals();
	FIFA_Init();
	FIFA_ReadBitmap();

	FIFA_WriteFile("/testfile", 3, 14, "holacomoestaAS");

	int cop = 0;
	char* cont = FIFA_ReadFile("/testfile", 0, 500, &cop);

	cont = realloc(cont, cop + 1);

	cont[cop] = '\0';

	printf("\nCONTENIDO: '%s' tamanio = %d\n", cont, cop);

	free(cont);

	FIFA_FreeBitmap();
	exit_gracefully_custom((void*)freeGlobals, 0);
}

int main(int argc, char **argv)
{

	//TestFIFAReadFile();
	//TestBitMapCreateAssignAndFree();
	TestFIFAWriteFile();

	Logger_CreateLog("./DMJ.log", "DMJ", true);
	char* path = "/path/to/file.txt";
	int last = StringUtils_LastIndexOf(path, '/');

	printf("Last = %d\n", last);

	char* tmp = string_substring_until(path, last);

	printf("Path = %s\n", tmp + 1);

	return 0;

	FIFA_Init();

	FIFA_ReadBitmap();

	bool b1 = FIFA_IsBlockUsed(0);
	bool b2 = FIFA_IsBlockUsed(1);
	bool b3 = FIFA_IsBlockUsed(2);
	bool b4 = FIFA_IsBlockUsed(3);

	printf("b1: %d \t b2: %d \t b3: %d \t b4:%d\n", b1, b2, b3, b4);

	FIFA_SetUsedBlock(0);
	FIFA_SetUsedBlock(3);

	b1 = FIFA_IsBlockUsed(0);
	b2 = FIFA_IsBlockUsed(1);
	b3 = FIFA_IsBlockUsed(2);
	b4 = FIFA_IsBlockUsed(3);

	 printf("b1: %d \t b2: %d \t b3: %d \t b4:%d\n", b1, b2, b3, b4);



	FIFA_FlushBitmap();

	FIFA_FreeBitmap();

	//startServer();
	//ThreadPool_FreeGracefully(threadPool);
	exit_gracefully_custom((void*)freeGlobals, 0);
}

