/******************************************************************************
*
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include "platform.h"
#include "ipu_platform_uc.h"
#include "xrt_queue.h"
#include "xil_printf.h"

#define ERT_PRINTF(fmt, arg...)   \
        printf("[ Microblaze ]" fmt "\n", ##arg)


uint32_t readReg(uint32_t addr) {
  uint32_t val;
	val=*((uint32_t*)(addr));
  ERT_PRINTF("Reading From Address 0x%lx value=0x%lx\n",addr,val);
  return val;
}

void writeReg(uint64_t addr,uint32_t value) {
  ERT_PRINTF("Writing to Address 0x%lx value=0x%lx\n",addr,value);
  *((uint32_t*)(addr))=value;
}

int main()
{
    init_platform();

    ERT_PRINTF("READ/WRITE TEST FOR SRAM\n");
    for (uint32_t offset = 0x4; offset < 0x20000000; offset<<=1) {
            writeReg((IPU_SRAM_BASEADDR+offset),0xABCD1234);
            readReg((IPU_SRAM_BASEADDR+offset));
    }
#if 0
    //RW to DDR
    ERT_PRINTF("READ/WRITE TEST FOR DDR\n");
    for (uint32_t offset = 0x4; offset < 0x200000000; offset<<=1) {
            writeReg((IPU_DDR_BASEADDR+offset),0xABCD1234);
            readReg((IPU_DDR_BASEADDR+offset));
    }
#endif
    //ACCESS INTC
    ERT_PRINTF("READ/WRITE TEST FOR INTC\n");
    writeReg(IPU_INTC_BASEADDR,0x1);
    readReg(IPU_INTC_BASEADDR);
    //ACCESS C2H Mailbox
    ERT_PRINTF("READ/WRITE TEST FOR C2HMAILBOX\n");
    writeReg(IPU_C2HMAILBOX_BASEADDR,0xAE);
    readReg(IPU_C2HMAILBOX_BASEADDR+0x8);
    //ERT_PRINTF H2C MailBox
    printf("READ/WRITE TEST FOR H2CMAILBOX\n");
    writeReg(IPU_H2CMAILBOX_BASEADDR,0xCD);
    readReg(IPU_H2CMAILBOX_BASEADDR+0x8);
    //ACCESS STRM2AXI0
    ERT_PRINTF("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI0_BASEADDR,0x1);
    readReg(IPU_STRM2AXI0_BASEADDR);
    //ACCESS STRM2AXI0
    ERT_PRINTF("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI1_BASEADDR,0x1);
    readReg(IPU_STRM2AXI1_BASEADDR);
    //ACCESS STRM2AXI0
    ERT_PRINTF("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI2_BASEADDR,0x1);
    readReg(IPU_STRM2AXI2_BASEADDR);

    cleanup_platform();
    return 0;
}
