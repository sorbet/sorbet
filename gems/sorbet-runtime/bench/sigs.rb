# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Sigs
    extend T::Sig

    def self.run
      GC.start
      GC.disable

      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond)
      begin_allocs = GC.stat(:total_allocated_objects)
      T::Utils.run_all_sig_blocks
      duration_s = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond) - t0
      end_allocs = GC.stat(:total_allocated_objects) - 1

      str = duration_s >= 1000 ? "#{(duration_s / 1000).round(3)} μs" : "#{duration_s.round(3)} ns"
      puts "run_all_sigs: #{str} #{end_allocs - begin_allocs}"
    end
  end
end
