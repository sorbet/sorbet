cc_library(
    name = "prism",
    srcs = glob(["src/**/*.c"]),
    hdrs = glob(["include/**/*.h"]),
    copts = ["-Wno-implicit-fallthrough"],
    # Route Prism's allocations through Sorbet's per-parse arena (see parser/prism/prism_xallocator.h). `defines`
    # propagates to everything that includes prism.h, so all of it sees the same xmalloc/xfree.
    defines = ["PRISM_XALLOCATOR"],
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = ["@com_stripe_ruby_typer//parser/prism:xallocator"],
)
