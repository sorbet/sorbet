# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module TUtils
    extend T::Sig

    def self.time_block(name, iterations_of_block: 1_000_000, &blk)
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
      type = T::Utils.coerce(Integer)
      time_block("T.unwrap_nilable(#{type})") do
        T::Utils.unwrap_nilable(type)
      end

      time_block("get_underlying_type(#{type})") do
        T::Utils::Nilable.get_underlying_type(type)
      end

      type = T::Utils.coerce(T.nilable(Integer))
      time_block("T.unwrap_nilable(#{type})") do
        T::Utils.unwrap_nilable(type)
      end

      time_block("get_underlying_type(#{type})") do
        T::Utils::Nilable.get_underlying_type(type)
      end

      type = T::Utils.coerce(T.any(Integer, Float))
      time_block("T.unwrap_nilable(#{type})") do
        T::Utils.unwrap_nilable(type)
      end

      time_block("get_underlying_type(#{type})") do
        T::Utils::Nilable.get_underlying_type(type)
      end
    end
  end
end
