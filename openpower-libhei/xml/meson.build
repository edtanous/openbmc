#-------------------------------------------------------------------------------
# Chip Data Files
#-------------------------------------------------------------------------------

build_cdb = find_program('build_chip_data_binary')

# The key for each entry in this dictionary is a subdirectory containing XML for
# a chip model. The value for each entry contains the expected output files that
# will be produced for each chip model. It is important to note that the script
# will generate all output files, regardless of what is listed, when the script
# is run. However, this list must be kept in sync with the expected output so
# that meson will know to run the script when an output file has changed or is
# missing.
cdb_files = {
    'p10' :
    [
        'chip_data_p10_10.cdb',
    ],

    'explorer' :
    [
        'chip_data_explorer_10.cdb',
    ],
}

foreach chip_dir, out_files : cdb_files

    source_dir = meson.current_source_dir() + '/' + chip_dir
    build_dir  = meson.current_build_dir()

    # Get all XML files in the chip directory. This is a bit of a workaround
    # because meson does not allow wildcards.
    xml_list = run_command('xml_list.sh', source_dir)
    in_files = xml_list.stdout().strip().split('\n')

    custom_target('build_cdb_' + chip_dir, build_by_default : true,
                  input : in_files, output : out_files,
                  command : [ build_cdb, '-i', source_dir, '-o', build_dir ],
                  install : true,
                  install_dir : join_paths(get_option('prefix'),
                                           get_option('datadir'),
                                           meson.project_name()))

endforeach

