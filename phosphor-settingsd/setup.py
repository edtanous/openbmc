from distutils.core import setup

setup(name='obmc-phosphor-settings',
      version='1.0',
      scripts=['settings_manager.py'],
      data_files=[('obmc-phosphor-settings', ['settings_file.py'])],
      )
