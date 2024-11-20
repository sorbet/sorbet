# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

def my_name(name:, prefix: "Mr")
  prefix + " " + name
end

def f(a, b=3, name:, prefix: "Mr")
  p a, b
  prefix + " " + name
end

class FinalWrapper
  extend T::Sig

  sig(:final) {params(name: T.untyped, prefix: T.untyped).returns(T.untyped)}
  def self.my_name_final(name:, prefix: "Mr")
    prefix + " " + name
  end

  sig(:final) {params(a: T.untyped, b: T.untyped, name: T.untyped, prefix: T.untyped).returns(T.untyped)}
  def self.f_final(a, b=3, name:, prefix: "Mr")
    p a, b
    prefix + " " + name
  end
end

puts my_name(name: "Paul", prefix: "Master")
puts my_name(name: "Paul")

puts FinalWrapper.my_name_final(name: "Paul", prefix: "Master")
puts FinalWrapper.my_name_final(name: "Paul")

puts f(1, {x: 5}, name: "Nathan")

puts FinalWrapper.f_final(1, {x: 5}, name: "Nathan")

def builder(name,
            default: :default_default,
            validate: :validate_default,
            overridable: :overridable_default,
            implied: :implied_default,
            skip_get: :skip_get_default,
            skip_set: :skip_set_default,
            inherit: :inherit_default)
  p name
  p default, validate, overridable, implied, skip_get, skip_set, inherit
end

class FinalWrapper2
  extend T::Sig

  sig(:final) do
    params(
      name: T.untyped,
      default: T.untyped,
      validate: T.untyped,
      overridable: T.untyped,
      implied: T.untyped,
      skip_get: T.untyped,
      skip_set: T.untyped,
      inherit: T.untyped
    ).returns(T.untyped)
  end
  def self.builder_final(name,
                         default: :default_default,
                         validate: :validate_default,
                         overridable: :overridable_default,
                         implied: :implied_default,
                         skip_get: :skip_get_default,
                         skip_set: :skip_set_default,
                         inherit: :inherit_default)
    p name
    p default, validate, overridable, implied, skip_get, skip_set, inherit
  end
end

builder(:first_last, default: :default_passed, inherit: :inherit_passed)
builder(:middles, validate: :validate_passed, implied: :implied_passed)
builder(:consecutive, skip_get: :skip_get_passed, skip_set: :skip_set_passed)
builder(:three_args, overridable: :overridable_passed, skip_get: :skip_get_passed, inherit: :inherit_passed)
builder(tricky: :keyword, args: :passed, as: :hash)

FinalWrapper2.builder_final(:first_last, default: :default_passed, inherit: :inherit_passed)
FinalWrapper2.builder_final(:middles, validate: :validate_passed, implied: :implied_passed)
FinalWrapper2.builder_final(:consecutive, skip_get: :skip_get_passed, skip_set: :skip_set_passed)
FinalWrapper2.builder_final(:three_args, overridable: :overridable_passed, skip_get: :skip_get_passed, inherit: :inherit_passed)
FinalWrapper2.builder_final(tricky: :keyword, args: :passed, as: :hash)

# static-init gets generated prior to the method definitions, so we look for the
# calls first.
