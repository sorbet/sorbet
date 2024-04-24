# frozen_string_literal: true
# typed: strict

module UsesFlatfiles::Client
  extend T::Sig

  sig {params(x: Flatfiles::MyFlatfile).returns(T.untyped)}
  def self.do_the_thing(x)
    x.foo
  end
end
