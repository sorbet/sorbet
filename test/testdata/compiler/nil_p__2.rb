# frozen_string_literal: true
# typed: true
# compiled: false

extend T::Sig

# We include this monkey patch in Stripe's codebase... but the compiler
# doesn't know how to compile it right now (see the reopen_basic_object.rb
# test), and it doesn't seem worth fixing right now.
#
# Instead, hide the re-opening in a file that is not compiled.
class ::BasicObject
  def nil?
    false
  end
end
