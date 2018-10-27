#ifndef KEMMENS_SOCKETMESSAGETYPES_H_
#define KEMMENS_SOCKETMESSAGETYPES_H_

/*	Definicion de tipos de mensajes para transferencias de datos por TCPIP
 */

//Message Types

#define MESSAGETYPE_STRING	1	//Strings puras
#define MESSAGETYPE_INT	2		//INTS de longitud variable

//PROTOCOLO MDJ (1)
#define MESSAGETYPE_MDJ_CHECKFILE 	10	//Comando: Validar Archivo
#define MESSAGETYPE_MDJ_CREATEFILE	11	//Comando: Crear Archivo
#define MESSAGETYPE_MDJ_GETDATA		12	//Comando: Obtener bytes de archivo
#define MESSAGETYPE_MDJ_PUTDATA		13	//Comando: Grabar datos a archivo
#define MESSAGETYPE_MDJ_DELETEFILE	14	//Comando: Eliminar un archivo


#endif /* KEMMENS_SOCKETMESSAGETYPES_H_ */
