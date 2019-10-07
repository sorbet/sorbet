load("@com_stripe_ruby_typer//test:pipeline_test.bzl", "pipeline_tests")

def compiler_tests(files):
    pipeline_tests(
        "test_corpus",
        files,
        "PosTests",
    )
