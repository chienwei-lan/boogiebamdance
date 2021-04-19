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

#define IPU_SQ_ADDR                       (IPU_DDR_BASEADDR)
#define IPU_CQ_ADDR                       (IPU_DDR_BASEADDR  + 0x1000)

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

static uint8_t there_is_pending_cmd = 0;


#define MB_PRINTF(fmt, arg...)   \
        printf("[ Microblaze ]" fmt "\n", ##arg)

struct ert_sq_cmd {
  union {
    struct {
      uint32_t state:4;   /* [3-0]   */
      uint32_t custom:8;  /* [11-4]  */
      uint32_t count:11;  /* [22-12] */     
      uint32_t opcode:5;  /* [27-23] */
      uint32_t type:3;    /* [30-28] */
      uint32_t tag:1      /* [31] */
    }; 
    uint32_t header;
  };

  struct {
    uint32_t cmd_identifier:16;  /* [15-0]  */
    uint32_t reserved:16;        /* [31-16] */
  }; 

}; 

struct ert_admin_sq_cmd {
  union {
    struct {
      uint32_t state:4;   /* [3-0]   */
      uint32_t custom:8;  /* [11-4]  */
      uint32_t count:11;  /* [22-12] */     
      uint32_t opcode:6;  /* [27-23] */
      uint32_t type:2;    /* [30-28] */
      uint32_t tag:1      /* [31] */
    }; 
    uint32_t header;
  };

  struct {
    uint32_t cmd_identifier:16;  /* [15-0]  */
    uint32_t reserved:16;        /* [31-16] */
  }; 

}; 

uint32_t readReg(uint32_t addr) {
  uint32_t val;
	val=*((uint32_t*)(addr));
  //MB_PRINTF("Reading From Address 0x%lx value=0x%lx\n",addr,val);
  return val;
}

void writeReg(uint32_t addr,uint32_t value) {
	//MB_PRINTF("Writing to Address 0x%lx value=0x%lx\n",addr,value);
  *((uint32_t*)(addr))=value;
}

void init_interrupt(void)
{
    MB_PRINTF("%s\n", __func__);
    writeReg(IPU_INTC_MER_ADDR,0x3);
    writeReg(IPU_INTC_IER_ADDR,0xFFFFFFFF);

    //writeReg(IPU_INTC_ISR_ADDR,0x3);

}


void init_command_queue(void)
{
    MB_PRINTF("Initial command queue\n");
}


void ipu_isr(void)
{
     MB_PRINTF("=> %s \n", __func__);
     uint32_t intc_mask = readReg(IPU_INTC_IPR_ADDR);


     MB_PRINTF("intc_mask 0x%x \n", intc_mask);

     if (intc_mask & 0x1) {// host interrupt
          MB_PRINTF("SQ door bell rings, go to answer it\n");
          there_is_pending_cmd = 1;

     }

     if (intc_mask & 0x2)
        MB_PRINTF("DPU comes back\n");


     writeReg(IPU_INTC_IAR_ADDR, intc_mask);
}


int32_t sq_tail_pointer_empty(void)
{

    while (readReg(IPU_H2C_MB_STATUS) & 0x1)
        return 1;

    return 0;
}


uint32_t fetch_cmd(void)
{
    MB_PRINTF(" => %s \n", __func__);

    return readReg(IPU_H2C_MB_RDDATA);
}


void submit_to_dpu(uint32_t slot_offset)
{
    MB_PRINTF(" => %s \n", __func__);

    writeReg(IPU_AIE_BASEADDR,  0x1);
}


void complete_cmd(uint32_t slot_offset)
{
    MB_PRINTF(" => %s \n", __func__);

    writeReg(IPU_C2H_MB_WRDATA, slot_offset);
}


void scheduler_loop(void)   
{
    MB_PRINTF("%s \n", __func__);
    while (1) {

        while (sq_tail_pointer_empty())
            continue;

         uint32_t intc_mask = readReg(IPU_INTC_IPR_ADDR);


         MB_PRINTF("intc_mask 0x%x \n", intc_mask);

        uint32_t slot_offset = fetch_cmd();
    
        submit_to_dpu(slot_offset);

        complete_cmd(slot_offset);

        break;
    }
}

void init_comm_channel(void) {
    uint32_t val;

#if 0
    writeReg(IPU_C2H_MB_IE,   0x0);
    writeReg(IPU_C2H_MB_RIT,  0x0);
    writeReg(IPU_C2H_MB_SIT,  0x0);

    writeReg(IPU_H2C_MB_IE,   0x0);
    writeReg(IPU_H2C_MB_RIT,  0x0);
    writeReg(IPU_H2C_MB_SIT,  0x0);
#endif
    writeReg(IPU_C2H_MB_RIT,  0xF);
    writeReg(IPU_C2H_MB_SIT,  0x0);

    val = readReg(IPU_C2H_MB_IS);
    writeReg(IPU_C2H_MB_IS,  val);
    writeReg(IPU_C2H_MB_IE,  0x3);

    writeReg(IPU_H2C_MB_RIT,  0xF);
    writeReg(IPU_H2C_MB_SIT,  0x0);

    val = readReg(IPU_H2C_MB_IS);
    writeReg(IPU_H2C_MB_IS,  val);
    writeReg(IPU_H2C_MB_IE,  0x3);


    writeReg(IPU_H2C_MB_CTRL, 0x3);
    writeReg(IPU_C2H_MB_CTRL, 0x3);
}

int main()
{
    init_platform();

    init_command_queue();

    microblaze_register_handler((XInterruptHandler)ipu_isr, (void *) 0);
    microblaze_enable_interrupts();

    init_interrupt();

    init_comm_channel();

    scheduler_loop();

    cleanup_platform();
    return 0;
}
