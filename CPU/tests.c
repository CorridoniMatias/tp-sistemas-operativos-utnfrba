/*
void ShowKeyAndValue(char* key, void* value)
{
	printf("%s : ", key);
	printf("%d\n",*((int*)value));
}
bool openFileVerificator2(t_dictionary* dictionary,char* path){
	if(dictionary_get(dictionary,path) != NULL ){
		void * respuesta = dictionary_get(dictionary,path);

		printf("%d\n : ",*((int*)respuesta) );
		printf("esta funcionando todo bien papa");
		return true;
	}
	printf("salio como el orto");
	return false;
}

//Test para ver que esta serializando bien
void * test (void * str){
	uint32_t pc = ((Operation*)str)->programCounter;
	declare_and_init(newPC, uint32_t,pc); //test declare and init con struct operation
	SerializedPart fieldForSAFA2 = {.size = sizeof(((Operation*)str)->programCounter), .data = newPC};

	printf("tam : %d \n",fieldForSAFA2.size);
	printf("pc : %d \n", *(int*)fieldForSAFA2.data);

	//esto es para probar como me llegaria el extra data a las funciones
	//Prueba unitaria arriba
	openFileVerificator2(((Operation*)str)->dictionary,"script1.txt");

	return 0;
}
void construirDiccionario(){

}



int main(void)
{
	 Operation extraData;
     extraData.programCounter = 5;



	void* aplanado = malloc(100);
	int offset = 0;
	int* dir1 = (int*)malloc(sizeof(int));
	*dir1 = 12560;
	int* dir2 = (int*)malloc(sizeof(int));
	*dir2 = 1310;
	int* dir3 = (int*)malloc(sizeof(int));
	*dir3 = 8826;
	memcpy(aplanado + offset, "script1.txt:", 12);
	offset += 12;
	memcpy(aplanado + offset, dir1, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ",script2.txt:", 13);
	offset += 13;
	memcpy(aplanado + offset, dir2, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ",otroArch.bat:", 14);
	offset += 14;
	memcpy(aplanado + offset, dir3, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ";\0", 2);

	t_dictionary* d = BuildDictionary(aplanado, 3);
	extraData.dictionary = d;


	dictionary_iterator(d, ShowKeyAndValue);
	openFileVerificator2(d,"script1.txt");
    test(&extraData);



	return 0;

}
int main (void){
		uint32_t idDtb = 0;
		uint32_t flag = 0;
		uint32_t pc = 1;
		uint32_t q = 3;
		uint32_t nfiles = 3;
		char* dir = "34567";
		declare_and_init(id, uint32_t,idDtb);
		declare_and_init(newPc, uint32_t,pc);
		declare_and_init(newNumberOfFiles, uint32_t,nfiles);
		declare_and_init(newQ, uint32_t,q);
		declare_and_init(newFlag, uint32_t,flag);

		char* path = "home/utnso";
		char* archs = "archivo1.txt,archivo2.txt,archivo3.txt;"

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA5 = {.size = sizeof(int32_t), .data = newPc};
		SerializedPart fieldForSAFA4 = {.size = strlen(dir), .data = dir};
		SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newFlag};
		SerializedPart fieldForSAFA6 = {.size = sizeof(int32_t), .data = newQ};
		SerializedPart fieldForSAFA7 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
	    SerializedPart fieldForSAFA3 = {.size = strlen(path)+1 , .data = path};
	    SerializedPart fieldForSAFA8 = {.size = strlen(archs)+1 , .data = archs};

	    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(8, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5,fieldForSAFA6,fieldForSAFA7,fieldForSAFA8);
		void* msgFromSAFA = packetToSAFAToBlockGDT;
		DeserializedData* data = Serialization_Deserialize(msgFromSafa);

		printf( "este es el valor %d \n,*(int*)data->parts[1]);


		 * TEST INTEGRAL
		 *
		 * 		uint32_t idDtb = 0;
		uint32_t flag = 1;
		uint32_t pc = 1;
		uint32_t q = 3;
		uint32_t nfiles = 3;
		char* dir = "34567";
		declare_and_init(id, uint32_t,idDtb);
		declare_and_init(newPc, uint32_t,pc);
		declare_and_init(newNumberOfFiles, uint32_t,nfiles);
		declare_and_init(newQ, uint32_t,q);
		declare_and_init(newFlag, uint32_t,flag);

		char* path = "home/utnso";
		char* archs = "archivo1.txt,archivo2.txt,archivo3.txt;";

		SerializedPart fieldForSAFA1 = {.size = sizeof(int32_t), .data =id};
		SerializedPart fieldForSAFA5 = {.size = sizeof(int32_t), .data = newPc};
		SerializedPart fieldForSAFA4 = {.size = strlen(dir), .data = dir};
		SerializedPart fieldForSAFA2 = {.size = sizeof(int32_t), .data = newFlag};
		SerializedPart fieldForSAFA6 = {.size = sizeof(int32_t), .data = newQ};
		SerializedPart fieldForSAFA7 = {.size = sizeof(int32_t), .data = newNumberOfFiles};
	    SerializedPart fieldForSAFA3 = {.size = strlen(path)+1 , .data = path};
	    SerializedPart fieldForSAFA8 = {.size = strlen(archs)+1 , .data = archs};


	    SerializedPart* packetToSAFAToBlockGDT = Serialization_Serialize(8, fieldForSAFA1, fieldForSAFA2, fieldForSAFA3, fieldForSAFA4,fieldForSAFA5,fieldForSAFA6,fieldForSAFA7,fieldForSAFA8);


		DeserializedData* data = Serialization_Deserialize(packetToSAFAToBlockGDT->data);

		 *
		 *
		 */

//return 0;
//}


