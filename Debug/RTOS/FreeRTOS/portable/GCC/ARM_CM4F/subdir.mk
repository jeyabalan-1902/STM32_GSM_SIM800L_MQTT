################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.c 

OBJS += \
./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.o 

C_DEPS += \
./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.d 


# Each subdirectory must supply rules for building sources it contributes
RTOS/FreeRTOS/portable/GCC/ARM_CM4F/%.o RTOS/FreeRTOS/portable/GCC/ARM_CM4F/%.su RTOS/FreeRTOS/portable/GCC/ARM_CM4F/%.cyclo: ../RTOS/FreeRTOS/portable/GCC/ARM_CM4F/%.c RTOS/FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I"C:/Users/kjeya/Documents/RTOS/Workspace/SIM800MQTT/RTOS/FreeRTOS/include" -I"C:/Users/kjeya/Documents/RTOS/Workspace/SIM800MQTT/RTOS/FreeRTOS/portable/GCC/ARM_CM4F" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-RTOS-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F

clean-RTOS-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F:
	-$(RM) ./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.cyclo ./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.d ./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.o ./RTOS/FreeRTOS/portable/GCC/ARM_CM4F/port.su

.PHONY: clean-RTOS-2f-FreeRTOS-2f-portable-2f-GCC-2f-ARM_CM4F

