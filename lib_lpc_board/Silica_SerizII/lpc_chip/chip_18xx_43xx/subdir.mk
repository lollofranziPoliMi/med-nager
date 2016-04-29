################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lpc_chip/chip_18xx_43xx/adc_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/atimer_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/clock_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/emc_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/enet_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/evrt_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/gpdma_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/gpio_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/i2c_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/i2s_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/lcd_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/rgu_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/ritimer_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/rtc_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/scu_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/sdmmc_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/ssp_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/uart_18xx_43xx.c \
../lpc_chip/chip_18xx_43xx/wwdt_18xx_43xx.c 

OBJS += \
./lpc_chip/chip_18xx_43xx/adc_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/atimer_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/clock_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/emc_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/enet_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/evrt_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/gpdma_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/gpio_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/i2c_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/i2s_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/lcd_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/rgu_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/ritimer_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/rtc_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/scu_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/sdmmc_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/ssp_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/uart_18xx_43xx.o \
./lpc_chip/chip_18xx_43xx/wwdt_18xx_43xx.o 

C_DEPS += \
./lpc_chip/chip_18xx_43xx/adc_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/atimer_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/clock_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/emc_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/enet_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/evrt_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/gpdma_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/gpio_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/i2c_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/i2s_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/lcd_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/rgu_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/ritimer_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/rtc_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/scu_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/sdmmc_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/ssp_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/uart_18xx_43xx.d \
./lpc_chip/chip_18xx_43xx/wwdt_18xx_43xx.d 


# Each subdirectory must supply rules for building sources it contributes
lpc_chip/chip_18xx_43xx/%.o: ../lpc_chip/chip_18xx_43xx/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCORE_M4 -D__REDLIB__ -D__CODE_RED -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_18xx_43xx" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_ip" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\board_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII\Silica_SerizII" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lib_lpc_board\..\..\..\..\..\software\CMSIS\CMSIS\Include" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


