# typed: false

[]

[1, 2, 3]

%i[symbol1 symbol2 symbol3]

%I[symbol4 symbol5 symbol6]

%w[string4 string5 string6]

%W[string7 string8 string9]

def has_named_rest_args(*args)
  [1, 2, *args, 3]
end

def has_anonymous_rest_args(*)
  [1, 2, *, 3]
end
