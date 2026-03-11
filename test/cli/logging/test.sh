LOG_FILE=$(mktemp)
echo "STDOUT BEGINS"
main/sorbet --silence-dev-message -e '1' --debug-log-file="$LOG_FILE" 2>&1
echo "DEBUG LOG BEGINS"
# only keep message parts, drop timings and the entire counter section
sed -E "s/[0-9]+\\-[0-9]+-[0-9]+T[0-9]+:[0-9]+:[0-9]+.[0-9]+/TIMESTAMP/"< "$LOG_FILE" |  # replace timestamps
  grep 'TIMESTAMP'                                                                    |  # Only give first lines
  grep -Eve ': [0-9]+\.?[0-9]*(e(-|\+))?[0-9]*ms$'                                    |  # remove timings
  grep -v debug-log-file                                                                 # remove header line that contains generated log name
rm "$LOG_FILE"
