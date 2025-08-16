# typed: true

module Reader
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.returns(Integer) }
  def foo; end
end

module Writer
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.params(arg0: Integer).returns(Integer) }
  def foo=(arg0); end
end

class Good < T::Struct
  include Reader
  include Writer

  prop :foo, Integer, override: true
end

class Bad < T::Struct
  include Reader
  include Writer

  prop :foo, String, override: true
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of abstract method `Reader#foo`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Return type `String` does not match return type of abstract method `Writer#foo=`
#       ^^^ error: Parameter `foo` of type `String` not compatible with type of abstract method `Writer#foo=`
end

class ReaderOnly < T::Struct
  include Reader

  prop :foo, Integer, override: :reader
end

class ReaderOnlyConst < T::Struct
  include Reader

  const :foo, Integer, override: true
end

class ReaderOnlyBad < T::Struct
  include Reader

  prop :foo, Integer, override: true # error: Method `ReaderOnlyBad#foo=` is marked `override` but does not override anything
end

class OverrideNothing < T::Struct
  prop :foo, Integer, override: true
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `OverrideNothing#foo` is marked `override` but does not override anything
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `OverrideNothing#foo=` is marked `override` but does not override anything
end

class OverrideAllowIncompatible < T::Struct
  include Reader

  prop :foo, String, override: {reader: {allow_incompatible: true}}
end

class OverridePrivateGood < T::Struct
  include Writer

  prop :foo, Integer, override: {writer: {allow_incompatible: :visibility}}

  private :foo=
end

class OverridePrivateBad < T::Struct
  include Writer

  prop :foo, Integer, override: {writer: true} # error: Method `foo=` is private in `OverridePrivateBad` but not in `Writer`

  private :foo=
end

class OverrideIncompatibleTrue < T::Struct
  include Reader
  include Writer

  prop :foo, String, override: {allow_incompatible: true}
end
