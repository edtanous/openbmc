## This file is a template.  The comment below is emitted
## into the rendered file; feel free to edit this file.
// WARNING: Generated header. Do not edit!

<%
objects = list(settingsDict.viewkeys())

def get_setting_type(setting_intf):
    setting = "sdbusplus::" + setting_intf.replace('.', '::')
    i = setting.rfind('::')
    setting = setting[:i] + '::server::' + setting[i+2:]
    return setting
%>\
#pragma once

% for object in objects:
<%
    includes = []
    include = settingsDict[object]['Interface']
    include = include.replace('.', '/')
    include = include + "/server.hpp"
    includes.append(include)
%>\
% endfor
% for i in set(includes):
#include "${i}"
% endfor

% for object in objects:
<%
    ns_list = []
    ns = get_setting_type(settingsDict[object]['Interface'])
    i = ns.rfind('::')
    ns = ns[:i]
    ns_list.append(ns)
%>\
% endfor
% for n in set(ns_list):
using namespace ${n};
% endfor

namespace phosphor
{
namespace settings
{

/** @class Manager
 *
 *  @brief Compose settings objects and put them on the bus.
 */
class Manager
{
    public:
        Manager() = delete;
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;
        Manager(Manager&&) = delete;
        Manager& operator=(Manager&&) = delete;
        virtual ~Manager() = default;

        /** @brief Constructor to put settings objects on to the bus.
         *  @param[in] bus - Bus to attach to.
         */
        Manager(sdbusplus::bus::bus& bus)
        {
            settings =
                std::make_tuple(
% for index, object in enumerate(objects):
<% type = get_setting_type(settingsDict[object]['Interface']) %>\
                    std::make_unique<${type}>(
                        bus,
  % if index < len(settingsDict) - 1:
                        "${object}"),
  % else:
                        "${object}"));
  % endif
% endfor

% for index, object in enumerate(objects):
  % if 'Defaults' in settingsDict[object].viewkeys():
    % for property, value in settingsDict[object]['Defaults'].items():
            std::get<${index}>(settings)->
                setPropertyByName("${property}", ${value});
    % endfor
  % endif
            bus.emit_object_added("${object}");

% endfor
        }

    private:
        /* @brief Composition of settings objects. */
        std::tuple<
% for index, object in enumerate(objects):
<% type = get_setting_type(settingsDict[object]['Interface']) %>\
  % if index < len(settingsDict) - 1:
            std::unique_ptr<${type}>,
  % else:
            std::unique_ptr<${type}>> settings;
  % endif
% endfor
};

} // namespace settings
} // namespace phosphor
