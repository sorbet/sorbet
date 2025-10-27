# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Abstract
    extend T::Sig

    def self.time_block(name, iterations_of_block: 1_000_000, iterations_in_block: 2, &blk)
      1_000.times(&blk) # warmup

      GC.start
      GC.disable

      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      iterations_of_block.times(&blk)
      duration_s = Process.clock_gettime(Process::CLOCK_MONOTONIC) - t0

      GC.enable

      ns_per_iter = duration_s * 1_000_000_000 / (iterations_of_block * iterations_in_block)
      duration_str = ns_per_iter >= 1000 ? "#{(ns_per_iter / 1000).round(3)} μs" : "#{ns_per_iter.round(3)} ns"
      puts "#{name}: #{duration_str}"
    end

    class AbstractClass
      extend T::Sig
      extend T::Helpers
      abstract!
    end

    class ConcreteClass < AbstractClass
    end

    class ConcreteNoAbstract
    end

class NormalParent
end
class NormalChild < NormalParent; end

class Module; include T::Sig; end



class SorbetRuntimeParent; end
class SorbetRuntimeChild < SorbetRuntimeParent; end

class AbstractParent
  extend T::Helpers
  abstract!
end
class ConcreteChild < AbstractParent
end

    def self.run
      # This is not quite _no_ abstraction penalty, because there appears to be
      # some inefficiency in the VM for defining `new` with the UnboundMethod
      # object representing Class.new.  But the abstraction penalty is almost
      # two orders of magnitude lower than it was prior to forwarding `new` on
      # abstract subclasses.
      puts("There should be minimal abstraction penalty for having abstract superclasses")
      time_block("Concrete + Abstract construction", iterations_of_block: 100_000, iterations_in_block: 9) do
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
        ConcreteClass.new
      end

      time_block("Concrete with no abstract construction", iterations_of_block: 100_000, iterations_in_block: 9) do
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
        ConcreteNoAbstract.new
      end

begin_allocs = GC.stat(:total_allocated_objects)
i = 0
while i < 100
  NormalChild.new
  i += 1
end
end_allocs = GC.stat(:total_allocated_objects)
puts("NormalChild.new: #{(end_allocs - begin_allocs) / 100.0}")

begin_allocs = GC.stat(:total_allocated_objects)
i = 0
while i < 100
  SorbetRuntimeChild.new
  i += 1
end
end_allocs = GC.stat(:total_allocated_objects)
puts("SorbetRuntimeChild.new: #{(end_allocs - begin_allocs) / 100.0}")


begin_allocs = GC.stat(:total_allocated_objects)
i = 0
while i < 100
  ConcreteChild.new
  i += 1
end
end_allocs = GC.stat(:total_allocated_objects)
puts("ConcreteChild.new: #{(end_allocs - begin_allocs) / 100.0}")

    end
  end
end
