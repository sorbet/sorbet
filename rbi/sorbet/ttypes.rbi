# typed: true

# These model the runtime representations of Sorbet's type syntax.  This is
# considered a private API and might be changed at any time without notice.

module T::Types
end

class T::Types::Base
  abstract!

  def self.method_added(method_name); end

  sig { abstract.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  # Should return `String` but this is hard because technically
  # `T::Types::Simple` returns `nil` if given an anonymous module.
  # We should fix this later.
  sig { abstract.returns(T.untyped) }
  def name; end

  def subtype_of?(t2); end
  def to_s; end
  def describe_obj(obj); end
  def error_message_for_obj(obj); end
  def error_message_for_obj_recursive(obj); end
  def validate!(obj); end
  def hash; end

  # Type equivalence, defined by serializing the type to a string (with
  # `#name`) and comparing the resulting strings for equality.
  #
  # NOTE: Equality on types is almost never what you want. The vast majority of
  # the time, what you actually care about is whether one type is a subtype of
  # another type. (This `==` method doesn't even implement "type equivalence"
  # which would be whether t1 <: t2 && t2 <: t1).
  sig { params(other: T.anything).returns(T::Boolean) }
  def ==(other); end
end

class T::Types::Simple < T::Types::Base
  sig { params(raw_type: Module).void }
  def initialize(raw_type); end

  sig { override.returns(T.nilable(String)) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(Module) }
  def raw_type; end
end

class T::Types::Union < T::Types::Base
  sig { params(types: T::Array[T.untyped]).void }
  def initialize(types); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(T::Array[T::Types::Base]) }
  def types; end

  sig { returns(T.nilable(T::Types::Base)) }
  def unwrap_nilable; end
end

class T::Types::Intersection < T::Types::Base
  sig { params(types: T::Array[T.anything]).void }
  def initialize(types); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(T::Array[T::Types::Base]) }
  def types; end
end

class T::Types::ClassOf < T::Types::Base
  sig { params(type: Module).void }
  def initialize(type); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def subtype_of_single?(other); end
  def describe_obj(obj); end

  sig { returns(Module) }
  def type; end

  def [](*types); end
end

class T::Types::FixedArray < T::Types::Base
  sig { params(types: T::Array[T.anything]).void }
  def initialize(types); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def describe_obj(obj); end

  sig { returns(T::Array[T::Types::Base]) }
  def types; end
end

class T::Types::FixedHash < T::Types::Base
  sig { params(types: T::Hash[T.untyped, T.anything]).void }
  def initialize(types); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def describe_obj(obj); end

  sig { returns(T::Hash[T.untyped, T::Types::Base]) }
  def types; end
end

class T::Types::Untyped < T::Types::Base
  sig { void }
  def initialize; end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end
end

class T::Types::Anything < T::Types::Base
  sig { void }
  def initialize; end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end
end

class T::Types::Proc < T::Types::Base
  sig { params(arg_types: T::Hash[T.untyped, T.anything], returns: T.anything).void }
  def initialize(arg_types, returns); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(T::Hash[T.untyped, T::Types::Base]) }
  def arg_types; end

  sig { returns(T::Types::Base) }
  def returns; end
end

class T::Types::NoReturn < T::Types::Base
  sig { void }
  def initialize; end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end
end

class T::Types::Enum < T::Types::Base
  sig { params(values: T.any(T::Set[T.anything], T::Array[T.anything], T::Hash[T.anything, T.anything])).void }
  def initialize(values); end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { override.returns(String) }
  def name; end

  def describe_obj(obj); end

  sig { returns(T.any(T::Set[T.anything], T::Array[T.anything], T::Hash[T.anything, T.anything])) }
  def values; end
end

class T::Types::TEnum < T::Types::Base
  sig { params(val: T::Enum).void }
  def initialize(val); end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { override.returns(String) }
  def name; end

  def describe_obj(obj); end

  sig { returns(T::Enum) }
  def val; end
end

class T::Types::SelfType < T::Types::Base
  sig { void }
  def initialize(); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end
end

# --- user-defined generics ---

class T::Types::TypeVariable < T::Types::Base
  def initialize(variance); end

  sig { override.returns(String) }
  def name; end

  def subtype_of_single?(type); end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(Symbol) }
  def variance; end
end

class T::Types::TypeMember < T::Types::TypeVariable
end

class T::Types::TypeTemplate < T::Types::TypeVariable
end

class T::Types::TypeParameter < T::Types::Base
  def initialize(name); end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def subtype_of_single?(type); end

  sig { override.returns(String) }
  def name; end

  def self.make(name); end
end

# --- stdlib generics ---

class T::Types::TypedArray < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { override.returns(Module) }
  def underlying_class; end

  def new(...); end
  def recursively_valid?(obj); end
end

class T::Types::TypedHash < T::Types::TypedEnumerable
  def initialize(keys:, values:); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  sig { returns(T::Types::Base) }
  def keys; end

  sig { returns(T::Types::Base) }
  def values; end

  sig { override.returns(Module) }
  def underlying_class; end

  def new(...); end
end

class T::Types::TypedEnumerable < T::Types::Base
  sig { params(type: T.anything).void }
  def initialize(type); end

  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def describe_obj(obj); end

  sig { overridable.returns(T::Types::Base) }
  def type; end

  sig { overridable.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedSet < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  def new(...); end

  sig { override.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedRange < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  def new(...); end

  sig { override.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedEnumerator < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  def new(...); end

  sig { override.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedEnumeratorLazy < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  def new(...); end

  sig { override.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedEnumeratorChain < T::Types::TypedEnumerable
  sig { override.returns(String) }
  def name; end

  sig { override.params(obj: Kernel).returns(T::Boolean) }
  def valid?(obj); end

  def recursively_valid?(obj); end

  def new(...); end

  sig { override.returns(Module) }
  def underlying_class; end
end

class T::Types::TypedClass < T::Types::Base
  sig {params(type: T.untyped).void}
  def initialize(type); end

  sig {returns(String)}
  def name; end

  sig {override.params(obj: Kernel).returns(T::Boolean)}
  def valid?(obj); end

  sig {returns(T::Types::Base)}
  def type; end

  sig {returns(T.class_of(Class))}
  def underlying_class; end
end

class T::Types::TypedModule < T::Types::Base
  sig {params(type: T.untyped).void}
  def initialize(type); end

  sig {returns(String)}
  def name; end

  sig {override.params(obj: Kernel).returns(T::Boolean)}
  def valid?(obj); end

  sig {returns(T::Types::Base)}
  def type; end

  sig {returns(T.class_of(Module))}
  def underlying_class; end
end
