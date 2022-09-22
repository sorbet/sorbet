# typed: true

# https://github.com/sorbet/sorbet/issues/6416

class Example
  extend T::Generic

  ::OnRootScope = type_member

  module Namespace; end
  Namespace::Inside = type_member
end
