/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "ipu_platform_uc.h"

#define IPU_INTC_ISR_ADDR                 (IPU_INTC_BASEADDR)        /* status */
#define IPU_INTC_IPR_ADDR                 (IPU_INTC_BASEADDR + 0x4)  /* pending */
#define IPU_INTC_IER_ADDR                 (IPU_INTC_BASEADDR + 0x8)  /* enable */
#define IPU_INTC_IAR_ADDR                 (IPU_INTC_BASEADDR + 0x0C) /* acknowledge */
#define IPU_INTC_MER_ADDR                 (IPU_INTC_BASEADDR + 0x1C) /* master enable */


uint32_t readReg(uint32_t addr) {
  uint32_t val;
	val=*((uint32_t*)(addr));
  printf("Reading From Address 0x%lx value=0x%lx\n",addr,val);
  return val;
}

void writeReg(uint32_t addr,uint32_t value) {
	printf("Writing to Address 0x%lx value=0x%lx\n",addr,value);
  *((uint32_t*)(addr))=value;
}

void init_interrupt(void)
{
    printf("%s\n", __func__);
    writeReg(IPU_INTC_MER_ADDR,0x1);
    writeReg(IPU_INTC_IER_ADDR,0xFFFFFFFF);

    writeReg(IPU_INTC_ISR_ADDR,0x1);

}


void init_command_queue(void)
{
    printf("Initial command queue\n");
}


void ipu_isr(void)
{
     printf("ISR ISR ISR\n");
     uint32_t intc_mask = readReg(IPU_INTC_IPR_ADDR);

     if (intc_mask & 0x1) // host interrupt
          printf("SQ door bell rings, go to answer it\n");

     if (intc_mask & 0x2)
        printf("DPU comes back\n");


     writeReg(IPU_INTC_IAR_ADDR, intc_mask);
} 

int main()
{
    init_platform();

    init_command_queue();

    microblaze_register_handler((XInterruptHandler) ipu_isr, (void *) 0);
    microblaze_enable_interrupts();

    init_interrupt();
#if 0
    //RW to SRAM
    printf("READ/WRITE TEST FOR SRAM\n");
    writeReg(IPU_SRAM_BASEADDR,0x1);
    readReg(IPU_SRAM_BASEADDR);
    //RW to DDR
    printf("READ/WRITE TEST FOR DDR\n");
    writeReg(IPU_DDR_BASEADDR,0x1);
    readReg(IPU_DDR_BASEADDR);
    //ACCESS INTC
    printf("READ/WRITE TEST FOR INTC\n");
    writeReg(IPU_INTC_BASEADDR,0x1);
    readReg(IPU_INTC_BASEADDR);
    //ACCESS C2H Mailbox
    printf("READ/WRITE TEST FOR C2HMAILBOX\n");
    writeReg(IPU_C2HMAILBOX_BASEADDR,0x1);
    readReg(IPU_C2HMAILBOX_BASEADDR);
    //ACCESS H2C MailBox
    printf("READ/WRITE TEST FOR H2CMAILBOX\n");
    writeReg(IPU_H2CMAILBOX_BASEADDR,0x1);
    readReg(IPU_H2CMAILBOX_BASEADDR);
    //ACCESS STRM2AXI0
    printf("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI0_BASEADDR,0x1);
    readReg(IPU_STRM2AXI0_BASEADDR);
    //ACCESS STRM2AXI0
    printf("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI1_BASEADDR,0x1);
    readReg(IPU_STRM2AXI1_BASEADDR);
    //ACCESS STRM2AXI0
    printf("READ/WRITE TEST FOR STRM2AXI0\n");
    writeReg(IPU_STRM2AXI2_BASEADDR,0x1);
    readReg(IPU_STRM2AXI2_BASEADDR);
#endif
    cleanup_platform();
    return 0;
}
