#ifndef HEADERS_DAMHANDLER_H_
#define HEADERS_DAMHANDLER_H_

#include <unistd.h>
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketCommons.h"
#include "kemmens/Serialization.h"
#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/SocketClient.h"

//lAddress: direccion logica del script

/*
 * Recibo ID del DTB y el path del archivo del CPU.
 * Me comunico con FM9 para obtener direccion logica.
 * Le aviso al SAFA que termino el script y le paso:
 * IdDTB, path, lAddress
 */
void DummyHandler(uint32_t idDTB, char* path);

/*
 * CPU lee el archivo, cuando termina la operacion le avisa al DAM
 * Este handler le avisa al SAFA (idDTB, path, lAddress) que finalizo
 */
void AbrirHandler(uint32_t idDTB, char* path);

/*
 * DAM le avisa al SAFA que se finalizo de crear archivo
 * El DAM solo le envia el Id del DTB
 */
void CrearHandler(uint32_t idDTB, char* path);

/*
 * DAM le avisa a SAFA que finalizo de borrar archivo.
 * Se le pasan el IdDTB, path, lAddress
 */
void BorrarHandler(uint32_t idDTB, char* path);

/*
 * Para el comando flush el DAM le avisa al SAFA que solo
 * solo necesita saber si termino la operacion (Id del DTB)
 */
void FlushHandler(uint32_t idDTB);

#endif /* HEADERS_DAMHANDLER_H_ */
