# typed: true
extend T::Sig

sig{params(a: Integer).void}

def foo(a)
end

foo( #error: `String("\n")` does not match
'
'
)
