# frozen_string_literal: true
# typed: true
require_relative '../../../lib/sorbet-runtime'

# Regression test: sorbet-runtime's sig machinery must keep working inside
# signal trap handlers, where Ruby raises ThreadError ("can't be called from
# trap context") for lock acquisition (Mutex#synchronize and friends). Both of
# these paths are trap-reachable: the FIRST call of an already-declared sig'd
# method (lazy sig wrapping), and declaring a sig'd method (e.g. a require
# from inside a trap handler that loads a file containing sigs).

class AlreadyDeclared
  extend T::Sig

  sig { returns(Integer) }
  def self.compute
    42
  end
end

first_call_result = nil
Signal.trap("USR1") { first_call_result = AlreadyDeclared.compute }
Process.kill("USR1", Process.pid)
deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 10
sleep(0.01) while first_call_result.nil? && Process.clock_gettime(Process::CLOCK_MONOTONIC) < deadline
puts "first-call-in-trap: #{first_call_result.inspect}"

declare_result = nil
Signal.trap("USR1") do
  k = Class.new do
    extend T::Sig

    sig { returns(Integer) }
    def self.seven
      7
    end
  end
  declare_result = k.seven
end
Process.kill("USR1", Process.pid)
deadline = Process.clock_gettime(Process::CLOCK_MONOTONIC) + 10
sleep(0.01) while declare_result.nil? && Process.clock_gettime(Process::CLOCK_MONOTONIC) < deadline
puts "declare-in-trap: #{declare_result.inspect}"
