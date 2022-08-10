# shellcheck disable=SC2069
main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer test/cli/autogen-errors/autogen-errors.rb 2>&1 1>/dev/null
