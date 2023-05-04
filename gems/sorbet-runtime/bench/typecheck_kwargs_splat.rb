# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module TypecheckKwargsSplat
    extend T::Sig

    class Example; end

    def self.time_block(name, iterations_of_block: 1_000_000, iterations_in_block: 2, &blk)
      10_000.times(&blk) # warmup

      GC.start
      GC.disable
      before_alloc = GC.stat(:total_allocated_objects)

      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      iterations_of_block.times(&blk)
      duration_s = Process.clock_gettime(Process::CLOCK_MONOTONIC) - t0

      after_alloc = GC.stat(:total_allocated_objects)
      GC.enable

      ns_per_iter = duration_s * 1_000_000_000 / (iterations_of_block * iterations_in_block)
      duration_str = ns_per_iter >= 1000 ? "#{(ns_per_iter / 1000).round(3)} μs" : "#{ns_per_iter.round(3)} ns"
      puts "#{name}: #{duration_str}"
      puts "Allocations for #{name}: #{after_alloc - before_alloc}"
    end

    def self.run
      time_block("sig {params(s: Symbol, rest: String, x: Integer, y: Integer).void} (with kwargs plus splat)") do
        arg_plus_kwargs(:foo, "foo", "bar", "baz", x: 1, y: 2)
        arg_plus_kwargs(:bar, "foo", "bar", "baz", x: 1)
      end
    end

    sig {params(s: Symbol, rest: String, x: Integer, y: Integer).void}
    def self.arg_plus_kwargs(s, *rest, x:, y: 0); end
  end
end
