#ifndef KEMMENS_UTILS_H_
#define KEMMENS_UTILS_H_

#include <openssl/md5.h>
<<<<<<< HEAD
#include <stdlib.h>
#include <string.h>
=======

#define declare_and_init(pointer_name, data_type, init_value) 								\
		data_type* pointer_name = (data_type*)malloc(sizeof(data_type)); 					\
		*pointer_name = init_value;															\

>>>>>>> e6427ced0d19b05d83f75e1cd052a3aa2cb6aae7

unsigned char* KemmensUtils_md5(void* content);

#endif /* KEMMENS_UTILS_H_ */
