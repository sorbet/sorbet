# typed: true
class Foo
  extend T::Sig

  A = T.type_alias {Integer}
  sig {params(a: A).returns(Integer)}
  def bar(a)
    a
  end

  Reactions = T.type_alias {T.enum(['+1', '-1', 'laugh', 'confused', 'heart', 'hooray'])}

  sig {returns(Reactions)}
  def react
    any
  end

  ADDRESS_TYPE = T.type_alias do
    T.nilable({
      line1: T.nilable(String),
      line2: T.nilable(String),
      city: T.nilable(String),
      state: T.nilable(String),
      postal_code: T.nilable(String),
      country: T.nilable(String),
    })
  end

  sig {returns(ADDRESS_TYPE)}
  def address
    any
  end

  def any; end
end
