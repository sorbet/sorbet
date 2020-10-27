# Pass two directories (it doesn't matter what they are), and verify that we get the right error back
main/sorbet --silence-dev-message --lsp --dir test/cli/lsp-two-input-dirs --dir test/cli 2>/dev/null
