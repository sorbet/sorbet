cc_library(
    name = "skarupke_maps",
    srcs = [],
    hdrs = ["bytell_hash_map.hpp", "flat_hash_map.hpp", "unordered_map.hpp"],
    visibility = ["//visibility:public"],
    linkstatic = select({
            "@com_stripe_ruby_typer//tools/config:linkshared": 0,
            "//conditions:default": 1,
        }),
)
