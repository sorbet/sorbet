# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Constructor

    class ExamplePoro
      def initialize(hash)
        @prop1 = hash.fetch(:prop1, nil)
        @prop2 = hash.fetch(:prop2, 0)
        @prop3 = hash.fetch(:prop3)
        @prop4 = hash.fetch(:prop4)
        @prop5 = hash.fetch(:prop5, [])
        @prop6 = hash.fetch(:prop6)
        @prop7 = hash.fetch(:prop7, {})
        @prop8 = hash.fetch(:prop8, nil)
        @prop9 = hash.fetch(:prop9, [])
        @prop10 = hash.fetch(:prop10, {})
      end
    end

    class Example < T::Struct
      class Subdoc < T::Struct
        prop :prop, String
      end

      prop :prop1, T.nilable(Integer)
      prop :prop2, Integer, default: 0
      prop :prop3, Integer
      prop :prop4, T::Array[Integer]
      prop :prop5, T::Array[Integer], default: []
      prop :prop6, T::Hash[String, Integer]
      prop :prop7, T::Hash[String, Integer], default: {}
      prop :prop8, T.nilable(Subdoc)
      prop :prop9, T::Array[Subdoc], default: []
      prop :prop10, T::Hash[String, Subdoc], default: {}
    end

    def self.run
      input = {
        prop3: 0,
        prop4: [],
        prop6: {},
      }.freeze

      100_000.times do
        ExamplePoro.new(input)
      end

      result = Benchmark.measure do
        1_000_000.times do
          ExamplePoro.new(input)
        end
      end

      puts "Plain Ruby (μs/iter):"
      puts result

      100_000.times do
        Example.new(input)
      end

      result = Benchmark.measure do
        1_000_000.times do
          Example.new(input)
        end
      end

      puts "T::Props.new, mostly nil input (μs/iter):"
      puts result

      subdoc = Example::Subdoc.new(prop: '').freeze
      input = {
        prop1: 0,
        prop2: 0,
        prop3: 0,
        prop4: [1, 2, 3].freeze,
        prop5: [1, 2, 3].freeze,
        prop6: {'foo' => 1, 'bar' => 2}.freeze,
        prop7: {'foo' => 1, 'bar' => 2}.freeze,
        prop8: subdoc,
        prop9: [subdoc, subdoc].freeze,
        prop10: {'foo' => subdoc, 'bar' => subdoc}.freeze,
      }.freeze

      100_000.times do
        Example.new(input)
      end

      result = Benchmark.measure do
        1_000_000.times do
          Example.new(input)
        end
      end

      puts "T::Props.new, all props set (μs/iter):"
      puts result
    end
  end
end
