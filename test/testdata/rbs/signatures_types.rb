# typed: strict
# enable-experimental-rbs-comments: true

extend T::Sig

module A; end
module B; end
module C; end

module Foo
  class Bar
    class Baz; end
  end
end

# Class instance types

#: -> Foo
def class_instance_type1; T.unsafe(nil); end
T.reveal_type(class_instance_type1) # error: Revealed type: `Foo`

#: -> ::Foo
def class_instance_type2; T.unsafe(nil); end
T.reveal_type(class_instance_type2) # error: Revealed type: `Foo`

#: -> Foo::Bar
def class_instance_type3; T.unsafe(nil); end
T.reveal_type(class_instance_type3) # error: Revealed type: `Foo::Bar`

#: -> Foo::Bar::Baz
def class_instance_type4; T.unsafe(nil); end
T.reveal_type(class_instance_type4) # error: Revealed type: `Foo::Bar::Baz`

#: -> ::Foo::Bar
def class_instance_type5; T.unsafe(nil); end
T.reveal_type(class_instance_type5) # error: Revealed type: `Foo::Bar`

# Class singleton types

#: -> singleton(Foo)
def class_singleton_type1; T.unsafe(nil); end
T.reveal_type(class_singleton_type1) # error: Revealed type: `T.class_of(Foo)`

#: -> singleton(::Foo)
def class_singleton_type2; T.unsafe(nil); end
T.reveal_type(class_singleton_type2) # error: Revealed type: `T.class_of(Foo)`

#: -> singleton(Foo::Bar)
def class_singleton_type3; T.unsafe(nil); end
T.reveal_type(class_singleton_type3) # error: Revealed type: `T.class_of(Foo::Bar)`

#: -> singleton(Foo::Bar::Baz)
def class_singleton_type4; T.unsafe(nil); end
T.reveal_type(class_singleton_type4) # error: Revealed type: `T.class_of(Foo::Bar::Baz)`

#: -> singleton(::Foo::Bar)
def class_singleton_type5; T.unsafe(nil); end
T.reveal_type(class_singleton_type5) # error: Revealed type: `T.class_of(Foo::Bar)`

# Union types

#: -> (Foo | Foo::Bar)
def union_type1; T.unsafe(nil); end
T.reveal_type(union_type1) # error: Revealed type: `T.any(Foo, Foo::Bar)`

#: -> (Foo | Foo::Bar | ::Foo::Bar::Baz)
def union_type2; T.unsafe(nil); end
T.reveal_type(union_type2) # error: Revealed type: `T.any(Foo, Foo::Bar, Foo::Bar::Baz)`

# Intersection types

#: -> (A & B & C)
def intersection_type1; T.unsafe(nil); end
T.reveal_type(intersection_type1) # error: Revealed type: `T.all(A, B, C)`

# Optional types

#: -> Foo?
def optional_type1; T.unsafe(nil); end
T.reveal_type(optional_type1) # error: Revealed type: `T.nilable(Foo)`

# Base types

#: -> self
def base_type1; T.unsafe(nil); end
T.reveal_type(base_type1) # error: Revealed type: `T.class_of(<root>)`

class BaseType2
  #: -> instance
  def self.base_type2; T.unsafe(nil); end
end
T.reveal_type(BaseType2.base_type2) # error: Revealed type: `BaseType2`

# TODO: unsupported
#: -> class
#     ^^^^^ error: RBS type `class` is not supported
def base_type3; T.unsafe(nil); end
T.reveal_type(base_type3) # error: Revealed type: `T.untyped`

#: -> bool
def base_type4; T.unsafe(nil); end
T.reveal_type(base_type4) # error: Revealed type: `T::Boolean`

#: -> nil
def base_type5; T.unsafe(nil); end
T.reveal_type(base_type5) # error: Revealed type: `NilClass`

#: -> top
def base_type6; T.unsafe(nil); end
T.reveal_type(base_type6) # error: Revealed type: `T.anything`

#: -> void
def base_type7; T.unsafe(nil); end
T.reveal_type(base_type7) # error: Revealed type: `Sorbet::Private::Static::Void`

# Generic types

#: -> Array[Integer]
def generic_type_array; T.unsafe(nil); end
T.reveal_type(generic_type_array) # error: Revealed type: `T::Array[Integer]`

#: -> ::Array[Integer]
def generic_type_array_root; T.unsafe(nil); end
T.reveal_type(generic_type_array_root) # error: Revealed type: `T::Array[Integer]`

#: -> Class
#     ^^^^^ error: Malformed type declaration. Generic class without type arguments `Class`
def non_generic_type_class; T.unsafe(nil); end
T.reveal_type(non_generic_type_class) # error: Revealed type: `T::Class[T.anything]`

#: -> ::Class
#     ^^^^^^^ error: Malformed type declaration. Generic class without type arguments `Class`
def non_generic_type_class_root; T.unsafe(nil); end
T.reveal_type(non_generic_type_class_root) # error: Revealed type: `T::Class[T.anything]`

#: -> Class[Integer]
def generic_type_class; T.unsafe(nil); end
T.reveal_type(generic_type_class) # error: Revealed type: `T::Class[Integer]`

#: -> ::Class[Integer]
def generic_type_class_root; T.unsafe(nil); end
T.reveal_type(generic_type_class_root) # error: Revealed type: `T::Class[Integer]`

#: -> Module
#     ^^^^^^ error: Malformed type declaration. Generic class without type arguments `Module`
def non_generic_type_module; T.unsafe(nil); end
T.reveal_type(non_generic_type_module) # error: Revealed type: `T::Module[T.anything]`

#: -> ::Module
#     ^^^^^^^^ error: Malformed type declaration. Generic class without type arguments `Module`
def non_generic_type_module_root; T.unsafe(nil); end
T.reveal_type(non_generic_type_module_root) # error: Revealed type: `T::Module[T.anything]`

#: -> Module[Integer]
def generic_type_module; T.unsafe(nil); end
T.reveal_type(generic_type_module) # error: Revealed type: `T::Module[Integer]`

#: -> ::Module[Integer]
def generic_type_module_root; T.unsafe(nil); end
T.reveal_type(generic_type_module_root) # error: Revealed type: `T::Module[Integer]`

#: -> Enumerable[Integer]
def generic_type_enumerable; T.unsafe(nil); end
T.reveal_type(generic_type_enumerable) # error: Revealed type: `T::Enumerable[Integer]`

#: -> ::Enumerable[Integer]
def generic_type_enumerable_root; T.unsafe(nil); end
T.reveal_type(generic_type_enumerable_root) # error: Revealed type: `T::Enumerable[Integer]`

#: -> Enumerator[Integer]
def generic_type_enumerator; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator) # error: Revealed type: `T::Enumerator[Integer]`

#: -> ::Enumerator[Integer]
def generic_type_enumerator_root; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator_root) # error: Revealed type: `T::Enumerator[Integer]`

#: -> Enumerator::Lazy[Integer]
def generic_type_enumerator_lazy; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator_lazy) # error: Revealed type: `T::Enumerator::Lazy[Integer]`

#: -> ::Enumerator::Lazy[Integer]
def generic_type_enumerator_lazy_root; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator_lazy_root) # error: Revealed type: `T::Enumerator::Lazy[Integer]`

#: -> Enumerator::Chain[Integer]
def generic_type_enumerator_chain; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator_chain) # error: Revealed type: `T::Enumerator::Chain[Integer]`

#: -> ::Enumerator::Chain[Integer]
def generic_type_enumerator_chain_root; T.unsafe(nil); end
T.reveal_type(generic_type_enumerator_chain_root) # error: Revealed type: `T::Enumerator::Chain[Integer]`

#: -> Hash[String, Integer]
def generic_type_hash; T.unsafe(nil); end
T.reveal_type(generic_type_hash) # error: Revealed type: `T::Hash[String, Integer]`

#: -> ::Hash[String, Integer]
def generic_type_hash_root; T.unsafe(nil); end
T.reveal_type(generic_type_hash_root) # error: Revealed type: `T::Hash[String, Integer]`

#: -> Set[Integer]
def generic_type_set; T.unsafe(nil); end
T.reveal_type(generic_type_set) # error: Revealed type: `T::Set[Integer]`

#: -> ::Set[Integer]
def generic_type_set_root; T.unsafe(nil); end
T.reveal_type(generic_type_set_root) # error: Revealed type: `T::Set[Integer]`

#: -> Range[Integer]
def generic_type_range; T.unsafe(nil); end
T.reveal_type(generic_type_range) # error: Revealed type: `T::Range[Integer]`

#: -> ::Range[Integer]
def generic_type_range_root; T.unsafe(nil); end
T.reveal_type(generic_type_range_root) # error: Revealed type: `T::Range[Integer]`

#: -> T::Array[Integer]
def generic_type_t_array; T.unsafe(nil); end
T.reveal_type(generic_type_t_array) # error: Revealed type: `T::Array[Integer]`

#: -> T::Hash[String, Integer]
def generic_type_t_hash; T.unsafe(nil); end
T.reveal_type(generic_type_t_hash) # error: Revealed type: `T::Hash[String, Integer]`

class GenericType1
  extend T::Generic

  T1 = type_member
end

class GenericType2
  extend T::Generic

  T1 = type_member
  T2 = type_member
end

class GenericType3
  extend T::Generic

  T1 = type_member
  T2 = type_member
  T3 = type_member
end

#: -> GenericType1[Integer]
def generic_type5; T.unsafe(nil); end
T.reveal_type(generic_type5) # error: Revealed type: `GenericType1[Integer]`

#: -> GenericType2[GenericType1[untyped], GenericType3[Integer, String, untyped]]
def generic_type6; T.unsafe(nil); end
T.reveal_type(generic_type6) # error: Revealed type: `GenericType2[GenericType1[T.untyped], GenericType3[Integer, String, T.untyped]]`

# Tuples

#: -> [Integer]
def tuple_type1; T.unsafe(nil); end
T.reveal_type(tuple_type1) # error: Revealed type: `[Integer] (1-tuple)`

#: -> [Integer, String, untyped]
def tuple_type2; T.unsafe(nil); end
T.reveal_type(tuple_type2) # error: Revealed type: `[Integer, String, T.untyped] (3-tuple)`

# Shapes

#: -> { id: String, name: String }
def shape_type1; T.unsafe(nil); end

#: -> {"a" => String, "b" => Integer}
def shape_type1; T.unsafe(nil); end
T.reveal_type(shape_type1) # error: Revealed type: `{String("a") => String, String("b") => Integer} (shape of T::Hash[T.untyped, T.untyped])`

#: -> {a: String, b: Integer}
def shape_type2; T.unsafe(nil); end
T.reveal_type(shape_type2) # error: Revealed type: `{a: String, b: Integer} (shape of T::Hash[T.untyped, T.untyped])`

#: -> {"a" => String, :b => Integer}
def shape_type3; T.unsafe(nil); end
T.reveal_type(shape_type3) # error: Revealed type: `{String("a") => String, b: Integer} (shape of T::Hash[T.untyped, T.untyped])`

# Proc types

#: -> ^(Integer, String) -> String
def proc_type1; T.unsafe(nil); end
T.reveal_type(proc_type1) # error: Revealed type: `T.proc.params(arg0: Integer, arg1: String).returns(String)`

#: -> ^() [self: Foo] -> void # error: Using `bind` is not permitted here
def proc_type2; T.unsafe(nil); end

#: -> ^(?) -> untyped
def proc_type3; T.unsafe(nil); end
T.reveal_type(proc_type3) # error: Revealed type: `T.untyped`

#: -> (^(?) -> untyped)?
def proc_type4; T.unsafe(nil); end
T.reveal_type(proc_type4) # error: Revealed type: `T.untyped`

# Mixed tests

# Translates to `T.noreturn`
#: -> bot
def base_type_last; T.unsafe(nil); end
T.reveal_type(base_type_last) # error: This code is unreachable

# TODO: unsupported
# Alias types (not yet supported)

#: -> foo
#     ^^^ error: Unable to resolve constant `type foo`
def alias_type1; T.unsafe(nil); end

# TODO: unsupported
# Interface types (not yet supported)

#: -> _A
#     ^^ error: RBS interfaces are not supported
def interface_type1; T.unsafe(nil); end

# TODO: unsupported
# Literal types (not yet supported)

#: -> "foo"
#     ^^^^^ error: RBS literal types are not supported
def literal_type1; T.unsafe(nil); end

# TODO: unsupported
# Untyped functions (not yet supported)

#: (?) -> untyped # error: Unexpected node type `RBS::Types::UntypedFunction` in method signature, expected `Function`
def untyped_function1; T.unsafe(nil); end # error: The method `untyped_function1` does not have a `sig`
