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
 * Copyright (C) 2012-2013 Kay Sievers <kay@vrfy.org>
 * Copyright (C) 2012 Harald Hoyer <harald@redhat.com>
 * Copyright (C) 2017 Aaron Plattner <aplattner@nvidia.com>
 */

#ifndef __SDBOOT_BGRT_H
#define __SDBOOT_BGRT_H

#include "splash.h"

struct bmp_file* find_bgrt(const EFI_SYSTEM_TABLE *sys_table, int *x, int *y);
#endif
