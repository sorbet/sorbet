load(":cli_test.bzl", "cli_test", "update_test")

[
    cli_test(script)
    for script in glob(["*/*.sh"])
]

update_test()
