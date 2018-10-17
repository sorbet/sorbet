# typed: true
Sorbet.sig{params(a: Integer).void}

def foo(a)
end

foo( #error: `String("\n")` doesn't match
'
'
)
