# frozen_string_literal: true

require 'minitest/autorun'
require 'minitest/spec'
require 'mocha/minitest'

require 'sorbet-runtime'
require 'oj'

module Opus
  module Types
    module Test
      module Wholesome; end
    end
  end
end

class Opus::Types::Test::Wholesome::TEnumOj < Minitest::Spec
  class MyEnum < T::Enum
    enums do
      X = new
    end
  end

  def assert_equal(exp, act, msg=nil)
    msg = message(msg, "") {diff exp, act}
    assert(exp.eql?(act), msg)
  end

  it 'works on a class' do
    serialized = Oj.dump(MyEnum::X)
    x_deser = Oj.load(serialized)

    assert_equal(MyEnum::X, x_deser)
  end
end
