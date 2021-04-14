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


#define IPU_H2C_MB_WRDATA                 (IPU_H2CMAILBOX_BASEADDR)        /* write data */
#define IPU_H2C_MB_RDDATA                 (IPU_H2CMAILBOX_BASEADDR+0x8)    /* read data */
#define IPU_H2C_MB_STATUS                 (IPU_H2CMAILBOX_BASEADDR+0x10)   /* status */
#define IPU_H2C_MB_ERROR                  (IPU_H2CMAILBOX_BASEADDR+0x14)   /* error */
#define IPU_H2C_MB_SIT                    (IPU_H2CMAILBOX_BASEADDR+0x18)   /* set interrupt threshold */
#define IPU_H2C_MB_RIT                    (IPU_H2CMAILBOX_BASEADDR+0x1C)   /* receive interrupt threshold */
#define IPU_H2C_MB_IS                     (IPU_H2CMAILBOX_BASEADDR+0x20)   /* interrupt status */
#define IPU_H2C_MB_IE                     (IPU_H2CMAILBOX_BASEADDR+0x24)   /* error */
#define IPU_H2C_MB_IP                     (IPU_H2CMAILBOX_BASEADDR+0x28)   /* interrupt pending */
#define IPU_H2C_MB_CTRL                   (IPU_H2CMAILBOX_BASEADDR+0x2C)   /* control */

#define IPU_C2H_MB_WRDATA                 (IPU_C2HMAILBOX_BASEADDR)        /* write data */
#define IPU_C2H_MB_RDDATA                 (IPU_C2HMAILBOX_BASEADDR+0x8)    /* read data */
#define IPU_C2H_MB_STATUS                 (IPU_C2HMAILBOX_BASEADDR+0x10)   /* status */
#define IPU_C2H_MB_ERROR                  (IPU_C2HMAILBOX_BASEADDR+0x14)   /* error */
#define IPU_C2H_MB_SIT                    (IPU_C2HMAILBOX_BASEADDR+0x18)   /* set interrupt threshold */
#define IPU_C2H_MB_RIT                    (IPU_C2HMAILBOX_BASEADDR+0x1C)   /* receive interrupt threshold */
#define IPU_C2H_MB_IS                     (IPU_C2HMAILBOX_BASEADDR+0x20)   /* interrupt status */
#define IPU_C2H_MB_IE                     (IPU_C2HMAILBOX_BASEADDR+0x24)   /* error */
#define IPU_C2H_MB_IP                     (IPU_C2HMAILBOX_BASEADDR+0x28)   /* interrupt pending */
#define IPU_C2H_MB_CTRL                   (IPU_C2HMAILBOX_BASEADDR+0x2C)   /* control */

#define FLAG_STI                          (1 << 0)
#define FLAG_RTI                          (1 << 1)

#define STATUS_EMPTY                      (1 << 0)
#define STATUS_FULL                       (1 << 1)
#define STATUS_STA                        (1 << 2)
#define STATUS_RTA                        (1 << 3)


uint32_t readReg(xclDeviceHandle& handle, uint32_t addr) 
{
  uint32_t value;
  xclRead(handle, XCL_ADDR_KERNEL_CTRL, addr, (void*)(&value), 4);
  printf("Reading From Address 0x%x value=0x%x\n",addr,value);
  return value;
}

void writeReg(xclDeviceHandle& handle, uint32_t addr,uint32_t value) 
{
	//printf("Writing to Address 0x%x value=0x%x\n",addr,value);
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

    constchar* header = new char[size];
    stream.read(header, size);

    const xclBin* blob = (const xclBin*)header;
    if (xclLoadXclBin(handle, blob)) {
        delete[] header;
        throw runtime_error("load xclbin failed");
    }
    std::cout << "Finished loading xclbin " << bit << std::endl;


    for (uint32_t i = 4; i < 0x100; i+=4) {
        uint32_t *val = (uint32_t *)(bit+i);
        writeReg(handle, IPU_SRAM_BASEADDR+i,*val);
    }

    //RW to SRAM
    //printf("READ/WRITE TEST FOR SRAM\n");
    writeReg(handle, IPU_SRAM_BASEADDR,0xABCDABCD);
    readReg(handle, IPU_SRAM_BASEADDR);
    writeReg(handle, IPU_H2C_MB_WRDATA,0xEF);

    readReg(handle, IPU_C2H_MB_RDDATA);


    readReg(handle, IPU_H2C_MB_STATUS);
    readReg(handle, IPU_H2C_MB_ERROR);
    readReg(handle, IPU_H2C_MB_IS);
    readReg(handle, IPU_H2C_MB_IP);
    readReg(handle, IPU_H2C_MB_CTRL);



    uint32_t cnt = 0;
    while (1) {
        if (cnt++ ==0x10000000) {
            uint32_t val = readReg(handle, IPU_DDR_BASEADDR);
            cnt = 0;
            if (val == 0x12341234) {
                std::cout << "val " << std::hex << val << std::endl;
                break;
            }
        }



    }

#if 0
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
#endif
    std::cout << "finished execution" << std::endl;

    return 0; 
}

