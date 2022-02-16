#pragma once

#include <util/ffdc_file.hpp>

#include <vector>

namespace util
{

/**
 * Create FDDC files from journal messages of relevant executables
 *
 * Parse the system journal looking for log entries created by the executables
 * of interest for logging. For each of these entries create a ffdc trace file
 * that will be used to create ffdc log entries. These files will be pushed
 * onto the stack of ffdc files.
 *
 * @param   i_files - vector of ffdc files that will become log entries
 */
void createFFDCTraceFiles(std::vector<FFDCFile>& i_files);

} // namespace util
