## This file is a template.  The comment below is emitted
## into the rendered file; feel free to edit this file.
// WARNING: Generated header. Do not edit!
<%
from collections import defaultdict
objects = list(settingsDict.viewkeys())
sdbusplus_namespaces = []
sdbusplus_includes = []
interfaces = []
props = defaultdict(list)

def get_setting_sdbusplus_type(setting_intf):
    setting = "sdbusplus::" + setting_intf.replace('.', '::')
    i = setting.rfind('::')
    setting = setting[:i] + '::server::' + setting[i+2:]
    return setting

def get_setting_type(setting_intf):
    setting = setting_intf.replace('.', '::')
    return setting
%>\
#pragma once

% for object in objects:
<%
    include = settingsDict[object]['Interface']
    include = include.replace('.', '/')
    include = include + "/server.hpp"
    sdbusplus_includes.append(include)
%>\
% endfor
#include <cereal/archives/json.hpp>
#include <fstream>
#include <utility>
#include <experimental/filesystem>
#include "config.h"

% for i in set(sdbusplus_includes):
#include "${i}"
% endfor

% for object in objects:
<%
    ns = get_setting_sdbusplus_type(settingsDict[object]['Interface'])
    i = ns.rfind('::')
    ns = ns[:i]
    sdbusplus_namespaces.append(ns)
%>\
% endfor

namespace phosphor
{
namespace settings
{

namespace fs = std::experimental::filesystem;

% for n in set(sdbusplus_namespaces):
using namespace ${n};
% endfor

% for object in objects:
<%
    intf = settingsDict[object]['Interface']
    interfaces.append(intf)
    if intf not in props:
        for property, value in settingsDict[object]['Defaults'].items():
            props[intf].append(property)
%>\
% endfor
% for intf in set(interfaces):
<%
    ns = intf.split(".")
    sdbusplus_type = get_setting_sdbusplus_type(intf)
%>\
% for n in ns:
namespace ${n}
{
% endfor

using Base = ${sdbusplus_type};
<% parent = "sdbusplus::server::object::object" + "<" + sdbusplus_type + ">" %>\
using Parent = ${parent};

class Impl : public Parent
{
    public:
        Impl(sdbusplus::bus::bus& bus, const char* path):
            Parent(bus, path, true),
            path(path)
        {
        }
        virtual ~Impl() = default;

% for arg in props[intf]:
<% t = arg[:1].lower() + arg[1:] %>\
        decltype(std::declval<Base>().${t}()) ${t}(decltype(std::declval<Base>().${t}()) value) override
        {
            auto result = Base::${t}();
            if (value != result)
            {
                fs::path p(SETTINGS_PERSIST_PATH);
                p /= path;
                fs::create_directories(p.parent_path());
                std::ofstream os(p.c_str(), std::ios::binary);
                cereal::JSONOutputArchive oarchive(os);
                result = Base::${t}(value);
                oarchive(*this);
            }
            return result;
        }
        using Base::${t};

% endfor
    private:
        fs::path path;
};

template<class Archive>
void save(Archive& a,
          const Impl& setting)
{
<%
    args = ["setting." + p[:1].lower() + p[1:] + "()" for p in props[intf]]
    args = ','.join(args)
%>\
    a(${args});
}

template<class Archive>
void load(Archive& a,
          Impl& setting)
{
% for arg in props[intf]:
<% t = "setting." + arg[:1].lower() + arg[1:] + "()" %>\
    decltype(${t}) ${arg}{};
% endfor
<%
    args = ','.join(props[intf])
%>\
    a(${args});
% for arg in props[intf]:
<% t = "setting." + arg[:1].lower() + arg[1:] + "(" + arg + ")" %>\
    ${t};
% endfor
}

% for n in reversed(ns):
} // namespace ${n}
% endfor
% endfor

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
            fs::path path{};
            settings =
                std::make_tuple(
% for index, object in enumerate(objects):
<% type = get_setting_type(settingsDict[object]['Interface']) + "::Impl" %>\
                    std::make_unique<${type}>(
                        bus,
  % if index < len(settingsDict) - 1:
                        "${object}"),
  % else:
                        "${object}"));
  % endif
% endfor

% for index, object in enumerate(objects):
  % for property, value in settingsDict[object]['Defaults'].items():
<% p = property[:1].lower() + property[1:] %>\
            path = fs::path(SETTINGS_PERSIST_PATH) / "${object}";
            if (fs::exists(path))
            {
                std::ifstream is(path.c_str(), std::ios::in);
                cereal::JSONInputArchive iarchive(is);
                iarchive(*std::get<${index}>(settings));
            }
            else
            {
                std::get<${index}>(settings)->
                    ${get_setting_sdbusplus_type(settingsDict[object]['Interface'])}::${p}(${value});
            }
  % endfor
            std::get<${index}>(settings)->emit_object_added();

% endfor
        }

    private:
        /* @brief Composition of settings objects. */
        std::tuple<
% for index, object in enumerate(objects):
<% type = get_setting_type(settingsDict[object]['Interface']) + "::Impl" %>\
  % if index < len(settingsDict) - 1:
            std::unique_ptr<${type}>,
  % else:
            std::unique_ptr<${type}>> settings;
  % endif
% endfor
};

} // namespace settings
} // namespace phosphor
