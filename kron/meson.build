project('kron', 'c',
  version: '0.1.0',
  default_options: ['warning_level=2', 'c_std=c11']
)

wayland_scanner = find_program('wayland-scanner')
wayland_client  = dependency('wayland-client')

# Генерируем код из xml протокола foreign-toplevel
foreign_toplevel_xml = 'protocols/wlr-foreign-toplevel-management-unstable-v1.xml'

ft_header = custom_target('ft-header',
  input:  foreign_toplevel_xml,
  output: 'wlr-foreign-toplevel-management-unstable-v1-client.h',
  command: [wayland_scanner, 'client-header', '@INPUT@', '@OUTPUT@'],
)
ft_code = custom_target('ft-code',
  input:  foreign_toplevel_xml,
  output: 'wlr-foreign-toplevel-management-unstable-v1-client.c',
  command: [wayland_scanner, 'private-code', '@INPUT@', '@OUTPUT@'],
)

# ── Зависимости compositor ──
wlroots        = dependency('wlroots-0.18')
wayland_server = dependency('wayland-server')
xkbcommon      = dependency('xkbcommon')

# ── Зависимости shell ──
gtk4        = dependency('gtk4')
layer_shell = dependency('gtk4-layer-shell-0')
gio         = dependency('gio-2.0')

# ── Compositor (kron) ──
executable('kron',
  files(
    'compositor/main.c',
    'compositor/server.c',
    'compositor/output.c',
    'compositor/toplevel.c',
    'compositor/input.c',
  ),
  dependencies: [wlroots, wayland_server, xkbcommon],
  install: true,
)

# ── Shell (kron-shell) ──
executable('kron-shell',
  files(
    'shell/main.c',
    'shell/panel.c',
    'shell/launcher.c',
    'shell/taskbar.c',
    'shell/notifications.c',
  ),
  [ft_header, ft_code],
  include_directories: include_directories('.'),
  dependencies: [gtk4, layer_shell, gio, wayland_client],
  install: true,
)

install_data('data/kron.desktop', install_dir: '/usr/share/wayland-sessions')
install_data('data/style.css',    install_dir: '/usr/share/kron')
