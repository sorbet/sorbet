# typed: strict
class Module; include T::Sig; end

sig { params(x: T.anything).void }
def example(x)
  int_validator = Validator::Cls.make(Integer)
  res = int_validator.parse!(x)
  T.reveal_type(res) # => Integer

  str_validator = Validator::Cls.make(String)
  res = str_validator.parse!(x)
  T.reveal_type(res) # => String

  int_or_str_validator = Validator::Any.make(int_validator, str_validator)
  T.reveal_type(int_or_str_validator)
  res = int_or_str_validator.parse!(x)
  T.reveal_type(res) # => T.any(Integer, String)

  res = Validator::Shape.make({x: 0.to_i, y: ''.to_s}).parse!(x)
  T.reveal_type(res) # => {x: Integer, y: String}
end


module Validator
  extend T::Generic
  abstract!

  # TODO(jez) Should these all be :out ?
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
        # TODO(jez) Blocked on smarter Class#=== intrinsic for T::Class
        return x
      else
        # TODO(jez) Might not want to print arbitrary value, might just want to
        # print the class.
        return ParseResult.new("#{x} is not an instance of #{@klass}")
      end
    end
  end

  class Any
    extend T::Generic
    include Validator

    Type = type_member { {fixed: T.any(Left, Right)} }
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
          Any[T.type_parameter(:Left), T.type_parameter(:Right)]
        ))
    end
    def self.make(left, right)
      self.new(left, right)
    end

    sig { override.params(x: T.anything).returns(T.any(Type, ParseResult)) }
    def try_parse(x)
      case (res = @left.try_parse(x))
      when ParseResult
        res = @right.try_parse(x)
        T.reveal_type(res)
        return res
      else
        T.reveal_type(res)
        return res
      end
    end
  end
end


