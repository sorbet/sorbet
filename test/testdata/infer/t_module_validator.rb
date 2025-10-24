# typed: strong
class Module; include T::Sig; end

class Validator
  extend T::Generic

  Mod = type_member(:out)

  # would like to make this:
  # sig {params(mod: T::Module[Mod]).void}
  sig {params(mod: T::Module[Mod]).void}
  def initialize(mod)
    @mod = mod
  end

  sig {
    type_parameters(:Mod)
      .params(mod: T::Module[T.type_parameter(:Mod)])
      .returns(Validator[T.type_parameter(:Mod)])
  }
  def self.make(mod)
    Validator[T.type_parameter(:Mod)].new(mod)
  end

  sig {
    type_parameters(:Val)
      .params(val: T.type_parameter(:Val))
      .returns(T.nilable(T.all(T.type_parameter(:Val), Mod)))
  }
  def try_validate(val)
    T.reveal_type(@mod) # error: `T::Module[Validator::Mod]`

    case val
    when @mod
      return val
    else
      return nil
    end
  end
end

module IFoo; end
class Foo
  include IFoo
end

sig { params(x: T.anything).void }
def example(x)
  res = Validator.make(Integer).try_validate('not an int')
  T.reveal_type(res) # error: `NilClass`
  res = Validator.make(Integer).try_validate(0)
  T.reveal_type(res) # error: `T.nilable(Integer)`
  res = Validator.make(Integer).try_validate(Object.new)
  T.reveal_type(res) # error: `T.nilable(Integer)`

  res = Validator.make(IFoo).try_validate(Foo.new)
  T.reveal_type(res) # error: `T.nilable(Foo)`
  res = Validator.make(IFoo).try_validate('')
  T.reveal_type(res) # error: `T.nilable(T.all(IFoo, String))`
end

