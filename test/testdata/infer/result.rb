# typed: true
extend T::Sig

module Result
  extend T::Sig
  extend T::Generic
  sealed!

  OkType = type_member(:out)
  ErrType = type_member(:out)

  class Ok < T::Struct
    extend T::Generic
    include Result

    OkType = type_member
    ErrType = type_member {{fixed: T.noreturn}}

    prop :val, OkType
  end

  class Err < T::Struct
    extend T::Generic
    include Result

    OkType = type_member {{fixed: T.noreturn}}
    ErrType = type_member

    prop :error, ErrType
  end
end

class SomethingElse; end

sig {params(res: Result[Integer, TypeError]).void}
def example(res)
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
