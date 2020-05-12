# frozen_string_literal: true
# typed: strict
# compiled: true

class Inexact < T::InexactStruct
end

class Child < Inexact
  prop :qux, Symbol
end

# TODO(jez) When this test passes, merge it into t_inexact_struct.rb

child = Child.new(qux: :my_symbol)
child.qux = :zl_flzoby
p child.qux
