cargs = ['-DG_LOG_DOMAIN="FuPluginCrosHammer"']

plugins += {meson.current_source_dir().split('/')[-1]: true}
plugin_quirks += files('cros-hammer.quirk')
plugin_builtins += static_library('fu_plugin_cros_hammer',
  rustgen.process('fu-cros-hammer.rs'),
  sources: [
    'fu-cros-hammer-common.c',
    'fu-cros-hammer-device.c',
    'fu-cros-hammer-firmware.c',
    'fu-cros-hammer-plugin.c',
  ],
  include_directories: plugin_incdirs,
  link_with: plugin_libs,
  c_args: cargs,
  dependencies: plugin_deps,
)

device_tests += files(
  'tests/cros-hammer.json',
  'tests/cros-hammer-setup.json',
)
