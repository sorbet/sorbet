# vim: ft=bzl sw=4 ts=4 et

# caveats ######################################################################

# Hard-coded files
# * probes.h
# * enc/encinit.c

# Templated files
# * verconf.h is just a templated version of what ruby produces

config_setting(
    name = "darwin",
    values = {"host_cpu": "darwin"},
)

config_setting(
    name = "linux",
    values = {"host_cpu": "k8"},
)


# configuration variables ######################################################

RUBY_VERSION = "2.6.3"
RUBY_CORE_VERSION = "2.6.0"

ARCH_LINUX = "x86_64-unknown-linux"
ARCH_DARWIN = "x86_64-darwin18"

ARCH = select({
    ":linux": ARCH_LINUX,
    ":darwin": ARCH_DARWIN,
})

# NOTE: rbconfig expects to find itself in a directory of the form:
#
# > `lib/ruby/<RUBY_CORE_VERSION>/<ARCH>`
#
# as shown by the line that defines `TOPDIR`. `TOPDIR` is used when computing
# the relative path to the rest of the standard library, the `include`
# directory, and the `bin` directory when building gems with native code.
# Without installing everything into a tree that matches this, there will be
# errors when using `mkmf`.
LIB_PREFIX = "lib/ruby/" + RUBY_CORE_VERSION

INC_PREFIX = "lib/ruby/include"

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
        "*.mk",
        "include/**/*.h",
        "**/Makefile.in",
        "template/*",
    ]),
    cmd = """
CC=$(CC) CFLAGS=$(CC_FLAGS) CPPFLAGS=$(CC_FLAGS) $(location configure) --enable-load-relative --without-gmp --disable-jit-support > /dev/null
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
        "enc/unicode/12.1.0/*.h",
    ]),
    hdrs = glob([
        "*.h",
        "*.inc",
        "enc/shift_jis.c",
        "enc/jis/*.h",
        "ccan/**/*.h",
    ]),
    includes = [
        "enc",
        "enc/trans",
        "enc/unicode/12.1.0",
    ],
    visibility = [ "//visibility:public" ],
)

cc_library(
  name = "ruby_crypt",
  hdrs = [
    "missing/crypt.h",
    "missing/des_tables.c",
  ],
  srcs = select({
    ":linux": ["missing/crypt.c"],
    ":darwin": [],
  }),
  deps = [ ":ruby_headers" ],
  visibility = [ "//visibility:public" ],
)

RUBY_COPTS = [
    "-D_XOPEN_SOURCE",
    "-Wno-constant-logical-operand",
    "-Wno-parentheses",
    "-D_REENTRANT",
    "-DNO_INITIAL_LOAD_PATH",
    "-Wno-macro-redefined", # they redefine NDEBUG

    # TODO: is this really necessary?
    "-Wno-string-plus-int",
] + select({
    ":linux": [],
    ":darwin": ["-D_DARWIN_C_SOURCE"],
})


# coroutine ####################################################################

cc_library(
    name = "coroutine/amd64",
    srcs = [
        "coroutine/amd64/Context.S",
        "coroutine/amd64/Context.h",
    ],
    hdrs = [
        "coroutine/amd64/Context.h",
    ],
    visibility = [ "//visibility:public" ],
)


# miniruby #####################################################################

filegroup(
    name = "miniruby_lib",
    srcs = glob([ "lib/**/*.rb" ]),
)

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

    data = [ ":miniruby_lib" ],

    srcs = [
        "ast.c",
        "addr2line.c",
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
        "transient_heap.c",
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
    ] + select({
      ":linux": [
          "missing/strlcpy.c",
          "missing/strlcat.c",
      ],
      ":darwin": [],
    }),

    deps = [
        ":miniruby_private_headers",
        ":ruby_headers",
        ":ruby_private_headers",
        ":coroutine/amd64",
        "@zlib//:zlib",
    ],

    copts = RUBY_COPTS + [ "-DRUBY_EXPORT" ],

    linkopts = select({
      ":linux": [
        "-lpthread",
        "-lcrypt",
        "-lrt",
      ],
      ":darwin": [
        "-framework",
        "Foundation",
        "-lpthread",
      ],
    }),

    includes = [ "include", "enc/unicode/12.1.0" ] + select({
      ":linux": ["missing"],
      ":darwin": []
    }),

    visibility = ["//visibility:public"],
)

# full ruby ####################################################################

RBCONFIG_LINUX = "/".join([LIB_PREFIX, ARCH_LINUX, "rbconfig.rb"])
RBCONFIG_DARWIN = "/".join([LIB_PREFIX, ARCH_DARWIN, "rbconfig.rb"])

RBCONFIG = select({
    ":linux": [RBCONFIG_LINUX],
    ":darwin": [RBCONFIG_DARWIN],
})

genrule(
    name = "generate-linux-rbconfig",
    srcs = [
        ":miniruby_lib",
        "bin/miniruby",
        "tool/mkconfig.rb",
        "config.status",
        "version.h",
        "lib/irb.rb",
    ],
    outs = [RBCONFIG_LINUX],
    cmd = """
cp $(location config.status) config.status
$(location bin/miniruby) \
        -I $$(dirname $(location lib/irb.rb)) \
        $(location tool/mkconfig.rb) \
        -cross_compiling=no \
        -arch=""" + ARCH_LINUX + """\
        -version=""" + RUBY_VERSION + """\
        -install_name=ruby \
        -so_name=ruby \
        -unicode_version=12.1.0 \
    > $@
""",
)

genrule(
    name = "generate-darwin-rbconfig",
    srcs = [
        ":miniruby_lib",
        "bin/miniruby",
        "tool/mkconfig.rb",
        "config.status",
        "version.h",
        "lib/irb.rb",
    ],
    outs = [RBCONFIG_DARWIN],
    cmd = """
cp $(location config.status) config.status
$(location bin/miniruby) \
        -I $$(dirname $(location lib/irb.rb)) \
        $(location tool/mkconfig.rb) \
        -cross_compiling=no \
        -arch=""" + ARCH_DARWIN + """\
        -version=""" + RUBY_VERSION + """\
        -install_name=ruby \
        -so_name=ruby \
        -unicode_version=12.1.0 \
    > $@
""",
)

genrule(
    name = "generate_prelude",
    srcs = [
        ":miniruby_lib",
        "bin/miniruby",
        "lib/erb.rb",
        "tool/colorize.rb",
        "tool/generic_erb.rb",
        "tool/vpath.rb",
        "template/prelude.c.tmpl",
        "prelude.rb",
        "gem_prelude.rb",
    ],
    outs = [ "prelude.c" ],
    cmd = """
$(location bin/miniruby) \
    -I $$(dirname $(location lib/erb.rb)) \
    -I $$(dirname $(location tool/vpath.rb)) \
    -I $$(dirname $(location prelude.rb)) \
    $(location tool/generic_erb.rb) \
    -o $@ \
    $(location template/prelude.c.tmpl) \
    $(location prelude.rb) \
    $(location gem_prelude.rb)
""",
)

genrule(
    name = "verconf",
    srcs = [ "bin/miniruby" ],
    outs = [ "verconf.h" ],
    cmd = """
prefix="/ruby.runfiles/ruby_2_6_3"
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
        "main.c",
    ],
    deps = [
            ":libruby"
    ]
)

cc_library(
    name = "libruby",

    srcs = [
        "prelude.c",
        "verconf.h",

        "ext/extinit.c",

        "enc/encinit.c",
        "enc/ascii.c",
        "enc/us_ascii.c",
        "enc/unicode.c",
        "enc/utf_8.c",

        "addr2line.c",
        "ast.c",
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
        "transient_heap.c",
        "util.c",
        "variable.c",
        "version.c",
        "vm.c",
        "vm_backtrace.c",
        "vm_dump.c",
        "vm_trace.c",
        "missing/explicit_bzero.c",
        "missing/setproctitle.c",
    ] + select({
        ":linux": [
            "missing/strlcpy.c",
            "missing/strlcat.c",
        ],
        ":darwin": [],
    }),

    deps = [
        ":miniruby_private_headers",
        ":coroutine/amd64",
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
        "@zlib//:zlib",
    ] + select({
        ":linux": [":ruby_crypt"],
        ":darwin": [],
    }),

    linkopts = select({
        ":linux": ["-lpthread", "-lrt"],
        ":darwin": [
            "-framework",
            "Foundation",
            "-lpthread",
        ],
    }),
    copts = RUBY_COPTS + ["-DRUBY_EXPORT","-DRUBY"],

    visibility = ["//visibility:public"],
)

genrule(
    name = "enc-generated-deps",
    srcs = [
        ":miniruby_lib",
        "bin/miniruby",
        "tool/colorize.rb",
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
    copts = RUBY_COPTS + [
        "-DRUBY_EXPORT",
        # IMPORTANT: without this no Init functions are generated
        "-DEXTSTATIC=1",
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
    copts = RUBY_COPTS + [
        # IMPORTANT: without this no Init functions are generated
        "-DRUBY", "-DONIG_ENC_REGISTER=rb_enc_register",

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
    copts = RUBY_COPTS + [
        "-DHAVE_LABS",
        "-DHAVE_LLABS",
        "-DHAVE_FINITE",
        "-DHAVE_RB_RATIONAL_NUM",
        "-DHAVE_RB_RATIONAL_DEN",
        "-DHAVE_RB_ARRAY_CONST_PTR",
        "-DHAVE_RB_SYM2STR",
        "'-DRUBY_BIGDECIMAL_VERSION=\"1.4.1\"'",
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
        ":miniruby_lib",
        "bin/miniruby",
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
        "-DENABLE_IPV6",
        "-DGAI_STRERROR_CONST",
        "-DHAVE_ARPA_INET_H",
        "-DHAVE_ARPA_NAMESER_H",
        "-DHAVE_CONST_AF_UNIX",
        "-DHAVE_CONST_SCM_RIGHTS",
        "-DHAVE_FREEADDRINFO",
        "-DHAVE_GAI_STRERROR",
        "-DHAVE_GETADDRINFO",
        "-DHAVE_GETHOSTBYNAME2",
        "-DHAVE_GETHOSTNAME",
        "-DHAVE_GETIFADDRS",
        "-DHAVE_GETNAMEINFO",
        "-DHAVE_GETSERVBYPORT",
        "-DHAVE_IFADDRS_H",
        "-DHAVE_IF_INDEXTONAME",
        "-DHAVE_IF_NAMETOINDEX",
        "-DHAVE_INET_NTOP",
        "-DHAVE_INET_PTON",
        "-DHAVE_NETINET_IN_SYSTM_H",
        "-DHAVE_NETINET_TCP_H",
        "-DHAVE_NETINET_UDP_H",
        "-DHAVE_NET_ETHERNET_H",
        "-DHAVE_NET_IF_H",
        "-DHAVE_RECVMSG",
        "-DHAVE_RESOLV_H",
        "-DHAVE_SENDMSG",
        "-DHAVE_SOCKET",
        "-DHAVE_SOCKETPAIR",
        "-DHAVE_STRUCT_IN_PKTINFO_IPI_SPEC_DST",
        "-DHAVE_STRUCT_MSGHDR_MSG_CONTROL",
        "-DHAVE_ST_IPI_SPEC_DST",
        "-DHAVE_ST_MSG_CONTROL",
        "-DHAVE_SYS_IOCTL_H",
        "-DHAVE_SYS_PARAM_H",
        "-DHAVE_SYS_UN_H",
        "-DHAVE_TYPE_SOCKLEN_T",
        "-DHAVE_TYPE_STRUCT_ADDRINFO",
        "-DHAVE_TYPE_STRUCT_IN6_PKTINFO",
        "-DHAVE_TYPE_STRUCT_IN_PKTINFO",
        "-DHAVE_TYPE_STRUCT_IPV6_MREQ",
        "-DHAVE_TYPE_STRUCT_IP_MREQ",
        "-DHAVE_TYPE_STRUCT_IP_MREQN",
        "-DHAVE_TYPE_STRUCT_SOCKADDR_STORAGE",
        "-DHAVE_TYPE_STRUCT_SOCKADDR_UN",
        "-DNEED_IF_INDEXTONAME_DECL",
        "-DNEED_IF_NAMETOINDEX_DECL",
        "-Wno-dangling-else",
        "-Wno-unused-const-variable",
    ] + select({
        ":linux": [
            "-DFD_PASSING_WORK_WITH_RECVMSG_MSG_PEEK",
            "-DHAVE_ACCEPT4",
            "-DHAVE_CONST_TCP_CLOSE",
            "-DHAVE_CONST_TCP_CLOSE_WAIT",
            "-DHAVE_CONST_TCP_CLOSING",
            "-DHAVE_CONST_TCP_ESTABLISHED",
            "-DHAVE_CONST_TCP_FIN_WAIT1",
            "-DHAVE_CONST_TCP_FIN_WAIT2",
            "-DHAVE_CONST_TCP_LAST_ACK",
            "-DHAVE_CONST_TCP_LISTEN",
            "-DHAVE_CONST_TCP_SYN_RECV",
            "-DHAVE_CONST_TCP_SYN_SENT",
            "-DHAVE_CONST_TCP_TIME_WAIT",
            "-DHAVE_NETPACKET_PACKET_H",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_ADVMSS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_ATO",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_BACKOFF",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_CA_STATE",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_FACKETS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_RECV",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_LAST_ACK_SENT",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_RECV",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_LAST_DATA_SENT",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_LOST",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_OPTIONS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_PMTU",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_PROBES",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RCV_MSS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RCV_RTT",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RCV_SPACE",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RCV_SSTHRESH",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_REORDERING",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RETRANS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RETRANSMITS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RTO",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RTT",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_RTTVAR",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_SACKED",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_SND_CWND",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_SND_MSS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_SND_SSTHRESH",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_STATE",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_TOTAL_RETRANS",
            "-DHAVE_STRUCT_TCP_INFO_TCPI_UNACKED",
            "-DHAVE_ST_TCPI_ADVMSS",
            "-DHAVE_ST_TCPI_ATO",
            "-DHAVE_ST_TCPI_BACKOFF",
            "-DHAVE_ST_TCPI_CA_STATE",
            "-DHAVE_ST_TCPI_FACKETS",
            "-DHAVE_ST_TCPI_LAST_ACK_RECV",
            "-DHAVE_ST_TCPI_LAST_ACK_SENT",
            "-DHAVE_ST_TCPI_LAST_DATA_RECV",
            "-DHAVE_ST_TCPI_LAST_DATA_SENT",
            "-DHAVE_ST_TCPI_LOST",
            "-DHAVE_ST_TCPI_OPTIONS",
            "-DHAVE_ST_TCPI_PMTU",
            "-DHAVE_ST_TCPI_PROBES",
            "-DHAVE_ST_TCPI_RCV_MSS",
            "-DHAVE_ST_TCPI_RCV_RTT",
            "-DHAVE_ST_TCPI_RCV_SPACE",
            "-DHAVE_ST_TCPI_RCV_SSTHRESH",
            "-DHAVE_ST_TCPI_REORDERING",
            "-DHAVE_ST_TCPI_RETRANS",
            "-DHAVE_ST_TCPI_RETRANSMITS",
            "-DHAVE_ST_TCPI_RTO",
            "-DHAVE_ST_TCPI_RTT",
            "-DHAVE_ST_TCPI_RTTVAR",
            "-DHAVE_ST_TCPI_SACKED",
            "-DHAVE_ST_TCPI_SND_CWND",
            "-DHAVE_ST_TCPI_SND_MSS",
            "-DHAVE_ST_TCPI_SND_SSTHRESH",
            "-DHAVE_ST_TCPI_STATE",
            "-DHAVE_ST_TCPI_TOTAL_RETRANS",
            "-DHAVE_ST_TCPI_UNACKED",
            "-DHAVE_TYPE_STRUCT_TCP_INFO",
        ],
        ":darwin": [
            "-DHAVE_FREEHOSTENT",
            "-DHAVE_GETIPNODEBYNAME",
            "-DHAVE_GETPEEREID",
            "-DHAVE_NETINET_TCP_FSM_H",
            "-DHAVE_NET_IF_DL_H",
            "-DHAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN",
            "-DHAVE_STRUCT_SOCKADDR_IN_SIN_LEN",
            "-DHAVE_STRUCT_SOCKADDR_SA_LEN",
            "-DHAVE_STRUCT_SOCKADDR_UN_SUN_LEN",
            "-DHAVE_ST_SA_LEN",
            "-DHAVE_ST_SIN6_LEN",
            "-DHAVE_ST_SIN_LEN",
            "-DHAVE_ST_SUN_LEN",
            "-DHAVE_SYS_SOCKIO_H",
            "-DHAVE_SYS_UCRED_H",
            "-DHAVE_SYS_UIO_H",
            "-DHAVE_TYPE_STRUCT_SOCKADDR_DL",
            "-DINET6",
        ]
    }),
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
    copts = RUBY_COPTS,
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

load("@com_stripe_ruby_typer//third_party/ruby:utils.bzl", "install_file", "install_dir")

filegroup(
    name = "ruby_lib",
    srcs =
        install_dir(
            src_prefix = "ext/pathname/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/bigdecimal/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/json/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/socket/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/date/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/digest/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "ext/digest/sha2/lib",
            out_prefix = LIB_PREFIX + "/digest",
        ) +

        install_dir(
            src_prefix = "ext/psych/lib",
            out_prefix = LIB_PREFIX,
        ) +

        install_dir(
            src_prefix = "lib",
            out_prefix = LIB_PREFIX,
        ) +

        RBCONFIG,

    visibility = ["//visibility:public"],
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
  base_dir="\$${base_dir}/ruby.runfiles/ruby_2_6_3"
fi

RUBYLIB="\$${RUBYLIB:-}\$${RUBYLIB:+:}\$${base_dir}/lib/ruby/2.6.0/""" + ARCH + """"
RUBYLIB="\$${RUBYLIB}:\$${base_dir}/lib/ruby/2.6.0"
export RUBYLIB

# TODO: Something is re-discovering the stdlib outside of the path provided by
# RUBYLIB, and when the original is a symlink it causes stdlib files to be
# loaded multiple times.
exec "\$$base_dir/bin/ruby" -W0 "\$$@"
EOF
    """,
)

filegroup(
    name = "ruby_runtime_env",
    srcs =
        install_dir(
            src_prefix = "include",
            out_prefix = INC_PREFIX,
        ) + [
        ":bin/ruby",
        ":ruby_lib",
        install_file(
            name = "install-ruby-config.h",
            src = "include/ruby/config.h",
            out = INC_PREFIX + "/ruby/config.h",
        ),
    ],
)

sh_binary(
    name = "ruby",
    data = [ ":ruby_runtime_env" ],
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
    deps = [ "@bazel_tools//tools/bash/runfiles" ],
    data = [ ":ruby" ],
    srcs = [ "smoke_test.sh" ],
    args = [ "$(location :ruby)" ],
)

test_suite(
    name = "ruby-2.6",
    tests = [ ":smoke_test" ],
)
