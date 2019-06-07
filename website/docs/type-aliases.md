---
id: type-aliases
title: Type Aliases
---

> TODO: This page is still a fragment. Contributions welcome!

```ruby
Alias = T.type_alias(Type)
```

This creates a type alias of `Type` called `Alias`. The type alias has exactly
the same behavior as the original type and can be used anywhere the original
type can be used. The converse is also true.

Note that the type alias will not show up in error messages.

```ruby
# typed: true
extend T::Sig

Int = T.type_alias(Integer)
Str = T.type_alias(String)

sig {params(x: Int).returns(Str)}
def foo(x)
  T.reveal_type(x) # Revealed type: Integer
  x.to_s
end

a = T.let(3, Integer)
foo(a)
b = T.let(3, Int)
foo(b)

c = foo(3)
T.reveal_type(c) # Revealed type: String
```

When creating a type alias from another type alias, you [must use `T.type_alias`
again][1]:

```ruby
A = T.type_alias(Integer)
B = T.type_alias(A)
```

For simple use cases, type aliases are nearly identical to just making a new
constant:

```ruby
# typed: true
extend T::Sig

A = T.type_alias(Integer)
sig {returns(A)}
def foo; 3; end

B = Integer
sig {returns(B)}
def bar; 3; end
```

However, when the type is more complex, you must use type aliases:

```ruby
# typed: true
extend T::Sig

A = T.type_alias(T.any(Integer, String))
sig {returns(A)}
def foo; 3; end

B = T.any(Integer, String)
sig {returns(B)} # error: Constant B is not a class or type alias
def bar; 3; end
```

[1]: https://sorbet.org/docs/error-reference#5034
