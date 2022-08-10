# typed: strict
# spacer for exclude-from-file-update

x = A.new.Foo
T.reveal_type(x) # error: `Integer`
