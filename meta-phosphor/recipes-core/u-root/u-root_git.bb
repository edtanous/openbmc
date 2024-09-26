DESCRIPTION = "u-root contains simple Go versions of many standard Linux tools, similar to busybox. It's a pure Go userland!"

GO_IMPORT = "github.com/u-root/u-root/"
inherit go

SRC_URI = "git://${GO_IMPORT};protocol=https;destsuffix=${PN}-${PV}/src/${GO_IMPORT}"
SRCREV = "64df2083953289d5f15839890437a138d77c8d74"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://src/${GO_IMPORT}/LICENSE;md5=3166936844c72a95321cd536eaf26413"

#GOBUILDFLAGS += "-build=bb ./cmds/{ip,ls,cp}"

# This package doesn't build for 32 bit targets because of what appears to be an int32 vs int64 issue with a maxrss variable
GO_INSTALL_FILTEROUT = "github.com/u-root/u-root/scripts/build_perf"

FILES_${PN} += "${GOBIN_FINAL}/*"
