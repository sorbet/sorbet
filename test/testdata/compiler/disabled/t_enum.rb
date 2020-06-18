# frozen_string_literal: true
# typed: strict
# compiled: true

# TODO(jez) There are many tests in sorbet (test/testdata/rewriter/t_enum*.rb)
# They could all probably be compiler tests.
class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

p MyEnum::X
p MyEnum::Y
