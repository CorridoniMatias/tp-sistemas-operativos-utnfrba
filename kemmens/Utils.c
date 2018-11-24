#include "kemmens/Utils.h"


//unsigned char* KemmensUtils_md5(void* content)
//{
//	void * digest = malloc(MD5_DIGEST_LENGTH);
//	MD5_CTX context;
//	MD5_Init(&context);
//	MD5_Update(&context, content, strlen(content) + 1);
//	MD5_Final(digest, &context);
//
//	return (unsigned char*)digest;
//}

void dictionary_putMAESTRO(t_dictionary* dictionary, char* key, void* value, void (*elementDestroyer)(void*))
{
	if (dictionary_has_key(dictionary, key)) {
		dictionary_remove_and_destroy(dictionary, key, elementDestroyer);
	}
	dictionary_put(dictionary, key, value);
}

void queue_to_list(t_queue* src, t_list* dest)
{

	int queueSize = queue_size(src);
	void* buffer;
	int bufferSize;
	void** auxiliarQueue = malloc(1);
	int totalSize = 0;
	int counter = 0;

	//Voy sacando de la cola, pero guardo en una estructura auxiliar (los voy a volver a poner)
	while(counter < queueSize)
	{
		buffer = queue_pop(src);
		bufferSize = sizeof(buffer);
		totalSize += bufferSize;
		//Importante reallocar la estructura auxiliar y tambien mallocear cada uno de sus elementos
		auxiliarQueue = realloc(auxiliarQueue, totalSize);
		auxiliarQueue[counter] = malloc(totalSize);
		memcpy(auxiliarQueue[counter++], buffer, bufferSize);
	}

	//Voy agregando cada elemento que lei tanto a la lista a retornar como a la cola pasada por parametro (para restaurarla)
	for(counter = 0; counter < queueSize; counter++)
	{
		list_add(dest, auxiliarQueue[counter]);
		queue_push(src, auxiliarQueue[counter]);
	}

	//Solo hago free de la cola auxiliar; no de sus campos (si no, libero porciones de memoria de elementos de la cola restaurada)
	free(auxiliarQueue);

	return;

}

void list_to_queue(t_list* src, t_queue* dest)
{

	int listSize = list_size(src);
	int pos;

	for (pos = 0; pos < listSize; pos++)
	{
		queue_push(dest, list_get(src, pos));
	}

	return;

}

void list_copy(t_list* src, t_list* dest)
{

	int listSize = list_size(src);
	int pos;

	for (pos = 0; pos < listSize; pos++)
	{
		list_add(dest, list_get(src, pos));
	}

	return;

}

void multiFree(int pointersCount, ...)
{

	if(pointersCount < 1)
	{
		return;
	}

	va_list arguments;
	va_start(arguments, pointersCount);

	void* ptr;
	int current;

	for(current = 0; current < pointersCount; current++)
	{
		ptr = va_arg(arguments, void*);
		free(ptr);
	}

	va_end(arguments);

}
