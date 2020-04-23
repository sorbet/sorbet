# typed: strict

# These model the runtime representations of Sorbet's type syntax.  This is
# considered a private API and might be changed at any time without notice.

module T::Types
end

class T::Types::Base
  def self.method_added(method_name); end
  def valid?(obj); end
  def name; end
  def subtype_of?(t2); end
  def to_s; end
  def describe_obj(obj); end
  def error_message_for_obj(obj); end
  def validate!(obj); end
  def hash; end
  def ==(other); end
end

class T::Types::Simple < T::Types::Base
  def initialize(raw_type); end
  def name; end
  def valid?(obj); end
  def raw_type; end
end

class T::Types::Union < T::Types::Base
  def initialize(types); end
  def name; end
  def valid?(obj); end
  def types; end
end

class T::Types::Intersection < T::Types::Base
  def initialize(types); end
  def name; end
  def valid?(obj); end
  def types; end
end

class T::Types::ClassOf < T::Types::Base
  def initialize(type); end
  def name; end
  def valid?(obj); end
  def subtype_of_single?(other); end
  def describe_obj(obj); end
  def type; end
end

class T::Types::FixedArray < T::Types::Base
  def initialize(types); end
  def name; end
  def valid?(obj); end
  def describe_obj(obj); end
  def types; end
end

class T::Types::FixedHash < T::Types::Base
  def initialize(types); end
  def name; end
  def valid?(obj); end
  def describe_obj(obj); end
  def types; end
end

class T::Types::Untyped < T::Types::Base
  def initialize; end
  def name; end
  def valid?(obj); end
end

class T::Types::Proc < T::Types::Base
  def initialize(arg_types, returns); end
  def name; end
  def valid?(obj); end
  def arg_types; end
  def returns; end
end

class T::Types::NoReturn < T::Types::Base
  def initialize; end
  def name; end
  def valid?(obj); end
end

class T::Types::Enum < T::Types::Base
  def initialize(values); end
  def valid?(obj); end
  def name; end
  def describe_obj(obj); end
  def values; end
end

class T::Types::SelfType < T::Types::Base
  def initialize(); end
  def name; end
  def valid?(obj); end
end

# --- user-defined generics ---

class T::Types::TypeVariable < T::Types::Base
  def initialize(variance); end
  def name; end
  def subtype_of_single?(type); end
  def valid?(obj); end
  def variance; end
end

class T::Types::TypeMember < T::Types::TypeVariable
end

class T::Types::TypeTemplate < T::Types::TypeVariable
end

class T::Types::TypeParameter < T::Types::Base
  def initialize(name); end
  def valid?(obj); end
  def subtype_of_single?(type); end
  def name; end
end

# --- stdlib generics ---

class T::Types::TypedArray < T::Types::TypedEnumerable
  def name; end
  def valid?(obj); end
  def new(*args); end
end

class T::Types::TypedHash < T::Types::TypedEnumerable
  def initialize(keys:, values:); end
  def name; end
  def valid?(obj); end
  def keys; end
  def values; end
end

class T::Types::TypedEnumerable < T::Types::Base
  def initialize(type); end
  def name; end
  def valid?(obj); end
  def describe_obj(obj); end
  def type; end
end

class T::Types::TypedSet < T::Types::TypedEnumerable
  def name; end
  def valid?(obj); end
  def new(*args); end
  def type; end
end

class T::Types::TypedRange < T::Types::TypedEnumerable
  def name; end
  def valid?(obj); end
  def new(*args); end
  def type; end
end

class T::Types::TypedEnumerator < T::Types::TypedEnumerable
  def name; end
  def valid?(obj); end
  def new(*args); end
  def type; end
end

