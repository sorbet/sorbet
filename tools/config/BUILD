package(default_visibility = ["//visibility:public"])

config_setting(
    name = "darwin_x86_64",
    constraint_values = [
        "@platforms//os:osx",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "linux_arm64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:arm64",
    ],
)

config_setting(
    name = "opt",
    values = {"compilation_mode": "opt"},
)

config_setting(
    name = "dbg",
    values = {"compilation_mode": "dbg"},
)

config_setting(
    name = "fastbuild",
    values = {"compilation_mode": "fastbuild"},
)

config_setting(
    name = "sanitized",
    values = {
        "define": "unsanitized=false",
    },
)

config_setting(
    name = "webasm",
    values = {
        "cpu": "webasm",
    },
)

config_setting(
    name = "webasm_opt",
    values = {
        "cpu": "webasm",
        "compilation_mode": "opt",
    },
)

config_setting(
    name = "linkshared",
    values = {
        "define": "linkshared=true",
    },
)

config_setting(
    name = "darwindbg",
    constraint_values = [
        "@platforms//os:osx",
    ],
    values = {
        "compilation_mode": "dbg",
    },
)

config_setting(
    name = "release",
    values = {
        "define": "release=true",
    },
)

config_setting(
    name = "jit_enabled",
    values = {
        "define": "jit_enabled=true",
    },
)
