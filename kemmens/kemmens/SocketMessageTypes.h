#ifndef KEMMENS_SOCKETMESSAGETYPES_H_
#define KEMMENS_SOCKETMESSAGETYPES_H_

/*	Definicion de tipos de mensajes para transferencias de datos por TCPIP
 */

//Message Types

#define MESSAGETYPE_STRING 1	//Strings puras
#define MESSAGETYPE_INT	2		//INTS de longitud variable
#define MESSAGETYPE_VOIDPOINTER	3   //void*, pára los packets

//PROTOCOLO MDJ (1)
#define MESSAGETYPE_MDJ_CHECKFILE 	10	//Comando: Validar Archivo
#define MESSAGETYPE_MDJ_CREATEFILE	11	//Comando: Crear Archivo
#define MESSAGETYPE_MDJ_GETDATA		12	//Comando: Obtener bytes de archivo
#define MESSAGETYPE_MDJ_PUTDATA		13	//Comando: Grabar datos a archivo
#define MESSAGETYPE_MDJ_DELETEFILE	14	//Comando: Eliminar un archivo

//PROTOCOLO DAM (2)

//#define MESSAGETYPE_DAM_ABRIR	20 //Comando: abrir <pathFile>
//#define MESSAGETYPE_DAM_FLUSH	21 //Comando flush <pathFile>

#define MESSAGETYPE_DAM_SAFA_DUMMY	220 //Finaliza carga del script del dummy
#define MESSAGETYPE_DAM_SAFA_ABRIR	221 //Comando: abrir
#define MESSAGETYPE_DAM_SAFA_CREAR	222 //Comando: crear
#define MESSAGETYPE_DAM_SAFA_BORRAR	223 //Comando: borrar
#define MESSAGETYPE_DAM_SAFA_FLUSH	224 //Comando: flush
#define MESSAGETYPE_DAM_SAFA_ERR	225 //Error en la operacion con SAFA

///PROTOCOLO S-AFA (5XX)

//MENSAJES PARA EL CPU
#define MESSAGETYPE_SAFA_CPU_EXECUTE 521		//Ejecutar un DTB con ciertos parametros (path, quantum, PC, etc)
#define MESSAGETYPE_SAFA_CPU_WAITRESPONSE 522	//Respuesta a una operacion Wait hecha por un CPU; 1:aceptada, 0:denegada

//PROTOCOLO CPU
#define MESSAGETYPE_CPU_ASKLINE 310
#define MESSAGETYPE_CPU_ASIGNAR 311
#define MESSAGETYPE_CPU_CLOSE 312
#define MESSAGETYPE_CPU_EOFORABORT 320
#define MESSAGETYPE_CPU_BLOCKDUMMY 321
#define MESSAGETYPE_CPU_BLOCKDTB 322
#define MESSAGETYPE_CPU_EOQUANTUM 323
#define MESSAGETYPE_CPU_WAIT 324
#define MESSAGETYPE_CPU_SIGNAL 325
#define MESSAGETYPE_CPU_ABRIR 330
#define MESSAGETYPE_CPU_FLUSH 331
#define MESSAGETYPE_CPU_BORRAR 332
#define MESSAGETYPE_CPU_CREAR 333

#endif /* KEMMENS_SOCKETMESSAGETYPES_H_ */
