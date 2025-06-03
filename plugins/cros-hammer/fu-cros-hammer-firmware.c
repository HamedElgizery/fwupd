/*
 * Copyright 2025 Hamed Elgizery <email>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include <fwupdplugin.h>

#include "fu-cros-hammer-firmware.h"
#include "fu-cros-hammer-struct.h"

struct _FuCrosHammerFirmware {
	FuFirmware parent_instance; // Could be FuFmapFirmware
	guint16 start_addr;
};

G_DEFINE_TYPE(FuCrosHammerFirmware, fu_cros_hammer_firmware, FU_TYPE_FIRMWARE)

static void
fu_cros_hammer_firmware_export(FuFirmware *firmware, FuFirmwareExportFlags flags, XbBuilderNode *bn)
{
	FuCrosHammerFirmware *self = FU_CROS_HAMMER_FIRMWARE(firmware);
	fu_xmlb_builder_insert_kx(bn, "start_addr", self->start_addr);
}

static gboolean
fu_cros_hammer_firmware_build(FuFirmware *firmware, XbNode *n, GError **error)
{
	FuCrosHammerFirmware *self = FU_CROS_HAMMER_FIRMWARE(firmware);
	guint64 tmp;

	/* TODO: load from .builder.xml */
	tmp = xb_node_query_text_as_uint(n, "start_addr", NULL);
	if (tmp != G_MAXUINT64 && tmp <= G_MAXUINT16)
		self->start_addr = tmp;

	/* success */
	return TRUE;
}

/*
static gboolean
fu_cros_hammer_firmware_validate(FuFirmware *firmware,
					GInputStream *stream,
					gsize offset,
					GError **error)
{
	return fu_struct_cros_hammer_header_validate_stream(stream,
						     offset,
						     error);
}
*/

static gboolean
fu_cros_hammer_firmware_parse(FuFirmware *firmware,
			      GInputStream *stream,
			      FuFirmwareParseFlags flags,
			      GError **error)
{
	// FuCrosHammerFirmware *self = FU_CROS_HAMMER_FIRMWARE(firmware);
	return TRUE;
}

static GByteArray *
fu_cros_hammer_firmware_write(FuFirmware *firmware, GError **error)
{
	FuCrosHammerFirmware *self = FU_CROS_HAMMER_FIRMWARE(firmware);
	g_autoptr(GByteArray) buf = g_byte_array_new();
	g_autoptr(GBytes) fw = NULL;

	/* data first */
	fw = fu_firmware_get_bytes_with_patches(firmware, error);
	if (fw == NULL)
		return NULL;

	/* TODO: write to the buffer with the correct format */

	g_assert(self != NULL); // nocheck:blocked

	/* success */
	return g_steal_pointer(&buf);
}

guint16
fu_cros_hammer_firmware_get_start_addr(FuCrosHammerFirmware *self)
{
	g_return_val_if_fail(FU_IS_CROS_HAMMER_FIRMWARE(self), G_MAXUINT16);
	return self->start_addr;
}

static void
fu_cros_hammer_firmware_init(FuCrosHammerFirmware *self)
{
	// _init are per instance init function
	fu_firmware_add_flag(FU_FIRMWARE(self), FU_FIRMWARE_FLAG_HAS_STORED_SIZE);
	fu_firmware_add_flag(FU_FIRMWARE(self), FU_FIRMWARE_FLAG_HAS_CHECKSUM);
	fu_firmware_add_flag(FU_FIRMWARE(self), FU_FIRMWARE_FLAG_HAS_VID_PID);
}

static void
fu_cros_hammer_firmware_class_init(FuCrosHammerFirmwareClass *klass)
{
	// _class_init are per class init function
	// used to define vfuncs and any class related methods
	FuFirmwareClass *firmware_class = FU_FIRMWARE_CLASS(klass);
	// firmware_class->validate = fu_cros_hammer_firmware_validate;
	firmware_class->parse = fu_cros_hammer_firmware_parse;
	firmware_class->write = fu_cros_hammer_firmware_write;
	firmware_class->build = fu_cros_hammer_firmware_build;
	firmware_class->export = fu_cros_hammer_firmware_export;
}

FuFirmware *
fu_cros_hammer_firmware_new(void)
{
	return FU_FIRMWARE(g_object_new(FU_TYPE_CROS_HAMMER_FIRMWARE, NULL));
}
