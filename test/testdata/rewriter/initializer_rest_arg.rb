# typed: strict

class A
  extend T::Sig

  sig {params(rest: String, kwargs: String).void}
  def initialize(*rest, **kwargs)
    @rest = rest
    @kwargs = kwargs
  end

  sig {params(arg_name: Symbol).returns(T.nilable(String))}
  def retrieve_field(arg_name)
    T.reveal_type(@kwargs) # error: `T::Hash[Symbol, String]`
    @kwargs[arg_name]
  end

  sig {params(index: Integer).returns(T.nilable(String))}
  def retrieve_nth(index)
    T.reveal_type(@rest) # error: `T::Array[String]`
    @rest[index]
  end
end
