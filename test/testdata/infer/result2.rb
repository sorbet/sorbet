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
      .returns(Result[T.type_parameter(:Ok), T.all(ErrType, T.type_parameter(:Err))])
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
        .returns(Result[T.type_parameter(:Ok), T.all(ErrType, T.type_parameter(:Err))])
    end
    def and_then(&blk)
      raise
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
        .returns(Result[T.type_parameter(:Ok), T.all(ErrType, T.type_parameter(:Err))])
    end
    def and_then(&blk)
      raise
    end
  end
end
