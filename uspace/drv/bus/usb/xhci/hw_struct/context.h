/*
 * Copyright (c) 2017 Ondrej Hlavaty <aearsis@eideo.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup drvusbxhci
 * @{
 */
/** @file
 * Context data structures of the xHC.
 *
 * Most of them are to be initialized to zero and passed ownership to the HC,
 * so they are mostly read-only.
 *
 * Feel free to write a setter when in need.
 */

#ifndef XHCI_CONTEXT_H
#define XHCI_CONTEXT_H

#include <stdint.h>
#include "common.h"

/**
 * Endpoint context: section 6.2.3
 */
typedef struct xhci_endpoint_ctx {
	xhci_dword_t data[2];
	xhci_qword_t data2;
	xhci_dword_t data3;
	xhci_dword_t reserved[3];

#define XHCI_EP_COUNT 31

#define XHCI_EP_TYPE_ISOCH_OUT		1
#define XHCI_EP_TYPE_BULK_OUT		2
#define XHCI_EP_TYPE_INTERRUPT_OUT	3
#define XHCI_EP_TYPE_CONTROL		4
#define XHCI_EP_TYPE_ISOCH_IN		5
#define XHCI_EP_TYPE_BULK_IN		6
#define XHCI_EP_TYPE_INTERRUPT_IN	7

#define XHCI_EP_TYPE_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[1], val, 5, 3)
#define XHCI_EP_MAX_PACKET_SIZE_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[1], val, 31, 16)
#define XHCI_EP_MAX_BURST_SIZE_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[1], val, 15, 8)
#define XHCI_EP_TR_DPTR_SET(ctx, val) \
	xhci_qword_set_bits(&(ctx).data2, (val >> 4), 63, 4)
#define XHCI_EP_DCS_SET(ctx, val) \
	xhci_qword_set_bits(&(ctx).data2, val, 0, 0)
#define XHCI_EP_MAX_ESIT_PAYLOAD_LO_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data3, val, 31, 16)
#define XHCI_EP_INTERVAL_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], val, 23, 16)
#define XHCI_EP_MAX_P_STREAMS_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], val, 14, 10)
#define XHCI_EP_MULT_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], val, 9, 8)
#define XHCI_EP_ERROR_COUNT_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[1], val, 2, 1)

#define XHCI_EP_STATE(ctx)              XHCI_DWORD_EXTRACT((ctx).data[0],  2,  0)
#define XHCI_EP_MULT(ctx)               XHCI_DWORD_EXTRACT((ctx).data[0],  9,  8)
#define XHCI_EP_MAX_P_STREAMS(ctx)      XHCI_DWORD_EXTRACT((ctx).data[0], 14, 10)
#define XHCI_EP_LSA(ctx)                XHCI_DWORD_EXTRACT((ctx).data[0], 15, 15)
#define XHCI_EP_INTERVAL(ctx)           XHCI_DWORD_EXTRACT((ctx).data[0], 23, 16)

#define XHCI_EP_ERROR_COUNT(ctx)        XHCI_DWORD_EXTRACT((ctx).data[1],  2,  1)
#define XHCI_EP_TYPE(ctx)               XHCI_DWORD_EXTRACT((ctx).data[1],  5,  3)
#define XHCI_EP_HID(ctx)                XHCI_DWORD_EXTRACT((ctx).data[1],  7,  7)
#define XHCI_EP_MAX_BURST_SIZE(ctx)     XHCI_DWORD_EXTRACT((ctx).data[1], 15,  8)
#define XHCI_EP_MAX_PACKET_SIZE(ctx)    XHCI_DWORD_EXTRACT((ctx).data[1], 31, 16)

#define XHCI_EP_DCS(ctx)                XHCI_QWORD_EXTRACT((ctx).data2,  0,  0)
#define XHCI_EP_TR_DPTR(ctx)            XHCI_QWORD_EXTRACT((ctx).data2, 63,  4)

#define XHCI_EP_MAX_ESIT_PAYLOAD_LO(ctx) XHCI_DWORD_EXTRACT((ctx).data3, 31,  16)

} __attribute__((packed)) xhci_ep_ctx_t;

/**
 * Slot context: section 6.2.2
 */
typedef struct xhci_slot_ctx {
	xhci_dword_t data [4];
	xhci_dword_t reserved [4];

#define XHCI_SLOT_ROUTE_STRING_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], (val & 0xFFFFF), 19, 0)
#define XHCI_SLOT_SPEED_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], (val & 0xF), 23, 20)
#define XHCI_SLOT_MTT_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], !!val, 25, 25)
#define XHCI_SLOT_CTX_ENTRIES_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[0], val, 31, 27)

#define XHCI_SLOT_ROOT_HUB_PORT_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[1], val, 23, 16)

#define XHCI_SLOT_TT_HUB_SLOT_ID_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[2], (val & 0xFF), 7, 0)
#define XHCI_SLOT_TT_HUB_PORT_SET(ctx, val) \
	xhci_dword_set_bits(&(ctx).data[2], (val & 0xFF), 15, 8)

#define XHCI_SLOT_ROUTE_STRING(ctx)     XHCI_DWORD_EXTRACT((ctx).data[0], 19,  0)
#define XHCI_SLOT_SPEED(ctx)            XHCI_DWORD_EXTRACT((ctx).data[0], 23, 20)
#define XHCI_SLOT_MTT(ctx)              XHCI_DWORD_EXTRACT((ctx).data[0], 25, 25)
#define XHCI_SLOT_CTX_ENTRIES(ctx)      XHCI_DWORD_EXTRACT((ctx).data[0], 31, 27)

#define XHCI_SLOT_MAX_EXIT_LATENCY(ctx) XHCI_DWORD_EXTRACT((ctx).data[1], 15,  0)
#define XHCI_SLOT_ROOT_HUB_PORT(ctx)    XHCI_DWORD_EXTRACT((ctx).data[1], 23, 16)
#define XHCI_SLOT_NUM_OF_PORTS(ctx)     XHCI_DWORD_EXTRACT((ctx).data[1], 31, 24)

#define XHCI_SLOT_TT_HUB_SLOT_ID(ctx)   XHCI_DWORD_EXTRACT((ctx).data[2],  7,  0)
#define XHCI_SLOT_TT_PORT_NUM(ctx)      XHCI_DWORD_EXTRACT((ctx).data[2], 15,  8)
#define XHCI_SLOT_TT_THINK_TIME(ctx)    XHCI_DWORD_EXTRACT((ctx).data[2], 17, 16)
#define XHCI_SLOT_INTERRUPTER(ctx)      XHCI_DWORD_EXTRACT((ctx).data[2], 31, 22)

#define XHCI_SLOT_DEVICE_ADDRESS(ctx)   XHCI_DWORD_EXTRACT((ctx).data[3],  7,  0)
#define XHCI_SLOT_SLOT_STATE(ctx)       XHCI_DWORD_EXTRACT((ctx).data[3], 31, 27)

} __attribute__((packed)) xhci_slot_ctx_t;

/**
 * Device context: section 6.2.1
 */
typedef struct xhci_device_ctx {
	xhci_slot_ctx_t slot_ctx;
	xhci_ep_ctx_t endpoint_ctx[XHCI_EP_COUNT];
} __attribute__((packed)) xhci_device_ctx_t;

/**
 * Stream context: section 6.2.4 {
 */
typedef struct xhci_stream_ctx {
	uint64_t data [2];
#define XHCI_STREAM_DCS(ctx)       XHCI_QWORD_EXTRACT((ctx).data[0],  0, 0)
#define XHCI_STREAM_SCT(ctx)       XHCI_QWORD_EXTRACT((ctx).data[0],  3, 1)
#define XHCI_STREAM_DEQ_PTR(ctx)   XHCI_QWORD_EXTRACT((ctx).data[0], 63, 4)
#define XHCI_STREAM_EDTLA(ctx)     XHCI_QWORD_EXTRACT((ctx).data[1], 24, 0)
} __attribute__((packed)) xhci_stream_ctx_t;

/**
 * Input control context: section 6.2.5.1
 * Note: According to section 6.2.5.1 figure 78,
 *       the context size register value in hccparams1
 *       dictates whether input control context shall have
 *       32 or 64 bytes, but in any case only dwords 0, 1 and 7
 *       are used, the rest are reserved.
 */
typedef struct xhci_input_ctrl_ctx {
	uint32_t data [8];
#define XHCI_INPUT_CTRL_CTX_DROP(ctx, idx) \
	XHCI_DWORD_EXTRACT((ctx).data[0], (idx), (idx))

#define XHCI_INPUT_CTRL_CTX_DROP_SET(ctx, idx) (ctx).data[0] |= (1 << (idx))
#define XHCI_INPUT_CTRL_CTX_DROP_CLEAR(ctx, idx) (ctx).data[0] &= ~(1 << (idx))

#define XHCI_INPUT_CTRL_CTX_ADD(ctx, idx) \
	XHCI_DWORD_EXTRACT((ctx).data[1], (idx), (idx))

#define XHCI_INPUT_CTRL_CTX_ADD_SET(ctx, idx) (ctx).data[1] |= (1 << (idx))
#define XHCI_INPUT_CTRL_CTX_ADD_CLEAR(ctx, idx) (ctx).data[1] &= ~(1 << (idx))

#define XHCI_INPUT_CTRL_CTX_CONFIG_VALUE(ctx)   XHCI_DWORD_EXTRACT((ctx).data[7],  7,  0)
#define XHCI_INPUT_CTRL_CTX_IFACE_NUMBER(ctx)   XHCI_DWORD_EXTRACT((ctx).data[7], 15,  8)
#define XHCI_INPUT_CTRL_CTX_ALTER_SETTING(ctx)  XHCI_DWORD_EXTRACT((ctx).data[7], 23, 16)
} __attribute__((packed)) xhci_input_ctrl_ctx_t;

/**
 * Input context: section 6.2.5
 */
typedef struct xhci_input_ctx {
	xhci_input_ctrl_ctx_t ctrl_ctx;
	xhci_slot_ctx_t slot_ctx;
	xhci_ep_ctx_t endpoint_ctx[XHCI_EP_COUNT];
} __attribute__((packed)) xhci_input_ctx_t;

/**
 * Port bandwidth context: section 6.2.6
 * The number of ports depends on the amount of ports available to the hub.
 */
typedef struct xhci_port_bandwidth_ctx {
	uint8_t reserved;
	uint8_t ports [];
} __attribute__((packed)) xhci_port_bandwidth_ctx_t;

#endif
