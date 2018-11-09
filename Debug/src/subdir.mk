################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../src/sport1_isr_process_samples_.asm 

C_SRCS += \
../src/analog_audio_test.c \
../src/main.c \
../src/parallel_flash_test.c \
../src/pb_led_test.c \
../src/proc_version_test.c \
../src/protocol.c \
../src/sdram_test.c \
../src/spdif_test.c \
../src/spi_flash_test.c \
../src/sram_test.c \
../src/system_init.c \
../src/temp_sensor_test.c \
../src/uart_test.c \
../src/wav.c \
../src/wdt_test.c 

SRC_OBJS += \
./src/analog_audio_test.doj \
./src/main.doj \
./src/parallel_flash_test.doj \
./src/pb_led_test.doj \
./src/proc_version_test.doj \
./src/protocol.doj \
./src/sdram_test.doj \
./src/spdif_test.doj \
./src/spi_flash_test.doj \
./src/sport1_isr_process_samples_.doj \
./src/sram_test.doj \
./src/system_init.doj \
./src/temp_sensor_test.doj \
./src/uart_test.doj \
./src/wav.doj \
./src/wdt_test.doj 

ASM_DEPS += \
./src/sport1_isr_process_samples_.d 

C_DEPS += \
./src/analog_audio_test.d \
./src/main.d \
./src/parallel_flash_test.d \
./src/pb_led_test.d \
./src/proc_version_test.d \
./src/protocol.d \
./src/sdram_test.d \
./src/spdif_test.d \
./src/spi_flash_test.d \
./src/sram_test.d \
./src/system_init.d \
./src/temp_sensor_test.d \
./src/uart_test.d \
./src/wav.d \
./src/wdt_test.d 


# Each subdirectory must supply rules for building sources it contributes
src/analog_audio_test.doj: ../src/analog_audio_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/analog_audio_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/main.doj: ../src/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/main.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parallel_flash_test.doj: ../src/parallel_flash_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/parallel_flash_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/pb_led_test.doj: ../src/pb_led_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/pb_led_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/proc_version_test.doj: ../src/proc_version_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/proc_version_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/protocol.doj: ../src/protocol.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/protocol.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sdram_test.doj: ../src/sdram_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/sdram_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/spdif_test.doj: ../src/spdif_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/spdif_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/spi_flash_test.doj: ../src/spi_flash_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/spi_flash_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sport1_isr_process_samples_.doj: ../src/sport1_isr_process_samples_.asm
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC Assembler'
	easm21k -file-attr ProjectName="21489_post" -proc ADSP-21489 -si-revision 0.2 -g -DCORE0 @includes-2ef85ddf244fb769d05b1384a9569310.txt -swc -gnu-style-dependencies -MM -Mo "src/sport1_isr_process_samples_.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sram_test.doj: ../src/sram_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/sram_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/system_init.doj: ../src/system_init.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/system_init.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/temp_sensor_test.doj: ../src/temp_sensor_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/temp_sensor_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/uart_test.doj: ../src/uart_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/uart_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/wav.doj: ../src/wav.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/wav.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/wdt_test.doj: ../src/wdt_test.c
	@echo 'Building file: $<'
	@echo 'Invoking: CrossCore SHARC C/C++ Compiler'
	cc21k -c -file-attr ProjectName="21489_post" -proc ADSP-21489 -flags-compiler --no_wrap_diagnostics -si-revision 0.2 -g -D_21489_EZBOARD_REV_0_2_ -DCORE0 @includes-ea7ecd0a2bb20d48011d9002b877f31f.txt -structs-do-not-overlap -no-const-strings -no-multiline -warn-protos -double-size-32 -swc -gnu-style-dependencies -MD -Mo "src/wdt_test.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


