# typed: true
# spacer for exclude-from-file-update

# Note that this file does not mention the type_alias that changes at all

x = AliasContainerChild.new.example
T.reveal_type(x) # error: `T.any(Float, Symbol)`
