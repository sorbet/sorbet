# frozen_string_literal: true
# typed: strict
# compiled: true

# pay-server does this stuff. It currently doesn't compile (haven't looked into
# why) and the benchmark harness requires single files, because it copies files
# around, any `require_relative` in the code would have its name changed.
#
# Just gonna hope that this code doesn't matter.
#
# class ::Thread
#   class << self
#     alias_method :new_without_lspace, :new
#     alias_method :fork_without_lspace, :fork
#     alias_method :start_without_lspace, :start
#   end
#     copied_lspace = Opus::LSpace.deep_copy_current
#
#     T.unsafe(self).new_without_lspace(*args) do
#       Opus::LSpace.enter_for_thread(copied_lspace)
#       yield(*args)
#     end
#   end
#
#   def self.fork(*args)
#     copied_lspace = Opus::LSpace.deep_copy_current
#
#     T.unsafe(self).fork_without_lspace(*args) do
#       Opus::LSpace.enter_for_thread(copied_lspace)
#       yield(*args)
#     end
#   end
#
#   def self.start(*args)
#     copied_lspace = Opus::LSpace.deep_copy_current
#
#     T.unsafe(self).start_without_lspace(*args) do
#       Opus::LSpace.enter_for_thread(copied_lspace)
#       yield(*args)
#     end
#   end
#end

class ::Exception
  T::Sig::WithoutRuntime.sig(:final) {returns(T.nilable(Opus::LSpace::Private::AbstractSpace)).checked(:never)}
  attr_accessor :lspace
end
module Opus; end
class Opus::LSpace
  extend T::Sig

  class Key; end

  KeyTypes = T.type_alias {T.any(Key, Symbol, String)}

  module Private
    class AbstractSpace
      extend T::Sig
      sig(:final) {returns(T::Hash[KeyTypes, T.untyped]).checked(:never)}
      attr_reader :data

      sig {params(data: T::Hash[KeyTypes, T.untyped]).void.checked(:never)}
      def initialize(data)
        @data = data
      end

      sig(:final) {params(key: KeyTypes).returns(T.untyped).checked(:never)}
      def [](key)
        @data[key]
      end
    end

    class WritableSpace < AbstractSpace
      sig {params(data: T::Hash[KeyTypes, T.untyped]).void.checked(:never)}
      def initialize(data)
        super
        @from_thread = T.let(Thread.current.object_id, Integer)
      end

      sig(:final) do
        params(
          key: KeyTypes,
          value: T.untyped,
        )
        .void
        .checked(:never)
      end
      def []=(key, value)
        # This doesn't capture the entire story, because it's common for people to bypass LSpace#[]=
        # and instead mutate the object directly. It's part of the way towards fixing up some of peoples shenanigans though.
        if Thread.current.object_id != @from_thread
          raise RuntimeError.new("Attempting to set an LSpace from a different thread that created it.")
        end

        @data[key] = value
      end
    end
  end

  sig(:final) {params(key: KeyTypes, value: T.untyped).void.checked(:never)}
  def self.[]=(key, value)
    current[key] = value
  end

  sig(:final) {params(key: KeyTypes).returns(T.untyped).checked(:never)}
  def self.[](key)
    Thread.current[:lspace_internal]&.[](key)
  end

  sig(:final) {returns(Private::WritableSpace).checked(:never)}
  def self.current
    lspace = Thread.current[:lspace]
    return lspace if lspace

    lspace = Private::WritableSpace.new({})

    Thread.current[:lspace] = lspace
    Thread.current[:lspace_internal] = lspace.data
    lspace
  end

  sig(:final) do
    type_parameters(:Result)
    .params(
      data: T::Hash[KeyTypes, T.untyped],
      blk: T.proc.returns(T.type_parameter(:Result)),
    )
    .returns(T.type_parameter(:Result))
    .checked(:never)
  end
  def self.with(data, &blk)
    # NOTE: This is an inlined version of `override_current`.
    #
    # We inline for to avoid method call overhead and to save a stack frame.
    previous = Thread.current[:lspace]
    begin
      self.current = Private::WritableSpace.new(current.data.merge(data))
      yield
    rescue => ex
      ex.lspace ||= current
      raise
    ensure
      self.current = previous
    end
  end

  sig(:final) {params(lspace: T.nilable(Private::AbstractSpace)).void.checked(:never)}
  private_class_method def self.current=(lspace)
    Thread.current[:lspace] = lspace
    Thread.current[:lspace_internal] = lspace&.data
  end

  # Clone the entire LSpace with a deep copy, preventing writes across boundaries.
  # Used for things like forking or thread creation.
  sig(:final) {returns(T::Hash[KeyTypes, T.untyped]).checked(:tests)}
  def self.deep_copy_current
    deep_copy(current.data)
  end

  # WARNING
  # This is only offered as a way of re-entering a cloned LSpace in a Thread or fork.
  # You should not be building code that relies on it directly. If you need to re-enter LSpace use `enter`.
  sig(:final) {params(data: T::Hash[KeyTypes, T.untyped]).void}
  def self.enter_for_thread(data)
    if Thread.current[:lspace]
      raise RuntimeError.new("You cannot call this if Thread.current has a LSpace in it")
    end

    self.current = Private::WritableSpace.new(data)
  end

  sig(:final) {params(data: Object).returns(T.untyped).checked(:never)}
  private_class_method def self.deep_copy(data)
    if data.is_a?(Array)
      data.map {|row| deep_copy(row)}
    elsif data.is_a?(Hash)
      copied = {}
      data.each do |key, value|
        copied[key] = deep_copy(value)
      end

      copied
    else
      data
    end
  end
end


i = 0
Opus::LSpace.with(foo: 1) do
  while i < 10_000_000

    # TODO(jez) Benchmark for time to lookup "unknown" key
    Opus::LSpace[:foo]

    i += 1
  end
end

puts i
