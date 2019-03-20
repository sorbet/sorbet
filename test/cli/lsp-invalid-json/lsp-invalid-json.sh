echo "Content-Length: 1

" | main/sorbet --silence-dev-message --lsp --disable-watchman test/cli/lsp-invalid-json 2>&1