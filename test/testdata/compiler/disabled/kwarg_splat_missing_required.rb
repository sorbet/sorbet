# typed: true
# compiled: true
# frozen_string_literal: true

class Test
  extend T::Sig

  def self.main(x:, **args)
    p x
    p args
  end
end

Test.main(**T.unsafe({}))
