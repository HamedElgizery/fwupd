---
title: Plugin: Cross Hammer
---

## Introduction

The Hammer is a bla bla bla.

## Firmware Format

The daemon will decompress the cabinet archive and extract a firmware blob in
a packed binary file format.

This plugin supports the following protocol ID:

* `com.cross.hammer`

## GUID Generation

These devices use the standard USB DeviceInstanceId values, e.g.

* `USB\ID_XXX`
* `USB\VID_273F&PID_1001`

## Update Behavior

The device is updated by bla bla bla.

## Vendor ID Security

The vendor ID is set from the USB vendor, in this instance set to `USB:0x273F`

## Quirk Use

This plugin uses the following plugin-specific quirks:

### CrosHammerStartAddr

The bla bla bla.

Since: 1.8.TODO

## External Interface Access

This plugin requires read/write access to `/dev/bus/usb`.

## Version Considerations

This plugin has been available since fwupd version `SET_VERSION_HERE`.

## Owners

Anyone can submit a pull request to modify this plugin, but the following people should be
consulted before making major or functional changes:

* Cross: @github-username
