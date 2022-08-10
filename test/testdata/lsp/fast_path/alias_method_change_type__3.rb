# typed: true
# spacer for exclude-from-file-update

# Note that this file does not mention `to_method`, only `from_method`

T.reveal_type(A.new.from_method) # error: `Integer`
