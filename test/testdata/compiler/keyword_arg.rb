# frozen_string_literal: true
# typed: true
# compiled: true
def my_name(name:, prefix: "Mr")
  prefix + " " + name
end

def f(a, b=3, name:, prefix: "Mr")
  p a, b
  prefix + " " + name
end

puts my_name(name: "Paul", prefix: "Master")
puts my_name(name: "Paul")

puts f(1, {x: 5}, name: "Nathan")

def builder(name, type,
            default: :default_default,
            validate: :validate_default,
            overridable: :overridable_default,
            implied: :implied_default,
            skip_get: :skip_get_default,
            skip_set: :skip_set_default,
            inherit: :inherit_default)
  p name
  p type
  p default, validate, overridable, implied, skip_get, skip_set, inherit
end

builder(:use_inputs, :maybe, default: :default_passed, inherit: :inherit_passed)
builder(:use_inputs, true, validate: :validate_passed, implied: :implied_passed)
builder(:use_inputs, true, skip_get: :skip_get_passed, skip_set: :skip_set_passed)
builder(:use_inputs, :three_args, overridable: :overridable_passed, skip_get: :skip_get_passed, inherit: :inherit_passed)
