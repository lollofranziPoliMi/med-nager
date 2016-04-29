################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lpc_board/boards_18xx_43xx/Silica_SerizII/board_Silica_SerizII.c \
../lpc_board/boards_18xx_43xx/Silica_SerizII/sysinit_Silica_SerizII.c 

OBJS += \
./lpc_board/boards_18xx_43xx/Silica_SerizII/board_Silica_SerizII.o \
./lpc_board/boards_18xx_43xx/Silica_SerizII/sysinit_Silica_SerizII.o 

C_DEPS += \
./lpc_board/boards_18xx_43xx/Silica_SerizII/board_Silica_SerizII.d \
./lpc_board/boards_18xx_43xx/Silica_SerizII/sysinit_Silica_SerizII.d 


# Each subdirectory must supply rules for building sources it contributes
lpc_board/boards_18xx_43xx/Silica_SerizII/%.o: ../lpc_board/boards_18xx_43xx/Silica_SerizII/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCORE_M4 -D__REDLIB__ -D__CODE_RED -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_18xx_43xx" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_ip" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\board_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII\Silica_SerizII" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lib_lpc_board\..\..\..\..\..\software\CMSIS\CMSIS\Include" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


