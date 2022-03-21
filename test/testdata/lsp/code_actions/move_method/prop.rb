# typed: strict
# selective-apply-code-action: refactor.extract
#
# No code actions should be available for the props

class Foo < T::Struct
  prop :bar, Integer
end
