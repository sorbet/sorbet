# typed: strict
# spacer for exclude-from-file-update

class A
  T.reveal_type(@@foo) # error: `Integer`
  # spacer for exclude from-file-update
  T.reveal_type(@@bar) # error: `T.untyped`
  #             ^^^^^ error: Use of undeclared variable `@@bar`
end
