project('debug-trigger', 'c',
	license: 'Apache-2.0',
	version: '0.1',
	default_options: [
		'warning_level=3',
		'werror=true',
		'c_std=gnu18',
	])

config = configuration_data()
debug_trigger_deps = []

if get_option('systemd')
	systemd = dependency('systemd')
	unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
	configure_file(input: 'systemd/system/debug-trigger@.service',
		       output: 'debug-trigger@.service',
		       copy: true,
		       install_dir: unitdir)
	debug_trigger_deps += dependency('libsystemd')
endif

config.set10('HAVE_SYSTEMD', get_option('systemd'))
config_h = configure_file(configuration: config, output: 'config.h')

udev = dependency('udev')
udevdir = udev.get_pkgconfig_variable('udevdir')

triggers = get_option('triggers')
foreach trigger : triggers
	src = 'udev/rules.d/debug-trigger-' + trigger + '.rules'
	dst = 'debug-trigger-' + trigger + '.rules'
	rulesdir = udevdir + '/rules.d'
	configure_file(input: src, output: dst, copy: true, install_dir: rulesdir)
endforeach

executable('debug-trigger', 'main.c', config_h,
	   dependencies: debug_trigger_deps,
	   install: true)
