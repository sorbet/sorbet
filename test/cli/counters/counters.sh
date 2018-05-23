LOG_FILE=$(mktemp)
main/sorbet -e '1' -q --debug-log-file="$LOG_FILE"
echo LOG BEGINS
# only keep message parts, drop timings and the entire counter section
grep "types.input.lines" "$LOG_FILE"
rm "$LOG_FILE"
