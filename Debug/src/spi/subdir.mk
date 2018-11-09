################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/spi/spi.c 

SRC_OBJS += \
./src/spi/spi.doj 

C_DEPS += \
./src/spi/spi.d 


# Each subdirectory must supply rules for building sources it contributes
src/spi/spi.doj: ../src/spi/spi.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/spi/spi.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


