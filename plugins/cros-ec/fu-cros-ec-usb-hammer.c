/*
 * Copyright 2025 Hamed Elgizery
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include <string.h>

#include "fu-cros-ec-common.h"
#include "fu-cros-ec-firmware.h"
#include "fu-cros-ec-hammer-touchpad-firmware.h"
#include "fu-cros-ec-struct.h"
#include "fu-cros-ec-usb-device.h"
#include "fu-cros-ec-usb-hammer.h"

struct _FuCrosEcUsbHammer {
	FuCrosEcUsbDevice parent_instance;
};

G_DEFINE_TYPE(FuCrosEcUsbHammer, fu_cros_ec_usb_hammer, FU_TYPE_CROS_EC_USB_DEVICE)

typedef struct {
	FuChunk *block;
	FuProgress *progress;
} FuCrosEcUsbBlockHelper;

gboolean
fu_cros_ec_usb_hammer_write_touchpad_firmware(FuDevice *device,
					      FuFirmware *firmware,
					      FuProgress *progress,
					      FwupdInstallFlags flags,
					      FuDevice *tp_device,
					      GError **error)
{
	FuCrosEcUsbHammer *self = FU_CROS_EC_USB_HAMMER(device);
	FuCrosEcHammerTouchpad *touchpad = FU_CROS_EC_HAMMER_TOUCHPAD(tp_device);
	gsize data_len = 0;
	guint32 maximum_pdu_size = 0;
	guint32 tp_fw_size = 0;
	guint32 tp_fw_address = 0;
	const guint8 *data_ptr = NULL;
	g_autoptr(GBytes) img_bytes = NULL;
	g_autoptr(GPtrArray) blocks = NULL;
	g_autoptr(FuStructCrosEcFirstResponsePdu) st_rpdu =
	    fu_struct_cros_ec_first_response_pdu_new();
	g_warning("DEBUG: START WRITE TOUCHAPD BEFORE");

	/* send start request */
	if (!fu_device_retry(device,
			     fu_cros_ec_usb_device_start_request_cb,
			     FU_CROS_EC_SETUP_RETRY_CNT,
			     st_rpdu,
			     error)) {
		g_prefix_error_literal(error, "touchpad: failed to send start request: ");
		return FALSE;
	}

	g_warning("DEBUG: START WRITE TOUCHAPD");

	/*
	 * Probably, can be replaced with the CrosEcUsbHammer's maximum_pdu_size,
	 * but opting for independence here.
	 */
	maximum_pdu_size = fu_struct_cros_ec_first_response_pdu_get_maximum_pdu_size(st_rpdu);
	img_bytes = fu_firmware_get_bytes(firmware, error);
	data_ptr = (const guint8 *)g_bytes_get_data(img_bytes, &data_len);
	tp_fw_address = fu_cros_ec_hammer_touchpad_get_fw_address(touchpad);
	tp_fw_size = fu_cros_ec_hammer_touchpad_get_fw_size(touchpad);

	if (tp_fw_address != (1u << 31)) { // This is for testing safety only, should be removed
		g_set_error_literal(error,
				    FWUPD_ERROR,
				    FWUPD_ERROR_INVALID_DATA,
				    "CRITICAL DEVICE ABOUT TO BE BRICKED HALTING");
		return FALSE;
	}

	if (data_ptr == NULL || data_len != tp_fw_size) {
		g_set_error(
		    error,
		    FWUPD_ERROR,
		    FWUPD_ERROR_INVALID_DATA,
		    "touchpad: image and section sizes do not match: image = %" G_GSIZE_FORMAT
		    " bytes vs touchpad section size = %" G_GSIZE_FORMAT " bytes",
		    data_len,
		    (gsize)tp_fw_size);
		return FALSE;
	}

	g_debug("touchpad: sending 0x%x bytes to 0x%x", (guint)data_len, tp_fw_address);

	blocks =
	    /* send in chunks of PDU size */
	    fu_chunk_array_new(data_ptr, data_len, tp_fw_address, 0x0, maximum_pdu_size);
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_set_steps(progress, blocks->len);
	fu_progress_set_status(progress, FWUPD_STATUS_DEVICE_WRITE);
	for (guint i = 0; i < blocks->len; i++) {
		FuCrosEcUsbBlockHelper helper = {
		    .block = g_ptr_array_index(blocks, i),
		    .progress = fu_progress_get_child(progress),
		};
		if (!fu_device_retry(FU_DEVICE(self),
				     fu_cros_ec_usb_device_transfer_block_cb,
				     FU_CROS_EC_MAX_BLOCK_XFER_RETRIES,
				     &helper,
				     error)) {
			g_prefix_error(error, "touchpad: failed to transfer block 0x%x: ", i);
			return FALSE;
		}
		g_warning("DEBUG: SUCCESS 0x%x", i);
		fu_progress_step_done(progress);
	}

	g_warning("DEBUG: TOUCHPAD DONE");
	return TRUE;
}

static gboolean
fu_cros_ec_usb_hammer_transfer_section(FuCrosEcUsbHammer *self,
				       FuFirmware *firmware,
				       FuCrosEcFirmwareSection *section,
				       FuProgress *progress,
				       GError **error)
{
	const guint8 *data_ptr = NULL;
	gsize data_len = 0;
	g_autoptr(GBytes) img_bytes = NULL;
	g_autoptr(GPtrArray) blocks = NULL;
	g_autoptr(FuStructCrosEcFirstResponsePdu) st_rpdu =
	    fu_struct_cros_ec_first_response_pdu_new();
	guint32 maximum_pdu_size;

	g_return_val_if_fail(section != NULL, FALSE);

	/* send start request */
	if (!fu_device_retry(FU_DEVICE(self),
			     fu_cros_ec_usb_device_start_request_cb,
			     FU_CROS_EC_SETUP_RETRY_CNT,
			     st_rpdu,
			     error)) {
		g_prefix_error_literal(error, "failed to send start request: ");
		return FALSE;
	}

	maximum_pdu_size = fu_struct_cros_ec_first_response_pdu_get_maximum_pdu_size(st_rpdu);

	g_warning("MAX_PDU_SZ: %u", maximum_pdu_size);

	img_bytes = fu_firmware_get_image_by_idx_bytes(firmware, section->image_idx, error);
	if (img_bytes == NULL) {
		g_prefix_error_literal(error, "failed to find section image: ");
		return FALSE;
	}

	data_ptr = (const guint8 *)g_bytes_get_data(img_bytes, &data_len);
	if (data_ptr == NULL || data_len != section->size) {
		g_set_error(error,
			    FWUPD_ERROR,
			    FWUPD_ERROR_INVALID_DATA,
			    "image (0x%x bytes) and section (0x%x bytes) sizes do not match",
			    (guint)data_len,
			    (guint)section->size);
		return FALSE;
	}

	// TODO (experimental): Smart update removed (Not in hammerd)
	/* smart update: trim trailing bytes
	while (data_len > 1 && (data_ptr[data_len - 1] == 0xff))
		data_len--;
	g_debug("trimmed %" G_GSIZE_FORMAT " trailing bytes", section->size - data_len);
	*/

	g_debug("sending 0x%x bytes to 0x%x", (guint)data_len, section->offset);

	/* send in chunks of PDU size */
	blocks = fu_chunk_array_new(data_ptr, data_len, section->offset, 0x0, maximum_pdu_size);
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_set_steps(progress, blocks->len);
	for (guint i = 0; i < blocks->len; i++) {
		FuCrosEcUsbBlockHelper helper = {
		    .block = g_ptr_array_index(blocks, i),
		    .progress = fu_progress_get_child(progress),
		};
		if (!fu_device_retry(FU_DEVICE(self),
				     fu_cros_ec_usb_device_transfer_block_cb,
				     FU_CROS_EC_MAX_BLOCK_XFER_RETRIES,
				     &helper,
				     error)) {
			g_prefix_error(error, "failed to transfer block 0x%x: ", i);
			return FALSE;
		}
		g_warning("DEBUG: SUCCESS BLOCK 0x%X", i);
		fu_progress_step_done(progress);
	}

	/* success */
	return TRUE;
}

static gboolean
fu_cros_ec_usb_hammer_is_valid_key(FuCrosEcUsbHammer *self, FuCrosEcFirmwareSection *section)
{
	return fu_cros_ec_usb_device_get_key_version(FU_CROS_EC_USB_DEVICE(self)) ==
	       section->key_version;
}

static gboolean
fu_cros_ec_usb_hammer_write_base_ec_firmware(FuDevice *device,
					     FuFirmware *firmware,
					     FuProgress *progress,
					     FwupdInstallFlags flags,
					     GError **error)
{
	FuCrosEcUsbHammer *self = FU_CROS_EC_USB_HAMMER(device);
	g_autoptr(GPtrArray) sections = NULL;
	FuCrosEcFirmware *cros_ec_firmware = FU_CROS_EC_FIRMWARE(firmware);

	g_warning("DEBUG: ENTERING WRITE BASE EC");

	fu_device_remove_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_SPECIAL);

	g_warning("WRITE ROUND START BASE EC");

	if (fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_REBOOTING_TO_RO)) {
		g_autoptr(FuStructCrosEcFirstResponsePdu) st_rpdu =
		    fu_struct_cros_ec_first_response_pdu_new();
		g_warning("DEBUG: AREA 1");

		fu_device_remove_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_REBOOTING_TO_RO);
		if (!fu_cros_ec_usb_device_stay_in_ro(FU_CROS_EC_USB_DEVICE(self), error)) {
			g_prefix_error_literal(error, "failed to send stay-in-ro subcommand: ");
			g_warning("DEBUG: AREA 1.1");
			return FALSE;
		}

		/* flush all data from endpoint to recover in case of error */
		if (!fu_cros_ec_usb_device_recovery(FU_CROS_EC_USB_DEVICE(self), error)) {
			g_prefix_error_literal(error, "failed to flush device to idle state: ");
			g_warning("DEBUG: AREA 1.2");
			return FALSE;
		}

		/* send start request */
		if (!fu_device_retry(device,
				     fu_cros_ec_usb_device_start_request_cb,
				     FU_CROS_EC_SETUP_RETRY_CNT,
				     st_rpdu,
				     error)) {
			g_warning("DEBUG: AREA 1.3");
			g_prefix_error_literal(error, "failed to send start request: ");
			return FALSE;
		}
		g_warning("DEBUG: AREA 1 DONE");
	}

	if (fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RW_WRITTEN) &&
	    fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(self))) {
		g_warning("DEBUG: AREA 2");
		/*
		 * We had previously written to the rw region (while we were
		 * booted from ro region), but somehow landed in ro again after
		 * a reboot. Since we wrote rw already, we wanted to jump
		 * to the new rw so we could evaluate ro.
		 *
		 * This is a transitory state due to the fact that we have to
		 * boot through RO to get to RW. Set another write required to
		 * allow the RO region to auto-jump to RW.
		 *
		 * Special flow: write phase skips actual write -> attach skips
		 * send of reset command, just sets wait for replug, and
		 * device restart status.
		 */
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_SPECIAL);
		g_warning("DEBUG: ADDING AWR");
		fu_device_add_flag(device, FWUPD_DEVICE_FLAG_ANOTHER_WRITE_REQUIRED);
		g_warning("DEBUG: AREA 2 DONE");
		return TRUE;
	}

	sections = fu_cros_ec_firmware_get_needed_sections(cros_ec_firmware, error);
	if (sections == NULL)
		return FALSE;

	/* In RO, and it is turn to write RW firmware */
	if (fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(self))) {
		FuCrosEcFirmwareSection *section = g_ptr_array_index(sections, 0);
		g_warning("KEY_VERSION: %u",
			  fu_cros_ec_usb_device_get_key_version(FU_CROS_EC_USB_DEVICE(self)));
		g_warning("MIN_ROLLBACK: %u",
			  fu_cros_ec_usb_device_get_min_rollback(FU_CROS_EC_USB_DEVICE(self)));
		g_warning("SECTION KEY_VERSION: %u", section->key_version);
		g_warning("SECTION MIN_ROLLBACK: %u", section->rollback);
		g_warning("FLASH_PROTECTION: %u",
			  fu_cros_ec_usb_device_get_flash_protection(FU_CROS_EC_USB_DEVICE(self)));
		if (section->key_version != -1u) {
			if (!fu_cros_ec_usb_hammer_is_valid_key(self, section)) {
				g_warning("EXITING HERE");
				g_set_error_literal(
				    error,
				    FWUPD_ERROR,
				    FWUPD_ERROR_INVALID_DATA,
				    "RW not compatible with RO section, aborting...");
				return FALSE;
			}
		} else {
			g_info("No RO key version found, updating RW unconditionally...");
		}
		if ((fu_cros_ec_usb_device_get_flash_protection(FU_CROS_EC_USB_DEVICE(self)) &
		     (1 << 8)) != 0) {
			g_warning("DEBUG: AREA 3");
			g_warning("DEBUG: ADDING AWR");
			fu_device_add_flag(device, FWUPD_DEVICE_FLAG_ANOTHER_WRITE_REQUIRED);
			if (!fu_cros_ec_usb_device_unlock_rw(FU_CROS_EC_USB_DEVICE(self), error)) {
				g_warning("DEBUG: AREA 3 ERROR (BAD)");
				return FALSE;
			}
			g_warning("DEBUG: AREA 3 DONE");
			return TRUE;
		}
	}

	g_warning("DEBUG: STARTING EC UPDATE");
	if (fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(self)))
		g_warning("DEBUG: RW");
	else
		g_warning("DEBUG: RO");
	/* progress */
	fu_progress_set_id(progress, G_STRLOC);
	fu_progress_set_steps(progress, sections->len);
	fu_progress_set_status(progress, FWUPD_STATUS_DEVICE_WRITE);
	for (guint i = 0; i < sections->len; i++) {
		FuCrosEcFirmwareSection *section = g_ptr_array_index(sections, i);
		g_autoptr(GError) error_local = NULL;

		if (!fu_cros_ec_usb_hammer_transfer_section(self,
							    firmware,
							    section,
							    fu_progress_get_child(progress),
							    &error_local)) {
			if (g_error_matches(error_local, FWUPD_ERROR, FWUPD_ERROR_READ)) {
				g_debug("failed to transfer section, trying another write, "
					"ignoring error: %s",
					error_local->message);
				g_warning("DEBUG: ADDING AWR");
				fu_device_add_flag(device,
						   FWUPD_DEVICE_FLAG_ANOTHER_WRITE_REQUIRED);
				fu_progress_finished(progress);
				return TRUE;
			}
			g_propagate_error(error, g_steal_pointer(&error_local));
			return FALSE;
		}

		// TODO: Fix me, section->version.triplet has no data...
		if (fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(self))) {
			fu_device_set_version(device, section->version.triplet);
		} else {
			fu_device_set_version_bootloader(device, section->version.triplet);
		}

		fu_progress_step_done(progress);
	}
	g_warning("DEBUG: EC UPDATE DONE");

	/* send done */
	fu_cros_ec_usb_device_send_done(FU_CROS_EC_USB_DEVICE(self));

	if (fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(self)))
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RW_WRITTEN);
	else
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RO_WRITTEN);

	/* logical XOR */
	if (fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RW_WRITTEN) !=
	    fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RO_WRITTEN)) {
		g_warning("DEBUG: ADDING AWR");
		fu_device_add_flag(device, FWUPD_DEVICE_FLAG_ANOTHER_WRITE_REQUIRED);
	}

	/* success */
	return TRUE;
}

static gboolean
fu_cros_ec_usb_hammer_ensure_children(FuCrosEcUsbHammer *self, GError **error)
{
	FuDevice *device = FU_DEVICE(self);
	g_autoptr(FuCrosEcHammerTouchpad) touchpad = NULL;
	g_warning("ENSURE CHILD");

	if (!fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_HAS_TOUCHPAD))
		return TRUE;

	touchpad = fu_cros_ec_hammer_touchpad_new(FU_DEVICE(device));
	fu_device_add_child(FU_DEVICE(device), FU_DEVICE(touchpad));

	/* success */
	return TRUE;
}

static gboolean
fu_cros_ec_usb_hammer_setup(FuDevice *device, GError **error)
{
	g_warning("SETUP IN HAMMER");
	if (!fu_cros_ec_usb_device_setup(device, error))
		return FALSE;

	if (!fu_cros_ec_usb_hammer_ensure_children(FU_CROS_EC_USB_HAMMER(device), error))
		return FALSE;

	/* success */
	return TRUE;
}

static gboolean
fu_cros_ec_usb_hammer_detach(FuDevice *device, FuProgress *progress, GError **error)
{
	if (fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RW_WRITTEN) &&
	    !fu_device_has_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RO_WRITTEN))
		return TRUE;

	if (fu_cros_ec_usb_device_get_in_bootloader(FU_CROS_EC_USB_DEVICE(device))) {
		/* If EC just rebooted - prevent jumping to RW during the update */
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_REBOOTING_TO_RO);
		g_debug("skipping immediate reboot in case of already in bootloader");
		/* in RO so skip reboot */
		return TRUE;
	}
	g_warning("DETACH FLASH PROTECTION: %u",
		  fu_cros_ec_usb_device_get_flash_protection(FU_CROS_EC_USB_DEVICE(device)));
	if ((fu_cros_ec_usb_device_get_flash_protection(FU_CROS_EC_USB_DEVICE(device)) >> 1) & 1) {
		g_warning("RO FLASH PROTECTED");
		/* in RW, and RO region is write protected, so jump to RO */
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_RO_WRITTEN);
		fu_device_add_private_flag(device, FU_CROS_EC_USB_DEVICE_FLAG_REBOOTING_TO_RO);
		fu_cros_ec_usb_device_reset_to_ro(FU_CROS_EC_USB_DEVICE(device));
		fu_device_add_flag(device, FWUPD_DEVICE_FLAG_WAIT_FOR_REPLUG);
	}

	/* success */
	return TRUE;
}

static void
fu_cros_ec_usb_hammer_init(FuCrosEcUsbHammer *self)
{
}

static void
fu_cros_ec_usb_hammer_class_init(FuCrosEcUsbHammerClass *klass)
{
	FuDeviceClass *device_class = FU_DEVICE_CLASS(klass);
	device_class->setup = fu_cros_ec_usb_hammer_setup;
	device_class->write_firmware = fu_cros_ec_usb_hammer_write_base_ec_firmware;
	// device_class->attach = fu_cros_ec_usb_hammer_attach;
	device_class->detach = fu_cros_ec_usb_hammer_detach;
	// device_class->reload = fu_cros_ec_usb_hammer_reload;
}
