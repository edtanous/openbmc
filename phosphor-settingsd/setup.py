from distutils.core import setup

setup(
    name='phosphor-settings',
    version='1.0',
    scripts=['settings_manager.py'],
    data_files=[('phosphor-settings', ['settings_file.py'])])
