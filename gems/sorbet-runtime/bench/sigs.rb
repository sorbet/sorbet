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

    # Measures the per-declaration cost of `sig` + method definition (the
    # _on_method_added hook and slow-wrapper install), without running the
    # resulting sig blocks.
    def self.run_declaration
      cls = Class.new do
        extend T::Sig
      end

      1_000.times do |i|
        cls.class_eval do
          sig { returns(Integer) }
          define_method(:"warmup_m#{i}") { 1 }
        end
      end

      GC.start
      GC.disable

      count = 20_000
      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond)
      begin_allocs = GC.stat(:total_allocated_objects)
      count.times do |i|
        cls.class_eval do
          sig { returns(Integer) }
          define_method(:"m#{i}") { 1 }
        end
      end
      duration_ns = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond) - t0
      allocs = GC.stat(:total_allocated_objects) - begin_allocs - 1

      GC.enable

      puts "declare sig'd method: #{(duration_ns.to_f / count).round(1)} ns/decl, #{(allocs.to_f / count).round(4)} allocs/decl"
    end

    # Measures re-wrapping an already-sig'd method through the public
    # T::Utils.wrap_method_with_call_validation_if_needed entry point (the
    # path mocha-style stubbing takes for every stubbed sig'd method).
    def self.run_wrap
      cls = Class.new do
        extend T::Sig

        sig { params(x: Integer).returns(Integer) }
        def compute(x)
          x
        end
      end
      cls.new.compute(1) # force the sig block to run
      signature = T::Utils.signature_for_instance_method(cls, :compute)
      original_method = cls.instance_method(:compute)

      1_000.times do
        T::Utils.wrap_method_with_call_validation_if_needed(cls, signature, original_method)
      end

      GC.start
      GC.disable

      count = 20_000
      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond)
      begin_allocs = GC.stat(:total_allocated_objects)
      count.times do
        T::Utils.wrap_method_with_call_validation_if_needed(cls, signature, original_method)
      end
      duration_ns = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond) - t0
      allocs = GC.stat(:total_allocated_objects) - begin_allocs - 1

      GC.enable

      puts "wrap_method_with_call_validation_if_needed: #{(duration_ns.to_f / count).round(1)} ns/wrap, #{(allocs.to_f / count).round(4)} allocs/wrap"
    end

    # Measures declaring a base + subclass that overrides ten sig'd methods and
    # running all the sig blocks, exercising the override-validation path
    # (subtype_of? checks between each override and its super sig).
    def self.run_override
      count = 2_000
      GC.start
      GC.disable
      t0 = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond)
      begin_allocs = GC.stat(:total_allocated_objects)
      count.times do
        base = Class.new do
          extend T::Sig
          10.times do |i|
            sig { overridable.params(x: Integer).returns(Integer) }
            define_method(:"m#{i}") { |x| x }
          end
        end
        Class.new(base) do
          extend T::Sig
          10.times do |i|
            sig { override.params(x: Integer).returns(Integer) }
            define_method(:"m#{i}") { |x| x }
          end
        end
        T::Utils.run_all_sig_blocks
      end
      duration_ns = Process.clock_gettime(Process::CLOCK_MONOTONIC, :nanosecond) - t0
      allocs = GC.stat(:total_allocated_objects) - begin_allocs - 1
      GC.enable

      puts "declare+override 10 sigs + run_all: #{(duration_ns.to_f / count).round(1)} ns/iter, #{(allocs.to_f / count).round(1)} allocs/iter"
    end
  end
end
