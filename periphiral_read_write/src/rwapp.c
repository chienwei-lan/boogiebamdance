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


const uint32_t  max_slots = 16;


static uint8_t there_is_pending_cmd = 0;
static uint8_t tail_pointer_polling = 0;

static uint16_t sq_tail_pointer = 0;
static uint16_t cq_tail_pointer = 0;

static uint32_t sq_slot_size = 0x1000;
static uint32_t cq_slot_size = 32;
static uint32_t num_slots = 16;

static uint32_t sq_offset = 0;
static uint32_t cq_offset = 0;


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

struct ert_cq_cmd {
    uint32_t return_code;
    uint32_t command_specific;
    struct {
        uint32_t sq_pointer:16;      /* [15-0]  */
        uint32_t reserved:16;        /* [31-16] */
    };
    struct {
        uint32_t cmd_identifier:16;  /* [15-0]  */
        uint32_t cmd_state:16;       /* [31-16] */
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

void ipu_isr(void)
{
     MB_PRINTF("=> %s \n", __func__);
     uint32_t intc_mask = readReg(IPU_INTC_IPR_ADDR);

     MB_PRINTF("intc_mask 0x%lx \n", intc_mask);

     if (intc_mask & 0x10) {// host interrupt
          MB_PRINTF("SQ door bell rings, go to answer it\n");
          there_is_pending_cmd = 1;

     }

     if (intc_mask & 0x2)
        MB_PRINTF("DPU comes back\n");


     writeReg(IPU_INTC_IAR_ADDR, intc_mask);
}

void init_interrupt(void)
{
    MB_PRINTF("%s\n", __func__);

    microblaze_register_handler((XInterruptHandler)ipu_isr, (void *) 0);

    microblaze_enable_interrupts();

    writeReg(IPU_INTC_MER_ADDR,0x3);
    writeReg(IPU_INTC_IER_ADDR,0xFFFFFFFF);

}


void init_command_queue(void)
{
    MB_PRINTF("Initial command queue\n");

    for (uint32_t offset = 0x1000; offset < 0x80000; offset <= 1) {

           writeReg(IPU_SRAM_BASEADDR+offset,offset);
           MB_PRINTF("offset 0x%lx, value 0x%lx\n", offset, readReg(IPU_SRAM_BASEADDR+offset));
    }

    sq_offset = IPU_SRAM_BASEADDR;
    cq_offset = IPU_SRAM_BASEADDR + num_slots*sq_slot_size;

    MB_PRINTF("SQ offset 0x%lx\n", sq_offset);
    MB_PRINTF("CQ offset 0x%lx\n", cq_offset);
}


int32_t sq_tail_pointer_empty(void)
{

    while (!there_is_pending_cmd)
        return 1;

    there_is_pending_cmd = 0;

    return 0;
}


uint32_t fetch_cmd(void)
{
    MB_PRINTF(" => %s \n", __func__);

    while (readReg(IPU_H2C_MB_STATUS) & 0x1)
        continue;

    return readReg(IPU_H2C_MB_RDDATA);
}


void submit_to_dpu(uint32_t sq_slot_idx)
{
    MB_PRINTF(" => %s \n", __func__);

    uint32_t sq_slot_offset = sq_slot_idx*sq_slot_size;

    MB_PRINTF("0x%lx = 0x%lx \n", sq_slot_offset, readReg(sq_slot_offset));
    MB_PRINTF("0x%lx = 0x%lx \n", sq_slot_offset+4, readReg(sq_slot_offset+4));

    writeReg(IPU_AIE_BASEADDR,  0x1);
}

uint32_t command_id(uint32_t sq_slot_offset)
{
    return readReg(sq_slot_offset+0x4);
}

void complete_cmd(uint32_t sq_slot_idx)
{
    MB_PRINTF(" => %s \n", __func__);

    uint32_t sq_slot_offset = sq_offset + sq_slot_idx*sq_slot_size;
    uint32_t cq_slot_offset = cq_offset + cq_tail_pointer*cq_slot_size;

    uint32_t cmd_id = command_id(sq_slot_offset);

    writeReg(cq_offset+0x0, 0x0);
    writeReg(cq_offset+0x8, sq_slot_idx);
    writeReg(cq_offset+0xc, cmd_id);

    //while (readReg(IPU_C2H_MB_STATUS) & 0x1)
    //    continue;

    writeReg(IPU_C2H_MB_WRDATA, cq_tail_pointer++);

}


void scheduler_loop(void)   
{
    MB_PRINTF("%s \n", __func__);
    while (1) {

        while (sq_tail_pointer_empty())
            continue;

        uint32_t sq_slot_idx = fetch_cmd();
    
        submit_to_dpu(sq_slot_idx);

        complete_cmd(sq_slot_idx);

        break;
    }
}

void init_comm_channel(void) {
    uint32_t val;

    if (tail_pointer_polling) {
        writeReg(IPU_C2H_MB_IE,   0x0);
        writeReg(IPU_C2H_MB_RIT,  0x0);
        writeReg(IPU_C2H_MB_SIT,  0x0);

        writeReg(IPU_H2C_MB_IE,   0x0);
        writeReg(IPU_H2C_MB_RIT,  0x0);
        writeReg(IPU_H2C_MB_SIT,  0x0);
    } else {
        writeReg(IPU_C2H_MB_RIT,  0x0);
        writeReg(IPU_C2H_MB_SIT,  0x0);

        val = readReg(IPU_C2H_MB_IS);
        writeReg(IPU_C2H_MB_IS,  val);
        writeReg(IPU_C2H_MB_IE,  0x3);

        writeReg(IPU_H2C_MB_RIT,  0x0);
        writeReg(IPU_H2C_MB_SIT,  0x0);

        val = readReg(IPU_H2C_MB_IS);
        writeReg(IPU_H2C_MB_IS,  val);
        writeReg(IPU_H2C_MB_IE,  0x3);

    }

    writeReg(IPU_H2C_MB_CTRL, 0x3);
    writeReg(IPU_C2H_MB_CTRL, 0x3);
}

int main()
{
    init_platform();

    init_command_queue();

    init_interrupt();

    init_comm_channel();

    scheduler_loop();

    cleanup_platform();
    return 0;
}
