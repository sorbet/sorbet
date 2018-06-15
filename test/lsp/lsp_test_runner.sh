#!/usr/bin/env bash
set -e

recfile="$1"
READ_PREFIX="Read: "
WRITE_PREFIX="Write: "

# we need to keep output & input pipes open
# if we don't do anything special, they will be
# closed after the first command is run.

in_pipe="$(mktemp -u)"
mkfifo -m 600 "$in_pipe"

out_pipe="$(mktemp -u)"
mkfifo -m 600 "$out_pipe"

cleanup() {
    rm "$in_pipe"
    rm "$out_pipe"
}
trap cleanup exit

main/sorbet --lsp < "$in_pipe" > "$out_pipe" &

# This should be
# exec {IN_FD}>"$in_pipe"
# but mac os has ancient version of bash.
exec 100>"$in_pipe"
exec 200<"$out_pipe"
IN_FD=100
OUT_FD=200

# Uncomment this line to be able to attach in debugger
# echo "lldb -p ${child_pid}" | pbcopy

while read -r line
do
    if [[ "$line" == ${READ_PREFIX}* ]] ;
    then
       payload=${line#$READ_PREFIX}
       bytelen=${#payload}

        text="Content-Length: ${bytelen}

$payload"
        echo -n "$text">&"$IN_FD"

    elif [[ "$line" == ${WRITE_PREFIX}* ]] ;
    then
        expected_payload=${line#$WRITE_PREFIX}
        read -r -u "$OUT_FD" header
        if [[ "$header" == "Content-Length: "* ]] ;
        then
            bytelen=${header#"Content-Length: "}
            bytelen=${bytelen%$'\r'}
            read -r -u "$OUT_FD" emptyline
            if [[ "$emptyline" != $'\r' ]];
            then
                echo "Expected empty line after header"
                exit 3
            fi
            read -r -n "${bytelen}" -u "$OUT_FD" payload
            if [[ "$payload" != "$expected_payload" ]];
            then
                diff  <(echo "$expected_payload" ) <(echo "$payload")
                exit 1
            fi
        else
            echo "Expected Context-Length:XXX, got $header"
            exit 4
        fi
    else
      echo "Unknown format: $line"
      exit 2
    fi
done < "$recfile"
