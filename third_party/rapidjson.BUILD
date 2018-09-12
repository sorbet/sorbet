cc_library(
    name = "rapidjson",
    hdrs = glob(["include/rapidjson/**/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    defines = [
        "RAPIDJSON_HAS_STDSTRING",
        "RAPIDJSON_HAS_CXX11_RANGE_FOR",
        "RAPIDJSON_HAS_CXX11_RVALUE_REFS",
        "RAPIDJSON_HAS_CXX11_TYPETRAITS",
        "RAPIDJSON_ASSERT(x)=ENFORCE(x)",
        ]
)
