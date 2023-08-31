# typed: strict
class Module; include T::Sig; end

class Parent
  sig do
    type_parameters(:U)
      .params(
        x: T.type_parameter(:U),
        blk: T.proc.params(x: T.type_parameter(:U)).returns(T.type_parameter(:U))
      )
        .void
  end
  def initialize(x, &blk)
  end
end

class Child < Parent
  sig {void}
  def initialize
    super(0) do |x|
      T.reveal_type(x) # error: `Integer`
      puts 'hello'
    end
  end
end
