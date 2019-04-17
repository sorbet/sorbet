# Generate large message (>10KiB) and send it to Sorbet. Forces Sorbet to read message in multiple read() calls.
# Make it invalid JSON so Sorbet echoes it back to us, confirming that it read the message correctly.
LSP_PAYLOAD="$(yes "a" | tr -d '\n' | head -c 10245)"
LSP_MESSAGE="Content-Length: ${#LSP_PAYLOAD}\r\n\r\n$LSP_PAYLOAD"

echo -e "$LSP_MESSAGE" | main/sorbet --silence-dev-message --lsp --disable-watchman test/cli/lsp-large-message 2>&1