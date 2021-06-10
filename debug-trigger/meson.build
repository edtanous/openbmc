project('debug-trigger', 'c', license: 'Apache-2.0', version: '0.1')
executable('debug-trigger', 'main.c', install: true)

if get_option('systemd')
	systemd = dependency('systemd')
	unitdir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
	configure_file(input: 'systemd/system/debug-trigger@.service',
		       output: 'debug-trigger@.service',
		       copy: true,
		       install_dir: unitdir)
endif

udev = dependency('udev')
udevdir = udev.get_pkgconfig_variable('udevdir')
udevrules = [
	'debug-trigger-kcs1',
	'debug-trigger-kcs2',
	'debug-trigger-kcs3',
	'debug-trigger-kcs4',
]

triggers = get_option('triggers')
foreach trigger : triggers
	src = 'udev/rules.d/debug-trigger-' + trigger + '.rules'
	dst = 'debug-trigger-' + trigger + '.rules'
	rulesdir = udevdir + '/rules.d'
	configure_file(input: src, output: dst, copy: true, install_dir: rulesdir)
endforeach
