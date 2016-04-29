################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RC663-Polling/NxpRdLib_PublicRelease/comps/phpalMifare/src/Sw/phpalMifare_Sw.c 

OBJS += \
./RC663-Polling/NxpRdLib_PublicRelease/comps/phpalMifare/src/Sw/phpalMifare_Sw.o 

C_DEPS += \
./RC663-Polling/NxpRdLib_PublicRelease/comps/phpalMifare/src/Sw/phpalMifare_Sw.d 


# Each subdirectory must supply rules for building sources it contributes
RC663-Polling/NxpRdLib_PublicRelease/comps/phpalMifare/src/Sw/%.o: ../RC663-Polling/NxpRdLib_PublicRelease/comps/phpalMifare/src/Sw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -D__CODE_RED -DCORE_M4 -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\CMSIS\CMSIS\Include" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_ip" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_chip\chip_18xx_43xx" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\board_common" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\freertos\freertos\Source\include" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\freertos\freertos\Source\portable\GCC\ARM_CM4F" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\lwip\lwip\contrib\apps\tcpecho" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\lwip\lpclwip" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\lwip\lwip\src\include" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\lwip\contrib\apps\tcpecho" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\lwip\lwip\src\include\ipv4" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\RC663-Polling\phSubBal\include" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\RC663-Polling\NxpRdLib_PublicRelease\types" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\RC663-Polling\NxpRdLib_PublicRelease\intfs" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\RC663-Polling\NxpRdLib_PublicRelease\comps\phpalSli15693" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\RC663-Polling\NxpRdLib_PublicRelease\comps\phpalSli15693\src\Sw" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lib_lpc_board\lpc_board\boards_18xx_43xx\Silica_SerizII\Silica_SerizII" -I"C:\nxp\LPCOpen_platform_v0.51\applications\lpc18xx_43xx\examples\SerizII_lpc\lwip_tcpecho_freertos\emWin516_Config" -I"C:/nxp/LPCOpen_platform_v0.51/applications/lpc18xx_43xx/examples/SerizII_lpc/lwip_tcpecho_freertos\..\..\..\..\..\software\emWin\emWin516_header" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


