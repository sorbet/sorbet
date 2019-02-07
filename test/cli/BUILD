load(":cli_test.bzl", "cli_test", "update_test")

test_suite(
    name = "cli",
    tests = [cli_test(script) for script in glob(["*/*.sh"])],
)

update_test()
