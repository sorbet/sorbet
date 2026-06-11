# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module DeepClone
    extend T::Sig

    def self.time_block(name, iterations_of_block: 100_000, &blk)
      1_000.times(&blk) # warmup

      GC.start
      GC.disable

      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC)
      iterations_of_block.times(&blk)
      duration_s = Process.clock_gettime(Process::CLOCK_MONOTONIC) - t0

      GC.enable

      ns_per_iter = duration_s * 1_000_000_000 / iterations_of_block
      duration_str = ns_per_iter >= 1000 ? "#{(ns_per_iter / 1000).round(3)} μs" : "#{ns_per_iter.round(3)} ns"
      puts "#{name}: #{duration_str}"
    end

    def self.run
      string_array = Array.new(20) { |i| "str#{i}" }.freeze
      mixed_array = Array.new(20) { |i| i.even? ? "str#{i}" : i }.freeze
      int_array = (1..20).to_a.freeze
      nested_hash = {
        'a' => {'b' => ['x', 'y', 1, 2], 'c' => 'z'},
        'd' => [{'e' => 'f'}, {'g' => 1}],
        'h' => 'i',
      }.freeze

      time_block("deep_clone_object(20-string array)") do
        T::Props::Utils.deep_clone_object(string_array)
      end

      time_block("deep_clone_object(20-elem mixed array)") do
        T::Props::Utils.deep_clone_object(mixed_array)
      end

      time_block("deep_clone_object(20-int array)") do
        T::Props::Utils.deep_clone_object(int_array)
      end

      time_block("deep_clone_object(nested hash)") do
        T::Props::Utils.deep_clone_object(nested_hash)
      end

      time_block("deep_clone_object(20-string array, freeze: true)") do
        T::Props::Utils.deep_clone_object(string_array, freeze: true)
      end
    end
  end
end
