# some of these are controlled by meson tristate features
plugin_deps = [
  gio,
  giounix,
  gnutls,
  gmodule,
  libusb,
  libarchive,
  libjsonglib,
  libxmlb,
  libdrm_amdgpu,
  protobufc,
  fwupdplugin_rs_dep,
]

plugins = {
  'uefi-capsule': false,
  'acpi-dmar': false,
  'acpi-facp': false,
  'acpi-ivrs': false,
  'acpi-phat': false,
  'algoltek-usb': false,
  'algoltek-usbcr': false,
  'amd-kria': false,
  'amd-pmc': false,
  'amd-gpu': false,
  'analogix': false,
  'android-boot': false,
  'asus-hid': false,
  'ata': false,
  'aver-hid': false,
  'bcm57xx': false,
  'bios': false,
  'bnr-dp': false,
  'ccgx': false,
  'ccgx-dmc': false,
  'cfu': false,
  'ch341a': false,
  'ch347': false,
  'corsair': false,
  'cpu': false,
  'cros-ec': false,
  'dell': false,
  'dell-dock': false,
  'dell-kestrel': false,
  'dfu': false,
  'ebitdo': false,
  'elantp': false,
  'elanfp': false,
  'elan-kbd': false,
  'emmc': false,
  'ep963x': false,
  'fastboot': false,
  'flashrom': false,
  'focalfp': false,
  'fpc': false,
  'fresco-pd': false,
  'genesys': false,
  'genesys-gl32xx': false,
  'goodix-moc': false,
  'goodix-tp': false,
  'gpio': false,
  'hpi-cfu': false,
  'huddly-usb': false,
  'hughski-colorhug': false,
  'intel-amt': false,
  'intel-cvs': false,
  'intel-gsc': false,
  'intel-mchi': false,
  'intel-mkhi': false,
  'intel-usb4': false,
  'iommu': false,
  'jabra': false,
  'jabra-file': false,
  'jabra-gnp': false,
  'kinetic-dp': false,
  'legion-hid2': false,
  'lenovo-thinklmi': false,
  'linux-display': false,
  'linux-lockdown': false,
  'linux-sleep': false,
  'linux-swap': false,
  'linux-tainted': false,
  'logind': false,
  'logitech-hidpp': false,
  'logitech-bulkcontroller': false,
  'logitech-rallysystem': false,
  'logitech-scribe': false,
  'logitech-tap': false,
  'mediatek-scaler': false,
  'modem-manager': false,
  'msr': false,
  'mtd': false,
  'nordic-hid': false,
  'nvme': false,
  'parade-lspcon': false,
  'parade-usbhub': false,
  'pci-bcr': false,
  'pci-mei': false,
  'pci-psp': false,
  'pixart-rf': false,
  'powerd': false,
  'qc-firehose': false,
  'qc-s5gen2': false,
  'qsi-dock': false,
  'realtek-mst': false,
  'redfish': false,
  'rp-pico': false,
  'rts54hub': false,
  'steelseries': false,
  'scsi': false,
  'synaptics-cape': false,
  'synaptics-cxaudio': false,
  'synaptics-mst': false,
  'synaptics-prometheus': false,
  'synaptics-rmi': false,
  'synaptics-vmm9': false,
  'system76-launch': false,
  'test': false,
  'telink-dfu':false,
  'thelio-io': false,
  'thunderbolt': false,
  'ti-tps6598x': false,
  'tpm': false,
  'uefi-db': false,
  'uefi-dbx': false,
  'uefi-esrt': false,
  'uefi-kek': false,
  'uefi-mok': false,
  'uefi-pk': false,
  'uefi-recovery': false,
  'uefi-sbat': false,
  'uf2': false,
  'upower': false,
  'usi-dock': false,
  'vbe': false,
  'vli': false,
  'wacom-raw': false,
  'wacom-usb': false,
  'wistron-dock': false,
}

umockdev_tests = []
umockdev_ioctls = []
device_tests = []
enumeration_data = []
foreach plugin, enabled: plugins
  subdir(plugin)
endforeach

if umockdev_integration_tests.allowed()
  envs = environment()
  envs.set('CACHE_DIRECTORY', join_paths(meson.project_build_root(), 'cache'))
  envs.set('DAEMON_BUILDDIR', join_paths(meson.project_build_root(), 'src'))
  envs.set('FWUPD_DATADIR_QUIRKS', join_paths(meson.project_build_root()))
  envs.set('GI_TYPELIB_PATH', join_paths(meson.project_build_root(), 'libfwupd'))
  envs.set('LD_LIBRARY_PATH', join_paths(meson.project_build_root(), 'libfwupd'))
  envs.set('PYTHONPATH', join_paths(meson.project_source_root(), 'data', 'tests'))
  envs.set('STATE_DIRECTORY', join_paths(meson.project_build_root(), 'state'))

  foreach suite: umockdev_tests
    r = run_command(unittest_inspector, suite,
                    check: true, env: envs)
    unit_tests = r.stdout().strip().split('\n')
    foreach ut: unit_tests
        test(ut, python3, args: [suite, ut], is_parallel: false, env: envs)
    endforeach
    install_data(suite,
      install_dir: installed_test_datadir,
    )
  endforeach
  foreach ioctl: umockdev_ioctls
    install_data(ioctl,
      install_dir: installed_test_datadir,
    )
  endforeach
endif
