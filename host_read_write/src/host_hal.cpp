/**********
Copyright (c) 2019, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************/
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <fstream>


#include "xclhal2.h"
#include "xclbin.h"

using namespace std;
#include "ipu_platform_host.h"

uint32_t readReg(xclDeviceHandle& handle, uint32_t addr) 
{
  uint32_t value;
  xclRead(handle, XCL_ADDR_KERNEL_CTRL, addr, (void*)(&value), 4);
  printf("Reading From Address 0x%x value=0x%x\n",addr,value);
  return value;
}

void writeReg(xclDeviceHandle& handle, uint32_t addr,uint8_t value) 
{
	printf("Writing to Address 0x%x value=0x%x\n",addr,value);
    xclWrite(handle, XCL_ADDR_KERNEL_CTRL, addr, (void*)(&value), 4);
}


////////MAIN FUNCTION//////////
int main(int argc, char **argv) {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string binaryFile = argv[1];

    //! Create device handle
    xclDeviceHandle handle; 
    handle = xclOpen(0, NULL, XCL_INFO);

    //! Load xclbin
    const char* bit = binaryFile.c_str();
    ifstream stream(bit);
    stream.seekg(0, stream.end);
    int size = stream.tellg();
    stream.seekg(0, stream.beg);

    char* header = new char[size];
    stream.read(header, size);

    const xclBin* blob = (const xclBin*)header;
    if (xclLoadXclBin(handle, blob)) {
        delete[] header;
        throw runtime_error("load xclbin failed");
    }
    std::cout << "Finished loading xclbin " << bit << std::endl;

    //RW to SRAM
    printf("READ/WRITE TEST FOR SRAM\n");
    writeReg(handle, IPU_SRAM_BASEADDR,0x1);
    readReg(handle, IPU_SRAM_BASEADDR);
    //RW to DDR
    printf("READ/WRITE TEST FOR DDR\n");
    writeReg(handle, IPU_DDR_BASEADDR,0x1);
    readReg(handle, IPU_DDR_BASEADDR);
    //ACCESS C2H Mailbox
    printf("READ/WRITE TEST FOR C2HMAILBOX\n");
    writeReg(handle, IPU_C2HMAILBOX_BASEADDR,0x1);
    readReg(handle, IPU_C2HMAILBOX_BASEADDR);
    //ACCESS H2C MailBox
    printf("READ/WRITE TEST FOR H2CMAILBOX\n");
    writeReg(handle, IPU_H2CMAILBOX_BASEADDR,0x1);
    readReg(handle, IPU_H2CMAILBOX_BASEADDR);

    std::cout << "finished execution" << std::endl;

    return 0; 
}

