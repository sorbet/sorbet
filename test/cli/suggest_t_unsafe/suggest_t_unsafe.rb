# typed: true

extend T::Sig

def custom_wrapper(arg0); end

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

bar = T.let(1, T.any(Integer, String))
bar.even?

"hi" + 1

T::Array[T.nilable(Integer)].new.map(&:even?)

sig {params(str: String).void}
def takes_string_kw(str:); end

sig {params(nilable_string: T.nilable(String)).void}
def takes_nilable_string(nilable_string)
  # This used to crash Sorbet, because there was no Loc for the `str` argument
  takes_string_kw(str: nilable_string)
end
