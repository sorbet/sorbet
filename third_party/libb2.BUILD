BLAKE2_COPTS = [
    "-Wno-unused-const-variable",
    "-Wno-unused-function",
]

genrule(
    name = "stub_config",
    srcs = [],
    outs = ["src/config.h"],
    cmd = """
    cat > $@ << EOF
#define HAVE_AVX
#define HAVE_SSE2
#define HAVE_SSE3
#define HAVE_SSSE3
#define HAVE_SSE4_1
#define HAVE_SSE4_2
EOF
""",
)

cc_library(
    name = "com_github_blake2_libb2",
    srcs = ["src/blake2s.c", "src/blake2b.c", "src/config.h"] + glob(["src/*.h"]),
    hdrs = [
        "src/blake2.h",
    ],
    copts = BLAKE2_COPTS + ["-msse2", "-mssse3", "-msse4.1", "-mavx"],
    includes = [
        "src",
    ],
    defines = ["SUFFIX="],
    visibility = ["//visibility:public"],
)
