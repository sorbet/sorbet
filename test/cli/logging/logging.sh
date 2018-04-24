LOG_FILE=$(mktemp)
main/ruby-typer -e '1' -q --debug-log-file="$LOG_FILE"
echo LOG BEGINS
# only keep message parts, drop timings and the entire counter section
sed -n '/^/,/^Counters and Histograms:/p;/^Counters and Histograms:/q' < "$LOG_FILE" |  # remove counters
  grep -Eve ': [0-9]+ms$'                                                            |  # remove timings
  grep -v debug-log-file                                                                # remove header line that contains generated log name
rm "$LOG_FILE"
