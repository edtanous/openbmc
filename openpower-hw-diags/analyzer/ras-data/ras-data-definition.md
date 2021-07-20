# RAS Data File definition

When hardware reports an error, the analyzer will call the isolator which will
return a list of active attentions in the hardware, referred to as `signatures`.
The analyzer will then filter and sort the list to find the root cause
signature. The RAS Data files are used to define, in a data driven fashion, the
appropriate RAS actions that should be taken for the root cause signature.

The RAS Data will be defined in the JSON data format. Each file will contain a
single JSON object (with nested values) which will define the RAS actions for a
single chip model and EC level.

## 1) `model_ec` keyword (required)

The value of this keyword is a `string` representing a 32-bit hexidecimal number
in the format `[0-9A-Fa-f]{8}`. This value is used to determine the chip model
and EC level in which this data is defined.

## 2) `version` keyword (required)

A new version number should be used for each new RAS data file format so that
user applications will know how to properly parse the files. The value of this
keyword is a positive integer. Initially, format version is `1`.

## 3) `units` keyword

The value of this keyword is a JSON object representing all of the guardable
unit targets on this chip. Each element of this object will have the format:

```
"<unit_name>" : "<relative_devtree_path>"
```

Where `<unit_name>` is simply an alphanumeric label for the unit and
`<relative_devtree_path>` is a string representing the devtree path of the unit
relative to the chip defined by the file. When necessary, the user application
should be able to concatenate the devtree path of the chip and the relative
devtree path of the unit to get the full devtree path of the unit.

## 4) `buses` keyword

The value of this keyword is a JSON object representing all of the buses
connected to this chip. Each element of this object will have the format:

```
"<bus_name>" : { <bus_details> }
```

Where `<bus_name>` is simply an alphanumeric label for the bus and
`<bus_details>` is a JSON object containing details of the bus connection.

### 4.1) `<bus_details>` object

This describes how the bus is connected to this chip. Note that the `unit`
keyword is optional and the chip is used as the endpoint connection instead.
This is usually intended to be used when the chip is the child and we need to
find the connected `parent` chip/unit.

| Keyword | Description                                                        |
|---------|--------------------------------------------------------------------|
| type    | The bus connection type. Values (string): `peer`, `parent`, `child`|
| unit    | Optional. The `<unit_name>` of the bus endpoint on this chip.      |

## 5) `actions` keyword (required)

The value of this keyword is a JSON object representing all of the defined
actions available for the file. Each element of this object contains an array of
RAS actions, to be performed in order, with the format:

```
"<action_name>" : [ { <action_element> }, ... ]
```

Where `<action_name>` is simply an alphanumeric label for a set of actions.
This will be the keyword referenced by the `signatures` or by a special
`<action_element>` for nested actions (see below).

### 5.1) `<action_element>` object

All `<action_element>` are JSON objects and they all require the `type` keyword,
which is used to determine the action type. The remaining required keywords are
dependent on the action type.

Actions with a `priority` keyword can only use the following values (string):

| Priority | Description                                                       |
|----------|-------------------------------------------------------------------|
| `HIGH`   | Serivce is mandatory.                                             |
| `MED`    | Service one at a time, in order, until issue is resolved.         |
| `MED_A`  | Same as `MED` except all in group A replaced at the same time.    |
| `MED_B`  | Same as `MED` except all in group B replaced at the same time.    |
| `MED_C`  | Same as `MED` except all in group C replaced at the same time.    |
| `LOW`    | Same as `MED*`, but only if higher priority service does not work.|

Actions with a `guard` keyword can only use the following values (boolean):

| Guard | Description                                                          |
|-------|----------------------------------------------------------------------|
| true  | Request guard on associated part.                                    |
| false | No guard request.                                                    |

#### 5.1.1) action type `action`

This is a special action type that allows using an action that has already been
defined (nested actions).

| Keyword | Description                                                        |
|---------|--------------------------------------------------------------------|
| type    | value (string): `action`                                           |
| name    | The `<action_name>` of a previously predefined action.             |

#### 5.1.2) action type `callout_self`

This will request to callout the chip defined by this file.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_self`                                    |
| priority | See `priority` table above.                                       |
| guard    | See `guard` table above.                                          |

#### 5.1.3) action type `callout_unit`

This will request to callout a unit of the chip defined by this file.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_unit`                                    |
| name     | The `<unit_name>` as defined by the `units` keyword.              |
| priority | See `priority` table above.                                       |
| guard    | See `guard` table above.                                          |

#### 5.1.4) action type `callout_connected`

This will request to callout a connected chip/unit on the other side of a bus.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_connected`                               |
| name     | The `<bus_name>` as defined by the `buses` keyword.               |
| priority | See `priority` table above.                                       |
| guard    | See `guard` table above.                                          |

#### 5.1.5) action type `callout_bus`

This will request to callout all parts associated with a bus (RX/TX endpoints
and everything else in between the endpoints). Bus callouts have very specific
priority:

- If an SMP cable exists, callout the cable with priority `HIGH`.
- Callout both RX and TX endpoints with priority `MED_A`.
- Callout everything else in between with priority `LOW`.

In some special cases, the callout priority of the endpoints may differ from the
default `MED_A`. In which case, the optional priority properties can be used.

| Keyword     | Description                                                    |
|-------------|----------------------------------------------------------------|
| type        | value (string): `callout_bus`                                  |
| name        | The `<bus_name>` as defined by the `buses` keyword.            |
| rx_priority | Optional priority of the receiving side endpoint               |
| tx_priority | Optional priority of the transfer side endpoint                |
| name        | The `<bus_name>` as defined by the `buses` keyword.            |
| guard       | See `guard` table above.                                       |

#### 5.1.6) action type `callout_clock`

This will request to callout a clock associated with this chip.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_clock`                                   |
| position | value (integer): 0 or 1                                           |
| priority | See `priority` table above.                                       |
| guard    | See `guard` table above.                                          |

#### 5.1.7) action type `callout_procedure`

This will request to callout a service procedure.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_procedure`                               |
| name     | The `procedures` table below.                                     |
| priority | See `priority` table above.                                       |

Supported procedures:

| Procedure | Description                                                      |
|-----------|------------------------------------------------------------------|
| LEVEL2    | Request Level 2 support                                          |

#### 5.1.8) action type `callout_part`

This will request special part callouts that cannot be managed by the other
callout actions (e.g. the PNOR).

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `callout_part`                                    |
| name     | The `parts` table below.                                          |
| priority | See `priority` table above.                                       |

Supported parts:

| Part     | Description                                                       |
|----------|-------------------------------------------------------------------|

#### 5.1.9) action type `plugin`

Some RAS actions require additional support that cannot be defined easily in
these data files. User application can defined plugins to perform these
additional tasks. Use of this keyword should be avoided if possible. Remember,
the goal is to make the user applications as data driven as possible to avoid
platform specific code.

| Keyword  | Description                                                       |
|----------|-------------------------------------------------------------------|
| type     | value (string): `plugin`                                          |
| name     | A string representing the plugin name.                            |

### 5.2) `actions` example

```json
    "actions" : {
        "self_L" : [
            {
                "type"     : "callout_self",
                "priority" : "LOW",
                "guard"    : false
            },
        ],
        "level2_M_self_L" : [
            {
                "type"     : "callout_procedure",
                "name"     : "LEVEL2",
                "priority" : "MED"
            },
            {
                "type" : "action",
                "name" : "self_L"
            }
        ]
    }
```

## 6) `signatures` keyword (required)

The value of this keyword is a JSON object representing all of the signatures
from this chip requiring RAS actions. Each element of this object will have the
format:

```
"<sig_id>" : { "<sig_bit>" : { "<sig_inst>" : "<action_name>", ... }, ... }
```

Where `<sig_id>` (16-bit), `<sig_bit>` (8-bit), and `<sig_inst>` (8-bit) are
lower case hexadecimal values with NO preceeding '0x'. See the details of these
fields in the isolator's `Signature` object. The `<action_name>` is a label
defined in by the `actions` keyword above.
