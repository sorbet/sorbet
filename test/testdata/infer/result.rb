# typed: true
extend T::Sig

module Result
  extend T::Sig
  extend T::Generic
  sealed!
  abstract!

  OkType = type_member(:out)
  ErrType = type_member(:out)

  sig do
    abstract
      .type_parameters(:Ok, :Err)
      .params(
        blk: T.proc.params(arg0: OkType)
               .returns(Result[T.type_parameter(:Ok), T.type_parameter(:Err)])
      )
      .returns(Result[T.type_parameter(:Ok), T.any(ErrType, T.type_parameter(:Err))])
  end
  def and_then(&blk)
  end

  class Ok < T::Struct
    extend T::Sig
    extend T::Generic
    include Result

    OkType = type_member
    ErrType = type_member {{fixed: T.noreturn}}

    prop :val, OkType

    sig do
      override
        .type_parameters(:Ok, :Err)
        .params(
          blk: T.proc.params(arg0: OkType)
                 .returns(Result[T.type_parameter(:Ok), T.type_parameter(:Err)])
        )
        .returns(Result[T.type_parameter(:Ok), T.any(ErrType, T.type_parameter(:Err))])
    end
    def and_then(&blk)
      yield self.val
    end
  end

  class Err < T::Struct
    extend T::Sig
    extend T::Generic
    include Result

    OkType = type_member {{fixed: T.noreturn}}
    ErrType = type_member

    prop :error, ErrType

    sig do
      override
        .type_parameters(:Ok, :Err)
        .params(
          blk: T.proc.params(arg0: OkType)
                 .returns(Result[T.type_parameter(:Ok), T.type_parameter(:Err)])
        )
        .returns(Result[T.type_parameter(:Ok), T.any(ErrType, T.type_parameter(:Err))])
    end
    def and_then(&blk)
      self
    end
  end
end

class SomethingElse; end

sig {params(res: Result[Integer, TypeError]).void}
def example1(res)
  case res
  when Result::Ok
    T.reveal_type(res) # error: `Result::Ok[Integer]`
    T.reveal_type(res.val) # error: `Integer`

    # Can widen the type to anything else (covariance)
    _unused = T.let(res, Result[Integer, SomethingElse])
  when Result::Err
    T.reveal_type(res) # error: `Result::Err[TypeError]`
    T.reveal_type(res.error) # error: `TypeError`

    # Can widen the type to anything else (covariance)
    _unused = T.let(res, Result[SomethingElse, TypeError])
  else
    T.absurd(res)
  end
end

sig {params(res: Result[Integer, TypeError]).void}
def example2(res)
  res1 = res.and_then do |arg0|
    T.reveal_type(arg0) # error: `Integer`
    ok = Result::Ok[String].new(val: arg0.to_s)
    T.reveal_type(ok) # error: `Result::Ok[String]`
    ok
  end
  T.reveal_type(res1) # error: `Result[String, TypeError]`

  res2 = res.and_then do |arg0|
    T.reveal_type(arg0) # error: `Integer`
    new_res = if arg0.even?
      Result::Err[ArgumentError].new(error: ArgumentError.new("Don't give an even number"))
    else
      Result::Ok[String].new(val: arg0.to_s)
    end
    T.reveal_type(new_res) # error: `T.any(Result::Err[ArgumentError], Result::Ok[String])`
    new_res
  end
  T.reveal_type(res2) # error: `Result[String, T.any(TypeError, ArgumentError)]`
end
