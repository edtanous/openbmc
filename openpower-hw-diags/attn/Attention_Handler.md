# Hardware Error Attention Handling for POWER Systems

Attentions are host error and debug breakpoint conditions that can be handled
by software running on the BMC. The host alerts the BMC of these conditions
via the Attention GPIO pin. The Attention Handler application (ATTN) monitors
this gpio for activity and services active attentions. When activity is
detected on the Attention GPIO, ATTN will query the hardware to determine the
active attention that needs to be serviced.

## Attention Handler Logic
The following logic is implemented in ATTN to allow for controlling the flow
of attention handling once an attention event has been detected.

- ATTN can adjust the handling priority of active attention events.

- ATTN can choose to defer or ignore handling of attention events.

- ATTN can handle any number of active attentions per GPIO event.

- ATTN can continue monitoring or stop monitoring the attention GPIO.

## Attention Conditions
The following is a list of attention conditions and actions.

1. SBE Attention (vital): This attention indicates that the SBE is in a state
   that is considered non-functional. ATTN will log this event and *...TBD*

2. System Checkstop (checkstop): This attention indicates an error condition
   in which the host cannot continue operating properly. ATTN will launch the
   Hardware Diagnostics application (Hwdiags) and may provide Hwdiags with
   information needed to help it analyze the checkstop *...TBD*

3. Special Attention (special): This attention indicates that one of the
   following conditions has occurred.

    - PHYP Breakpoint (breakpoint): ATTN will send notification with the
      processor, core and thread number associated with the event.

    - PHYP Terminate Immediate (PHYP TI): ATTN will dump and re-ipl.

    - OPAL Terminate Immediate (OPAL TI): ATTN will dump and re-ipl.

    - Core Code to SP (PHYP CoreCode): ATTN will log this event.

    - Other (instruction stop, core recovery handshake): Attention will log
      these events.

##Command Line Interface

ATTN supports the following command line interface *...TBD*

##Building

This application is built using the standard Meson/Ninja build setup.

    meson build && cd build && ninja
