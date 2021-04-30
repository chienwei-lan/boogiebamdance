ifeq ($(IPU_HW_EMU_HOME),)
  $(error IPU_HW_EMU_HOME is not set)
endif
CFLAGS=-Wall -O0 -g3 -c -fmessage-length=0 -MT"./obj/platform.o" -I./../platform -I./../../include -I$(IPU_HW_EMU_HOME)/sw/periphiral_read_write/ -I$(IPU_HW_EMU_HOME)/sw/periphiral_read_write/platform/include -I./ -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -I$(IPU_HW_EMU_HOME)/release/header  -I$(IPU_HW_EMU_HOME)/release/driver
LINKFLAGS=-Wl,-T -Wl,lscript.ld -L$(IPU_HW_EMU_HOME)/sw/periphiral_read_write/platform/lib -mlittle-endian -mcpu=v11.0 -mxl-soft-mul -Wl,--no-relax -Wl,--gc-sections

export HW_EM_ENABLE_SOCKET=false
export HW_EM_K2X_SOCKET=false

mkdir obj

all : compile run

compile : rwapp.c $(PWD)/../platform/platform.c
	echo 'Building file: ./src/rwapp.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/rwapp.d" -MT"./obj/rwapp.o" -o "obj/rwapp.o" "rwapp.c"
	echo 'Finished building: rwapp.c'
	echo 'Building file: $(PWD)/../platform/platform.c'
	echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc $(CFLAGS) -MF"./obj/platform.d" -MT"./obj/platform.o" -o "obj/platform.o" "$(PWD)/../platform/platform.c"
	echo 'Finished building: $(PWD)/../platform/platform.c'
	echo 'Building target: apprw.elf'
	echo 'Invoking: MicroBlaze gcc linker'
	mb-gcc $(LINKFLAGS) -o "apprw.elf"  ./obj/rwapp.o ./obj/platform.o   -Wl,--start-group,-lxil,-lgcc,-lc,--end-group
	echo 'Finished building target: apprw.elf'

run: apprw.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/apprw.elf

run_dbg: apprw.elf
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/apprw.elf -g

clean :
	rm -rf ert.elf obj/*.o obj/*.d run_hw_emu
