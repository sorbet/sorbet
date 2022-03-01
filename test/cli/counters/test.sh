LOG_FILE=$(mktemp)
main/sorbet --silence-dev-message -e '1' -q --counters --debug-log-file="$LOG_FILE"
echo LOG BEGINS
# only keep message parts, drop timings and the entire counter section
grep "types.input.lines" "$LOG_FILE"
rm "$LOG_FILE"
