DESCRIPTION = "A dynamic programming language and runtime environment, \
with built-in support for concurrency, distribution and fault tolerance"
SECTION = "devel/erlang"
PRIORITY = "optional"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=ff253ad767462c46be284da12dda33e8"

BBCLASSEXTEND = "native nativesdk"

DEPENDS_append_class-target = " erlang-native ncurses zlib openssl"
DEPENDS_append_class-native = " ncurses-native zlib-native openssl-native"
DEPENDS_append_class-nativesdk = " erlang-native nativesdk-zlib nativesdk-openssl nativesdk-ncurses"


SRC_URI = "git://github.com/erlang/otp;branch=master"
SRC_URI += "file://fix-wx-configure.patch"
SRC_URI += "file://erlang-fix-build-issue-in-Yocto.patch"

S = "${WORKDIR}/git"

PARALLEL_MAKE = ""

inherit autotools-brokensep

PACKAGECONFIG ??= ""
PACKAGECONFIG_class-native ??= ""
PACKAGECONFIG_class-nativesdk = ""
PACKAGECONFIG[odbc] = "--with-odbc,--without-odbc,libodbc"
PACKAGECONFIG[lttng] = "--with-dynamic-trace=lttng,--without-dynamic-trace,lttng-ust"
PACKAGECONFIG[wx] = "--with-observer,--without-observer,wxwidgets,"
PACKAGECONFIG[sctp] = ",,,lksctp-tools"
PACKAGECONFIG[java] = ",--without-javac,,"

TARGET_CC_ARCH += "${LDFLAGS}"

EXTRA_OEMAKE = "BUILD_CC='${BUILD_CC}'"

# EXTRA_OECONF_append_arm = " --disable-smp-support --disable-hipe"
# EXTRA_OECONF_append_armeb = " --disable-smp-support --disable-hipe"
EXTRA_OECONF_append_mipsel = " --disable-smp-support --disable-hipe"
EXTRA_OECONF_append_sh3 = " --disable-smp-support --disable-hipe"
EXTRA_OECONF_append_sh4 = " --disable-smp-support --disable-hipe"

NATIVE_BIN = "${STAGING_LIBDIR_NATIVE}/erlang/bin"

EXTRA_OECONF_class-native += " \
    --with-ssl=${STAGING_DIR_NATIVE} \
    "
EXTRA_OECONF_class-nativesdk += " \
    --with-ssl=${STAGING_DIR_NATIVE}"

CACHED_CONFIGUREVARS += "ac_cv_prog_javac_ver_1_2=no ac_cv_prog_javac_ver_1_5=no erl_xcomp_sysroot=${STAGING_DIR_TARGET}"
CACHED_CONFIGUREVARS_class-native += "ac_cv_prog_javac_ver_1_2=no ac_cv_prog_javac_ver_1_5=no erl_xcomp_sysroot=${STAGING_DIR_NATIVE}"
CACHED_CONFIGUREVARS_class-nativesdk += "ac_cv_prog_javac_ver_1_2=no ac_cv_prog_javac_ver_1_5=no erl_xcomp_sysroot=${STAGING_DIR_NATIVE}"

do_configure_class-target() {
    cd ${S}; ./otp_build autoconf; cd -
    cd ${S}/erts; autoreconf; cd -

    . ${CONFIG_SITE}

    SHLIB_LD='${CC}' \
    oe_runconf

    sed -i -e 's|$(ERL_TOP)/bin/dialyzer|${NATIVE_BIN}/dialyzer --output_plt $@ -pa $(ERL_TOP)/lib/kernel/ebin -pa $(ERL_TOP)/lib/stdlib/ebin|' lib/dialyzer/src/Makefile
}

do_configure_class-native() {
    cd ${S}; ./otp_build autoconf
    TARGET=${HOST_SYS} \
    oe_runconf
}

do_configure_class-nativesdk() {

    rm -rf ${S}/lib/wx

    cd ${S}; ./otp_build autoconf
    TARGET=${HOST_SYS} \
    oe_runconf
}


do_compile_prepend_class-native() {
    export TARGET=${HOST_SYS}
}

do_compile_prepend_class-nativesdk() {
    export TARGET=${HOST_SYS}
}

do_compile_class-target() {
    TARGET=${TARGET_SYS} \
    PATH=${NATIVE_BIN}:$PATH \
    oe_runmake
}

do_install_prepend_class-native() {
    export TARGET=${HOST_SYS}
}

do_install_prepend_class-nativesdk() {
    export TARGET=${HOST_SYS}
}

do_install_class-target() {
    TARGET=${TARGET_SYS} \
    PATH=${NATIVE_BIN}:$PATH \
    oe_runmake 'INSTALL_PREFIX=${D}' install
    for f in erl start
        do sed -i -e 's:ROOTDIR=.*:ROOTDIR=${libdir}/erlang:' \
            ${D}/${libdir}/erlang/erts-*/bin/$f ${D}/${libdir}/erlang/bin/$f
    done

    rm -f ${D}/${libdir}/erlang/Install

    # Actually wx is not suitable with erlang embedded
    rm -rf ${D}/${libdir}/erlang/lib/wx-*
    chown -R root:root ${D}${libdir}/erlang
}

do_install_prepend_class-native() {
    export TARGET=${HOST_SYS}
}

do_install_append_class-nativesdk() {
    rm -f ${D}/${libdir}/erlang/Install
    chown -R root:root ${D}${libdir}/erlang
}

do_install_append() {
	# remove erlang code that does not need to be loaded by the erlang runtime
	rm -rf ${D}/${libdir}/erlang/lib/erts-${ERTS_VERSION}/ebin
}
