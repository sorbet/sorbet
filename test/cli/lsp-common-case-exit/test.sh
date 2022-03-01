#!/usr/bin/env bash
set -e

# we need to keep output & input pipes open
# if we don't do anything special, they will be
# closed after the first command is run.

in_pipe="$(mktemp -u)"
mkfifo -m 600 "$in_pipe"

#out_pipe="$(mktemp -u)"
#mkfifo -m 600 "$out_pipe"

cleanup() {
    rm "$in_pipe"
}
trap cleanup exit

# TODO: Delurk this and support running Sorbet with a trace input.
main/sorbet --silence-dev-message --lsp --disable-watchman --dir test/cli/lsp-common-case-exit < "$in_pipe" &
sorbet_pid=$!

# This should be
# exec {IN_FD}>"$in_pipe"
# but mac os has ancient version of bash.
exec 100>"$in_pipe"
IN_FD=100

echo -e 'Content-Length: 2053\r\n\r\n{"jsonrpc":"2.0","id":0,"method":"initialize","params":{"processId":1,"rootPath":"/Users/jvilk/stripe/pay-server","rootUri":"file:///Users/jvilk/stripe/pay-server","capabilities":{"workspace":{"applyEdit":true,"workspaceEdit":{"documentChanges":true},"didChangeConfiguration":{"dynamicRegistration":true},"didChangeWatchedFiles":{"dynamicRegistration":true},"symbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"executeCommand":{"dynamicRegistration":true},"configuration":true,"workspaceFolders":true},"textDocument":{"publishDiagnostics":{"relatedInformation":true},"synchronization":{"dynamicRegistration":true,"willSave":true,"willSaveWaitUntil":true,"didSave":true},"completion":{"dynamicRegistration":true,"contextSupport":true,"completionItem":{"snippetSupport":true,"commitCharactersSupport":true,"documentationFormat":["markdown","plaintext"]},"completionItemKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25]}},"hover":{"dynamicRegistration":true,"contentFormat":["markdown","plaintext"]},"signatureHelp":{"dynamicRegistration":true,"signatureInformation":{"documentationFormat":["markdown","plaintext"]}},"definition":{"dynamicRegistration":true},"references":{"dynamicRegistration":true},"documentHighlight":{"dynamicRegistration":true},"documentSymbol":{"dynamicRegistration":true,"symbolKind":{"valueSet":[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26]}},"codeAction":{"dynamicRegistration":true},"codeLens":{"dynamicRegistration":true},"formatting":{"dynamicRegistration":true},"rangeFormatting":{"dynamicRegistration":true},"onTypeFormatting":{"dynamicRegistration":true},"rename":{"dynamicRegistration":true},"documentLink":{"dynamicRegistration":true},"typeDefinition":{"dynamicRegistration":true},"implementation":{"dynamicRegistration":true},"colorProvider":{"dynamicRegistration":true}}},"trace":"off","workspaceFolders":[{"uri":"file:///Users/jvilk/stripe/pay-server","name":"pay-server"}]}}Content-Length: 52\r\n\r\n{"jsonrpc":"2.0","method":"initialized","params":{}}Content-Length: 58\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"shutdown","params":null}Content-Length: 47\r\n\r\n{"jsonrpc":"2.0","method":"exit","params":null}'>&"$IN_FD"
# Should exit cleanly.
wait $sorbet_pid
