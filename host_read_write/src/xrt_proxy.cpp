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


#include <iostream>
#include <string.h>

#include "xclhal2.h"
#include "xrt_proxy.h"
#include "xrt_queue_host.h"
#include "xrt_queue.h"

static int g_dbo_hdl = 0;
static void *g_dbo_p = NULL;
static uint32_t g_dbo_paddr = 0;

static int g_ebo_hdl = 0;
static void *g_ebo_p = NULL;

#define EBO_HDL 1
#define DBO_HDL 2

extern void writeReg(xclDeviceHandle& handle, uint32_t addr, uint32_t value);
uint32_t readReg(xclDeviceHandle& handle, uint32_t addr);

static void
free_ebo()
{
	free(g_ebo_p);
	g_ebo_p = NULL;
	g_ebo_hdl = 0;
}

static void
free_dbo()
{
	free(g_dbo_p);
	g_dbo_p = NULL;
	g_dbo_hdl = 0;
}

static int
alloc_ebo(size_t size)
{
	if (size > XRT_SUB_Q1_SLOT_SIZE) {
		std::cout << "Wrong command size." << std::endl;
		return -EINVAL;
	}

	if (g_ebo_hdl) {
		std::cout << "Cmd BO is already allocated." << std::endl;
		return -EBUSY;
	}

	g_ebo_p = malloc(size);
	if (!g_ebo_p) {
		std::cout << "Fail to alloc BO." << std::endl;
		return -ENOMEM;
	}
	memset(g_ebo_p, 0, size);

	g_ebo_hdl = EBO_HDL;
	return g_ebo_hdl;
}

static int
alloc_dbo(size_t size)
{
	if (size > XRT_PROXY_MEM_SIZE) {
		std::cout << "No enough memory." << std::endl;
		return -ENOMEM;
	}

	if (g_dbo_hdl) {
		std::cout << "BO is already allocated." << std::endl;
		return -EBUSY;
	}

	g_dbo_p = malloc(size);
	if (!g_dbo_p) {
		std::cout << "Fail to alloc BO." << std::endl;
		return -ENOMEM;
	}
	memset(g_dbo_p, 0, size);

	g_dbo_hdl = DBO_HDL;
	g_dbo_paddr = XRT_PROXY_MEM_BASE;

	return g_dbo_hdl;
}

static void *
map_ebo()
{
	return g_ebo_p;
}

static void *
map_dbo()
{
	return g_dbo_p;
}

static void
read_device_data(xclDeviceHandle& handle, uint32_t *data, uint32_t addr, uint32_t size)
{
	for (int i = 0; i < size / 4; i++) {
		data[i] = readReg(handle, addr + (i<<2));
	}
}

static void
enqueue(xclDeviceHandle& handle, uint32_t *data, uint32_t addr)
{
	for (int i = 0; i < XRT_SUB_Q1_SLOT_SIZE / 4; i++) {
		writeReg(handle, addr + (i<<2), data[i]);

	}
}

static void
dequeue(xclDeviceHandle& handle, uint32_t *data, uint32_t addr)
{
	for (int i = 0; i < XRT_COM_Q1_SLOT_SIZE / 4; i++) {
		data[i] = readReg(handle, addr + (i<<2));
	}
}

static void
write_sq_doorbell(xclDeviceHandle& handle, uint16_t tail)
{
	writeReg(handle, IPU_H2CMAILBOX_BASEADDR, tail);
}

uint16_t
read_cq_doorbell(xclDeviceHandle& handle)
{
	while (1) {
		uint8_t status = readReg(handle, IPU_C2HMAILBOX_BASEADDR + 0x10);
		if (!(status & 1))
			return readReg(handle, IPU_C2HMAILBOX_BASEADDR + 0x8);
	}
}

int
xp_xclAllocBO(xclDeviceHandle handle, size_t size, int unused, unsigned flags)
{
	printf("__larry_debug: enter %s\n", __func__);

	if (flags & XCL_BO_FLAGS_EXECBUF)
		return alloc_ebo(size);

	return alloc_dbo(size);

}

void *
xp_xclMapBO(xclDeviceHandle handle, unsigned int boHandle, bool write)
{
	printf("__larry_debug: enter %s\n", __func__);

	if (boHandle == EBO_HDL)
		return map_ebo();

	if (boHandle == DBO_HDL)
		return map_dbo();

	std::cout << "BO " << boHandle << " does not exist." << std::endl;
	return NULL;
}

int
xp_xclGetBOProperties(xclDeviceHandle handle, unsigned int boHandle, xclBOProperties *properties)
{
	printf("__larry_debug: enter %s\n", __func__);

	if (boHandle != DBO_HDL || g_dbo_hdl != DBO_HDL) {
		std::cout << "BO " << boHandle << " does not exist." << std::endl;
		return -EINVAL;
	}

	properties->paddr = g_dbo_paddr;
	return 0;
}

int
xp_xclExecBuf(xclDeviceHandle handle, unsigned int cmdBO)
{
	printf("__larry_debug: enter %s\n", __func__);

	if (cmdBO != EBO_HDL) {
		std::cout << "BO " << cmdBO << " does not exist." << std::endl;
		return -EINVAL;
	}

	uint16_t tail = 0;
	enqueue(handle, (uint32_t *)g_ebo_p, XRT_QUEUE1_SUB_BASE);
	write_sq_doorbell(handle, tail);

	return 0;
}

int
xp_xclExecWait(xclDeviceHandle handle, int timeoutMilliSec)
{
	printf("__larry_debug: enter %s\n", __func__);

	uint16_t cq_tail = read_cq_doorbell(handle);

	struct xrt_com_queue_entry comq;
	dequeue(handle, (uint32_t *)&comq, 0x800);

	std::cout << "completed cid is " << comq.cid << std::endl;
	std::cout << "completed sqhead is " << comq.sqhead << std::endl;
	std::cout << "completed state is " << comq.cstate << std::endl;

	return 1;
}

int
xp_xclSyncBO(xclDeviceHandle handle, unsigned int boHandle, xclBOSyncDirection dir, size_t size, size_t offset)
{
	if (boHandle != DBO_HDL || g_dbo_hdl != DBO_HDL) {
		std::cout << "BO " << boHandle << " does not exist." << std::endl;
		return -EINVAL;
	}

	if (dir != XCL_BO_SYNC_BO_FROM_DEVICE) {
		std::cout << "Only sync from device is supported." << std::endl;
		return -ENOTSUP;
	}

	read_device_data(handle, (uint32_t *)g_dbo_p, g_dbo_paddr, size);

	return 0;
}

void
xp_xclFreeBO(xclDeviceHandle handle, unsigned int boHandle)
{
	printf("__larry_debug: enter %s\n", __func__);

	if (boHandle == g_ebo_hdl)
		free_ebo();
	else if (boHandle == g_dbo_hdl)
		free_dbo();
	else
		std::cout << "BO " << boHandle << " does not exist." << std::endl;
}
