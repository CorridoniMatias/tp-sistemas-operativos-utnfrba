#ifndef KEMMENS_SOCKETMESSAGETYPES_H_
#define KEMMENS_SOCKETMESSAGETYPES_H_

/*	Definicion de tipos de mensajes para transferencias de datos por TCPIP
 */

//Message Types

#define MESSAGETYPE_STRING 1	//Strings puras

//PROTOCOLO MDJ
#define MESSAGETYPE_MDJ_CHECKFILE 	2	//Comando: Validar Archivo
#define MESSAGETYPE_MDJ_CREATEFILE	3	//Comando: Crear Archivo
#define MESSAGETYPE_MDJ_GETDATA		4	//Comando: Obtener bytes de archivo
#define MESSAGETYPE_MDJ_PUTDATA		5	//Comando: Grabar datos a archivo


#endif /* KEMMENS_SOCKETMESSAGETYPES_H_ */
