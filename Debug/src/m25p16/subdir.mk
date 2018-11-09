################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/m25p16/m25p16.c 

SRC_OBJS += \
./src/m25p16/m25p16.doj 

C_DEPS += \
./src/m25p16/m25p16.d 


# Each subdirectory must supply rules for building sources it contributes
src/m25p16/m25p16.doj: ../src/m25p16/m25p16.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/m25p16/m25p16.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


