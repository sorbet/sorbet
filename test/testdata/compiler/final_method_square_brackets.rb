# typed: true
# compiled: true
# frozen_string_literal: true
# run_filecheck: INITIAL

class Parent
  extend T::Sig
  extend T::Helpers

  sig(:final) {void}
  def []
    puts "final method!"
  end
end

Parent.new.[]

# We used to have an ordering issue between codegen for name-based intrinsics
# and codegen for final methods: name-based intrinsics were tried first, and the
# name-based intrinsic for [] would bail to a Ruby VM call if there was type
# information.  Which meant that the codegen for the final method would never
# actually run, and therefore we would always call through the Ruby VM, which
# is not what the user intended.
#
# This test is intended to ensure we don't run into that situation again.

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @"direct_func_Parent#2[]"
# INITIAL{LITERAL}: }
