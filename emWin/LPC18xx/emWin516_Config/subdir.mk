################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../emWin516_Config/GUIConf.c 

OBJS += \
./emWin516_Config/GUIConf.o 

C_DEPS += \
./emWin516_Config/GUIConf.d 


# Each subdirectory must supply rules for building sources it contributes
emWin516_Config/%.o: ../emWin516_Config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -D__CODE_RED -DCORE_M3 -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_LPC6\emWin\emWin516_Config" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_LPC6\emWin\emWin516_header" -O3 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -std=gnu99 -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


