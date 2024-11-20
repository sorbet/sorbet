# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def takes_block(arg, &blk)
    puts arg
    call_param(blk)
  end


  def call_param(proc)
    proc.call
  end
end

A.new.takes_block("hello") {
  puts "world"
}
