# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Enum
    extend T::Sig

    class Example; end

    class MyEnum < T::Enum
      enums do
        X = new
        Y = new
        Z = new
      end
    end

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

    def self.all_comparisons
      time_block("T::Enum == T::Enum", iterations_of_block: 100_000, iterations_in_block: 9) do
        # rubocop:disable Lint/BinaryOperatorWithIdenticalOperands
        # rubocop:disable Lint/Void
        MyEnum::X == MyEnum::X
        MyEnum::X == MyEnum::X
        MyEnum::X == MyEnum::X

        MyEnum::Y == MyEnum::Y
        MyEnum::Y == MyEnum::Y
        MyEnum::Y == MyEnum::Y

        MyEnum::Z == MyEnum::Z
        MyEnum::Z == MyEnum::Z
        MyEnum::Z == MyEnum::Z
        # rubocop:enable Lint/Void
        # rubocop:enable Lint/BinaryOperatorWithIdenticalOperands
      end

      time_block("T::Enum == String", iterations_of_block: 100_000, iterations_in_block: 9) do
        # rubocop:disable Lint/Void
        MyEnum::X == 'x'
        MyEnum::X == 'x'
        MyEnum::X == 'x'

        MyEnum::Y == 'y'
        MyEnum::Y == 'y'
        MyEnum::Y == 'y'

        MyEnum::Z == 'z'
        MyEnum::Z == 'z'
        MyEnum::Z == 'z'
        # rubocop:enable Lint/Void
      end
    end

    def self.run
      puts("before T::Configuration.enable_legacy_t_enum_migration_mode")

      all_comparisons

      T::Configuration.enable_legacy_t_enum_migration_mode
      T::Configuration.soft_assert_handler = lambda do |str, extra|
        # Empty
        # (Not trying to benchmark the performance of the soft_assert_handler)
      end
      puts("\nafter T::Configuration.enable_legacy_t_enum_migration_mode")

      all_comparisons
    end
  end
end
