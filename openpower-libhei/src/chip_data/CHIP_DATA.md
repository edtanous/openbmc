# Chip Data Files

The Chip Data Files define everything we need to know about a chip type. Their
purpose is to keep libhei agnostic to specific chip information. Required
information includes:
 * All hardware addresses needed for error isolation.
 * A definition of how errors propagate from register to register.
 * A list of top level registers to use as a starting point for isolation.
 * A list of additional registers to capture for each register bit (for debug).
 * Rules defining how to clear and mask register bits (not available if
   `__HEI_READ_ONLY` is defined).

File extensions are not required, but it is recommended to use the
extension `.cdb` (chip data binary).

## Requirements

 * These files must be consumable by different host architectures. So the data
   fields within the files will be stored in big-endian format (endian.h).

## File Format

### 1) File metadata

The following data will be defined at the very beginning of each Chip Data File:

| Bytes |         Desc        |               Value/Example                |
|:-----:|:-------------------:|:------------------------------------------:|
|     8 | magic number        | 0x4348495044415441 (ascii for "CHIPDATA")  |
|     4 | chip model/level    | Unique ID defined by data file owner [^1]  |
|     1 | file version        | Version 1 => 0x01, Version 2 => 0x02, etc. |
|     1 | # of register types | 1-255 (0 is invalid) [^2]                  |
|     2 | reserved            | Initialized to 0                           |

[^1]: The user application will use this field to determine which Chip Data
      File(s) should be used for the chip(s) that exist in the user
      application's system.

[^2]: A chip can have more than one register type and the properties for each
      type could vary. Therefore, all of the registers described by this file
      will be grouped by type.

### 2) Register Type Lists

These lists will immediately following the File metadata. The number of lists
that exist in a file can be found in the File metadata. There should only be
one list per supported register type. Only the following register types are
supported:

 * Power Systems SCOM register
    * Type value:    0x01
    * Address size:  4 bytes
    * Register size: 8 bytes
    * Bit order:     ascending (0-63, left to right)

 * Power Systems Indirect SCOM register
    * Type value:    0x02
    * Address size:  8 bytes
    * Register size: 8 bytes
    * Bit order:     ascending (0-63, left to right)

#### 2.1) Register Type List metadata

Each list will start with the following data:

| Bytes |          Desc          |               Value/Example               |
|:-----:|:----------------------:|:-----------------------------------------:|
|     1 | register type          | See type value for supported types above  |
|     2 | # of registers in list | 1-65535 (0 is invalid)                    |
|     1 | reserved               | Initialized to 0                          |

#### 2.2) Registers

The list of all registers of a specific type will immediately following the
Register Type List metadata. The number of registers in the list is found in
that metadata.

#### 2.2.1) Register metadata

| Bytes |          Desc          |               Value/Example                |
|:-----:|:----------------------:|:------------------------------------------:|
|     2 | register ID            | Unique ID defined by data file owner [^3]  |
|     1 | # of address instances | 1-255 (0 is invalid) [^4]                  |
|     1 | attribute flags        | for each bit 0:disabled and 1:enabled [^5] |

[^3]: This ID must be unique for all registers among all register types within a
      Chip Data File. The value will be used as part of the error signature for
      each reported error.

[^4]: A register could have multiple instances within a chip. See the Register
      Instance Address metadata for details.

[^5]: Supported attributes (bits ordered 0-7, left to right):
      0:   When enabled, the register is readable.
      1:   When enabled, the register is writable.
      2-7: Reserved (default disabled)

##### 2.2.2) Register Instance metadata

A register could have multiple instances within a chip. For example, the same
register could exist for each core on a processor chip. Each instance will
share some information, but will also have some differences (e.g. the address
value). Therefore, the following metadata will exist for each instance of the
register.

| Bytes |    Desc    |                     Value/Example                     |
|:-----:|:----------:|:-----------------------------------------------------:|
|     1 | instance # | 0-x, where x is one less than the # of instances [^6] |
|     y | address    | The address size y is defined by the register type    |

[^6]: This will be used as part of the error signature for each reported error.

### 3) Isolation Tree

**TBD:**

## Appendix

### 1) Expressions

Expressions are used in various locations within a Chip Data File. They can be
used to characterize operations carried out against registers and/or integer
constants. For example, `<some_register> & (0xffff << 16)`.

The first byte of every expression is the expression type. The data immediately
following this field is dependent on the expression type.

#### 1.1) Non-recursive expressions

##### 1.1.1) Register reference expression

This is a special expression that indicates the value of the target register
should be used in this expression. Generally, this mean reading the register
value from hardware.

The following is the complete byte definition of the expression:

| Bytes |                    Description/Value                     |
|:-----:|:--------------------------------------------------------:|
|     1 | expression type = 0x01                                   |
|     2 | register ID, see 2.2.1) Register metadata                |
|     1 | register instance, see 2.2.2) Register Instance metadata |

As you can see, the register ID and instance can be used to find this register's
metadata (e.g. the address) from the register lists (see section 2).

##### 1.1.2) Integer constant expression

This simply contains an unsigned 64-bit integer constant.

The following is the complete byte definition of the expression:

| Bytes |          Description/Value          |
|:-----:|:-----------------------------------:|
|     1 | expression type = 0x02              |
|     8 | An unsigned 64-bit integer constant |

#### 1.2) Recursive expressions

These expressions will contain other expressions. Each sub-expression will
always be resolved before handling the containing expression. For example, say
we need to do something like:

    `REG_1 & ~REG_2 | CONST_1`

Where `REG_*` are register reference expressions and `CONST_*` are integer
constant expressions. Following standard C++ order of operations, that would be
evaluate into an expression like:

    `OR( AND( REG_1, NOT(REG_2) ), CONST_1 )`

Where the NOT will be evaluated first, then the AND, then finally the OR.

##### 1.2.1) AND expression

A bitwise AND operation (i.e. `EXPR_1 & EXPR_2`).

| Bytes |   Description/Value    |
|:-----:|:----------------------:|
|     1 | expression type = 0x10 |
|     * | left sub-expression    |
|     * | right sub-expression   |

##### 1.2.2) OR expression

A bitwise OR operation (i.e. `EXPR_1 | EXPR_2`).

| Bytes |   Description/Value    |
|:-----:|:----------------------:|
|     1 | expression type = 0x11 |
|     * | left sub-expression    |
|     * | right sub-expression   |

##### 1.2.3) NOT expression

A bitwise NOT operation (i.e. `~EXPR`).

| Bytes |   Description/Value    |
|:-----:|:----------------------:|
|     1 | expression type = 0x12 |
|     * | sub-expression         |

##### 1.2.4) Left shift expression

A left shift operation (i.e. `EXPR << shift_value`).

| Bytes |   Description/Value    |
|:-----:|:----------------------:|
|     1 | expression type = 0x13 |
|     1 | Shift value            |
|     * | sub-expression         |

##### 1.2.5) Right shift expression

A left shift operation (i.e. `EXPR >> shift_value`).

| Bytes |   Description/Value    |
|:-----:|:----------------------:|
|     1 | expression type = 0x14 |
|     1 | Shift value            |
|     * | sub-expression         |

