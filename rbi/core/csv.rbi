# typed: true
class CSV < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T::Array[String])

  DEFAULT_OPTIONS = T.let(T.unsafe(nil), Hash)
  VERSION = T.let(T.unsafe(nil), String)

  type_parameters(:U).sig(
      path: T.any(String, File),
      options: T::Hash[Symbol, T.type_parameter(:U)],
      blk: T.proc(arg0: T::Array[String]).returns(BasicObject),
  )
  .returns(NilClass)
  def self.foreach(path, options=_, &blk); end
end

class CSV::FieldInfo < Struct
end

class CSV::MalformedCSVError < RuntimeError
end

class CSV::Row < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end

class CSV::Table < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end
