# Should ignore second error, as LSP should exit before processing it.
echo -e "Content-Length: 1\r\n\r\n\nContent-Length: 47\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"exit\",\"params\":null}Content-Length: 1\r\n\r\n\n" | main/sorbet --silence-dev-message --lsp --disable-watchman --dir test/cli/lsp-invalid-json-and-exit 2>&1
