# typed: true

class Other
  extend T::Generic
  extend T::Helpers
  extend T::Sig

  Template = type_template
  Member = type_member
end

first = T.let(nil, Other::Template)
                 # ^^^^^^^^^^^^^^^ error: `type_template` type `T.class_of(Other)::Template` used outside of the class definition
#
second = T.let(nil, Other::Member)
                  # ^^^^^^^^^^^^^ error: `type_member` type `Other::Member` used outside of the class definition

class A
  extend T::Sig

  def foo
    T.let(nil, Other::Template)
             # ^^^^^^^^^^^^^^^ error: `type_template` type `T.class_of(Other)::Template` used outside of the class definition
  end

  def self.foo
    T.let(nil, Other::Member)
             # ^^^^^^^^^^^^^ error: `type_member` type `Other::Member` used outside of the class definition
  end
end
