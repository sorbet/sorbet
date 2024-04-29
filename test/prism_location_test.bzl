def prism_test_suite(name, srcs):
    for src in srcs:
        test_name = src.replace(".rb", "").replace("/", "_") + "_location_test"
        native.sh_test(
            name = test_name,
            srcs = ["prism_location_test.sh"],
            args = [src],
            tags = ["prism_location_test"],
            data = [
                "//main:sorbet",
                "@bazel_tools//tools/bash/runfiles",
                src,
            ],
        )
