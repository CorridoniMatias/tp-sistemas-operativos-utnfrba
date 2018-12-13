#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>

int main()
{

	char* ipSAFA 	= readline("Ingrese IP SAFA (enter para localhost): ");
	char* ipFM9		= readline("Ingrese IP FM9 (enter para localhost): ");
	char* ipMDJ		= readline("Ingrese IP MDJ (enter para localhost): ");
	char* ipDAM		= readline("Ingrese IP DAM (enter para localhost): ");



	free(ipSAFA);
	free(ipDAM);
	free(ipMDJ);
	free(ipFM9);

	return 0;
}


