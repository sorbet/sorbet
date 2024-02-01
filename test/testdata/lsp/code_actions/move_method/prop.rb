# typed: strict
# selective-apply-code-action: refactor.extract
# assert-no-code-action: refactor.extract
#
# No code actions should be available for the props

class Foo < T::Struct
  prop :bar, Integer
 # | apply-code-action: [A] Move method to a new module
end
