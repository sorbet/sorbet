# typed: strict
# selective-apply-code-action: refactor.extract

class Foo < T::Struct
  prop :bar, Integer
 # ^ apply-code-action: [A] Move method to a new module
end
