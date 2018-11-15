################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPUsManager.c \
../ConsoleHandler.c \
../FM9_Server.c \
../InvertedPageTable.c \
../Paging.c \
../Storage.c \
../bibliotecaFM9.c \
../main.c 

OBJS += \
./CPUsManager.o \
./ConsoleHandler.o \
./FM9_Server.o \
./InvertedPageTable.o \
./Paging.o \
./Storage.o \
./bibliotecaFM9.o \
./main.o 

C_DEPS += \
./CPUsManager.d \
./ConsoleHandler.d \
./FM9_Server.d \
./InvertedPageTable.d \
./Paging.d \
./Storage.d \
./bibliotecaFM9.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/tp-2018-2c-nuGetChar/kemmens" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


