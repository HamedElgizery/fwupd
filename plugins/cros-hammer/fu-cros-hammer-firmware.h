/*
 * Copyright 2025 Hamed Elgizery <email>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <fwupdplugin.h>

#define FU_TYPE_CROS_HAMMER_FIRMWARE (fu_cros_hammer_firmware_get_type())
G_DECLARE_FINAL_TYPE(
    FuCrosHammerFirmware,
    fu_cros_hammer_firmware,
    FU,
    CROS_HAMMER_FIRMWARE,
    FuFirmware) // TODO: Extend from suitable firmware type if exists
		// https://github.com/fwupd/fwupd/blob/main/libfwupdplugin/fu-ihex-firmware.c
		// probably will be "FuFmapFirmware"
		// https://www.chromium.org/chromium-os/developer-library/guides/firmware/firmware-porting-guide/fmap/

FuFirmware *
fu_cros_hammer_firmware_new(void);
guint16
fu_cros_hammer_firmware_get_start_addr(FuCrosHammerFirmware *self);
