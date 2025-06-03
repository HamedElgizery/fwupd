/*
 * Copyright 2025 Hamed Elgizery <email>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "fu-cros-hammer-device.h"
#include "fu-cros-hammer-firmware.h"
#include "fu-cros-hammer-plugin.h"

struct _FuCrosHammerPlugin {
	FuPlugin parent_instance;
};

G_DEFINE_TYPE(FuCrosHammerPlugin, fu_cros_hammer_plugin, FU_TYPE_PLUGIN)

static void
fu_cros_hammer_plugin_init(FuCrosHammerPlugin *self)
{
}

static void
fu_cros_hammer_plugin_constructed(GObject *obj)
{
	FuPlugin *plugin = FU_PLUGIN(obj);
	FuContext *ctx = fu_plugin_get_context(plugin);
	fu_context_add_quirk_key(ctx, "CrosHammerStartAddr");
	fu_plugin_add_device_gtype(plugin, FU_TYPE_CROS_HAMMER_DEVICE);
	fu_plugin_add_firmware_gtype(plugin, NULL, FU_TYPE_CROS_HAMMER_FIRMWARE);
}

static void
fu_cros_hammer_plugin_class_init(FuCrosHammerPluginClass *klass)
{
	FuPluginClass *plugin_class = FU_PLUGIN_CLASS(klass);
	plugin_class->constructed = fu_cros_hammer_plugin_constructed;
}
