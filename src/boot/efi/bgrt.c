/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Copyright (C) 2017 Aaron Plattner <aplattner@nvidia.com>
 *
 * Based on code from
 * http://blog.fpmurphy.com/2015/07/access-bsrt-information-and-boot-logo-from-uefi-shell.html
 * which contained this license:
 *
 *  Copyright (c) 2015  Finnbarr P. Murphy.   All rights reserved.
 *
 *  Show BGRT info, save image to file if option selected
 *
 *  License: BSD License
 *
 */

#include <efi.h>
#include <efiapi.h>
#include <efilib.h>

#include "bgrt.h"

// From http://dox.ipxe.org/Acpi20_8h_source.html
///
/// Root System Description Pointer Structure
///
typedef struct {
  UINT64  Signature;
  UINT8   Checksum;
  UINT8   OemId[6];
  UINT8   Revision;
  UINT32  RsdtAddress;
  UINT32  Length;
  UINT64  XsdtAddress;
  UINT8   ExtendedChecksum;
  UINT8   Reserved[3];
} EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER;

#define EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION 0x02

// From http://www.bluestop.org/edk2/docs/trunk/_acpi_system_description_table_8h_source.html
typedef struct {
  UINT32    Signature;
  UINT32    Length;
  UINT8     Revision;
  UINT8     Checksum;
  CHAR8     OemId[6];
  CHAR8     OemTableId[8];
  UINT32    OemRevision;
  UINT32    CreatorId;
  UINT32    CreatorRevision;
} EFI_ACPI_SDT_HEADER;

// Boot Graphics Resource Table definition
typedef struct {
    EFI_ACPI_SDT_HEADER Header;
    UINT16 Version;
    UINT8 Status;
    UINT8 ImageType;
    UINT64 ImageAddress;
    UINT32 ImageOffsetX;
    UINT32 ImageOffsetY;
} EFI_ACPI_BGRT;

struct bmp_file* find_bgrt(const EFI_SYSTEM_TABLE *sys_table, int *x, int *y)
{
    EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER *rsdp = NULL;
    EFI_ACPI_SDT_HEADER *xsdt;
    EFI_ACPI_BGRT *bgrt = NULL;
    UINTN i;

    // Find the root system description pointer.
    for (i = 0; i < sys_table->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *table = &sys_table->ConfigurationTable[i];
        EFI_GUID *table_guid = &table->VendorGuid;
        EFI_GUID acpi_20_guid = ACPI_20_TABLE_GUID;

        if (CompareGuid(table_guid, &acpi_20_guid)) {
            if (CompareMem("RSD PTR ", table->VendorTable, 8) == 0) {
                rsdp = table->VendorTable;
                break;
            }
        }
    }

    if (!rsdp ||
        rsdp->Revision < EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION)
        return NULL;

    xsdt = (EFI_ACPI_SDT_HEADER*)rsdp->XsdtAddress;
    if (xsdt->Signature != EFI_SIGNATURE_32('X', 'S', 'D', 'T'))
        return NULL;

    {
        UINTN count = (xsdt->Length - sizeof(*xsdt)) / sizeof(void*);
        EFI_ACPI_SDT_HEADER **p = (void*)(xsdt + 1);

        for (i = 0; i < count; i++) {
            const EFI_ACPI_SDT_HEADER *entry = p[i];

            if (entry->Signature == EFI_SIGNATURE_32('B', 'G', 'R', 'T')) {
                // TODO: Validate checksum
                bgrt = (EFI_ACPI_BGRT*)entry;
            }
        }
    }

    if (!bgrt)
        return NULL;

    *x = bgrt->ImageOffsetX;
    *y = bgrt->ImageOffsetY;
    return (struct bmp_file*)bgrt->ImageAddress;
}
