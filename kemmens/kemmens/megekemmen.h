#ifndef MEGEKEMMEN_H_
#define MEGEKEMMEN_H_
//MegaCommons <> inclusive language

/*
 *		INCLUIR ESTE ARCHIVO PARA ACCEDER A LAS FUNCIONALIDADES BASICAS DE LAS KEMMENS
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * 		ACCION: Destruye el CommandInterpreter y el log, y finaliza el programa
 * 		PARAMETROS:
 * 			exit_code: Codigo de salida (0 en caso de exito)
 */
void exit_gracefully(int exit_code);

/**
 * 		ACCION: Ejecuta una cierta funcion, y luego destruye el CommandInterpreter y el log, y finaliza el programa
 * 		PARAMETROS:
 * 			operations(): Funcion (que no recibe parametros) a ejecutar previo a terminar
 * 			exit_code: Codigo de salida (0 en caso de exito)
 */
void exit_gracefully_custom(void (*operations)(), int exit_code);

#endif /* MEGEKEMMEN_H_ */
