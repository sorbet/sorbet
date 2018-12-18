LOG_FILE=$(mktemp)
main/sorbet --silence-dev-message -e '1' -q --debug-log-file="$LOG_FILE"
echo LOG BEGINS
# only keep message parts, drop timings and the entire counter section
sed -E "s/\\[[0-9]+\\-[0-9]+-[0-9]+ [0-9]+:[0-9]+:[0-9]+.[0-9]+\\]/TIMESTAMP/"< "$LOG_FILE" |  # replace timestamps
  grep 'TIMESTAMP'                                                                       |  # Only give first lines
  grep -Eve ': [0-9]+\.?[0-9]*ms$'                                                       |  # remove timings
  grep -v debug-log-file                                                                     # remove header line that contains generated log name
rm "$LOG_FILE"
