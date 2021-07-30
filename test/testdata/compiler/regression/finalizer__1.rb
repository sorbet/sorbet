# typed: false
# frozen_string_literal: true
# compiled: false

def finalizer_method
end

ObjectSpace.define_finalizer self, self.method(:finalizer_method)

class TestStruct < T::Struct
    const :amount, Integer
end

$stdout.puts "before"
$stdout.flush

require_relative './finalizer__2'
Opus::Utils::DeepFreeze.freeze!(TestStruct.new(amount: 10)).amount.nil?
