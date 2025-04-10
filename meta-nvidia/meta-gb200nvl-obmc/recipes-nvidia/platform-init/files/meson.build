project(
    'platform-init',
    'cpp',
    version: '0.1',
    default_options: ['warning_level=3'],
)

gpiodcxx = dependency('libgpiodcxx', default_options: ['bindings=cxx'])

libexec = get_option('prefix') + '/' + get_option('libexecdir')

exe = executable(
    'platform-init',
    'platform_init.cpp',
    dependencies: [gpiodcxx],
    install: true,
    install_dir: libexec,
)

systemd_system_unit_dir = join_paths(get_option('prefix'), 'lib', 'systemd', 'system')
install_data('platform_init.service', install_dir: systemd_system_unit_dir)
