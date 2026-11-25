# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module TypeDerivation
    extend T::Sig

    class Example; end

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

    def self.run
      time_block("T.nilable(Integer)") do
        T.nilable(Integer)
        T.nilable(Integer)
      end

      time_block("T.nilable(Example)") do
        T.nilable(Example)
        T.nilable(Example)
      end

      time_block("T.any(Integer, Float)") do
        T.any(Integer, Float)
        T.any(Integer, Float)
      end

      time_block("T.all(Comparable, Integer)") do
        T.all(Comparable, Integer)
        T.all(Comparable, Integer)
      end

      time_block("T::Array[Integer]") do
        T::Array[Integer]
        T::Array[Integer]
      end

      time_block("T::Hash[String, Integer]") do
        T::Hash[String, Integer]
        T::Hash[String, Integer]
      end

      time_block("T::Hash[T.untyped, T.untyped]") do
        T::Hash[T.untyped, T.untyped]
        T::Hash[T.untyped, T.untyped]
      end

      pooled = T::Utils.coerce(Integer)
      same_name_a = T::Types::Simple.new(Integer)
      same_name_b = T::Types::Simple.new(Integer)
      # rubocop:disable Lint/BinaryOperatorWithIdenticalOperands
      # rubocop:disable Lint/Void
      time_block("Base#== identity hit") do
        pooled == pooled
        pooled == pooled
      end

      time_block("Base#== equal-name distinct instances") do
        same_name_a == same_name_b
        same_name_a == same_name_b
      end
      # rubocop:enable Lint/Void
      # rubocop:enable Lint/BinaryOperatorWithIdenticalOperands

      simple_int = T::Utils.coerce(Integer)
      simple_num = T::Utils.coerce(Numeric)
      union = T::Utils.coerce(T.any(Integer, Float))
      time_block("subtype_of?(Simple, Simple)") do
        simple_int.subtype_of?(simple_num)
        simple_num.subtype_of?(simple_int)
      end

      time_block("subtype_of? Simple vs Union") do
        union.subtype_of?(simple_num)
        simple_int.subtype_of?(union)
      end
    end
  end
end
