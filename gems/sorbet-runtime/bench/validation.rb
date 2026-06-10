# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Validation
    extend T::Sig

    def self.time_block(name, iterations_of_block: 1_000_000, iterations_in_block: 1, &blk)
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

    def self.run
      ints3 = [1, 2, 3].freeze
      ints100 = (1..100).to_a.freeze
      array_type = T::Utils.coerce(T::Array[Integer])

      time_block("T::Array[Integer].recursively_valid?(3-int array)") do
        array_type.recursively_valid?(ints3)
      end

      time_block("T::Array[Integer].recursively_valid?(100-int array)", iterations_of_block: 100_000) do
        array_type.recursively_valid?(ints100)
      end

      hash_type = T::Utils.coerce(T::Hash[Symbol, String])
      hash10 = (1..10).to_h { |i| [:"k#{i}", "v#{i}"] }.freeze

      time_block("T::Hash[Symbol, String].recursively_valid?(10 entries)", iterations_of_block: 300_000) do
        hash_type.recursively_valid?(hash10)
      end

      simple_type = T::Utils.coerce(Integer)
      time_block("Simple(Integer).recursively_valid?(1)") do
        simple_type.recursively_valid?(1)
      end

      tuple_type = T::Utils.coerce([Integer, String])
      tuple = [1, 'a'].freeze
      time_block("[Integer, String] tuple valid?") do
        tuple_type.valid?(tuple)
      end

      time_block("[Integer, String] tuple recursively_valid?") do
        tuple_type.recursively_valid?(tuple)
      end

      shape_type = T::Utils.coerce({a: Integer, b: String})
      shape_ok = {a: 1, b: 'b'}.freeze
      shape_extra = {a: 1, b: 'b', c: :c}.freeze
      time_block("{a:, b:} shape valid? (ok)") do
        shape_type.valid?(shape_ok)
      end

      time_block("{a:, b:} shape valid? (extra key)") do
        shape_type.valid?(shape_extra)
      end

      time_block("{a:, b:} shape recursively_valid? (ok)") do
        shape_type.recursively_valid?(shape_ok)
      end

      intersection_type = T::Utils.coerce(T.all(Comparable, Integer))
      time_block("T.all(Comparable, Integer) valid?(1)") do
        intersection_type.valid?(1)
      end

      time_block("T.all(Comparable, Integer) recursively_valid?(1)") do
        intersection_type.recursively_valid?(1)
      end

      union_type = T::Utils.coerce(T.any(Integer, Float, Symbol))
      time_block("T.any(Integer, Float, Symbol).recursively_valid?(:sym)") do
        union_type.recursively_valid?(:sym)
      end

      require 'set'
      set_type = T::Utils.coerce(T::Set[Integer])
      int_set = Set[1, 2, 3].freeze
      time_block("T::Set[Integer].valid?(3-int set)") do
        set_type.valid?(int_set)
      end

      time_block("T::Set[Integer].recursively_valid?(3-int set)") do
        set_type.recursively_valid?(int_set)
      end
    end
  end
end
