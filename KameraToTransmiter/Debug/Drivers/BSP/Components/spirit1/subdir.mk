################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/spirit1/radio_target.c 

OBJS += \
./Drivers/BSP/Components/spirit1/radio_target.o 

C_DEPS += \
./Drivers/BSP/Components/spirit1/radio_target.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/spirit1/%.o Drivers/BSP/Components/spirit1/%.su Drivers/BSP/Components/spirit1/%.cyclo: ../Drivers/BSP/Components/spirit1/%.c Drivers/BSP/Components/spirit1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G071xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/emils/Desktop/Augstskola/Magistrs/Magistra darbs/STM32_kods/KameraToTransmiter/KameraToTransmiter/Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-spirit1

clean-Drivers-2f-BSP-2f-Components-2f-spirit1:
	-$(RM) ./Drivers/BSP/Components/spirit1/radio_target.cyclo ./Drivers/BSP/Components/spirit1/radio_target.d ./Drivers/BSP/Components/spirit1/radio_target.o ./Drivers/BSP/Components/spirit1/radio_target.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-spirit1
