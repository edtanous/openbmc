## This file is a template.  The comment below is emitted
## into the rendered file; feel free to edit this file.
// WARNING: Generated header. Do not edit!
<%
import re
from collections import defaultdict
objects = list(settingsDict.viewkeys())
sdbusplus_namespaces = []
sdbusplus_includes = []
interfaces = []
props = defaultdict(list)
validators = defaultdict(tuple)

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
#include <regex>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include "config.h"
#include <xyz/openbmc_project/Common/error.hpp>

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

namespace persistent
{

// A setting d-bus object /foo/bar/baz is persisted in the filesystem with the
// same path. This eases re-construction of settings objects when we restore
// from the filesystem. This can be a problem though when you have two objects
// such as - /foo/bar and /foo/bar/baz. This is because 'bar' will be treated as
// a file in the first case, and a subdir in the second. To solve this, suffix
// files with a trailing __. The __ is a safe character sequence to use, because
// we won't have d-bus object paths ending with this.
// With this the objects would be persisted as - /foo/bar__ and /foo/bar/baz__.
constexpr auto fileSuffix = "__";

}

% for n in set(sdbusplus_namespaces):
using namespace ${n};
% endfor

% for object in objects:
<%
    intf = settingsDict[object]['Interface']
    interfaces.append(intf)
    if intf not in props:
        for property, property_metadata in settingsDict[object]['Properties'].items():
            props[intf].append(property)
            for attribute, value in property_metadata.items():
                if attribute == 'Validation':
                    if value['Type'] == "range":
                        validators[property] = (value['Type'], value['Validator'], value['Unit'])
                    else:
                        validators[property] = (value['Type'], value['Validator'])
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
<% fname = "validate"+arg %>\
        decltype(std::declval<Base>().${t}()) ${t}(decltype(std::declval<Base>().${t}()) value) override
        {
            auto result = Base::${t}();
            if (value != result)
            {
            % if arg in validators.keys():
                if (!${fname}(value))
                {
                    namespace error =
                        sdbusplus::xyz::openbmc_project::Common::Error;
                    namespace metadata =
                        phosphor::logging::xyz::openbmc_project::Common;
                    phosphor::logging::report<error::InvalidArgument>(
                        metadata::InvalidArgument::ARGUMENT_NAME("${t}"),
                    % if validators[arg][0] != "regex":
                        metadata::InvalidArgument::ARGUMENT_VALUE(std::to_string(value).c_str()));
                    % else:
                        metadata::InvalidArgument::ARGUMENT_VALUE(value.c_str()));
                    % endif
                    return result;
                }
             % endif
                fs::path p(SETTINGS_PERSIST_PATH);
                p /= path;
                p += persistent::fileSuffix;
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
% for arg in props[intf]:
% if arg in validators.keys():
<% funcName = "validate"+arg %>\
<% t = arg[:1].lower() + arg[1:] %>\

        bool ${funcName}(decltype(std::declval<Base>().${t}()) value)
        {
            bool matched = false;
        % if (arg in validators.keys()) and (validators[arg][0] == 'regex'):
            std::regex regexToCheck("${validators[arg][1]}");
            matched = std::regex_search(value, regexToCheck);
            if (!matched)
            {
                std::string err = "Input parameter for ${arg} is invalid "
                    "Input: " + value + " not in the format of this regex: "
                    "${validators[arg][1]}";
                using namespace phosphor::logging;
                log<level::ERR>(err.c_str());
            }
        % elif (arg in validators.keys()) and (validators[arg][0] == 'range'):
<% lowhigh = re.split('\.\.', validators[arg][1]) %>\
            if ((value <= ${lowhigh[1]}) && (value >= ${lowhigh[0]}))
            {
                matched = true;
            }
            else
            {
                std::string err = "Input parameter for ${arg} is invalid "
                    "Input: " + std::to_string(value) + "in uint: "
                    "${validators[arg][2]} is not in range:${validators[arg][1]}";
                using namespace phosphor::logging;
                log<level::ERR>(err.c_str());
            }
        % elif (arg in validators.keys()):
            <% assert("Unknown validation type: arg") %>\
        % endif
            return matched;
        }
% endif
% endfor
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
  % for property, value in settingsDict[object]['Properties'].items():
<% p = property[:1].lower() + property[1:] %>\
<% defaultValue = value['Default'] %>\
            path = fs::path(SETTINGS_PERSIST_PATH) / "${object}";
            path += persistent::fileSuffix;
            if (fs::exists(path))
            {
                std::ifstream is(path.c_str(), std::ios::in);
                cereal::JSONInputArchive iarchive(is);
                iarchive(*std::get<${index}>(settings));
            }
            else
            {
                std::get<${index}>(settings)->
                    ${get_setting_sdbusplus_type(settingsDict[object]['Interface'])}::${p}(${defaultValue});
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
