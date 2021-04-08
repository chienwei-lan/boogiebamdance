ifeq ($(IPU_HW_EMU_HOME),)
  $(error IPU_HW_EMU_HOME is not set)
endif
CFLAGS=-Wall -O0 -g3 -c -fmessage-length=0 -MT"./obj/platform.o" -I./platform/include -I./src/ -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -I$(IPU_HW_EMU_HOME)/release/header 
LINKFLAGS=-Wl,-T -Wl,src/lscript.ld -L./platform/lib -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -Wl,--gc-sections

all : compile run

compile : src/rwapp.c src/platform.c
	echo 'Building file: ./src/rwapp.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/rwapp.d" -MT"./obj/rwapp.o" -o "obj/rwapp.o" "src/rwapp.c"
	echo 'Finished building: src/rwapp.c'
	echo 'Building file: ./src/platform.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/platform.d" -MT"./obj/platform.o" -o "obj/platform.o" "src/platform.c"
	echo 'Finished building: ../src/platform.c'
	echo 'Building target: apprw.elf'
	echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc $(LINKFLAGS) -o "apprw.elf"  ./obj/rwapp.o ./obj/platform.o   -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
	echo 'Finished building target: apprw.elf'

run: apprw.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/apprw.elf

run_dbg: apprw.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/apprw.elf -g

clean :
	rm -rf apprw.elf obj/*.o obj/*.d run_hw_emu
