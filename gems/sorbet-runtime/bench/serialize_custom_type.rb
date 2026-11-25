# frozen_string_literal: true
# typed: true

require 'benchmark'

require_relative '../lib/sorbet-runtime'

module SorbetBenchmarks
  module SerializeCustomType
    class MyCustomType
      extend T::Props::CustomType

      attr_accessor :value

      def initialize(value)
        @value = value
      end

      # Not used in this benchmark.
      def self.deserialize(value)
        result = new
        result.value = value.clone.freeze
        result
      end

      def self.serialize(instance)
        instance.value
      end
    end

    # A CustomType whose serialized form is an untyped container of scalars,
    # exercising valid_serialization?'s recursive scalar_type? walk.
    class ContainerCustomType
      extend T::Props::CustomType

      attr_accessor :value

      def initialize(value=nil)
        @value = value
      end

      def self.deserialize(value)
        new(value.clone.freeze)
      end

      def self.serialize(instance)
        instance.value
      end
    end

    def self.time_block(name, iterations_of_block: 1_000_000, iterations_in_block: 2, &blk)
      1_000.times(&blk) # warmup

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
      puts "Allocations for #{name}: #{(after_alloc - before_alloc) / iterations_in_block}"
    end

    def self.run
      input = MyCustomType.new(123)

      100_000.times do
        T::Props::CustomType.checked_serialize(input)
      end

      result = Benchmark.measure do
        1_000_000.times do
          T::Props::CustomType.checked_serialize(input)
        end
      end

      puts "T::Props::CustomType.checked_serialize (μs/iter):"
      puts result

      time_block("T::Props::CustomType.scalar_type?('str')") do
        T::Props::CustomType.scalar_type?("str")
        T::Props::CustomType.scalar_type?("str")
      end

      # checked_serialize on a CustomType whose serialized form is an untyped
      # array of scalars: valid_serialization? recurses through scalar_type? for
      # every element.
      container_input = ContainerCustomType.new([1, "two", 3.0, true, :sym, 6, "seven", 8, 9, 10])
      time_block("checked_serialize(untyped container)") do
        T::Props::CustomType.checked_serialize(container_input)
        T::Props::CustomType.checked_serialize(container_input)
      end
    end
  end
end
