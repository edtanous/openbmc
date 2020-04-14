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

 * These files must be consumable by different host architectures. So all data
   fields within the files will be stored in big-endian format (use endian.h).

## File Format

### 1) File metadata

The following data will be defined at the very beginning of each Chip Data File:

| Bytes |         Desc         |               Value/Example                |
|:-----:|:---------------------|:-------------------------------------------|
|     8 | chip data keyword    | 0x4348495044415441 (ascii for "CHIPDATA")  |
|     4 | chip model/level     | Unique ID defined by data file owner       |
|     1 | file version         | Version 1 => 0x01, Version 2 => 0x02, etc. |

The user application will use the chip model/level ID to determine which Chip
Data File(s) should be used for the chip(s) that exist in the user application's
system.

### 2) Registers

Immediately following the metadata in section 1, there will be a list of all
registers referenced by the isolation nodes for this chip starting with:

| Bytes | Desc             | Value/Example                 |
|:-----:|:-----------------|:------------------------------|
|     4 | register keyword | 0x43484950 (ascii for "REGS") |
|     3 | # of registers   | 0 is invalid                  |

Then, each register will start with:

| Bytes | Desc                   | Value/Example                             |
|:-----:|:-----------------------|:------------------------------------------|
|     3 | register ID            | Unique ID defined by data file owner      |
|     1 | register type          | See appendix for supported register types |
|     1 | attribute flags        | For each bit 0:disabled and 1:enabled     |
|     1 | # of address instances | 0 is invalid                              |

The register ID must be unique for all registers withing the Chip Data File.

Supported attribute flags (bits ordered 0-7, left to right):
 0:   When enabled, the register is readable.
 1:   When enabled, the register is writable.
 2-7: Reserved (default disabled)

#### 2.1) Register Instances

A register could have multiple instances within a chip, each with a different
address. A common example is that the same register could exist for each core on
a processor chip.

So, immdiately following a register's metadata there will be a list of all
instance addresses for a register. Each instance will have the following:

| Bytes | Desc       | Value/Example                                    |
|:-----:|:-----------|:-------------------------------------------------|
|     1 | instance # | Unique value within the register                 |
|     * | address    | The address size is defined by the register type |

### 3) Isolation Nodes

Hardware errors will be reported via registers organized in a tree-like
structure. Isolation of these errors will traverse the tree from a root node
down to the actual bit that raised the attention.

Immediately following all of the metadata described in section 2:

| Bytes | Desc                   | Value/Example                 |
|:-----:|:-----------------------|:------------------------------|
|     4 | isolation node keyword | 0x4e4f4445 (ascii for "NODE") |
|     2 | # of isolation nodes   | 0 is invalid                  |

Then, each node will start the following data:

| Bytes | Desc                   | Value/Example                        |
|:-----:|:-----------------------|:-------------------------------------|
|     2 | node ID                | Unique ID defined by data file owner |
|     1 | register type          | See appendix for supported types     |
|     1 | # of node instances    | 0 is invalid                         |

The node ID must be unique for all nodes within a Chip Data File.

**IMPORTANT:**
All registers referenced in a node's isolation rules must be of the same
register type expressed in this field. This will ensure there is no ambiguity
when resolving the bitwise expressions in the isolation rules.

#### 3.1) Isolation Node Instances

Much like a register, a node can have multiple instances. Each instance will
have the following:

| Bytes | Desc                   | Value/Example                |
|:-----:|:-----------------------|:-----------------------------|
|     1 | instance #             | Unique value within the node |
|     1 | # of capture registers |                              |
|     1 | # of isolation rules   | 0 is invalid                 |
|     1 | # of child nodes       |                              |

##### 3.1.1) Capture Registers

This list specifies which registers to capture and store for debugging purposes.
It is suggested to at least capture all registers referenced in a node's rules,
but it is not required and it is left up to the user application to decide which
registers to capture, if any.

Each capture register will have the following metadata, if any exist, and will
immediately following the metadata for each node instance.

| Bytes | Desc              | Value/Example             |
|:-----:|:------------------|:--------------------------|
|     3 | register ID       | See section 2 for details |
|     1 | register instance | See section 2 for details |

##### 3.1.2) Isolation Rules

Each node instance will represent a register, or set of registers. The
register(s) can be configured to represent one or more attention types.
Therefore, the isolation rules are used to define how attentions are reported.
Expressions (see appendix) will be used to perform bitwise operations on
register values. Any bits set after the expressions have been resolved will
indicate active attentions.

Each rule will have the following metadata and will immediately following the
capture register metadata for a node instance, if any exists.

| Bytes | Desc            | Value/Example                              |
|:-----:|:----------------|:-------------------------------------------|
|     1 | attention type  | See appendix for supported attention types |
|     * | rule expression | See expression definition in appendix      |

Note that the size of the expression field is variable. See the appendix for
details.

##### 3.1.3) Child Nodes

Any bits set after an isolation rule resolution represents an active attention.
A child node should exist for any bits in the resolution that indicate the
attention originated from another node.

The following metadata will exist for each child node associated with a node
instance:

| Bytes | Desc                               | Value/Example     |
|:-----:|:-----------------------------------|:------------------|
|     1 | bit position within the resolution | See notes below   |
|     2 | child node ID                      | See node metadata |
|     1 | child node instance                | See node metadata |

Notes:
* The size of the isolation rule resolution is defined by the register type
  represented by this node.
* The bit position will not exceed the register size.
* The order of the bit position is dependent on the register type.

### 4) Root Nodes

After all of the Isolation Nodes have been specified, there will be a short list
of all the root nodes to each of the isolation trees. The metadata for this
section starts with:

| Bytes | Desc                        | Value/Example                 |
|:-----:|:----------------------------|:------------------------------|
|     4 | root isolation node keyword | 0x524f4f54 (ascii for "ROOT") |
|     1 | # of root nodes             | 0 is invalid                  |

Each isolation tree will report attentions for a single attention type and there
can only be one tree per attention type. Immediately following the above
metadata will be the following for each root node:

| Bytes | Desc               | Value/Example                              |
|:-----:|:-------------------|:-------------------------------------------|
|     1 | attention type     | See appendix for supported attention types |
|     2 | root node ID       | See node ID description in section 3       |
|     1 | root node instance | See node instance description in section 3 |

## Appendix

### 1) Supported Register Types

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

### 2) Supported Attention Types

| Value | Description                                                 |
|:-----:|:------------------------------------------------------------|
|     1 | System checkstop hardware attention                         |
|     2 | Unit checkstop hardware attention                           |
|     3 | Recoverable hardware attention                              |
|     4 | SW or HW event requiring action by the service processor FW |
|     5 | SW or HW event requiring action by the host FW              |

### 3) Expressions

Expressions are used in various locations within a Chip Data File. They can be
used to characterize operations carried out against registers and/or integer
constants. For example, `<some_register> & (0xffff << 16)`.

The first byte of every expression is the expression type. The data immediately
following this field is dependent on the expression type.

#### 3.1) Non-recursive expressions

##### 3.1.1) Register reference expression

This is a special expression that indicates the value of the target register
should be used in this expression. Generally, this means reading the register
value from hardware.

The following is the complete byte definition of the expression:

| Bytes | Description/Value                                     |
|:-----:|:------------------------------------------------------|
|     1 | expression type = 0x01                                |
|     3 | register ID, see section 2 for register details       |
|     1 | register instance, see section 2 for register details |

As you can see, the register ID and instance can be used to find this register's
metadata (e.g. the address) from the register lists (see section 2).

##### 3.1.2) Integer constant expression

This simply contains an unsigned integer constant.

The following is the complete byte definition of the expression:

| Bytes | Description/Value                                 |
|:-----:|:--------------------------------------------------|
|     1 | expression type = 0x02                            |
|     * | An unsigned integer constant (**see note below**) |

**IMPORTANT:** The size of the constant is determined by the register type
specified by the containing node. See section 3 for node details.

#### 3.2) Recursive expressions

These expressions will contain other expressions. Each sub-expression will
always be resolved before handling the containing expression. For example, say
we need to do something like:

    REG_1 & ~REG_2 | CONST_1

Where `REG_*` are register reference expressions and `CONST_*` are integer
constant expressions. Following standard C++ order of operations, that would be
evaluate into an expression like:

    OR( AND( REG_1, NOT(REG_2) ), CONST_1 )

Where the NOT will be evaluated first, then the AND, then finally the OR.

##### 3.2.1) AND expression

A bitwise AND operation (i.e. `EXPR_1 & EXPR_2`).

| Bytes | Description/Value      |
|:-----:|:-----------------------|
|     1 | expression type = 0x10 |
|     1 | # of sub-expressions   |
|     * | all sub-expressions    |

##### 3.2.2) OR expression

A bitwise OR operation (i.e. `EXPR_1 | EXPR_2`).

| Bytes | Description/Value      |
|:-----:|:-----------------------|
|     1 | expression type = 0x11 |
|     1 | # of sub-expressions   |
|     * | all sub-expressions    |

##### 3.2.3) NOT expression

A bitwise NOT operation (i.e. `~EXPR`).

| Bytes | Description/Value      |
|:-----:|:-----------------------|
|     1 | expression type = 0x12 |
|     * | sub-expression         |

##### 3.2.4) Left shift expression

A left shift operation (i.e. `EXPR << shift_value`).

| Bytes | Description/Value      |
|:-----:|:-----------------------|
|     1 | expression type = 0x13 |
|     1 | shift value            |
|     * | sub-expression         |

##### 3.2.5) Right shift expression

A left shift operation (i.e. `EXPR >> shift_value`).

| Bytes | Description/Value      |
|:-----:|:-----------------------|
|     1 | expression type = 0x14 |
|     1 | shift value            |
|     * | sub-expression         |

