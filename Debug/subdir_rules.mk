################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/Applications/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gcc-9.2.1" -c @"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug/syscfg/device_config.opt"  -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DWSN_USE_DISPLAY -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc" -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/source" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/source/ti/posix/gcc" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" -I"/Applications/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/arm-none-eabi/include" -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1065572015: ../rfWsnNode.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"/Users/njsnacker/ti/sysconfig_1_8_2/sysconfig_cli.sh" -s "/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/.metadata/product.json" -o "syscfg" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_easylink_config.h: build-1065572015 ../rfWsnNode.syscfg
syscfg/ti_easylink_config.c: build-1065572015
syscfg/device_config.opt: build-1065572015
syscfg/ti_devices_config.c: build-1065572015
syscfg/ti_radio_config.c: build-1065572015
syscfg/ti_radio_config.h: build-1065572015
syscfg/ti_drivers_config.c: build-1065572015
syscfg/ti_drivers_config.h: build-1065572015
syscfg/ti_utils_build_linker.cmd.genlibs: build-1065572015
syscfg/syscfg_c.rov.xs: build-1065572015
syscfg/ti_utils_runtime_model.gv: build-1065572015
syscfg/ti_utils_runtime_Makefile: build-1065572015
syscfg/: build-1065572015

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/Applications/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gcc-9.2.1" -c @"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug/syscfg/device_config.opt"  -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DWSN_USE_DISPLAY -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc" -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/source" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/source/ti/posix/gcc" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" -I"/Users/njsnacker/ti/simplelink_cc13x2_26x2_sdk_5_20_00_52/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" -I"/Applications/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-9-2019-q4-major/arm-none-eabi/include" -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -I"/Users/njsnacker/Development/ti_ccs/CC1312R1_Ping_Pong_example_gcc/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


