#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple helloworld application"
SECTION = "examples"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://Makefile"
SRC_URI += "file://selinuxtest.fc"
SRC_URI += "file://selinuxtest.if"
SRC_URI += "file://selinuxtest.te"

S = "${WORKDIR}"

DEPENDS += "checkpolicy-native policycoreutils-native m4-native semodule-utils-native refpolicy-targeted"

EXTRA_OEMAKE += "BINDIR=${STAGING_BINDIR_NATIVE}"
EXTRA_OEMAKE += "HEADERDIR=${RECIPE_SYSROOT}${datadir}/selinux/targeted/include"

#do_compile() {
#	     ${CC} ${LDFLAGS} helloworld.c -o helloworld
#}

#do_install() {
#	     install -d ${D}${bindir}
#	     install -m 0755 helloworld ${D}${bindir}
#}
