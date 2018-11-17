#ifndef KEMMENS_UTILS_H_
#define KEMMENS_UTILS_H_

#include <openssl/md5.h>
#include <stdlib.h>
#include <string.h>

unsigned char* KemmensUtils_md5(void* content);

#endif /* KEMMENS_UTILS_H_ */
