load("@com_stripe_ruby_typer//test:pipeline_test.bzl", "pipeline_tests")

def compiler_tests(name, files):
    pipeline_tests(
        name,
        files,
        "PosTests",
    )
