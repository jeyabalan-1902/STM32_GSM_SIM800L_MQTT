################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RTOS/FreeRTOS/croutine.c \
../RTOS/FreeRTOS/event_groups.c \
../RTOS/FreeRTOS/list.c \
../RTOS/FreeRTOS/queue.c \
../RTOS/FreeRTOS/stream_buffer.c \
../RTOS/FreeRTOS/tasks.c \
../RTOS/FreeRTOS/timers.c 

OBJS += \
./RTOS/FreeRTOS/croutine.o \
./RTOS/FreeRTOS/event_groups.o \
./RTOS/FreeRTOS/list.o \
./RTOS/FreeRTOS/queue.o \
./RTOS/FreeRTOS/stream_buffer.o \
./RTOS/FreeRTOS/tasks.o \
./RTOS/FreeRTOS/timers.o 

C_DEPS += \
./RTOS/FreeRTOS/croutine.d \
./RTOS/FreeRTOS/event_groups.d \
./RTOS/FreeRTOS/list.d \
./RTOS/FreeRTOS/queue.d \
./RTOS/FreeRTOS/stream_buffer.d \
./RTOS/FreeRTOS/tasks.d \
./RTOS/FreeRTOS/timers.d 


# Each subdirectory must supply rules for building sources it contributes
RTOS/FreeRTOS/%.o RTOS/FreeRTOS/%.su RTOS/FreeRTOS/%.cyclo: ../RTOS/FreeRTOS/%.c RTOS/FreeRTOS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F446xx -c -I../Core/Inc -I"C:/Users/kjeya/Documents/RTOS/Workspace/SIM800MQTT/RTOS/FreeRTOS/include" -I"C:/Users/kjeya/Documents/RTOS/Workspace/SIM800MQTT/RTOS/FreeRTOS/portable/GCC/ARM_CM4F" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-RTOS-2f-FreeRTOS

clean-RTOS-2f-FreeRTOS:
	-$(RM) ./RTOS/FreeRTOS/croutine.cyclo ./RTOS/FreeRTOS/croutine.d ./RTOS/FreeRTOS/croutine.o ./RTOS/FreeRTOS/croutine.su ./RTOS/FreeRTOS/event_groups.cyclo ./RTOS/FreeRTOS/event_groups.d ./RTOS/FreeRTOS/event_groups.o ./RTOS/FreeRTOS/event_groups.su ./RTOS/FreeRTOS/list.cyclo ./RTOS/FreeRTOS/list.d ./RTOS/FreeRTOS/list.o ./RTOS/FreeRTOS/list.su ./RTOS/FreeRTOS/queue.cyclo ./RTOS/FreeRTOS/queue.d ./RTOS/FreeRTOS/queue.o ./RTOS/FreeRTOS/queue.su ./RTOS/FreeRTOS/stream_buffer.cyclo ./RTOS/FreeRTOS/stream_buffer.d ./RTOS/FreeRTOS/stream_buffer.o ./RTOS/FreeRTOS/stream_buffer.su ./RTOS/FreeRTOS/tasks.cyclo ./RTOS/FreeRTOS/tasks.d ./RTOS/FreeRTOS/tasks.o ./RTOS/FreeRTOS/tasks.su ./RTOS/FreeRTOS/timers.cyclo ./RTOS/FreeRTOS/timers.d ./RTOS/FreeRTOS/timers.o ./RTOS/FreeRTOS/timers.su

.PHONY: clean-RTOS-2f-FreeRTOS

