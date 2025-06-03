/*
 * Copyright 2025 Hamed Elgizery <email>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <fwupdplugin.h>

#define FU_TYPE_CROS_HAMMER_DEVICE (fu_cros_hammer_device_get_type())
G_DECLARE_FINAL_TYPE(FuCrosHammerDevice,
		     fu_cros_hammer_device,
		     FU,
		     CROS_HAMMER_DEVICE,
		     FuUsbDevice) // TODO: Extend from appropriate device type

/*
 * This method will most likely not be needed if we are using Fmap firmware
 * we will be using sections&images instead
 */
// guint16
// fu_cros_hammer_device_get_start_addr(FuCrosHammerDevice *self);
