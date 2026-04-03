project('kron', 'c',
  version: '0.1.0',
  default_options: ['warning_level=2', 'c_std=c11']
)

wayland_scanner = find_program('wayland-scanner')
wayland_client  = dependency('wayland-client')
wayland_protos  = dependency('wayland-protocols')

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

# Генерируем layer-shell protocol header
wl_protos_dir = wayland_protos.get_variable('pkgdatadir')
layer_shell_xml = join_paths(
  wl_protos_dir, 'staging/ext-session-lock/ext-session-lock-v1.xml'
)

# wlr-layer-shell из wlroots
wlroots_dir = run_command('pkg-config', '--variable=pkgdatadir', 'wlroots-0.18',
  check: false).stdout().strip()

ls_xml = 'protocols/wlr-layer-shell-unstable-v1.xml'

ls_header = custom_target('ls-header',
  input:  ls_xml,
  output: 'wlr-layer-shell-unstable-v1-protocol.h',
  command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

# xdg-shell protocol header (нужен wlroots)
xdg_shell_xml = '/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml'
xdg_shell_header = custom_target('xdg-shell-header',
  input:  xdg_shell_xml,
  output: 'xdg-shell-protocol.h',
  command: [wayland_scanner, 'server-header', '@INPUT@', '@OUTPUT@'],
)

# ── Зависимости compositor ──
wlroots        = dependency('wlroots-0.18')
wayland_server = dependency('wayland-server')
xkbcommon      = dependency('xkbcommon')

# ── Зависимости shell ──
gtk4        = dependency('gtk4')
layer_shell = dependency('gtk4-layer-shell-0')
gio         = dependency('gio-2.0')

compositor_inc = include_directories('compositor', '.')

# ── Compositor (kron) ──
executable('kron',
  files(
    'compositor/main.c',
    'compositor/server.c',
    'compositor/output.c',
    'compositor/toplevel.c',
    'compositor/input.c',
    'compositor/animation.c',
  ),
  [ls_header, xdg_shell_header],
  include_directories: compositor_inc,
  c_args: ['-DWLR_USE_UNSTABLE'],
  dependencies: [wlroots, wayland_server, xkbcommon, dependency('m')],
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
    'shell/desktop.c',
    'shell/dock.c',
  ),
  [ft_header, ft_code],
  include_directories: include_directories('.'),
  dependencies: [gtk4, layer_shell, gio, wayland_client],
  install: true,
)

install_data('data/kron.desktop', install_dir: '/usr/share/wayland-sessions')
install_data('data/style.css',    install_dir: '/usr/share/kron')
install_data('data/wallpaper.jpg', install_dir: '/usr/share/kron')
