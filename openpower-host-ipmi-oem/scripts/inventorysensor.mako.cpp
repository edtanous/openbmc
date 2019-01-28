## This file is a template.  The comment below is emitted
## into the rendered file; feel free to edit this file.

// !!! WARNING: This is a GENERATED Code..Please do NOT Edit !!!

#include "oemhandler.hpp"

extern const ObjectIDMap invSensors = {
% for key in sensorDict.iterkeys():
   % if key:
{"${key}",
    {
<%
       objectPath = sensorDict[key]
       sensorID = objectPath["sensorID"]
       sensorType = objectPath["sensorType"]
       eventReadingType = objectPath["eventReadingType"]
       offset = objectPath["offset"]
%>
        ${sensorID},${sensorType},${eventReadingType},${offset}
    }
},
   % endif
% endfor
};

