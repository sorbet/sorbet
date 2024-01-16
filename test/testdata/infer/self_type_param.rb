# typed: true

class GenericClass
  extend(T::Sig)
  extend(T::Generic)

  Elem = type_member

  sig {returns(Elem)}
  def return_type_member; T.unsafe(nil) end

  sig {params(block: T.proc.params(arg0: Elem).void).void}
  def yield_type_member(&block); end

  def kwargs(b: 1); end

  def block_arg
    yield_type_member { |x, y, z| }
  end

  def as_optional_hash
    kwargs(return_type_member)
    #      ^^^^^^^^^^^^^^^^^^ error: Too many positional arguments provided
  end

  def splat_expand
    a, b = return_type_member
  end
end
