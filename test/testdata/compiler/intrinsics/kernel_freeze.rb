# frozen_string_literal: true
# compiled: true
# typed: true
# run_filecheck: OPT

module Main
  extend T::Sig

  # OPT-LABEL: @func_Main.7example
  # OPT: sorbet_vm_freeze
  # OPT-NOT: sorbet_callFuncWithCache
  # OPT{LITERAL}: }
  sig {params(x: T::Array[Integer]).void}
  def self.example(x)
    x.freeze
  end
end

Main.example([1,2,3])
