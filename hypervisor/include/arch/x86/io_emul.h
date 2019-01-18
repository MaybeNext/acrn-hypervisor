/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IO_EMUL_H
#define IO_EMUL_H


/* Define emulated port IO index */
#define PIC_MASTER_PIO_IDX	0U
#define PIC_SLAVE_PIO_IDX	(PIC_MASTER_PIO_IDX + 1U)
#define PIC_ELC_PIO_IDX		(PIC_SLAVE_PIO_IDX + 1U)
#define PCI_CFGADDR_PIO_IDX	(PIC_ELC_PIO_IDX + 1U)
#define PCI_CFGDATA_PIO_IDX	(PCI_CFGADDR_PIO_IDX + 1U)
#define UART_PIO_IDX		(PCI_CFGDATA_PIO_IDX + 1U)
#define PM1A_EVT_PIO_IDX	(UART_PIO_IDX + 1U)
#define PM1A_CNT_PIO_IDX	(PM1A_EVT_PIO_IDX + 1U)
#define PM1B_EVT_PIO_IDX	(PM1A_CNT_PIO_IDX + 1U)
#define PM1B_CNT_PIO_IDX	(PM1B_EVT_PIO_IDX + 1U)
#define RTC_PIO_IDX		(PM1B_CNT_PIO_IDX + 1U)
#define EMUL_PIO_IDX_MAX	(RTC_PIO_IDX + 1U)

/**
 * @brief General post-work for MMIO emulation
 *
 * @param vcpu The virtual CPU that triggers the MMIO access
 * @param io_req The I/O request holding the details of the MMIO access
 *
 * @pre io_req->type == REQ_MMIO
 *
 * @remark This function must be called when \p io_req is completed, after
 * either a previous call to emulate_io() returning 0 or the corresponding VHM
 * request transferring to the COMPLETE state.
 */
void emulate_mmio_post(const struct acrn_vcpu *vcpu, const struct io_request *io_req);

/**
 * @brief Post-work of VHM requests for MMIO emulation
 *
 * @param vcpu The virtual CPU that triggers the MMIO access
 *
 * @pre vcpu->req.type == REQ_MMIO
 *
 * @remark This function must be called after the VHM request corresponding to
 * \p vcpu being transferred to the COMPLETE state.
 */
void dm_emulate_mmio_post(struct acrn_vcpu *vcpu);

/**
 * @brief General post-work for all kinds of VHM requests for I/O emulation
 *
 * @param vcpu The virtual CPU that triggers the MMIO access
 */
void emulate_io_post(struct acrn_vcpu *vcpu);

/**
 * @brief Emulate \p io_req for \p vcpu
 *
 * Handle an I/O request by either invoking a hypervisor-internal handler or
 * deliver to VHM.
 *
 * @param vcpu The virtual CPU that triggers the MMIO access
 * @param io_req The I/O request holding the details of the MMIO access
 *
 * @retval 0       Successfully emulated by registered handlers.
 * @retval IOREQ_PENDING The I/O request is delivered to VHM.
 * @retval -EIO    The request spans multiple devices and cannot be emulated.
 * @retval -EINVAL \p io_req has an invalid type.
 * @retval <0 on other errors during emulation.
 */
int32_t emulate_io(struct acrn_vcpu *vcpu, struct io_request *io_req);

/**
 * @brief The handler of VM exits on I/O instructions
 *
 * @param vcpu The virtual CPU which triggers the VM exit on I/O instruction
 */
int32_t pio_instr_vmexit_handler(struct acrn_vcpu *vcpu);

/**
 * @brief Allow a VM to access a port I/O range
 *
 * This API enables direct access from the given \p vm to the port I/O space
 * starting from \p port_address to \p port_address + \p nbytes - 1.
 *
 * @param vm The VM whose port I/O access permissions is to be changed
 * @param port_address The start address of the port I/O range
 * @param nbytes The size of the range, in bytes
 */
void   allow_guest_pio_access(struct acrn_vm *vm, uint16_t port_address, uint32_t nbytes);

/**
 * @brief Register a port I/O handler
 *
 * @param vm      The VM to which the port I/O handlers are registered
 * @param pio_idx The emulated port io index
 * @param range   The emulated port io range
 * @param io_read_fn_ptr The handler for emulating reads from the given range
 * @param io_write_fn_ptr The handler for emulating writes to the given range
 * @pre pio_idx < EMUL_PIO_IDX_MAX
 */
void   register_pio_emulation_handler(struct acrn_vm *vm, uint32_t pio_idx,
		const struct vm_io_range *range, io_read_fn_t io_read_fn_ptr, io_write_fn_t io_write_fn_ptr);

/**
 * @brief Register a MMIO handler
 *
 * This API registers a MMIO handler to \p vm before it is launched.
 *
 * @param vm The VM to which the MMIO handler is registered
 * @param read_write The handler for emulating accesses to the given range
 * @param start The base address of the range \p read_write can emulate
 * @param end The end of the range (exclusive) \p read_write can emulate
 * @param handler_private_data Handler-specific data which will be passed to \p read_write when called
 *
 * @retval 0 Registration succeeds
 * @retval -EINVAL \p read_write is NULL, \p end is not larger than \p start or \p vm has been launched
 */
int32_t register_mmio_emulation_handler(struct acrn_vm *vm,
	hv_mem_io_handler_t read_write, uint64_t start,
	uint64_t end, void *handler_private_data);

#endif /* IO_EMUL_H */