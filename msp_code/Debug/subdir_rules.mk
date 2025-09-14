################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/PID" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/yuntai" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/motor" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/track" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/K230" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/WIT" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/OLED_Hardware_SPI" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Debug" -I"D:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-595352687: ../wit-oled-hardware-spi.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/ti/ccs2020/ccs/utils/sysconfig_1.24.0/sysconfig_cli.bat" --script "C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/wit-oled-hardware-spi.syscfg" -o "." -s "D:/ti/mspm0_sdk_2_05_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-595352687 ../wit-oled-hardware-spi.syscfg
device.opt: build-595352687
device.cmd.genlibs: build-595352687
ti_msp_dl_config.c: build-595352687
ti_msp_dl_config.h: build-595352687
Event.dot: build-595352687

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/PID" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/yuntai" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/motor" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/track" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/K230" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/WIT" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/OLED_Hardware_SPI" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Debug" -I"D:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: D:/ti/mspm0_sdk_2_05_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/PID" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/yuntai" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/motor" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/track" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/K230" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/WIT" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/OLED_Hardware_SPI" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Debug" -I"D:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


