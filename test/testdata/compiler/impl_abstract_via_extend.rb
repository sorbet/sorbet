# frozen_string_literal: true
# typed: strict
# compiled: true
# run_filecheck: INITIAL

module IFoo
  extend T::Helpers
  extend T::Sig
  abstract!

  sig {abstract.returns(Integer)}
  def foo; end
end

# INITIAL{LITERAL}-LABEL: define i64 @"func_IFoo#foo"
# INITIAL: call i64 @sorbet_callSuper
# INITIAL{LITERAL}: }

require_relative './impl_abstract_via_extend__1'
