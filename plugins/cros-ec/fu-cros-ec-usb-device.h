/*
 * Copyright 2020 Benson Leung <bleung@chromium.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <fwupdplugin.h>

#include "fu-cros-ec-firmware.h"

#define FU_CROS_EC_SETUP_RETRY_CNT 5

#define FU_TYPE_CROS_EC_USB_DEVICE (fu_cros_ec_usb_device_get_type())
G_DECLARE_DERIVABLE_TYPE(FuCrosEcUsbDevice,
			 fu_cros_ec_usb_device,
			 FU,
			 CROS_EC_USB_DEVICE,
			 FuUsbDevice)

struct _FuCrosEcUsbDeviceClass {
	FuUsbDeviceClass parent_class;
};

#define FU_CROS_EC_USB_SUBCLASS_GOOGLE_UPDATE 0x53
#define FU_CROS_EC_USB_PROTOCOL_GOOGLE_UPDATE 0xff

#define FU_CROS_EC_USB_DEVICE_FLAG_RO_WRITTEN		     "ro-written"
#define FU_CROS_EC_USB_DEVICE_FLAG_RW_WRITTEN		     "rw-written"
#define FU_CROS_EC_USB_DEVICE_FLAG_REBOOTING_TO_RO	     "rebooting-to-ro"
#define FU_CROS_EC_USB_DEVICE_FLAG_SPECIAL		     "special"
#define FU_CROS_EC_USB_DEVICE_FLAG_HAS_TOUCHPAD		     "has-touchpad"
#define FU_CROS_EC_USB_DEVICE_FLAG_CMD_BLOCK_DIGEST_REQUIRED "block-digest-required"

guint32
fu_cros_ec_usb_device_get_flash_protection(FuCrosEcUsbDevice *self);

gboolean
fu_cros_ec_usb_device_get_in_bootloader(FuCrosEcUsbDevice *self);

gboolean
fu_cros_ec_usb_device_transfer_block_cb(FuDevice *device, gpointer user_data, GError **error);

gboolean
fu_cros_ec_usb_device_start_request_cb(FuDevice *device, gpointer user_data, GError **error);

gboolean
fu_cros_ec_usb_device_recovery(FuCrosEcUsbDevice *self, GError **error);

gboolean
fu_cros_ec_usb_device_setup(FuDevice *device, GError **error);

void
fu_cros_ec_usb_device_send_done(FuCrosEcUsbDevice *self);

gboolean
fu_cros_ec_usb_device_send_subcommand(FuCrosEcUsbDevice *self,
				      guint16 subcommand,
				      guint8 *cmd_body,
				      gsize body_size,
				      guint8 *resp,
				      gsize *resp_size,
				      gboolean allow_less,
				      GError **error);

gboolean
fu_cros_ec_usb_device_unlock_rw(FuCrosEcUsbDevice *self, GError **error);

gboolean
fu_cros_ec_usb_device_transfer_section(FuCrosEcUsbDevice *self,
				       FuFirmware *firmware,
				       FuCrosEcFirmwareSection *section,
				       FuProgress *progress,
				       GError **error);
gboolean
fu_cros_ec_usb_device_stay_in_ro(FuCrosEcUsbDevice *self, GError **error);

void
fu_cros_ec_usb_device_reset_to_ro(FuCrosEcUsbDevice *self);

gboolean
fu_cros_ec_usb_device_jump_to_rw(FuCrosEcUsbDevice *self);
