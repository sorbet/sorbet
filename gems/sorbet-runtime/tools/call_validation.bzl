def call_validation_rb_files():
    tests = []
    updaters = []

    configs = [
        ("2_7", "--bind-call=true"),
    ]

    for version, opts in configs:
        committed = "lib/types/private/methods/call_validation_{}.rb".format(version)
        generated = "lib/types/private/methods/call_validation_{}_gen.rb".format(version)

        native.genrule(
            name = "generate_call_validation_{}_rb".format(version),
            outs = [generated],
            cmd = "$(location :generate_call_validation) {} > $(location {})".format(opts, generated),
            tools = [
                ":generate_call_validation",
            ],
        )

        test_name = "verify_call_validation_{}_rb".format(version)
        native.sh_test(
            name = test_name,
            srcs = ["test/verify_call_validation.sh"],
            args = [
                "test",
                "$(locations {})".format(committed),
                "$(locations {})".format(generated),
            ],
            data = [
                committed,
                generated,
            ],
            size = "small",
        )
        tests.append(test_name)

        updater_name = "update_call_validation_{}_rb".format(version)
        native.sh_test(
            name = updater_name,
            srcs = ["test/verify_call_validation.sh"],
            args = [
                "update",
                "$(locations {})".format(committed),
                "$(locations {})".format(generated),
            ],
            data = [
                committed,
                generated,
            ],
            size = "small",
            tags = [
                # Avoid being caught with `//...`
                "manual",
                # Forces the test to be run locally, without sandboxing
                "local",
                # Unconditionally run this rule, and don't run in the sandbox
                "external",
            ],
        )
        updaters.append(updater_name)

    native.test_suite(
        name = "verify_call_validation",
        tests = tests,
    )

    native.test_suite(
        name = "update_call_validation",
        tests = updaters,
        tags = ["manual"],
    )
