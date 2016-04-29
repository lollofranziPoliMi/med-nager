################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include lpc_ip/subdir.mk
-include lpc_chip/chip_common/subdir.mk
-include lpc_chip/chip_18xx_43xx/subdir.mk
-include lpc_board/boards_18xx_43xx/Silica_SerizII/subdir.mk
-include lpc_board/board_common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pre-build main-build

# Main-build Target
main-build: libboard_Silica_SerizII.a

# Tool invocations
libboard_Silica_SerizII.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Archiver'
	arm-none-eabi-ar -r "libboard_Silica_SerizII.a" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ARCHIVES) libboard_Silica_SerizII.a
	-@echo ' '

pre-build:
	-if test -f "libboard_Silica_SerizII.a" ; then cp -f "libboard_Silica_SerizII.a" ../"libboard_Silica_SerizII.a" ; fi
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "libboard_Silica_SerizII.a" ; cp -f "libboard_Silica_SerizII.a" ../"libboard_Silica_SerizII.a" ; # arm-none-eabi-objdump -h -S "libboard_Silica_SerizII.a" >"libboard_Silica_SerizII.lss"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build post-build

-include ../makefile.targets
