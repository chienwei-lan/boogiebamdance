/******************************************************************************
*
* Copyright (C) 2009 - 2021 Xilinx, Inc.  All rights reserved.
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
#include "xrt_queue.h"

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


const uint32_t max_slots = 16;

static uint8_t tail_pointer_polling = 0;
static uint8_t there_is_pending_cmd = 0;

static uint16_t cq_tail_pointer = 0;

static uint32_t sq_slot_size = XRT_SUB_Q1_SLOT_SIZE;
static uint32_t cq_slot_size = XRT_COM_Q1_SLOT_SIZE;

static uint16_t sq_num_slots = XRT_QUEUE1_SLOT_NUM;
static uint16_t cq_num_slots = XRT_QUEUE1_SLOT_NUM;

static uint32_t sq_slot_mask = 3;
static uint32_t cq_slot_mask = 3;

static uint32_t sq_offset = 0;
static uint32_t cq_offset = 0;


#define MB_PRINTF(fmt, arg...)   \
        printf("[ Microblaze ]" fmt "\n", ##arg)


inline static uint32_t sq_cmd_id_addr(uint32_t slot_addr)
{
    return slot_addr + sizeof(uint32_t);
}

inline static uint32_t sq_cu_mask_addr(uint32_t slot_addr)
{
    return sq_cmd_id_addr(slot_addr) + sizeof(uint32_t);
}

inline static uint32_t sq_bo_addr_addr(uint32_t slot_addr)
{
    return sq_cu_mask_addr(slot_addr) + sizeof(uint32_t);
}

inline static uint32_t cq_return_code_addr(uint32_t slot_addr) // 0x0
{
    return slot_addr;
}

inline static uint32_t cq_cmd_specific_addr(uint32_t slot_addr) // 0x4
{
    return cq_return_code_addr(slot_addr) + sizeof(uint32_t);
}

inline static uint32_t cq_sq_pointer_addr(uint32_t slot_addr)  // 0x8
{
    return cq_cmd_specific_addr(slot_addr) + sizeof(uint32_t);
}

inline static uint32_t cq_cmd_id_addr(uint32_t slot_addr) // 0xC
{
    return cq_sq_pointer_addr(slot_addr) + sizeof(uint32_t);
}

inline static uint32_t cq_cmd_state_addr(uint32_t slot_addr) // 0xE
{
    return cq_cmd_id_addr(slot_addr) + sizeof(uint16_t);
}

inline static uint32_t sq_slot_addr(uint32_t sq_slot_idx)
{
    return sq_offset + sq_slot_idx*sq_slot_size;
}

inline static uint32_t cq_slot_addr(uint32_t cq_tail_pointer)
{
    return cq_offset + cq_tail_pointer*cq_slot_size;
}


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
#if 0
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
#endif

void init_command_queue(void)
{
    MB_PRINTF("Initial command queue\n");

    cq_tail_pointer = 0;

    sq_slot_size = XRT_SUB_Q1_SLOT_SIZE;
    cq_slot_size = XRT_COM_Q1_SLOT_SIZE;

    sq_num_slots = XRT_QUEUE1_SLOT_NUM;
    cq_num_slots = XRT_QUEUE1_SLOT_NUM;

    sq_slot_mask = sq_num_slots-1;
    cq_slot_mask = cq_num_slots-1;

    sq_offset = IPU_SRAM_BASEADDR;
    cq_offset = IPU_SRAM_BASEADDR + sq_num_slots*sq_slot_size;

    MB_PRINTF("sq_slot_size 0x%lx\n", sq_slot_size);
    MB_PRINTF("cq_slot_size 0x%lx\n", cq_slot_size);
    MB_PRINTF("sq_num_slots %d\n", sq_num_slots);
    MB_PRINTF("cq_num_slots %d\n", cq_num_slots);
    MB_PRINTF("sq_slot_mask 0x%lx\n", sq_slot_mask);
    MB_PRINTF("cq_slot_mask 0x%lx\n", cq_slot_mask);

    MB_PRINTF("SQ offset 0x%lx\n", sq_offset);
    MB_PRINTF("CQ offset 0x%lx\n", cq_offset);
}

inline static uint16_t sq_dequeue(void)
{
    MB_PRINTF(" => %s \n", __func__);

    while (readReg(IPU_H2C_MB_STATUS) & 0x1)
        continue;

    return readReg(IPU_H2C_MB_RDDATA) & sq_slot_mask;
}

inline static void submit_to_dpu(uint16_t sq_slot_idx)
{
    MB_PRINTF(" => %s sq_slot_idx %d\n", __func__, sq_slot_idx);

    uint32_t sq_addr = sq_slot_addr(sq_slot_idx);

    //MB_PRINTF("0x%lx = 0x%lx \n", sq_addr, readReg(sq_addr));
    //MB_PRINTF("0x%lx = 0x%lx \n", sq_cmd_id_addr(sq_addr), readReg(sq_cmd_id_addr(sq_addr)));
    //MB_PRINTF("0x%lx = 0x%lx \n", sq_bo_addr_addr(sq_addr), readReg(sq_bo_addr_addr(sq_addr)));
    writeReg(readReg(sq_bo_addr_addr(sq_addr)), 0x6C6C6548);
    writeReg(readReg(sq_bo_addr_addr(sq_addr))+4, 0x6F57206F);
    writeReg(readReg(sq_bo_addr_addr(sq_addr))+8, 0x00646C72);

    //writeReg(IPU_AIE_BASEADDR,  0x1);
}

inline static uint16_t sq_cmd_id(uint32_t sq_slot_offset)
{
    return readReg(sq_slot_offset+0x4) & 0xFFFF;
}

inline static void cq_enqueue(uint16_t sq_slot_idx)
{
    MB_PRINTF(" => %s \n", __func__);
    uint32_t cq_tail = cq_tail_pointer++;

    cq_tail &= cq_slot_mask;

    uint32_t sq_addr = sq_slot_addr(sq_slot_idx);
    uint32_t cq_addr = cq_slot_addr((cq_tail));

    uint16_t cmd_id = sq_cmd_id(sq_addr);

    MB_PRINTF("cq_addr 0x%lx \n", cq_addr);
#if 0
    not sure why hang here
    writeReg(cq_addr, 0x0);
    writeReg(cq_sq_pointer_addr(cq_addr), sq_slot_idx);
    writeReg(cq_cmd_id_addr(cq_addr), cmd_id);
#endif
    writeReg(IPU_C2H_MB_WRDATA, cq_tail);
}


void cu_task(void)   
{
    MB_PRINTF("=> %s \n", __func__);

    while (1) {

        uint16_t sq_slot_idx = sq_dequeue();

        submit_to_dpu(sq_slot_idx);

        cq_enqueue(sq_slot_idx);

    }
    MB_PRINTF("<= %s \n", __func__);

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
        MB_PRINTF("=> %s \n", __func__);

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

    init_comm_channel();

    cu_task();

    cleanup_platform();
    return 0;
}
