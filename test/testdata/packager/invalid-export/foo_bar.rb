# typed: strict

module Foo::Bar
  class Exists
    extend T::Sig

    sig {void}
    def self.hello; end
  end

  class HasTypeMember
    extend T::Generic

    TypeMember = type_member
  end
end
