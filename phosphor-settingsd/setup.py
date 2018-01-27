from distutils.core import setup
from distutils.command.build import build
import yaml
import pprint


class CustomBuild(build):
    def run(self):
        build.run(self)

        SETTINGS_FILE = 'settings.yaml'
        OUTPUT_FILE = 'settings_file.py'

        with open(SETTINGS_FILE) as s:
            data = yaml.safe_load(s)

        with open(OUTPUT_FILE, 'w') as f:
            f.write('SETTINGS=\\\n')
            pprint.pprint(data, stream=f)


setup(
    name='phosphor-settings',
    version='1.0',
    scripts=['settings_manager.py'],
    data_files=[('phosphor-settings', ['settings_file.py'])],
    cmdclass={'build': CustomBuild})

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
