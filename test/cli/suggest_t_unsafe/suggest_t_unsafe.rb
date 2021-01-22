# typed: true

def custom_wrapper(arg0); end

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

bar = T.let(1, T.any(Integer, String))
bar.even?

"hi" + 1
