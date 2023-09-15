# typed: true

class A
  def to; end

  alias_method :from, :to
  alias_method :from, :to_bad
  #                   ^^^^^^^ error: Can't make method alias from `from` to non existing method `to_bad`
  #            ^^^^^ error: Redefining method alias `A#from` from `A#to` to `Sorbet::Private::Static#<bad-method-alias-stub>`
end

class B
  alias_method :from, :does_not_exist1 # error: Can't make method alias from `from` to non existing method `does_not_exist1`
  alias_method :from, :does_not_exist2 # error: Can't make method alias from `from` to non existing method `does_not_exist2`
end
