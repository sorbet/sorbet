# typed: false

class A
  define_method :transaction_currency do; end

  # This alias_method would error in `typed: true` because Sorbet can't see the
  # transaction_currency definition above.
  #
  # The __02.rb file simulate an rbi file for this define method.
  # Because alias_method is handled by resolver, namer will have already seen
  # the rbi defining transaction_currency by the time we get to this
  alias_method :currency, :transaction_currency
end

class B
  alias_method :foo, :does_not_exist
end

# foo should get stubbed, so we should be able to call it and use it anywhere
x = B.new.foo(1)
B.new.foo('', :sym, x)
