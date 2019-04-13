#!/usr/bin/env bash
set -e


IS_REGENERATING_REC_FILE=false

if [[ $# -eq 0 ]] ; then
    echo 'Need a path to lsp recording as an argument'
    exit 1
elif [[ $# -eq 2 ]]; then
    IS_REGENERATING_REC_FILE=true
fi


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

main/sorbet --silence-dev-message --lsp --disable-watchman --enable-lsp-all "$(dirname "$recfile")" < "$in_pipe" > "$out_pipe" &

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
        echo -e -n "Content-Length: ${bytelen}\r\n\r\n">&"$IN_FD"
        # Don't interpret escape characters in payload.
        echo -n "$payload">&"$IN_FD"
        if [ "$IS_REGENERATING_REC_FILE" == true ]; then
            echo "$line"
        fi

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
            if [ "$IS_REGENERATING_REC_FILE" != true ]; then
                if [[ "$payload" != "$expected_payload" ]];
                then
                    diff -u  <(echo "$expected_payload" | python -m json.tool)\
                         <(echo "$payload" | python -m json.tool)
                    exit 1
                fi
            else
                echo "${WRITE_PREFIX}${payload}"
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
