# typed: strict
class Module; include T::Sig; end

sig { params(x: T.anything).void }
def example(x)
  int_validator = Validator::Cls.make(Integer)
  res = int_validator.parse!(x)
  T.reveal_type(res) # error: `Integer`

  str_validator = Validator::Cls.make(String)
  res = str_validator.parse!(x)
  T.reveal_type(res) # error: `String`

  res = Validator::Any.make(int_validator, str_validator).parse!(x)
  T.reveal_type(res) # error: `T.any(Integer, String)`
end


module Validator
  extend T::Generic
  abstract!

  Type = type_member

  class ParseResult < StandardError
    sig { params(msg: T.nilable(String)).void }
    def initialize(msg = nil)
      super
    end
  end

  sig do
    abstract
      .params(x: T.anything)
      .returns(T.any(Type, ParseResult))
  end
  def try_parse(x); end

  sig(:final) {params(x: T.anything).returns(Type)}
  def parse!(x)
    case (res = self.try_parse(x))
    when ParseResult then Kernel.raise(res)
    else res
    end
  end

  class Cls
    extend T::Generic
    include Validator

    Type = type_member

    # TODO(jez) T::Module?
    sig { params(klass: T::Class[Type]).void }
    def initialize(klass)
      @klass = klass
    end

    private_class_method :new
    sig do
      type_parameters(:Type)
        .params(klass: T::Class[T.type_parameter(:Type)])
        .returns(T.all(T.attached_class, Cls[T.type_parameter(:Type)]))
    end
    def self.make(klass)
      self.new(klass)
    end

    sig { override.params(x: T.anything).returns(T.any(Type, ParseResult)) }
    def try_parse(x)
      case x
      when @klass
        T.reveal_type(x) # error: `Validator::Cls::Type`
        return x
      else
        # In real code, might not want to print arbitrary value, might just
        # want to print the class (PII, etc.).
        return ParseResult.new("#{x} is not an instance of #{@klass}")
      end
    end
  end

  class Any
    extend T::Generic
    include Validator

    # TODO(jez) Would like to be able to use fixed here
    TypeReal = type_member { {fixed: T.any(Left, Right)} }
    #                                      ^^^^ error: is not allowed in this context
    #                                            ^^^^^ error: is not allowed in this context
    Type = type_member
    Left = type_member
    Right = type_member

    sig { params(left: Validator[Left], right: Validator[Right]).void }
    def initialize(left, right)
      @left = left
      @right = right
    end

    private_class_method :new
    sig do
      type_parameters(:Left, :Right)
        .params(
          left: Validator[T.type_parameter(:Left)],
          right: Validator[T.type_parameter(:Right)]
        )
        .returns(T.all(
          T.attached_class,
          Any[
            T.any(T.type_parameter(:Left), T.type_parameter(:Right)),
            T.type_parameter(:Left),
            T.type_parameter(:Right)
          ]
        ))
    end
    def self.make(left, right)
      self.new(left, right)
    end

    sig { override.params(x: T.anything).returns(T.any(Type, ParseResult)) }
    def try_parse(x)
      # TODO(jez) Would be able to avoid `T.cast` here if we could fix `Type`
      # to `T.any(Left, Right)`
      case (res = @left.try_parse(x))
      when ParseResult
        res = @right.try_parse(x)
        T.reveal_type(res) # error: `T.any(Validator::ParseResult, Validator::Any::Right)`
        return T.cast(res, T.any(Type, ParseResult))
      else
        T.reveal_type(res) # error: `Validator::Any::Left`
        return T.cast(res, Type)
      end
    end
  end
end


