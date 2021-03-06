//----------------------------------------------------------------------------//
//|
//|             MachOKit - A Lightweight Mach-O Parsing Library
//|             core.c
//|
//|             D.V.
//|             Copyright (c) 2014-2015 D.V. All rights reserved.
//|
//| Permission is hereby granted, free of charge, to any person obtaining a
//| copy of this software and associated documentation files (the "Software"),
//| to deal in the Software without restriction, including without limitation
//| the rights to use, copy, modify, merge, publish, distribute, sublicense,
//| and/or sell copies of the Software, and to permit persons to whom the
//| Software is furnished to do so, subject to the following conditions:
//|
//| The above copyright notice and this permission notice shall be included
//| in all copies or substantial portions of the Software.
//|
//| THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//| OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//| MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//| IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//| CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//| TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//| SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//----------------------------------------------------------------------------//

#include "core_internal.h"

//----------------------------------------------------------------------------//
#pragma mark -  Errors
//----------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
const char *
mk_error_string(mk_error_t error)
{
    switch (error & ~MK_EMEMORY_ERROR) {
        case MK_ESUCCESS:
            return "SUCCESS";
        case MK_ECLIENT_ERROR:
            return "CLIENT ERROR";
        case MK_ECLIENT_INVALID_RESULT:
            return "INVALID CLIENT RESULT";
        case MK_EINTERNAL_ERROR:
            return "INTERNAL ERROR";
        case MK_EINVAL:
            return "BAD INPUT";
        case MK_EINVALID_DATA:
            return "INVALID DATA";
        case MK_ENOT_FOUND:
            return "NOT FOUND";
        case MK_EUNAVAILABLE:
            return "UNAVAILABLE";
        case MK_EOUT_OF_RANGE:
            return "OUT OF RANGE";
        case MK_EOVERFLOW:
            return "OVERFLOW";
        case MK_EUNDERFLOW:
            return "UNDERFLOW";
        case MK_EBAD_ACCESS:
            return "BAD ACCESS";
        default:
            return "";
    }
}

//----------------------------------------------------------------------------//
#pragma mark - Ranges
//----------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
mk_vm_range_t
mk_vm_range_make(mk_vm_address_t location, mk_vm_size_t length)
{
    mk_vm_range_t retValue;
    retValue.location = location;
    retValue.length = length;
    return retValue;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_range_contains_address(mk_vm_range_t range, mk_vm_offset_t offset, mk_vm_address_t address)
{
    mk_error_t err;
    if ((err = mk_vm_address_apply_offset(address, offset, &address)))
        return err;
    
    if (MK_VM_SIZE_MAX - range.length < range.location)
        return MK_EOVERFLOW;
    
    if (address < range.location || address >= range.location + range.length)
        return MK_ENOT_FOUND;
    
    return MK_ESUCCESS;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_range_contains_range(mk_vm_range_t outer_range, mk_vm_range_t inner_range, bool partial)
{
    if (MK_VM_SIZE_MAX - outer_range.length < outer_range.location)
        return MK_EOVERFLOW;
    if (MK_VM_SIZE_MAX - inner_range.length < inner_range.location)
        return MK_EOVERFLOW;
    
    if (partial)
    {
        if (inner_range.location < outer_range.location && inner_range.location + inner_range.length < outer_range.location)
            return MK_ENOT_FOUND;
        else if (inner_range.location > outer_range.location + outer_range.length)
            return MK_ENOT_FOUND;
    }
    else
    {
        if (inner_range.location < outer_range.location)
            return MK_ENOT_FOUND;
        else if (inner_range.location + inner_range.length > outer_range.location + outer_range.length)
            return MK_ENOT_FOUND;
    }
    
    return MK_ESUCCESS;
}

//----------------------------------------------------------------------------//
#pragma mark -  Type Safe Operations
//----------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_address_apply_offset(mk_vm_address_t addr, mk_vm_offset_t offset, mk_vm_address_t *result)
{
    // Check for overflow
    if (MK_VM_ADDRESS_MAX - offset < addr)
        return MK_EOVERFLOW;
    
    if (result)
        *result = addr + offset;
    
    return MK_ESUCCESS;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_address_add(mk_vm_address_t addr1, mk_vm_address_t addr2, mk_vm_address_t *result)
{
    // Check for overflow
    if (MK_VM_ADDRESS_MAX - addr2 < addr1)
        return MK_EOVERFLOW;
    
    if (result)
        *result = addr1 + addr2;
    
    return MK_ESUCCESS;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_address_substract(mk_vm_address_t left, mk_vm_address_t right, mk_vm_address_t *result)
{
    // Check for underflow
    if (right > left)
        return MK_EUNDERFLOW;
    
    if (result)
        *result = left - right;
    
    return MK_ESUCCESS;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_error_t
mk_vm_address_check_length(mk_vm_address_t addr, mk_vm_size_t length)
{
    if (MK_VM_SIZE_MAX - length < addr)
        return MK_EOVERFLOW;
    else
        return MK_ESUCCESS;
}

//---------------------------------------------------------------------------//
#pragma mark -  Byte Order
//---------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
static uint16_t _mk_swap16 (uint16_t input)
{ return OSSwapInt16(input); }

//|++++++++++++++++++++++++++++++++++++|//
static uint16_t _mk_nswap16 (uint16_t input)
{ return input; }

//|++++++++++++++++++++++++++++++++++++|//
static uint32_t _mk_swap32 (uint32_t input)
{ return OSSwapInt32(input); }

//|++++++++++++++++++++++++++++++++++++|//
static uint32_t _mk_nswap32 (uint32_t input)
{ return input; }

//|++++++++++++++++++++++++++++++++++++|//
static uint64_t _mk_swap64 (uint64_t input)
{ return OSSwapInt64(input); }

//|++++++++++++++++++++++++++++++++++++|//
static uint64_t _mk_nswap64 (uint64_t input)
{ return input; }

//|++++++++++++++++++++++++++++++++++++|//
static uint8_t*
_mk_swap (uint8_t *input, size_t length)
{
    for (size_t i=0; i<length/2; i++) {
        uint8_t temp = input[i];
        input[i] = input[length-i-1];
        input[length-i-1] = temp;
    }
    
    return input;
}

//|++++++++++++++++++++++++++++++++++++|//
static uint8_t* _mk_nswap (uint8_t *input, size_t __unused length)
{ return input; }

const mk_byteorder_t mk_byteorder_direct = {
    &_mk_nswap16,
    &_mk_nswap32,
    &_mk_nswap64,
    &_mk_nswap
};

const mk_byteorder_t mk_byteorder_swapped = {
    &_mk_swap16,
    &_mk_swap32,
    &_mk_swap64,
    &_mk_swap
};

//----------------------------------------------------------------------------//
#pragma mark -  Classes
//----------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
static mk_context_t*
__mk_type_get_context(mk_type_ref self)
{
#pragma unused (self)
    return NULL;
}

//|++++++++++++++++++++++++++++++++++++|//
static bool
__mk_type_equal(mk_type_ref self, mk_type_ref other)
{
    return self == other;
}

//|++++++++++++++++++++++++++++++++++++|//
static size_t
__mk_type_copy_description(mk_type_ref self, char* output, size_t output_len)
{
    return snprintf(output, output_len, "<%s %p>", mk_type_name(self), self);
}

const struct _mk_type_vtable _mk_type_class = {
    .super = NULL,
    .name = "",
    .get_context = &__mk_type_get_context,
    .equal = &__mk_type_equal,
    .copy_description = &__mk_type_copy_description,
};

//----------------------------------------------------------------------------//
#pragma mark -  Runtime
//----------------------------------------------------------------------------//

//|++++++++++++++++++++++++++++++++++++|//
bool
mk_type_is(mk_type_ref mk, intptr_t type)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk)->vtable;
    return (vtable == (void*)type);
}

//|++++++++++++++++++++++++++++++++++++|//
bool
mk_type_is_kind_of(mk_type_ref mk, intptr_t type)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk)->vtable;
    while (vtable != NULL) {
        if (vtable == (void*)type) return true;
        vtable = (typeof(vtable))vtable->super;
    }
    return false;
}

//|++++++++++++++++++++++++++++++++++++|//
const char*
mk_type_name(mk_type_ref mk)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk)->vtable;
    while (vtable->name == NULL)
        vtable = (typeof(vtable))vtable->super;
    return vtable->name;
}

//|++++++++++++++++++++++++++++++++++++|//
mk_context_t*
mk_type_get_context(mk_type_ref mk)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk)->vtable;
    while (vtable->get_context == NULL)
        vtable = (typeof(vtable))vtable->super;
    return vtable->get_context(mk);
}

//|++++++++++++++++++++++++++++++++++++|//
bool
mk_type_equal(mk_type_ref mk1, mk_type_ref mk2)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk1)->vtable;
    while (vtable->equal == NULL)
        vtable = (typeof(vtable))vtable->super;
    return vtable->equal(mk1, mk2);
}

//|++++++++++++++++++++++++++++++++++++|//
size_t
mk_type_copy_description(mk_type_ref mk, char* output, size_t output_len)
{
    struct _mk_type_vtable *vtable = (struct _mk_type_vtable*)((_mk_runtime_base_t*)mk)->vtable;
    while (vtable->copy_description == NULL)
        vtable = (typeof(vtable))vtable->super;
    return vtable->copy_description(mk, output, output_len);
}
