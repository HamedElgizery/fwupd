/*
 * Copyright 2025 Hamed Elgizery <email>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "fu-cros-hammer-common.h"
#include "fu-cros-hammer-device.h"
#include "fu-cros-hammer-firmware.h"
#include "fu-cros-hammer-struct.h"

/* this can be set using Flags=example in the quirk file  */
// Add flags here for different devices (for handling per device behavior)
// Values macros could also be added
// e.g. https://github.com/fwupd/fwupd/blob/main/plugins/cros-ec/fu-cros-ec-usb-device.c
#define FU_CROS_HAMMER_DEVICE_FLAG_EXAMPLE "example"

struct _FuCrosHammerDevice {
	FuUsbDevice parent_instance;
	guint16 start_addr; // Probably not needed
};

G_DEFINE_TYPE(FuCrosHammerDevice, fu_cros_hammer_device, FU_TYPE_USB_DEVICE)

static void
fu_cros_hammer_device_to_string(FuDevice *device, guint idt, GString *str)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	fwupd_codec_string_append_hex(str, idt, "StartAddr", self->start_addr);
}

/* TODO: this is only required if the device instance state is required elsewhere */
// guint16
// fu_cros_hammer_device_get_start_addr(FuCrosHammerDevice *self)
// {
// g_return_val_if_fail(FU_IS_CROS_HAMMER_DEVICE(self), G_MAXUINT16);
// 	return self->start_addr;
// }

static gboolean
fu_cros_hammer_device_detach(FuDevice *device, FuProgress *progress, GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);

	/* sanity check */
	if (fu_device_has_flag(device, FWUPD_DEVICE_FLAG_IS_BOOTLOADER)) {
		g_debug("already in bootloader mode, skipping");
		return TRUE;
	}

	/* TODO: switch the device into bootloader mode */
	g_assert(self != NULL); // nocheck:blocked

	fu_device_add_flag(device, FWUPD_DEVICE_FLAG_WAIT_FOR_REPLUG);
	return TRUE;
}

static gboolean
fu_cros_hammer_device_attach(FuDevice *device, FuProgress *progress, GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);

	/* sanity check */
	if (!fu_device_has_flag(device, FWUPD_DEVICE_FLAG_IS_BOOTLOADER)) {
		g_debug("already in runtime mode, skipping");
		return TRUE;
	}

	/* TODO: switch the device into runtime mode */
	g_assert(self != NULL); // nocheck:blocked

	fu_device_add_flag(device, FWUPD_DEVICE_FLAG_WAIT_FOR_REPLUG);
	return TRUE;
}

static gboolean
fu_cros_hammer_device_reload(FuDevice *device, GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	/* TODO: reprobe the hardware, or delete this vfunc to use ->setup() as a fallback */
	g_assert(self != NULL); // nocheck:blocked
	return TRUE;
}

static gboolean
fu_cros_hammer_device_probe(FuDevice *device, GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);

	/* FuUsbDevice->probe */
	if (!FU_DEVICE_CLASS(fu_cros_hammer_device_parent_class)->probe(device, error))
		return FALSE;

	/* TODO: probe the device for properties available before it is opened */
	if (fu_device_has_private_flag(device, FU_CROS_HAMMER_DEVICE_FLAG_EXAMPLE))
		self->start_addr = 0x100;
	/* success */
	return TRUE;
}

static gboolean
fu_cros_hammer_device_setup(FuDevice *device, GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);

	/* UsbDevice->setup */
	if (!FU_DEVICE_CLASS(fu_cros_hammer_device_parent_class)->setup(device, error))
		return FALSE;

	/* TODO: get the version and other properties from the hardware while open */
	g_assert(self != NULL); // nocheck:blocked
	fu_device_set_version(device, "1.2.3");

	/* success */
	return TRUE;
}

static gboolean
fu_cros_hammer_device_prepare(FuDevice *device,
			      FuProgress *progress,
			      FwupdInstallFlags flags,
			      GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	/* TODO: anything the device has to do before the update starts */
	g_assert(self != NULL); // nocheck:blocked
	return TRUE;
}

static gboolean
fu_cros_hammer_device_cleanup(FuDevice *device,
			      FuProgress *progress,
			      FwupdInstallFlags flags,
			      GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	/* TODO: anything the device has to do when the update has completed */
	g_assert(self != NULL); // nocheck:blocked
	return TRUE;
}

static FuFirmware *
fu_cros_hammer_device_prepare_firmware(FuDevice *device,
				       GInputStream *stream,
				       FuProgress *progress,
				       FwupdInstallFlags flags,
				       GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	g_autoptr(FuFirmware) firmware = fu_cros_hammer_firmware_new();

	/* TODO: you do not need to use this vfunc if not checking attributes */
	if (self->start_addr !=
	    fu_cros_hammer_firmware_get_start_addr(FU_CROS_HAMMER_FIRMWARE(firmware))) {
		g_set_error(
		    error,
		    FWUPD_ERROR,
		    FWUPD_ERROR_INVALID_FILE,
		    "start address mismatch, got 0x%04x, expected 0x%04x",
		    fu_cros_hammer_firmware_get_start_addr(FU_CROS_HAMMER_FIRMWARE(firmware)),
		    self->start_addr);
		return NULL;
	}

	if (!fu_firmware_parse_stream(firmware, stream, 0x0, flags, error))
		return NULL;
	return g_steal_pointer(&firmware);
}

static gboolean
fu_cros_hammer_device_write_blocks(FuCrosHammerDevice *self,
				   FuChunkArray *chunks,
				   FuProgress *progress,
				   GError **error)
{
	/* progress */
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_set_steps(progress, fu_chunk_array_length(chunks));
	for (guint i = 0; i < fu_chunk_array_length(chunks); i++) {
		g_autoptr(FuChunk) chk = NULL;

		/* prepare chunk */
		chk = fu_chunk_array_index(chunks, i, error);
		if (chk == NULL)
			return FALSE;

		/* TODO: send to hardware */

		/* update progress */
		fu_progress_step_done(progress);
	}

	/* success */
	return TRUE;
}

static gboolean
fu_cros_hammer_device_write_firmware(FuDevice *device,
				     FuFirmware *firmware,
				     FuProgress *progress,
				     FwupdInstallFlags flags,
				     GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);
	g_autoptr(GInputStream) stream = NULL;
	g_autoptr(FuChunkArray) chunks = NULL;

	/* progress */
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_add_flag(progress, FU_PROGRESS_FLAG_GUESSED);
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_WRITE, 44, NULL);
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_VERIFY, 35, NULL);

	/* get default image */
	stream = fu_firmware_get_stream(firmware, error);
	if (stream == NULL)
		return FALSE;

	/* write each block */
	chunks = fu_chunk_array_new_from_stream(stream,
						self->start_addr,
						FU_CHUNK_PAGESZ_NONE,
						64 /* block_size */,
						error);
	if (chunks == NULL)
		return FALSE;
	if (!fu_cros_hammer_device_write_blocks(self,
						chunks,
						fu_progress_get_child(progress),
						error))
		return FALSE;
	fu_progress_step_done(progress);

	/* TODO: verify each block */
	fu_progress_step_done(progress);

	/* success! */
	return TRUE;
}

static gboolean
fu_cros_hammer_device_set_quirk_kv(FuDevice *device,
				   const gchar *key,
				   const gchar *value,
				   GError **error)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(device);

	/* TODO: parse value from quirk file */
	if (g_strcmp0(key, "CrosHammerStartAddr") == 0) {
		guint64 tmp = 0;
		if (!fu_strtoull(value, &tmp, 0, G_MAXUINT16, FU_INTEGER_BASE_AUTO, error))
			return FALSE;
		self->start_addr = tmp;
		return TRUE;
	}

	/* failed */
	g_set_error_literal(error,
			    FWUPD_ERROR,
			    FWUPD_ERROR_NOT_SUPPORTED,
			    "quirk key not supported");
	return FALSE;
}

static void
fu_cros_hammer_device_set_progress(FuDevice *self, FuProgress *progress)
{
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_add_flag(progress, FU_PROGRESS_FLAG_GUESSED);
	fu_progress_add_step(progress, FWUPD_STATUS_DECOMPRESSING, 0, "prepare-fw");
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_RESTART, 0, "detach");
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_WRITE, 57, "write");
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_RESTART, 0, "attach");
	fu_progress_add_step(progress, FWUPD_STATUS_DEVICE_BUSY, 43, "reload");
}

static void
fu_cros_hammer_device_init(FuCrosHammerDevice *self)
{
	self->start_addr = 0x5000;
	fu_device_set_version_format(FU_DEVICE(self), FWUPD_VERSION_FORMAT_TRIPLET);
	fu_device_set_remove_delay(FU_DEVICE(self), FU_DEVICE_REMOVE_DELAY_RE_ENUMERATE);
	fu_device_add_protocol(FU_DEVICE(self), "com.cross.hammer");
	fu_device_add_flag(FU_DEVICE(self), FWUPD_DEVICE_FLAG_UPDATABLE);
	fu_device_add_flag(FU_DEVICE(self), FWUPD_DEVICE_FLAG_UNSIGNED_PAYLOAD);
	fu_device_add_icon(FU_DEVICE(self), "icon-name");
	fu_device_register_private_flag(FU_DEVICE(self), FU_CROS_HAMMER_DEVICE_FLAG_EXAMPLE);
	fu_usb_device_add_interface(FU_USB_DEVICE(self), 0x01);
}

static void
fu_cros_hammer_device_finalize(GObject *object)
{
	FuCrosHammerDevice *self = FU_CROS_HAMMER_DEVICE(object);

	/* TODO: free any allocated instance state here */
	g_assert(self != NULL); // nocheck:blocked

	G_OBJECT_CLASS(fu_cros_hammer_device_parent_class)->finalize(object);
}

static void
fu_cros_hammer_device_class_init(FuCrosHammerDeviceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FuDeviceClass *device_class = FU_DEVICE_CLASS(klass);
	object_class->finalize = fu_cros_hammer_device_finalize;
	device_class->to_string = fu_cros_hammer_device_to_string;
	device_class->probe = fu_cros_hammer_device_probe;
	device_class->setup = fu_cros_hammer_device_setup;
	device_class->reload = fu_cros_hammer_device_reload;
	device_class->prepare = fu_cros_hammer_device_prepare;
	device_class->cleanup = fu_cros_hammer_device_cleanup;
	device_class->attach = fu_cros_hammer_device_attach;
	device_class->detach = fu_cros_hammer_device_detach;
	device_class->prepare_firmware = fu_cros_hammer_device_prepare_firmware;
	device_class->write_firmware = fu_cros_hammer_device_write_firmware;
	device_class->set_quirk_kv = fu_cros_hammer_device_set_quirk_kv;
	device_class->set_progress = fu_cros_hammer_device_set_progress;
}
