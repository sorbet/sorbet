# typed: true
# spacer for exclude-from-file-update

A.new.from_method
A.new.from_method_new # error: Method `from_method_new` does not exist on `A`
A.new.to_method
A.new.to_method_new # error: Method `to_method_new` does not exist on `A`

T.reveal_type(A.new.from_method) # error: `Integer`
