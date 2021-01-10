# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Setters

    class ExamplePoro
      attr_accessor :attr
    end

    class ExampleStruct < T::Struct
      prop :prop, Integer
      prop :nilable, T.nilable(Integer), default: 0
    end

    class OverrideStruct < T::Struct
      class SetHookDecorator < T::Props::Decorator
        def prop_set(instance, prop, val, rules=prop_rules)
          super
        end
      end

      def self.decorator_class
        SetHookDecorator
      end

      prop :prop, Integer
    end

    # Note we manually unroll loops 10x since loop overhead may be non-negligible here
    def self.run
      poro = ExamplePoro.new

      10_000.times do
        poro.attr = 0
      end

      result = Benchmark.measure do
        100_000.times do
          poro.attr = 0
          poro.attr = 1
          poro.attr = 2
          poro.attr = 3
          poro.attr = 4
          poro.attr = 5
          poro.attr = 6
          poro.attr = 7
          poro.attr = 8
          poro.attr = 9
        end
      end
      puts "Plain Ruby attr_writer (μs/iter):"
      puts result

      struct = ExampleStruct.new(prop: 0)

      10_000.times do
        struct.prop = 0
      end

      result = Benchmark.measure do
        100_000.times do
          struct.prop = 0
          struct.prop = 1
          struct.prop = 2
          struct.prop = 3
          struct.prop = 4
          struct.prop = 5
          struct.prop = 6
          struct.prop = 7
          struct.prop = 8
          struct.prop = 9
        end
      end
      puts "T::Struct setter, simple type (μs/iter):"
      puts result

      struct = ExampleStruct.new(prop: 0)

      10_000.times do
        struct.nilable = 0
      end

      result = Benchmark.measure do
        100_000.times do
          struct.nilable = 0
          struct.nilable = 1
          struct.nilable = 2
          struct.nilable = 3
          struct.nilable = 4
          struct.nilable = 5
          struct.nilable = 6
          struct.nilable = 7
          struct.nilable = 8
          struct.nilable = 9
        end
      end
      puts "T::Struct setter, nilable type (μs/iter):"
      puts result

      struct = OverrideStruct.new(prop: 0)

      10_000.times do
        struct.prop = 0
      end

      result = Benchmark.measure do
        100_000.times do
          struct.prop = 0
          struct.prop = 1
          struct.prop = 2
          struct.prop = 3
          struct.prop = 4
          struct.prop = 5
          struct.prop = 6
          struct.prop = 7
          struct.prop = 8
          struct.prop = 9
        end
      end
      puts "T::Struct setter, given prop_set override (μs/iter):"
      puts result
    end
  end
end

