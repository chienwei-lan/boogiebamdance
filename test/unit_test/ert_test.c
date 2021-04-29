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

#define IPU_INTC_ISR_ADDR                 (IPU_INTC_BASEADDR)        /* status */
#define IPU_INTC_IPR_ADDR                 (IPU_INTC_BASEADDR + 0x4)  /* pending */
#define IPU_INTC_IER_ADDR                 (IPU_INTC_BASEADDR + 0x8)  /* enable */
#define IPU_INTC_IAR_ADDR                 (IPU_INTC_BASEADDR + 0x0C) /* acknowledge */
#define IPU_INTC_MER_ADDR                 (IPU_INTC_BASEADDR + 0x1C) /* master enable */


#define ERT_PRINTF(fmt, arg...)   \
        printf("[ Microblaze ]" fmt "\n", ##arg)


uint32_t readReg(uint32_t addr) {
  uint32_t val;
	val=*((uint32_t*)(addr));
  ERT_PRINTF("Reading From Address 0x%lx value=0x%lx\n",addr,val);
  return val;
}

void writeReg(uint32_t addr,uint32_t value) {
  ERT_PRINTF("Writing to Address 0x%lx value=0x%lx\n",addr,value);
  *((uint32_t*)(addr))=value;
}

void ert_test_isr(void)
{
     ERT_PRINTF("=> %s \n", __func__);
     uint32_t intc_mask = readReg(IPU_INTC_IPR_ADDR);

     ERT_PRINTF("intc_mask 0x%lx \n", intc_mask);

     writeReg(IPU_INTC_IAR_ADDR, intc_mask);
}

void init_ert_test_interrupt(void)
{
    ERT_PRINTF("%s\n", __func__);

    microblaze_register_handler((XInterruptHandler)ert_test_isr, (void *) 0);

    microblaze_enable_interrupts();

    writeReg(IPU_INTC_MER_ADDR,0x1);
    writeReg(IPU_INTC_IER_ADDR,0xFFFFFFFF);


    writeReg(IPU_INTC_ISR_ADDR,0x1);

}

int main()
{
    init_platform();
    uint32_t val;

    ERT_PRINTF("READ/WRITE TEST FOR SRAM\n");
    for (uint32_t offset = 0x4; offset < 0x80000; offset<<=1) {
            writeReg((IPU_SRAM_BASEADDR+offset),0xABCD1234);
            val = readReg((IPU_SRAM_BASEADDR+offset));
            if (val !=0xABCD1234) {
                ERT_PRINTF("Result mismatch write 0xABCD1234, failed @ addr 0x%lx, read 0x%lx \n", offset, val);
                return 0;
            }
    }
    //RW to DDR
    ERT_PRINTF("READ/WRITE TEST FOR DDR\n");
#if 0
    for (uint32_t offset = 0x0; offset < 0x1000; offset+=4) {
            writeReg((IPU_DDR_BASEADDR+offset),0xABCD1234);
            val = readReg((IPU_DDR_BASEADDR+offset));
            if (val !=0xABCD1234) {
                ERT_PRINTF("Result mismatch write 0xABCD1234, failed @ addr 0x%lx, read 0x%lx \n", IPU_DDR_BASEADDR+offset, val);
                return 0;
            }
    }
#endif
    for (uint32_t offset = 0x4; offset < 0x20000000; offset<<=1) {
            writeReg((IPU_DDR_BASEADDR+offset),0xABCD1234);
            val = readReg((IPU_DDR_BASEADDR+offset));
            if (val !=0xABCD1234) {
                ERT_PRINTF("Result mismatch write 0xABCD1234, failed @ addr 0x%lx, read 0x%lx \n", IPU_DDR_BASEADDR+offset, val);
                return 0;
            }
    }

    init_ert_test_interrupt();
    //ACCESS INTC
 //   ERT_PRINTF("READ/WRITE TEST FOR INTC\n");
 //   writeReg(IPU_INTC_BASEADDR,0x1);
    //readReg(IPU_INTC_BASEADDR);
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


    ERT_PRINTF("ERT UNIT TEST PASSED\n");
    cleanup_platform();
    return 0;
}
