# Hold stdin open while Sorbet executes to prevent reader thread from closing process early.
yes " " | main/sorbet --silence-dev-message --lsp --watchman-path /fake/watchman/path test/cli/lsp-watchman-exit 2>&1