---
id: type-aliases
title: Type Aliases
sidebar_label: T.type_alias
---

```ruby
Alias = T.type_alias {Type}
```

This creates a type alias of `Type` called `Alias`. In the context of Sorbet,
the type alias has exactly the same behavior as the original type and can be
used anywhere the original type can be used. The converse is also true.

Note that the type alias will not show up in error messages.

```ruby
# typed: true
extend T::Sig

Int = T.type_alias {Integer}
Str = T.type_alias {String}

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
A = T.type_alias {Integer}
B = T.type_alias {A}
```

For simple use cases, type aliases are nearly identical to just making a new
constant:

```ruby
# typed: true
extend T::Sig

A = T.type_alias {Integer}
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

A = T.type_alias {T.any(Integer, String)}
sig {returns(A)}
def foo; 3; end

B = T.any(Integer, String)
sig {returns(B)} # error: Constant B is not a class or type alias
def bar; 3; end
```

Note that because type aliases are a Sorbet construct, they cannot be used in
certain runtime contexts. For instance, it is not possible to match an
expression against a type alias in a `case` expression.

```ruby
# typed: true
extend T::Sig

class A; end
class B; end
class C; end

AB = T.type_alias {T.any(A, B)}
sig {params(x: T.any(AB, C)).returns(Integer)}
def invalid(x) # error: Returning value that does not conform to method result type
  case x
  when AB then 1 # <- this line is problematic
  when C then 2
  end
end
```

We could refactor this example to use `A, B` in the `when` and `AB` in the
`sig`. However, this introduces coupling between the definition of `AB` and our
method. If we ever updated the definition of `AB`, we would need to update the
definition of our method as well.

```ruby
sig {params(x: T.any(AB, C)).returns(Integer)}
def valid(x)
  case x
  when A, B then 1
  when C then 2
  end
end
```

## What about type aliases for method signatures?

Sometimes a question arises like, "Is there a way to factor an entire method
signature into a type alias, not just types for individual arguments?"

No, there is not. This is mostly for simplicity of implementation within Sorbet.

Two workarounds are:

1.  Define type aliases for all argument and return types of the methods in
    question.
2.  Factor shared arguments into a typed data structure (perhaps using
    [T::Struct]), and update the methods in question to take that structure.

Note that types for lambdas and procs can be written in type aliases using
[proc types](procs.md).

## What about recursive type aliases?

Some languages have recursive type aliases. For example, TypeScript allows
writing type aliases like this one which vaguely describes the type of all JSON
documents (example uses TypeScript syntax):

```typescript
type JSON = null | number | string | JSON[] | {[arg: string]: JSON};
```

Sorbet does not support recursive type aliases. To have types that reference
themselves, use [class types].

```ruby
class SelfReferential
  extend T::Sig

  sig {returns(T.nilable(SelfReferential))}
  attr_reader :val

  sig {params(val: T.nilable(SelfReferential)).void}
  def initialize(val); @val = val; end
end
```

Unfortunately for the case of typing JSON, this generally leads to more
verbosity than in other languages, but can still accomplish something similar:

[→ Full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20strict%0A%0Amodule%20MyJSON%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AHelpers%0A%20%20sealed!%0A%0A%20%20sig%20%7Bparams%28json%3A%20T.untyped%29.returns%28MyJSON%29%7D%0A%20%20def%20self.from_untyped%28json%29%0A%20%20%20%20case%20json%0A%20%20%20%20when%20nil%20then%20JSONNull.instance%0A%20%20%20%20when%20String%20then%20JSONString.new%28val%3A%20json%29%0A%20%20%20%20when%20Numeric%20then%20JSONNumber.new%28val%3A%20json%29%0A%20%20%20%20when%20Array%20then%20JSONArray.new%28val%3A%20json.map%20%7B%7Cj%7C%20from_untyped%28j%29%7D%29%0A%20%20%20%20when%20Hash%20then%20JSONObject.new%28val%3A%20json.transform_values%20%7B%7Cj%7C%20from_untyped%28j%29%7D%29%0A%20%20%20%20else%20raise%28ArgumentError.new%28%22malformed%20json%22%29%29%0A%20%20%20%20end%0A%20%20end%0A%0A%20%20class%20JSONNull%0A%20%20%20%20include%20MyJSON%0A%20%20%20%20include%20Singleton%0A%20%20end%0A%0A%20%20class%20JSONString%20%3C%20T%3A%3AStruct%0A%20%20%20%20include%20MyJSON%0A%20%20%20%20prop%20%3Aval%2C%20String%0A%20%20end%0A%0A%20%20class%20JSONNumber%20%3C%20T%3A%3AStruct%0A%20%20%20%20include%20MyJSON%0A%20%20%20%20prop%20%3Aval%2C%20Numeric%0A%20%20end%0A%0A%20%20class%20JSONArray%20%3C%20T%3A%3AStruct%0A%20%20%20%20include%20MyJSON%0A%20%20%20%20prop%20%3Aval%2C%20T%3A%3AArray%5BMyJSON%5D%0A%20%20end%0A%0A%20%20class%20JSONObject%20%3C%20T%3A%3AStruct%0A%20%20%20%20include%20MyJSON%0A%20%20%20%20prop%20%3Aval%2C%20T%3A%3AHash%5BString%2C%20MyJSON%5D%0A%20%20end%0Aend)

For the specific example of typing JSON, note that most Sorbet users tend to
just use `T::Hash[String, T.untyped]` or `T.untyped`. Serializing and
deserializing JSON is usually handled better by purpose-built serialization
libraries. The type of "all JSON documents" is usually unnaturally wide—it's
better to have an explicit step which converts the loosely JSON data structure
into a more structured internal representation.

[1]: https://sorbet.org/docs/error-reference#5034
[t::struct]: tstruct.md
[class types]: class-types.md
