# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module Getters

    class ExamplePoro
      attr_accessor :attr
    end

    class ExampleStruct < T::Struct
      prop :prop, Integer
      prop :ifunset, T.nilable(Integer), ifunset: 0
    end

    # Note we manually unroll loops 10x since loop overhead may be non-negligible here
    def self.run
      poro = ExamplePoro.new
      poro.attr = 0
      10_000.times { poro.attr }
      result = Benchmark.measure do
        100_000.times do
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
          poro.attr
        end
      end
      puts "Plain Ruby attr_reader, μs/iter:"
      puts result

      struct = ExampleStruct.new(prop: 0)
      10_000.times { struct.prop }
      result = Benchmark.measure do
        100_000.times do
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
          struct.prop
        end
      end
      puts "T::Struct getter, fast path, μs/iter:"
      puts result

      struct = ExampleStruct.new(prop: 0)
      10_000.times { struct.ifunset }
      result = Benchmark.measure do
        100_000.times do
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
          struct.ifunset
        end
      end
      puts "T::Struct getter, with ifunset, μs/iter:"
      puts result
    end
  end
end
