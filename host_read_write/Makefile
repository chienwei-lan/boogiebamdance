ifeq ($(IPU_HW_EMU_HOME),)
  $(error IPU_HW_EMU_HOME is not set)
endif

TARGET := hw_emu

#Include Libraries
include opencl/opencl.mk
include xcl2/xcl2.mk
CXXFLAGS += $(xcl2_CXXFLAGS) -I$(XILINX_XRT)/include -I$(IPU_HW_EMU_HOME)/release/header
LDFLAGS += $(xcl2_LDFLAGS)
HOST_SRCS += $(xcl2_SRCS)
CXXFLAGS += $(opencl_CXXFLAGS) -Wall -O0 -g -std=c++11
LDFLAGS += $(opencl_LDFLAGS)

HOST_SRCS += src/host_hal.cpp
HOST_SRCS += src/xrt_proxy.cpp

# Host compiler global settings
CXXFLAGS += -fmessage-length=0
LDFLAGS += -lrt -lstdc++ -L$(XILINX_XRT)/lib -lxrt_hwemu  

all: host run 

# Building Host
host: $(HOST_SRCS) $(HOST_HDRS)
	$(CXX) $(CXXFLAGS) $(HOST_SRCS) $(HOST_HDRS) -o '$@' $(LDFLAGS)

run: host
	$(IPU_HW_EMU_HOME)/release/run.sh -a $(PWD)/dummy.elf &
	XCL_EMULATION_MODE=hw_emu ./host dummy.xclbin

# Cleaning stuff
clean:
	rm host

