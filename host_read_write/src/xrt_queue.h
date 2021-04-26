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

#ifndef __XRT_QUEUE_H__
#define __XRT_QUEUE_H__

#define XRT_SUB_Q1_SLOT_SIZE	512
#define	XRT_COM_Q1_SLOT_SIZE	16

#define XRT_QUEUE1_SLOT_NUM	4

#define XRT_Q1_SUB_SIZE		(XRT_SUB_Q1_SLOT_SIZE * XRT_QUEUE1_SLOT_NUM)
#define	XRT_Q1_COM_SIZE		(XRT_COM_Q1_SLOT_SIZE * XRT_QUEUE1_SLOT_NUM)

enum xrt_cmd_opcode {
	XRT_CMD_OP_START_CU = 0x100,
};

struct xrt_sub_queue_entry {
	union {
		struct {
			uint32_t state:4;   /* [3-0]   */
			uint32_t custom:8;  /* [11-4]  */
			uint32_t count:11;  /* [22-12] */
			uint32_t opcode:9;  /* [31-23] */
			uint16_t rsvd1;
			uint16_t cid;
		};
		uint32_t header[2];
	};
	uint32_t data[1];
};

struct xrt_com_queue_entry {
	uint32_t rcode;		/* return code */
	uint32_t result;	/* command specific result */
	uint16_t sqhead;	/* submission queue header */
	uint16_t rsvd;
	uint16_t cid;		/* command id */
	uint16_t cstate;	/* command state */
};

struct xrt_cmd_start_cu {
	union {
		struct {
			uint32_t state:4;   /* [3-0]   */
			uint32_t custom:8;  /* [11-4]  */
			uint32_t count:11;  /* [22-12] */
			uint32_t opcode:9;  /* [31-23] */
			uint16_t cid;
			uint16_t rsvd1;
		};
		uint32_t header[2];
	};
	uint32_t cu_mask;	/* mandatory cu mask */
	uint32_t data[1];
};

struct xrt_cmd_queue {
	uint32_t	sq_head;
	uint32_t	sq_tail;
	uint64_t	sq_base;

	uint32_t	cq_head;
	uint32_t	cq_tail;
	uint64_t	cq_base;
};

#endif
