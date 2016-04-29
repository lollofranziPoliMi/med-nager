################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lpc_ip/adc_001.c \
../lpc_ip/atimer_001.c \
../lpc_ip/emc_001.c \
../lpc_ip/enet_001.c \
../lpc_ip/fpu_init.c \
../lpc_ip/gpdma_001.c \
../lpc_ip/gpiogrpint_001.c \
../lpc_ip/gpioint_001.c \
../lpc_ip/gpiopinint_001.c \
../lpc_ip/i2c_001.c \
../lpc_ip/i2s_001.c \
../lpc_ip/lcd_001.c \
../lpc_ip/ritimer_001.c \
../lpc_ip/rtc_001.c \
../lpc_ip/sdmmc_001.c \
../lpc_ip/ssp_001.c \
../lpc_ip/timer_001.c \
../lpc_ip/usart_001.c 

OBJS += \
./lpc_ip/adc_001.o \
./lpc_ip/atimer_001.o \
./lpc_ip/emc_001.o \
./lpc_ip/enet_001.o \
./lpc_ip/fpu_init.o \
./lpc_ip/gpdma_001.o \
./lpc_ip/gpiogrpint_001.o \
./lpc_ip/gpioint_001.o \
./lpc_ip/gpiopinint_001.o \
./lpc_ip/i2c_001.o \
./lpc_ip/i2s_001.o \
./lpc_ip/lcd_001.o \
./lpc_ip/ritimer_001.o \
./lpc_ip/rtc_001.o \
./lpc_ip/sdmmc_001.o \
./lpc_ip/ssp_001.o \
./lpc_ip/timer_001.o \
./lpc_ip/usart_001.o 

C_DEPS += \
./lpc_ip/adc_001.d \
./lpc_ip/atimer_001.d \
./lpc_ip/emc_001.d \
./lpc_ip/enet_001.d \
./lpc_ip/fpu_init.d \
./lpc_ip/gpdma_001.d \
./lpc_ip/gpiogrpint_001.d \
./lpc_ip/gpioint_001.d \
./lpc_ip/gpiopinint_001.d \
./lpc_ip/i2c_001.d \
./lpc_ip/i2s_001.d \
./lpc_ip/lcd_001.d \
./lpc_ip/ritimer_001.d \
./lpc_ip/rtc_001.d \
./lpc_ip/sdmmc_001.d \
./lpc_ip/ssp_001.d \
./lpc_ip/timer_001.d \
./lpc_ip/usart_001.d 


# Each subdirectory must supply rules for building sources it contributes
lpc_ip/%.o: ../lpc_ip/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCORE_M4 -D__REDLIB__ -D__CODE_RED -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_18xx_43xx" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_ip" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\board_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII\Silica_SerizII" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lib_lpc_board\..\..\..\..\..\software\CMSIS\CMSIS\Include" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


