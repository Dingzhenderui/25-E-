################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Drivers/motor/%.o: ../Drivers/motor/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2020/ccs/tools/compiler/ti-cgt-armllvm_4.0.3.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/PID" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/yuntai" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/motor" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/track" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/K230" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/WIT" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/OLED_Hardware_SPI" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Debug" -I"D:/ti/mspm0_sdk_2_05_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_05_00_05/source" -I"C:/Users/ZHANG ZHIRUI/workspace_ccstheia/basemodel/Drivers/MSPM0" -DMOTION_DRIVER_TARGET_MSPM0 -DMPU6050 -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"Drivers/motor/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


