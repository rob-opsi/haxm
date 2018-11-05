/*
 * Copyright (c) 2009 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the copyright holder nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "include/vmx.h"
#include "include/dump_vmcs.h"
#include "include/compiler.h"
#include "../include/hax.h"

static uint32_t dump_vmcs_list[] = {
    VMX_PIN_CONTROLS,
    VMX_PRIMARY_PROCESSOR_CONTROLS,
    VMX_SECONDARY_PROCESSOR_CONTROLS,
    VMX_EXCEPTION_BITMAP,
    VMX_PAGE_FAULT_ERROR_CODE_MASK,
    VMX_PAGE_FAULT_ERROR_CODE_MATCH,
    VMX_EXIT_CONTROLS,
    VMX_EXIT_MSR_STORE_COUNT,
    VMX_EXIT_MSR_LOAD_COUNT,
    VMX_ENTRY_CONTROLS,
    VMX_ENTRY_MSR_LOAD_COUNT,
    VMX_ENTRY_INTERRUPT_INFO,
    VMX_ENTRY_EXCEPTION_ERROR_CODE,
    VMX_ENTRY_INSTRUCTION_LENGTH,
    VMX_TPR_THRESHOLD,

    VMX_CR0_MASK,
    VMX_CR4_MASK,
    VMX_CR0_READ_SHADOW,
    VMX_CR4_READ_SHADOW,
    VMX_CR3_TARGET_COUNT,
    VMX_CR3_TARGET_VAL_BASE, // x6008-x6206

    VMX_VPID,
    VMX_IO_BITMAP_A,
    VMX_IO_BITMAP_B,
    VMX_MSR_BITMAP,
    VMX_EXIT_MSR_STORE_ADDRESS,
    VMX_EXIT_MSR_LOAD_ADDRESS,
    VMX_ENTRY_MSR_LOAD_ADDRESS,
    VMX_TSC_OFFSET,
    VMX_VAPIC_PAGE,
    VMX_APIC_ACCESS_PAGE,
    VMX_EPTP,
    VMX_PREEMPTION_TIMER,

    VMX_INSTRUCTION_ERROR_CODE,

    VM_EXIT_INFO_REASON,
    VM_EXIT_INFO_INTERRUPT_INFO,
    VM_EXIT_INFO_EXCEPTION_ERROR_CODE,
    VM_EXIT_INFO_IDT_VECTORING,
    VM_EXIT_INFO_IDT_VECTORING_ERROR_CODE,
    VM_EXIT_INFO_INSTRUCTION_LENGTH,
    VM_EXIT_INFO_INSTRUCTION_INFO,
    VM_EXIT_INFO_QUALIFICATION,
    VM_EXIT_INFO_IO_ECX,
    VM_EXIT_INFO_IO_ESI,
    VM_EXIT_INFO_IO_EDI,
    VM_EXIT_INFO_IO_EIP,
    VM_EXIT_INFO_GUEST_LINEAR_ADDRESS,
    VM_EXIT_INFO_GUEST_PHYSICAL_ADDRESS,

    HOST_RIP,
    HOST_RSP,
    HOST_CR0,
    HOST_CR3,
    HOST_CR4,

    HOST_CS_SELECTOR,
    HOST_DS_SELECTOR,
    HOST_ES_SELECTOR,
    HOST_FS_SELECTOR,
    HOST_GS_SELECTOR,
    HOST_SS_SELECTOR,
    HOST_TR_SELECTOR,
    HOST_FS_BASE,
    HOST_GS_BASE,
    HOST_TR_BASE,
    HOST_GDTR_BASE,
    HOST_IDTR_BASE,

    HOST_SYSENTER_CS,
    HOST_SYSENTER_ESP,
    HOST_SYSENTER_EIP,

    GUEST_RIP,
    GUEST_RFLAGS,
    GUEST_RSP,
    GUEST_CR0,
    GUEST_CR3,
    GUEST_CR4,

    GUEST_ES_SELECTOR,
    GUEST_CS_SELECTOR,
    GUEST_SS_SELECTOR,
    GUEST_DS_SELECTOR,
    GUEST_FS_SELECTOR,
    GUEST_GS_SELECTOR,
    GUEST_LDTR_SELECTOR,
    GUEST_TR_SELECTOR,

    GUEST_ES_AR,
    GUEST_CS_AR,
    GUEST_SS_AR,
    GUEST_DS_AR,
    GUEST_FS_AR,
    GUEST_GS_AR,
    GUEST_LDTR_AR,
    GUEST_TR_AR,

    GUEST_ES_BASE,
    GUEST_CS_BASE,
    GUEST_SS_BASE,
    GUEST_DS_BASE,
    GUEST_FS_BASE,
    GUEST_GS_BASE,
    GUEST_LDTR_BASE,
    GUEST_TR_BASE,
    GUEST_GDTR_BASE,
    GUEST_IDTR_BASE,

    GUEST_ES_LIMIT,
    GUEST_CS_LIMIT,
    GUEST_SS_LIMIT,
    GUEST_DS_LIMIT,
    GUEST_FS_LIMIT,
    GUEST_GS_LIMIT,
    GUEST_LDTR_LIMIT,
    GUEST_TR_LIMIT,
    GUEST_GDTR_LIMIT,
    GUEST_IDTR_LIMIT,

    GUEST_VMCS_LINK_PTR,
    GUEST_DEBUGCTL,
    GUEST_PAT,
    GUEST_EFER,
    GUEST_PERF_GLOBAL_CTRL,
    GUEST_PDPTE0,
    GUEST_PDPTE1,
    GUEST_PDPTE2,
    GUEST_PDPTE3,

    GUEST_DR7,
    GUEST_PENDING_DBE,
    GUEST_SYSENTER_CS,
    GUEST_SYSENTER_ESP,
    GUEST_SYSENTER_EIP,
    GUEST_SMBASE,
    GUEST_INTERRUPTIBILITY,
    GUEST_ACTIVITY_STATE,
};

static const char *get_vmcs_component_name(int num)
{
    // Intel SDM Vol. 3C: Table 24-17. Structure of VMCS Component Encoding
#define HASH(x) \
    (((x) & 0x003E) >> 1) /* Index */ | \
    (((x) & 0x0C00) >> 4) /* Type  */ | \
    (((x) & 0x6000) >> 5) /* Width */
#define CASE(x) \
    case HASH(x): \
        return #x

    switch (HASH(num)) {
    CASE(VMX_PIN_CONTROLS);
    CASE(VMX_PRIMARY_PROCESSOR_CONTROLS);
    CASE(VMX_SECONDARY_PROCESSOR_CONTROLS);
    CASE(VMX_EXCEPTION_BITMAP);
    CASE(VMX_PAGE_FAULT_ERROR_CODE_MASK);
    CASE(VMX_PAGE_FAULT_ERROR_CODE_MATCH);
    CASE(VMX_EXIT_CONTROLS);
    CASE(VMX_EXIT_MSR_STORE_COUNT);
    CASE(VMX_EXIT_MSR_LOAD_COUNT);
    CASE(VMX_ENTRY_CONTROLS);
    CASE(VMX_ENTRY_MSR_LOAD_COUNT);
    CASE(VMX_ENTRY_INTERRUPT_INFO);
    CASE(VMX_ENTRY_EXCEPTION_ERROR_CODE);
    CASE(VMX_ENTRY_INSTRUCTION_LENGTH);
    CASE(VMX_TPR_THRESHOLD);
    CASE(VMX_CR0_MASK);
    CASE(VMX_CR4_MASK);
    CASE(VMX_CR0_READ_SHADOW);
    CASE(VMX_CR4_READ_SHADOW);
    CASE(VMX_CR3_TARGET_COUNT);
    CASE(VMX_CR3_TARGET_VAL_BASE);
    CASE(VMX_VPID);
    CASE(VMX_IO_BITMAP_A);
    CASE(VMX_IO_BITMAP_B);
    CASE(VMX_MSR_BITMAP);
    CASE(VMX_EXIT_MSR_STORE_ADDRESS);
    CASE(VMX_EXIT_MSR_LOAD_ADDRESS);
    CASE(VMX_ENTRY_MSR_LOAD_ADDRESS);
    CASE(VMX_TSC_OFFSET);
    CASE(VMX_VAPIC_PAGE);
    CASE(VMX_APIC_ACCESS_PAGE);
    CASE(VMX_EPTP);
    CASE(VMX_PREEMPTION_TIMER);
    CASE(VMX_INSTRUCTION_ERROR_CODE);
    CASE(VM_EXIT_INFO_REASON);
    CASE(VM_EXIT_INFO_INTERRUPT_INFO);
    CASE(VM_EXIT_INFO_EXCEPTION_ERROR_CODE);
    CASE(VM_EXIT_INFO_IDT_VECTORING);
    CASE(VM_EXIT_INFO_IDT_VECTORING_ERROR_CODE);
    CASE(VM_EXIT_INFO_INSTRUCTION_LENGTH);
    CASE(VM_EXIT_INFO_INSTRUCTION_INFO);
    CASE(VM_EXIT_INFO_QUALIFICATION);
    CASE(VM_EXIT_INFO_IO_ECX);
    CASE(VM_EXIT_INFO_IO_ESI);
    CASE(VM_EXIT_INFO_IO_EDI);
    CASE(VM_EXIT_INFO_IO_EIP);
    CASE(VM_EXIT_INFO_GUEST_LINEAR_ADDRESS);
    CASE(VM_EXIT_INFO_GUEST_PHYSICAL_ADDRESS);
    CASE(HOST_RIP);
    CASE(HOST_RSP);
    CASE(HOST_CR0);
    CASE(HOST_CR3);
    CASE(HOST_CR4);
    CASE(HOST_CS_SELECTOR);
    CASE(HOST_DS_SELECTOR);
    CASE(HOST_ES_SELECTOR);
    CASE(HOST_FS_SELECTOR);
    CASE(HOST_GS_SELECTOR);
    CASE(HOST_SS_SELECTOR);
    CASE(HOST_TR_SELECTOR);
    CASE(HOST_FS_BASE);
    CASE(HOST_GS_BASE);
    CASE(HOST_TR_BASE);
    CASE(HOST_GDTR_BASE);
    CASE(HOST_IDTR_BASE);
    CASE(HOST_SYSENTER_CS);
    CASE(HOST_SYSENTER_ESP);
    CASE(HOST_SYSENTER_EIP);
    CASE(HOST_PAT);
    CASE(HOST_EFER);
    CASE(HOST_PERF_GLOBAL_CTRL);
    CASE(GUEST_RIP);
    CASE(GUEST_RFLAGS);
    CASE(GUEST_RSP);
    CASE(GUEST_CR0);
    CASE(GUEST_CR3);
    CASE(GUEST_CR4);
    CASE(GUEST_ES_SELECTOR);
    CASE(GUEST_CS_SELECTOR);
    CASE(GUEST_SS_SELECTOR);
    CASE(GUEST_DS_SELECTOR);
    CASE(GUEST_FS_SELECTOR);
    CASE(GUEST_GS_SELECTOR);
    CASE(GUEST_LDTR_SELECTOR);
    CASE(GUEST_TR_SELECTOR);
    CASE(GUEST_ES_AR);
    CASE(GUEST_CS_AR);
    CASE(GUEST_SS_AR);
    CASE(GUEST_DS_AR);
    CASE(GUEST_FS_AR);
    CASE(GUEST_GS_AR);
    CASE(GUEST_LDTR_AR);
    CASE(GUEST_TR_AR);
    CASE(GUEST_ES_BASE);
    CASE(GUEST_CS_BASE);
    CASE(GUEST_SS_BASE);
    CASE(GUEST_DS_BASE);
    CASE(GUEST_FS_BASE);
    CASE(GUEST_GS_BASE);
    CASE(GUEST_LDTR_BASE);
    CASE(GUEST_TR_BASE);
    CASE(GUEST_GDTR_BASE);
    CASE(GUEST_IDTR_BASE);
    CASE(GUEST_ES_LIMIT);
    CASE(GUEST_CS_LIMIT);
    CASE(GUEST_SS_LIMIT);
    CASE(GUEST_DS_LIMIT);
    CASE(GUEST_FS_LIMIT);
    CASE(GUEST_GS_LIMIT);
    CASE(GUEST_LDTR_LIMIT);
    CASE(GUEST_TR_LIMIT);
    CASE(GUEST_GDTR_LIMIT);
    CASE(GUEST_IDTR_LIMIT);
    CASE(GUEST_VMCS_LINK_PTR);
    CASE(GUEST_DEBUGCTL);
    CASE(GUEST_PAT);
    CASE(GUEST_EFER);
    CASE(GUEST_PERF_GLOBAL_CTRL);
    CASE(GUEST_PDPTE0);
    CASE(GUEST_PDPTE1);
    CASE(GUEST_PDPTE2);
    CASE(GUEST_PDPTE3);
    CASE(GUEST_DR7);
    CASE(GUEST_PENDING_DBE);
    CASE(GUEST_SYSENTER_CS);
    CASE(GUEST_SYSENTER_ESP);
    CASE(GUEST_SYSENTER_EIP);
    CASE(GUEST_SMBASE);
    CASE(GUEST_INTERRUPTIBILITY);
    CASE(GUEST_ACTIVITY_STATE);
    default:
        return "";
    }
#undef HASH
#undef CASE
}

void dump_vmcs(struct vcpu_t *vcpu)
{
    uint32_t i, enc, n;
    const char *name;

    uint32_t *list = dump_vmcs_list;
    n = ARRAY_ELEMENTS(dump_vmcs_list);

    for (i = 0; i < n; i++) {
        enc = list[i];
        name = get_vmcs_component_name(enc);
        vmread_dump(vcpu, enc, name);
    }
}
