################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DAMInterface.c \
../Tests.c \
../bibliotecaDAM.c \
../main.c 

OBJS += \
./DAMInterface.o \
./Tests.o \
./bibliotecaDAM.o \
./main.o 

C_DEPS += \
./DAMInterface.d \
./Tests.d \
./bibliotecaDAM.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/tp-2018-2c-nuGetChar/kemmens" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


