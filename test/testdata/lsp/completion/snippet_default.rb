# typed: true

class Test
  extend T::Sig

  sig {params(x: Integer, y: Symbol, z: String).void}
  def method_with_defaults(x, y:, z: ''); end
end

Test.new.method_with_default # error: does not exist
#                           ^ apply-completion: [A] item: 0
