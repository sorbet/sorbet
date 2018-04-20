LOG_FILE=$(mktemp)
main/ruby-typer -e '1' -q --debug-log-file="$LOG_FILE"
echo LOG BEGINS
# only keep message parts and drop timings
grep -v "ms" "$LOG_FILE" | grep -v debug-log-file
