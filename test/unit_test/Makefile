ifeq ($(IPU_HW_EMU_HOME),)
  $(error IPU_HW_EMU_HOME is not set)
endif
CFLAGS=-Wall -O0 -g3 -c -fmessage-length=0 -MT"./obj/platform.o" -I./platform/include -I./src/ -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -I$(IPU_HW_EMU_HOME)/release/header 
LINKFLAGS=-Wl,-T -Wl,src/lscript.ld -L./platform/lib -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -Wl,--gc-sections

all : compile run

compile : src/ert_test.c src/platform.c
	echo 'Building file: ./src/ert_test.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/ert_test.d" -MT"./obj/ert_test.o" -o "obj/ert_test.o" "src/ert_test.c"
	echo 'Finished building: src/ert_test.c'
	echo 'Building file: ./src/platform.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/platform.d" -MT"./obj/platform.o" -o "obj/platform.o" "src/platform.c"
	echo 'Finished building: ../src/platform.c'
	echo 'Building target: ert_test.elf'
	echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc $(LINKFLAGS) -o "ert_test.elf"  ./obj/ert_test.o ./obj/platform.o   -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
	echo 'Finished building target: ert_test.elf'

run: ert_test.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/ert_test.elf

run_dbg: ert_test.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/ert_test.elf -g

clean :
	rm -rf ert_test.elf obj/*.o obj/*.d run_hw_emu
