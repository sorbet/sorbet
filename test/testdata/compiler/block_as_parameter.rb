# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

class A
  def takes_block(arg, &blk)
    puts arg
    call_param(blk)
  end
# INITIAL-LABEL: "func_A#11takes_block"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc

# OPT-LABEL: "func_A#11takes_block"
# OPT: call i64 @rb_block_proc

  def call_param(proc)
    proc.call
  end
end

A.new.takes_block("hello") {
  puts "world"
}
