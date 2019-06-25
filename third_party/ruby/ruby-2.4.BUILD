# vim: ft=bzl sw=4 ts=4 et

# caveats ######################################################################

# Hard-coded files
# * probes.h
# * enc/encinit.c

# Templated files
# * verconf.h is just a templated version of what ruby produces


# configuration ################################################################

genrule(
    name = "run_configure",
    outs = [
        "include/ruby/config.h",
        "config.status",
    ],
    srcs = glob([
        "configure",
        "tool/*",
        "*.h",
        "include/**/*.h",
        "**/Makefile.in",
        "template/*",
    ]),
    cmd = """
CC=$(CC) CFLAGS=$(CC_FLAGS) CPPFLAGS=$(CC_FLAGS) $(location configure) > /dev/null
find .ext -name config.h -type f -exec cp {} $(location include/ruby/config.h) \;
cp config.status $(location config.status)
""",
)

# shared headers ###############################################################

cc_library(
    name = "ruby_headers",
    srcs = glob([
        "include/**/*.h",
    ]),
    hdrs = [
        "include/ruby/config.h",
    ],
    includes = [ "include" ],
    visibility = [ "//visibility:public" ],
)

cc_library(
    name = "ruby_private_headers",
    srcs = glob([
        "*.h",
        "*.inc",
        "enc/*.h",
        "enc/trans/*.h",
        "enc/unicode/9.0.0/*.h",
    ]),
    hdrs = glob([
        "*.h",
        "*.inc",
        "enc/shift_jis.c",
        "enc/jis/*.h",
    ]),
    includes = [
        "enc",
        "enc/trans",
        "enc/unicode/9.0.0",
    ],
    visibility = [ "//visibility:private" ],
)


# miniruby #####################################################################

cc_library(
    name = "miniruby_private_headers",
    hdrs = [
        "lex.c",
        "thread_pthread.c",
        "thread_sync.c",
        "id.c",
        "eval_error.c",
        "vsnprintf.c",
        "vm_insnhelper.c",
        "vm_args.c",
        "vm_exec.c",
        "vm_eval.c",
        "vm_method.c",
        "id_table.c",
        "eval_jump.c",
        "siphash.c",
    ],
)

cc_binary(
    name = "bin/miniruby",

    data = [ ":ruby_lib_staged" ],

    srcs = [
        "main.c",
        "dmydln.c",
        "dmyext.c",
        "miniinit.c",
        "miniprelude.c",
        "array.c",
        "bignum.c",
        "class.c",
        "compar.c",
        "compile.c",
        "complex.c",
        "cont.c",
        "debug.c",
        "dir.c",
        "dln_find.c",
        "encoding.c",
        "enum.c",
        "enumerator.c",
        "error.c",
        "eval.c",
        "file.c",
        "gc.c",
        "hash.c",
        "inits.c",
        "io.c",
        "iseq.c",
        "load.c",
        "marshal.c",
        "math.c",
        "node.c",
        "numeric.c",
        "object.c",
        "pack.c",
        "parse.c",
        "proc.c",
        "process.c",
        "random.c",
        "range.c",
        "rational.c",
        "re.c",
        "regcomp.c",
        "regenc.c",
        "regerror.c",
        "regexec.c",
        "regparse.c",
        "regsyntax.c",
        "ruby.c",
        "safe.c",
        "signal.c",
        "sprintf.c",
        "st.c",
        "strftime.c",
        "string.c",
        "struct.c",
        "symbol.c",
        "thread.c",
        "time.c",
        "transcode.c",
        "util.c",
        "variable.c",
        "version.c",
        "vm.c",
        "vm_backtrace.c",
        "vm_dump.c",
        "vm_trace.c",
        "enc/ascii.c",
        "enc/us_ascii.c",
        "enc/unicode.c",
        "enc/utf_8.c",
        "enc/trans/newline.c",
        "missing/explicit_bzero.c",
        "missing/setproctitle.c",
    ] + glob([
        "ccan/**/*.h",
    ]),

    deps = [
        ":miniruby_private_headers",
        ":ruby_headers",
        ":ruby_private_headers",
    ],

    copts = [
        "-DRUBY_EXPORT",
        "-D_XOPEN_SOURCE",
        "-Wno-constant-logical-operand",
        "-Wno-parentheses",
        "-D_DARWIN_C_SOURCE",
        "-D_REENTRANT",

        # TODO: is this really necessary?
        "-Wno-string-plus-int",
    ],

    linkopts = [
        "-framework",
        "Foundation",
        "-lpthread",
    ],

    includes = [ "include", "enc/unicode/9.0.0" ],

    visibility = ["//visibility:public"],
)

# full ruby ####################################################################

# TODO: don't hardcode the arch string
# TODO: don't hardcode the ruby version
genrule(
    name = "generate-rbconfig",
    srcs = [
        ":bin/miniruby",
        ":ruby_lib_staged",
        "lib/irb.rb",
        "tool/mkconfig.rb",
        "config.status",
        "version.h",
    ],
    outs = [ "rbconfig.rb" ],
    cmd = """
cp $(location config.status) config.status
$(location bin/miniruby) \
        -I $$(dirname $(location lib/irb.rb)) \
        $(location tool/mkconfig.rb) \
        -cross_compiling=no \
        -arch=x86_64-darwin18 \
        -version=2.4.3 \
        -install_name=ruby \
        -so_name=ruby \
        -unicode_version=9.0.0 \
    > $(location rbconfig.rb)
""",
)

genrule(
    name = "prelude",
    srcs = [
        ":bin/miniruby",
        ":ruby_lib",
        "lib/irb.rb",
        "tool/generic_erb.rb",
        "tool/vpath.rb",
        "template/prelude.c.tmpl",
        "prelude.rb",
        "enc/prelude.rb",
        "gem_prelude.rb",
    ],
    outs = [ "prelude.c" ],
    cmd = """
# -----------------
$(location bin/miniruby) \
    -I. \
    -I$$(dirname $(location lib/irb.rb)) \
    -I$$(dirname $(location prelude.rb)) \
    $(location tool/generic_erb.rb) \
    -c -o $(location prelude.c) \
    $(location template/prelude.c.tmpl) \
    $(location prelude.rb) \
    $(location enc/prelude.rb) \
    $(location gem_prelude.rb)
""",
)

genrule(
    name = "verconf",
    srcs = [ ":bin/miniruby" ],
    outs = [ "verconf.h" ],
    cmd = """
prefix=$$(dirname $(location bin/miniruby))
cat > $(location verconf.h) <<EOF
#define RUBY_BASE_NAME                  "ruby"
#define RUBY_VERSION_NAME               RUBY_BASE_NAME"-"RUBY_LIB_VERSION
#define RUBY_LIB_VERSION_STYLE          3    /* full */
#define RUBY_EXEC_PREFIX                "$$prefix"
#define RUBY_LIB_PREFIX                 RUBY_EXEC_PREFIX"/lib/ruby"
#define RUBY_ARCH_PREFIX_FOR(arch)      RUBY_LIB_PREFIX"/"arch
#define RUBY_SITEARCH_PREFIX_FOR(arch)  RUBY_LIB_PREFIX"/"arch
#define RUBY_LIB                        RUBY_LIB_PREFIX"/"RUBY_LIB_VERSION
#define RUBY_ARCH_LIB_FOR(arch)         RUBY_LIB"/"arch
#define RUBY_SITE_LIB                   RUBY_LIB_PREFIX"/site_ruby"
#define RUBY_SITE_ARCH_LIB_FOR(arch)    RUBY_SITE_LIB2"/"arch
#define RUBY_VENDOR_LIB                 RUBY_LIB_PREFIX"/vendor_ruby"
#define RUBY_VENDOR_ARCH_LIB_FOR(arch)  RUBY_VENDOR_LIB2"/"arch
EOF
"""
)

cc_binary(
    name = "bin/ruby",

    srcs = [
        "prelude.c",
        "verconf.h",

        "ext/extinit.c",

        "enc/encinit.c",
        "enc/ascii.c",
        "enc/us_ascii.c",
        "enc/unicode.c",
        "enc/utf_8.c",

        "main.c",
        "dln.c",
        "localeinit.c",
        "loadpath.c",
        "array.c",
        "bignum.c",
        "class.c",
        "compar.c",
        "compile.c",
        "complex.c",
        "cont.c",
        "debug.c",
        "dir.c",
        "dln_find.c",
        "encoding.c",
        "enum.c",
        "enumerator.c",
        "error.c",
        "eval.c",
        "file.c",
        "gc.c",
        "hash.c",
        "inits.c",
        "io.c",
        "iseq.c",
        "load.c",
        "marshal.c",
        "math.c",
        "node.c",
        "numeric.c",
        "object.c",
        "pack.c",
        "parse.c",
        "proc.c",
        "process.c",
        "random.c",
        "range.c",
        "rational.c",
        "re.c",
        "regcomp.c",
        "regenc.c",
        "regerror.c",
        "regexec.c",
        "regparse.c",
        "regsyntax.c",
        "ruby.c",
        "safe.c",
        "signal.c",
        "sprintf.c",
        "st.c",
        "strftime.c",
        "string.c",
        "struct.c",
        "symbol.c",
        "thread.c",
        "time.c",
        "transcode.c",
        "util.c",
        "variable.c",
        "version.c",
        "vm.c",
        "vm_backtrace.c",
        "vm_dump.c",
        "vm_trace.c",
        "missing/explicit_bzero.c",
        "missing/setproctitle.c",
    ] + glob([
        "ccan/**/*.h",
    ]),

    deps = [
        ":miniruby_private_headers",
        ":ruby_headers",
        ":ruby_private_headers",
        ":enc",
        ":trans",
        ":ext/pathname",
        ":ext/stringio",
        ":ext/bigdecimal",
        ":ext/json/generator",
        ":ext/json/parser",
        ":ext/socket",
        ":ext/io/wait",
        ":ext/zlib",
        ":ext/date",
        ":ext/digest",
        ":ext/psych",
        ":ext/strscan",
    ],

    copts = [
        "-DRUBY_EXPORT",
        "-D_XOPEN_SOURCE",
        "-Wno-constant-logical-operand",
        "-Wno-parentheses",
        "-D_DARWIN_C_SOURCE",
        "-D_REENTRANT",

        "-DRUBY",

        # TODO: is this really necessary?
        "-Wno-string-plus-int",
    ],

    linkopts = [
        "-framework",
        "Foundation",
        "-lpthread",
    ],

    visibility = ["//visibility:public"],
)

genrule(
    name = "enc-generated-deps",
    srcs = [
        "bin/miniruby",
        "tool/generic_erb.rb",
        "tool/vpath.rb",
        "enc/make_encmake.rb",
        "template/encdb.h.tmpl",
        "template/transdb.h.tmpl",
        "lib/erb.rb",
        "lib/cgi/util.rb",
        "lib/optparse.rb",
        "lib/fileutils.rb",
    ] +
    # NOTE: this glob determines the encodings that are available in encdb.h, as
    # encdb.h.tmpl reads this directory to determine encodings to use
    glob([ "enc/**/*.h", "enc/**/*.c" ]),
    outs = [
        "encdb.h",
        "transdb.h",
    ],
    cmd = """
$(location bin/miniruby) \
    -I$$(dirname $(location lib/erb.rb)) \
    -I$$(dirname $(location tool/vpath.rb)) \
    $(location tool/generic_erb.rb) \
    -c -o $(location encdb.h) $(location template/encdb.h.tmpl) \
    $$(dirname $(location enc/ascii.c)) enc

$(location bin/miniruby) \
    -I$$(dirname $(location lib/erb.rb)) \
    -I$$(dirname $(location tool/vpath.rb)) \
    $(location tool/generic_erb.rb) \
    -c -o $(location transdb.h) $(location template/transdb.h.tmpl) \
    $$(dirname $(location enc/trans/transdb.c)) enc/trans
""",
)

cc_library(
    name = "trans",
    srcs = [
        "enc/trans/big5.c",
        "enc/trans/chinese.c",
        "enc/trans/ebcdic.c",
        "enc/trans/emoji.c",
        "enc/trans/emoji_iso2022_kddi.c",
        "enc/trans/emoji_sjis_docomo.c",
        "enc/trans/emoji_sjis_kddi.c",
        "enc/trans/emoji_sjis_softbank.c",
        "enc/trans/escape.c",
        "enc/trans/gb18030.c",
        "enc/trans/gbk.c",
        "enc/trans/iso2022.c",
        "enc/trans/japanese.c",
        "enc/trans/japanese_euc.c",
        "enc/trans/japanese_sjis.c",
        "enc/trans/korean.c",
        "enc/trans/newline.c",
        "enc/trans/single_byte.c",
        "enc/trans/transdb.c",
        "enc/trans/utf8_mac.c",
        "enc/trans/utf_16_32.c",
    ],
    hdrs = [
        "transdb.h",
    ],
    deps = [
        ":ruby_headers",
        ":ruby_private_headers",
    ],
    copts = [
        "-DRUBY_EXPORT",
        "-D_XOPEN_SOURCE",
        "-Wno-constant-logical-operand",
        "-Wno-parentheses",
        "-D_DARWIN_C_SOURCE",
        "-D_REENTRANT",

        # IMPORTANT: without this no Init functions are generated
        "-DEXTSTATIC=1",

        # TODO: is this really necessary?
        "-Wno-string-plus-int",

        # for r "enc/gb2312.c"
        "-Wno-implicit-function-declaration",
    ],
    linkstatic = 1,
)

cc_library(
    name = "enc",
    srcs = [
        "enc/big5.c",
        "enc/cp949.c",
        "enc/emacs_mule.c",
        "enc/encdb.c",
        "enc/euc_jp.c",
        "enc/euc_kr.c",
        "enc/euc_tw.c",
        "enc/gb18030.c",
        "enc/gb2312.c",
        "enc/gbk.c",
        "enc/iso_8859_1.c",
        "enc/iso_8859_10.c",
        "enc/iso_8859_11.c",
        "enc/iso_8859_13.c",
        "enc/iso_8859_14.c",
        "enc/iso_8859_15.c",
        "enc/iso_8859_16.c",
        "enc/iso_8859_2.c",
        "enc/iso_8859_3.c",
        "enc/iso_8859_4.c",
        "enc/iso_8859_5.c",
        "enc/iso_8859_6.c",
        "enc/iso_8859_7.c",
        "enc/iso_8859_8.c",
        "enc/iso_8859_9.c",
        "enc/koi8_r.c",
        "enc/koi8_u.c",
        "enc/shift_jis.c",
        "enc/utf_16be.c",
        "enc/utf_16le.c",
        "enc/utf_32be.c",
        "enc/utf_32le.c",
        "enc/windows_1250.c",
        "enc/windows_1251.c",
        "enc/windows_1252.c",
        "enc/windows_1253.c",
        "enc/windows_1254.c",
        "enc/windows_1257.c",
        "enc/windows_31j.c",
    ],
    hdrs = [
        "encdb.h",
    ],
    deps = [
        ":ruby_headers",
        ":ruby_private_headers",
    ],
    copts = [
        "-D_XOPEN_SOURCE",
        "-Wno-constant-logical-operand",
        "-Wno-parentheses",
        "-D_DARWIN_C_SOURCE",
        "-D_REENTRANT",

        # IMPORTANT: without this no Init functions are generated
        "-DRUBY", "-DONIG_ENC_REGISTER=rb_enc_register",

        # TODO: is this really necessary?
        "-Wno-string-plus-int",

        # for r "enc/gb2312.c"
        "-Wno-implicit-function-declaration",
    ],
    linkstatic = 1,
)


# extensions ###################################################################

# NOTE: update `Init_ext` below if you add a new extension.
genrule(
    name = "ext/extinit",
    outs = [ "ext/extinit.c" ],
    cmd = """
cat > $(location ext/extinit.c) <<EOF
#include "ruby/ruby.h"

#define init(func, name) {      \
    extern void func(void);     \
    ruby_init_ext(name, func);  \
}

void ruby_init_ext(const char *name, void (*init)(void));

void Init_ext(void)
{
    init(Init_pathname, "pathname.so");
    init(Init_stringio, "stringio.so");
    init(Init_bigdecimal, "bigdecimal.so");
    init(Init_parser, "json/ext/parser");
    init(Init_generator, "json/ext/generator");
    init(Init_socket, "socket.so");
    init(Init_wait, "io/wait");
    init(Init_zlib, "zlib");
    init(Init_date_core, "date_core");
    init(Init_bubblebabble, "digest/bubblebabble");
    init(Init_sha1, "digest/sha1");
    init(Init_sha2, "digest/sha2.so");
    init(Init_rmd160, "digest/rmd160");
    init(Init_md5, "digest/md5");
    init(Init_digest, "digest.so");
    init(Init_psych, "psych.so");
    init(Init_strscan, "strscan");
}
EOF
""",
)

cc_library(
    name = "ext/pathname",
    srcs = [ "ext/pathname/pathname.c" ],
    deps = [ ":ruby_headers" ],
    linkstatic = 1,
)

cc_library(
    name = "ext/stringio",
    srcs = [ "ext/stringio/stringio.c" ],
    deps = [ ":ruby_headers" ],
    linkstatic = 1,
)

cc_library(
    name = "ext/bigdecimal",
    srcs = [
        "ext/bigdecimal/bigdecimal.c",
        "ext/bigdecimal/bigdecimal.h",
    ],
    # TODO: these are generated by extconf.rb. Is there a way that we can hijack
    # that to produce output we can consume directly?
    copts = [
        "-DHAVE_LABS",
        "-DHAVE_LLABS",
        "-DHAVE_FINITE",
        "-DHAVE_RB_RATIONAL_NUM",
        "-DHAVE_RB_RATIONAL_DEN",
        "-DHAVE_RB_ARRAY_CONST_PTR",
        "-DHAVE_RB_SYM2STR",
    ],
    deps = [ ":ruby_headers" ],
    linkstatic = 1,
)

cc_library(
    name = "ext/json/generator",
    srcs = [
        "ext/json/generator/generator.c",
        "ext/json/generator/generator.h",
        "ext/json/fbuffer/fbuffer.h",
    ],
    copts = [ "-DJSON_GENERATOR" ],
    deps = [ ":ruby_headers" ],
    linkstatic = 1,
)

cc_library(
    name = "ext/json/parser",
    srcs = [
        "ext/json/parser/parser.c",
        "ext/json/parser/parser.h",
        "ext/json/fbuffer/fbuffer.h",
    ],
    copts = [
        "-DHAVE_RB_ENC_RAISE",
    ],
    deps = [ ":ruby_headers" ],
    linkstatic = 1,
)

genrule(
    name = "generate-ext/socket-constants",
    srcs = [
        ":bin/miniruby",
        ":ruby_lib",
        "lib/optparse.rb",
        "ext/socket/mkconstants.rb",
    ],
    outs = [
        "ext/socket/constdefs.h",
        "ext/socket/constdefs.c",
    ],
    cmd = """
$(location bin/miniruby) \
    -I $$(dirname $(location lib/optparse.rb)) \
    $(location ext/socket/mkconstants.rb) \
    -H $(location ext/socket/constdefs.h) \
    -o $(location ext/socket/constdefs.c)
""",
)

cc_library(
    name = "ext/socket",
    includes = [
        "ext/socket"
    ],
    hdrs = [
        "ext/socket/constdefs.h",
        "ext/socket/constdefs.c",
        "ext/socket/sockport.h",
        "ext/socket/rubysocket.h",
        "ext/socket/addrinfo.h",
    ],
    srcs = [
        "ext/socket/ancdata.c",
        "ext/socket/basicsocket.c",
        "ext/socket/constants.c",
        "ext/socket/getaddrinfo.c",
        "ext/socket/getnameinfo.c",
        "ext/socket/ifaddr.c",
        "ext/socket/init.c",
        "ext/socket/ipsocket.c",
        "ext/socket/option.c",
        "ext/socket/raddrinfo.c",
        "ext/socket/socket.c",
        "ext/socket/sockssocket.c",
        "ext/socket/tcpserver.c",
        "ext/socket/tcpsocket.c",
        "ext/socket/udpsocket.c",
        "ext/socket/unixserver.c",
        "ext/socket/unixsocket.c",
    ],
    copts = [
        "-DHAVE_SYS_UIO_H",
        "-DHAVE_NETINET_IN_SYSTM_H",
        "-DHAVE_NETINET_TCP_H",
        "-DHAVE_NETINET_TCP_FSM_H",
        "-DHAVE_NETINET_UDP_H",
        "-DHAVE_ARPA_INET_H",
        "-DHAVE_NET_ETHERNET_H",
        "-DHAVE_SYS_UN_H",
        "-DHAVE_IFADDRS_H",
        "-DHAVE_SYS_IOCTL_H",
        "-DHAVE_SYS_SOCKIO_H",
        "-DHAVE_NET_IF_H",
        "-DHAVE_SYS_PARAM_H",
        "-DHAVE_SYS_UCRED_H",
        "-DHAVE_NET_IF_DL_H",
        "-DHAVE_ARPA_NAMESER_H",
        "-DHAVE_RESOLV_H",
        "-DHAVE_STRUCT_SOCKADDR_SA_LEN",
        "-DHAVE_ST_SA_LEN",
        "-DHAVE_STRUCT_SOCKADDR_IN_SIN_LEN",
        "-DHAVE_ST_SIN_LEN",
        "-DHAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN",
        "-DHAVE_ST_SIN6_LEN",
        "-DHAVE_TYPE_STRUCT_SOCKADDR_UN",
        "-DHAVE_STRUCT_SOCKADDR_UN_SUN_LEN",
        "-DHAVE_ST_SUN_LEN",
        "-DHAVE_TYPE_STRUCT_SOCKADDR_DL",
        "-DHAVE_TYPE_STRUCT_SOCKADDR_STORAGE",
        "-DHAVE_TYPE_STRUCT_ADDRINFO",
        "-DHAVE_TYPE_SOCKLEN_T",
        "-DHAVE_TYPE_STRUCT_IN_PKTINFO",
        "-DHAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST",
        "-DHAVE_ST_IPI_SPEC_DST",
        "-DHAVE_TYPE_STRUCT_IN6_PKTINFO",
        "-DHAVE_TYPE_STRUCT_IP_MREQ",
        "-DHAVE_TYPE_STRUCT_IP_MREQN",
        "-DHAVE_TYPE_STRUCT_IPV6_MREQ",
        "-DHAVE_STRUCT_MSGHDR_MSG_CONTROL",
        "-DHAVE_ST_MSG_CONTROL",
        "-DHAVE_SOCKET",
        "-DHAVE_SENDMSG",
        "-DHAVE_RECVMSG",
        "-DHAVE_FREEHOSTENT",
        "-DHAVE_FREEADDRINFO",
        "-DHAVE_GAI_STRERROR",
        "-DGAI_STRERROR_CONST",
        "-DHAVE_INET_NTOP",
        "-DHAVE_INET_PTON",
        "-DHAVE_GETSERVBYPORT",
        "-DHAVE_GETIFADDRS",
        "-DHAVE_GETPEEREID",
        "-DHAVE_IF_INDEXTONAME",
        "-DNEED_IF_INDEXTONAME_DECL",
        "-DHAVE_IF_NAMETOINDEX",
        "-DNEED_IF_NAMETOINDEX_DECL",
        "-DHAVE_GETIPNODEBYNAME",
        "-DHAVE_GETHOSTBYNAME2",
        "-DHAVE_SOCKETPAIR",
        "-DHAVE_GETHOSTNAME",
        "-DENABLE_IPV6",
        "-DINET6",
        "-DHAVE_CONST_AF_UNIX",
        "-DHAVE_CONST_SCM_RIGHTS",
        "-DHAVE_GETNAMEINFO",
        "-DHAVE_GETADDRINFO",
        "-Wno-dangling-else",
        "-Wno-unused-const-variable",
    ],
    deps = [ ":ruby_headers", ":ruby_private_headers" ],
    linkstatic = 1,
)

cc_library(
    name = "ext/io/wait",
    srcs = [ "ext/io/wait/wait.c" ],
    deps = [ ":ruby_headers" ],
    copts = [
        '-DHAVE_SYS_IOCTL_H',
        '-DFIONREAD_HEADER="<sys/ioctl.h>"',
    ],
    linkstatic = 1,
)

cc_library(
    name = "ext/zlib",
    srcs = [ "ext/zlib/zlib.c" ],
    deps = [
        ":ruby_headers",
        "@zlib//:zlib",
    ],
    copts = [
        "-DHAVE_ZLIB_H",
        "-DOS_CODE=OS_UNIX",
        "-DHAVE_CRC32_COMBINE",
        "-DHAVE_ADLER32_COMBINE",
        "-DHAVE_TYPE_Z_CRC_T",
    ],
    linkstatic = 1,
)

cc_library(
    name = "ext/date",
    srcs = [
        "ext/date/date_core.c",
        "ext/date/date_parse.c",
        "ext/date/date_strftime.c",
        "ext/date/date_strptime.c",
    ],
    hdrs = [
        "ext/date/date_tmx.h",
        "ext/date/zonetab.h",
    ],
    deps = [
        ":ruby_headers",
    ],
    copts = [
    ],
    linkstatic = 1,
)

cc_library(
    name = "ext/digest",
    srcs = [
        "ext/digest/digest.c",
        "ext/digest/bubblebabble/bubblebabble.c",
        "ext/digest/sha1/sha1.c",
        "ext/digest/sha1/sha1init.c",
        "ext/digest/sha2/sha2.c",
        "ext/digest/sha2/sha2init.c",
        "ext/digest/rmd160/rmd160.c",
        "ext/digest/rmd160/rmd160init.c",
        "ext/digest/md5/md5.c",
        "ext/digest/md5/md5init.c",
    ],
    hdrs = [
        "ext/digest/defs.h",
        "ext/digest/digest.h",
        "ext/digest/sha1/sha1.h",
        "ext/digest/sha1/sha1cc.h",
        "ext/digest/sha2/sha2.h",
        "ext/digest/sha2/sha2cc.h",
        "ext/digest/rmd160/rmd160.h",
        "ext/digest/md5/md5.h",
        "ext/digest/md5/md5cc.h",
    ],
    deps = [
        ":ruby_headers",
    ],
    copts = [
        "-DHAVE_CONFIG_H",
        "-DHAVE_SHA256_TRANSFORM",
        "-DHAVE_SHA512_TRANSFORM",
        "-DHAVE_TYPE_SHA256_CTX",
        "-DHAVE_TYPE_SHA512_CTX",
        "-DHAVE_SYS_CDEFS_H",
    ],
    linkstatic = 1,
)

cc_library(
    name = "ext/psych",
    srcs = [
        "ext/psych/psych.c",
        "ext/psych/psych_emitter.c",
        "ext/psych/psych_parser.c",
        "ext/psych/psych_to_ruby.c",
        "ext/psych/psych_yaml_tree.c",
        "ext/psych/yaml/api.c",
        "ext/psych/yaml/dumper.c",
        "ext/psych/yaml/emitter.c",
        "ext/psych/yaml/loader.c",
        "ext/psych/yaml/parser.c",
        "ext/psych/yaml/reader.c",
        "ext/psych/yaml/scanner.c",
        "ext/psych/yaml/writer.c",
    ] + glob([ "ext/psych/**/*.h" ]),
    includes = [
        "ext/psych",
        "ext/psych/yaml",
        "include/ruby",
    ],
    copts = [
        "-DHAVE_DLFCN_H",
        "-DHAVE_INTTYPES_H",
        "-DHAVE_MEMORY_H",
        "-DHAVE_STDINT_H",
        "-DHAVE_STDLIB_H",
        "-DHAVE_STRINGS_H",
        "-DHAVE_STRING_H",
        "-DHAVE_SYS_STAT_H",
        "-DHAVE_SYS_TYPES_H",
        "-DHAVE_UNISTD_H",
        "-DHAVE_CONFIG_H",
    ],
    deps = [
        ":ruby_headers",
    ],
    linkstatic = 1,
)

cc_library(
    name = "ext/strscan",
    srcs = [
        "ext/strscan/strscan.c",
    ],
    deps = [
        ":ruby_headers",
        ":ruby_private_headers",
    ],
    linkstatic = 1,
)


# core library #################################################################

genrule(
    name = "lib/rbconfig",
    outs = [ "lib/rbconfig.rb" ],
    srcs = [ "rbconfig.rb" ],
    cmd = "cp $< $@",
)

genrule(
    name = "ruby_ext/pathname",
    srcs = [ "ext/pathname/lib/pathname.rb" ],
    outs = [ "lib/pathname.rb" ],
    cmd = "cp $< $@",
)

genrule(
    name = "ruby_ext/bigdecimal",
    srcs = [
        "ext/bigdecimal/lib/bigdecimal/jacobian.rb",
        "ext/bigdecimal/lib/bigdecimal/ludcmp.rb",
        "ext/bigdecimal/lib/bigdecimal/math.rb",
        "ext/bigdecimal/lib/bigdecimal/newton.rb",
        "ext/bigdecimal/lib/bigdecimal/util.rb",
    ],
    outs = [
        "lib/bigdecimal/jacobian.rb",
        "lib/bigdecimal/ludcmp.rb",
        "lib/bigdecimal/math.rb",
        "lib/bigdecimal/newton.rb",
        "lib/bigdecimal/util.rb",
    ],

    cmd = """
 cp $(location ext/bigdecimal/lib/bigdecimal/jacobian.rb) $(location lib/bigdecimal/jacobian.rb)
 cp $(location ext/bigdecimal/lib/bigdecimal/ludcmp.rb)   $(location lib/bigdecimal/ludcmp.rb)
 cp $(location ext/bigdecimal/lib/bigdecimal/math.rb)     $(location lib/bigdecimal/math.rb)
 cp $(location ext/bigdecimal/lib/bigdecimal/newton.rb)   $(location lib/bigdecimal/newton.rb)
 cp $(location ext/bigdecimal/lib/bigdecimal/util.rb)     $(location lib/bigdecimal/util.rb)
"""
)

genrule(
    name = "ruby_ext/json",
    srcs = [
        "ext/json/lib/json.rb",
        "ext/json/lib/json/add/bigdecimal.rb",
        "ext/json/lib/json/add/complex.rb",
        "ext/json/lib/json/add/core.rb",
        "ext/json/lib/json/add/date.rb",
        "ext/json/lib/json/add/date_time.rb",
        "ext/json/lib/json/add/exception.rb",
        "ext/json/lib/json/add/ostruct.rb",
        "ext/json/lib/json/add/range.rb",
        "ext/json/lib/json/add/rational.rb",
        "ext/json/lib/json/add/regexp.rb",
        "ext/json/lib/json/add/struct.rb",
        "ext/json/lib/json/add/symbol.rb",
        "ext/json/lib/json/add/time.rb",
        "ext/json/lib/json/common.rb",
        "ext/json/lib/json/ext.rb",
        "ext/json/lib/json/generic_object.rb",
        "ext/json/lib/json/version.rb",
    ],
    outs = [
        "lib/json.rb",
        "lib/json/add/bigdecimal.rb",
        "lib/json/add/complex.rb",
        "lib/json/add/core.rb",
        "lib/json/add/date.rb",
        "lib/json/add/date_time.rb",
        "lib/json/add/exception.rb",
        "lib/json/add/ostruct.rb",
        "lib/json/add/range.rb",
        "lib/json/add/rational.rb",
        "lib/json/add/regexp.rb",
        "lib/json/add/struct.rb",
        "lib/json/add/symbol.rb",
        "lib/json/add/time.rb",
        "lib/json/common.rb",
        "lib/json/ext.rb",
        "lib/json/generic_object.rb",
        "lib/json/version.rb",
    ],
    cmd = """
cp $(location ext/json/lib/json.rb)                $(location lib/json.rb)
cp $(location ext/json/lib/json/add/bigdecimal.rb) $(location lib/json/add/bigdecimal.rb)
cp $(location ext/json/lib/json/add/complex.rb)    $(location lib/json/add/complex.rb)
cp $(location ext/json/lib/json/add/core.rb)       $(location lib/json/add/core.rb)
cp $(location ext/json/lib/json/add/date.rb)       $(location lib/json/add/date.rb)
cp $(location ext/json/lib/json/add/date_time.rb)  $(location lib/json/add/date_time.rb)
cp $(location ext/json/lib/json/add/exception.rb)  $(location lib/json/add/exception.rb)
cp $(location ext/json/lib/json/add/ostruct.rb)    $(location lib/json/add/ostruct.rb)
cp $(location ext/json/lib/json/add/range.rb)      $(location lib/json/add/range.rb)
cp $(location ext/json/lib/json/add/rational.rb)   $(location lib/json/add/rational.rb)
cp $(location ext/json/lib/json/add/regexp.rb)     $(location lib/json/add/regexp.rb)
cp $(location ext/json/lib/json/add/struct.rb)     $(location lib/json/add/struct.rb)
cp $(location ext/json/lib/json/add/symbol.rb)     $(location lib/json/add/symbol.rb)
cp $(location ext/json/lib/json/add/time.rb)       $(location lib/json/add/time.rb)
cp $(location ext/json/lib/json/common.rb)         $(location lib/json/common.rb)
cp $(location ext/json/lib/json/ext.rb)            $(location lib/json/ext.rb)
cp $(location ext/json/lib/json/generic_object.rb) $(location lib/json/generic_object.rb)
cp $(location ext/json/lib/json/version.rb)        $(location lib/json/version.rb)
"""
)

genrule(
    name = "ruby_ext/socket",
    srcs = [ "ext/socket/lib/socket.rb" ],
    outs = [ "lib/socket.rb" ],
    cmd = "cp $< $@",
)

genrule(
    name = "ruby_ext/date",
    srcs = [ "ext/date/lib/date.rb" ],
    outs = [ "lib/date.rb" ],
    cmd = "cp $< $@",
)

genrule(
    name = "ruby_ext/digest",
    srcs = [
        "ext/digest/lib/digest.rb",
        "ext/digest/sha2/lib/sha2.rb",
    ],
    outs = [
        "lib/digest.rb",
        "lib/digest/sha2.rb",
    ],
    cmd = """
cp $(location ext/digest/lib/digest.rb) $(location lib/digest.rb)
cp $(location ext/digest/sha2/lib/sha2.rb) $(location lib/digest/sha2.rb)
""",
)

genrule(
    name = "ruby_ext/psych",
    srcs = glob([
        "ext/psych/lib/psych.rb",
        "ext/psych/lib/psych/core_ext.rb",
        "ext/psych/lib/psych/visitors/depth_first.rb",
        "ext/psych/lib/psych/visitors/json_tree.rb",
        "ext/psych/lib/psych/visitors/emitter.rb",
        "ext/psych/lib/psych/visitors/visitor.rb",
        "ext/psych/lib/psych/visitors/yaml_tree.rb",
        "ext/psych/lib/psych/visitors/to_ruby.rb",
        "ext/psych/lib/psych/scalar_scanner.rb",
        "ext/psych/lib/psych/versions.rb",
        "ext/psych/lib/psych/omap.rb",
        "ext/psych/lib/psych/set.rb",
        "ext/psych/lib/psych/nodes.rb",
        "ext/psych/lib/psych/streaming.rb",
        "ext/psych/lib/psych/nodes/node.rb",
        "ext/psych/lib/psych/nodes/mapping.rb",
        "ext/psych/lib/psych/nodes/document.rb",
        "ext/psych/lib/psych/nodes/stream.rb",
        "ext/psych/lib/psych/nodes/sequence.rb",
        "ext/psych/lib/psych/nodes/scalar.rb",
        "ext/psych/lib/psych/nodes/alias.rb",
        "ext/psych/lib/psych/parser.rb",
        "ext/psych/lib/psych/class_loader.rb",
        "ext/psych/lib/psych/tree_builder.rb",
        "ext/psych/lib/psych/json/ruby_events.rb",
        "ext/psych/lib/psych/json/tree_builder.rb",
        "ext/psych/lib/psych/json/stream.rb",
        "ext/psych/lib/psych/json/yaml_events.rb",
        "ext/psych/lib/psych/coder.rb",
        "ext/psych/lib/psych/stream.rb",
        "ext/psych/lib/psych/syntax_error.rb",
        "ext/psych/lib/psych/y.rb",
        "ext/psych/lib/psych/visitors.rb",
        "ext/psych/lib/psych/deprecated.rb",
        "ext/psych/lib/psych/handler.rb",
        "ext/psych/lib/psych/handlers/recorder.rb",
        "ext/psych/lib/psych/handlers/document_stream.rb",
        "ext/psych/lib/psych/exception.rb",
    ]),
    outs = [
        "lib/psych.rb",
        "lib/psych/core_ext.rb",
        "lib/psych/visitors/depth_first.rb",
        "lib/psych/visitors/json_tree.rb",
        "lib/psych/visitors/emitter.rb",
        "lib/psych/visitors/visitor.rb",
        "lib/psych/visitors/yaml_tree.rb",
        "lib/psych/visitors/to_ruby.rb",
        "lib/psych/scalar_scanner.rb",
        "lib/psych/versions.rb",
        "lib/psych/omap.rb",
        "lib/psych/set.rb",
        "lib/psych/nodes.rb",
        "lib/psych/streaming.rb",
        "lib/psych/nodes/node.rb",
        "lib/psych/nodes/mapping.rb",
        "lib/psych/nodes/document.rb",
        "lib/psych/nodes/stream.rb",
        "lib/psych/nodes/sequence.rb",
        "lib/psych/nodes/scalar.rb",
        "lib/psych/nodes/alias.rb",
        "lib/psych/parser.rb",
        "lib/psych/class_loader.rb",
        "lib/psych/tree_builder.rb",
        "lib/psych/json/ruby_events.rb",
        "lib/psych/json/tree_builder.rb",
        "lib/psych/json/stream.rb",
        "lib/psych/json/yaml_events.rb",
        "lib/psych/coder.rb",
        "lib/psych/stream.rb",
        "lib/psych/syntax_error.rb",
        "lib/psych/y.rb",
        "lib/psych/visitors.rb",
        "lib/psych/deprecated.rb",
        "lib/psych/handler.rb",
        "lib/psych/handlers/recorder.rb",
        "lib/psych/handlers/document_stream.rb",
        "lib/psych/exception.rb",
    ],
    cmd = """
cp  $(location ext/psych/lib/psych.rb)                          $(location lib/psych.rb)
cp  $(location ext/psych/lib/psych/core_ext.rb)                 $(location lib/psych/core_ext.rb)
cp  $(location ext/psych/lib/psych/visitors/depth_first.rb)     $(location lib/psych/visitors/depth_first.rb)
cp  $(location ext/psych/lib/psych/visitors/json_tree.rb)       $(location lib/psych/visitors/json_tree.rb)
cp  $(location ext/psych/lib/psych/visitors/emitter.rb)         $(location lib/psych/visitors/emitter.rb)
cp  $(location ext/psych/lib/psych/visitors/visitor.rb)         $(location lib/psych/visitors/visitor.rb)
cp  $(location ext/psych/lib/psych/visitors/yaml_tree.rb)       $(location lib/psych/visitors/yaml_tree.rb)
cp  $(location ext/psych/lib/psych/visitors/to_ruby.rb)         $(location lib/psych/visitors/to_ruby.rb)
cp  $(location ext/psych/lib/psych/scalar_scanner.rb)           $(location lib/psych/scalar_scanner.rb)
cp  $(location ext/psych/lib/psych/versions.rb)                 $(location lib/psych/versions.rb)
cp  $(location ext/psych/lib/psych/omap.rb)                     $(location lib/psych/omap.rb)
cp  $(location ext/psych/lib/psych/set.rb)                      $(location lib/psych/set.rb)
cp  $(location ext/psych/lib/psych/nodes.rb)                    $(location lib/psych/nodes.rb)
cp  $(location ext/psych/lib/psych/streaming.rb)                $(location lib/psych/streaming.rb)
cp  $(location ext/psych/lib/psych/nodes/node.rb)               $(location lib/psych/nodes/node.rb)
cp  $(location ext/psych/lib/psych/nodes/mapping.rb)            $(location lib/psych/nodes/mapping.rb)
cp  $(location ext/psych/lib/psych/nodes/document.rb)           $(location lib/psych/nodes/document.rb)
cp  $(location ext/psych/lib/psych/nodes/stream.rb)             $(location lib/psych/nodes/stream.rb)
cp  $(location ext/psych/lib/psych/nodes/sequence.rb)           $(location lib/psych/nodes/sequence.rb)
cp  $(location ext/psych/lib/psych/nodes/scalar.rb)             $(location lib/psych/nodes/scalar.rb)
cp  $(location ext/psych/lib/psych/nodes/alias.rb)              $(location lib/psych/nodes/alias.rb)
cp  $(location ext/psych/lib/psych/parser.rb)                   $(location lib/psych/parser.rb)
cp  $(location ext/psych/lib/psych/class_loader.rb)             $(location lib/psych/class_loader.rb)
cp  $(location ext/psych/lib/psych/tree_builder.rb)             $(location lib/psych/tree_builder.rb)
cp  $(location ext/psych/lib/psych/json/ruby_events.rb)         $(location lib/psych/json/ruby_events.rb)
cp  $(location ext/psych/lib/psych/json/tree_builder.rb)        $(location lib/psych/json/tree_builder.rb)
cp  $(location ext/psych/lib/psych/json/stream.rb)              $(location lib/psych/json/stream.rb)
cp  $(location ext/psych/lib/psych/json/yaml_events.rb)         $(location lib/psych/json/yaml_events.rb)
cp  $(location ext/psych/lib/psych/coder.rb)                    $(location lib/psych/coder.rb)
cp  $(location ext/psych/lib/psych/stream.rb)                   $(location lib/psych/stream.rb)
cp  $(location ext/psych/lib/psych/syntax_error.rb)             $(location lib/psych/syntax_error.rb)
cp  $(location ext/psych/lib/psych/y.rb)                        $(location lib/psych/y.rb)
cp  $(location ext/psych/lib/psych/visitors.rb)                 $(location lib/psych/visitors.rb)
cp  $(location ext/psych/lib/psych/deprecated.rb)               $(location lib/psych/deprecated.rb)
cp  $(location ext/psych/lib/psych/handler.rb)                  $(location lib/psych/handler.rb)
cp  $(location ext/psych/lib/psych/handlers/recorder.rb)        $(location lib/psych/handlers/recorder.rb)
cp  $(location ext/psych/lib/psych/handlers/document_stream.rb) $(location lib/psych/handlers/document_stream.rb)
cp  $(location ext/psych/lib/psych/exception.rb)                $(location lib/psych/exception.rb)
"""
)

filegroup(
    name = "ruby_lib",
    srcs = [ "lib/rbconfig.rb", ":ruby_lib_staged" ],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "ruby_lib_staged",
    srcs = [
        ":ruby_ext/pathname",
        ":ruby_ext/bigdecimal",
        ":ruby_ext/json",
        ":ruby_ext/socket",
        ":ruby_ext/date",
        ":ruby_ext/digest",
        ":ruby_ext/psych",
    ] + glob([ "lib/**/*.rb" ]),
    visibility = ["//visibility:private"],
)


# wrapper script ###############################################################

genrule(
    name = "generate-ruby.sh",
    outs = [ "ruby.sh" ],
    cmd = """
cat >> $(location ruby.sh) <<EOF
#!/bin/bash

set -euo pipefail

base_dir="\$$(dirname \$${BASH_SOURCE[0]})"

# was this script invoked via 'bazel run"?
if [ -d "\$$base_dir/ruby.runfiles" ]; then
  export RUBYLIB="\$${RUBYLIB:-}:\$$base_dir/ruby.runfiles/ruby_2_4_3/lib"
else
  export RUBYLIB="\$${RUBYLIB:-}:\$$base_dir/lib"
fi

exec "\$$base_dir/bin/ruby" "\$$@"
EOF
    """,
)

sh_binary(
    name = "ruby",
    data = [ ":bin/ruby", ":ruby_lib" ],
    srcs = [ "ruby.sh" ],
    visibility = ["//visibility:public"],
)


# tests ########################################################################

genrule(
    name = "generate_smoke_test.sh",
    outs = [ "smoke_test.sh" ],
    cmd = """
cat > $(location smoke_test.sh) <<EOF
#!/bin/bash

export PATH="\$$(dirname \$$1):\$$PATH"

# Simple smoke-test
ruby -e '1 + 1'

# Require something from the stdlib
ruby -e 'require "set"'

EOF
""",
)

sh_test(
    name = "smoke_test",
    deps = [ ":ruby", "@bazel_tools//tools/bash/runfiles" ],
    srcs = [ "smoke_test.sh" ],
    args = [ "$(location :ruby)" ],
)

test_suite(
    name = "ruby-2.4",
    tests = [ ":smoke_test" ],
)
