# frozen_string_literal: true
# typed: true
# compiled: true

class A
  extend T::Sig

  sig {type_parameters(:U)
        .params(array: T::Array[T.untyped],
                blk: T.proc.returns(T.type_parameter(:U)))
        .returns(T.type_parameter(:U))}
  def self.with_wrap(array, &blk)
    p "inside with_wrap"
    o = Thread.current[:override]
    s = Thread.current[:shortest]
    begin
      Thread.current[:override] = :tweak
      Thread.current[:shortest] = :segment
      yield
    ensure
      p "running ensure #{o} #{s}"
      Thread.current[:override] = o
      Thread.current[:shortest] = s
    end
  end
end

require_relative 'return_across_ensure__2.rb'
