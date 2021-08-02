
subdir('p10')      # for p10_data_files
subdir('explorer') # for explorer_data_files

data_files = [
    p10_data_files,
    explorer_data_files,
]

install_data( data_files, install_dir : join_paths(get_option('prefix'),
                                                   get_option('datadir'),
                                                   meson.project_name()) )

