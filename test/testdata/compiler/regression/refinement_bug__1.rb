# typed: false
# frozen_string_literal: true
# compiled: false

require_relative './refinement_bug__2'

val = A.new

class Object
  def blank?
    puts "Object blank?"
    !self
  end
end

A.foo
val.test

module Thinger
  refine Object do
    def blank?
      puts "refined blank?"
      !self
    end
  end
end

# Before the bugfix to use `sorbet_vm_sendish` [1], this test would crash here,
# as the `blank?` method would be called from a context that required a cref,
# and vm_sendish makes the assumption that you can always get one via the method
# entry on the stack. The change that sorbet_vm_sendish introduces is that when
# we dispatch to a refined method, it instead behaves like `vm_call0_body`,
# which interprets the send as though it was called from a cfunc context.
#
# [1] https://github.com/stripe/sorbet_llvm/pull/343
A.foo
val.test

using Thinger

A.foo
val.test
