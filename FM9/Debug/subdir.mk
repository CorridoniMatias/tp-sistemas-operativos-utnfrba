################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPUsManager.c \
../ConsoleHandler.c \
../FM9Interface.c \
../FM9_Server.c \
../FM9lib.c \
../InvertedPageTable.c \
../Paging.c \
../Segmentation.c \
../Storage.c \
../Tests.c \
../main.c 

OBJS += \
./CPUsManager.o \
./ConsoleHandler.o \
./FM9Interface.o \
./FM9_Server.o \
./FM9lib.o \
./InvertedPageTable.o \
./Paging.o \
./Segmentation.o \
./Storage.o \
./Tests.o \
./main.o 

C_DEPS += \
./CPUsManager.d \
./ConsoleHandler.d \
./FM9Interface.d \
./FM9_Server.d \
./FM9lib.d \
./InvertedPageTable.d \
./Paging.d \
./Segmentation.d \
./Storage.d \
./Tests.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/tp-2018-2c-nuGetChar/kemmens" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


