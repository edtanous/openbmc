FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://gb200nvl-obmc.cfg \
    file://0001-The-GB200NVL-BMC-is-an-Aspeed-Ast2600-based-BMC.patch \
"

