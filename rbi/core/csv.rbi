# typed: core
class CSV < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T::Array[T.nilable(String)])

  DEFAULT_OPTIONS = T.let(T.unsafe(nil), Hash)
  VERSION = T.let(T.unsafe(nil), String)

  sig do
    type_parameters(:U).params(
        path: T.any(String, ::Sorbet::Private::Static::IOLike),
        options: T::Hash[Symbol, T.type_parameter(:U)],
        blk: T.proc.params(arg0: T::Array[T.nilable(String)]).void,
    )
    .void
  end
  def self.foreach(path, options=T.unsafe(nil), &blk); end

  sig do
    params(
        io: T.any(::Sorbet::Private::Static::IOLike, String),
        options: T::Hash[Symbol, T.untyped],
    )
    .void
  end
  def initialize(io=T.unsafe(nil), options=T.unsafe(nil)); end

  sig do
    params(
        str: String,
        options: T::Hash[Symbol, T.untyped],
        blk: T.nilable(T.proc.params(arg0: T::Array[T.nilable(String)]).void)
    )
    .returns(T.nilable(T::Array[T::Array[T.nilable(String)]]))
  end
  def self.parse(str, options=T.unsafe(nil), &blk); end

  sig do
    params(
        str: String,
        options: T::Hash[Symbol, T.untyped],
    )
    .returns(T::Array[T.nilable(String)])
  end
  def self.parse_line(str, options=T.unsafe(nil)); end

  sig {returns(T::Array[T::Array[T.nilable(String)]])}
  def read; end

  sig {returns(T.nilable(T::Array[T.nilable(String)]))}
  def readline; end

  sig do
    params(
        path: String,
        options: T::Hash[Symbol, T.untyped],
    )
    .returns(T::Array[T::Array[T.nilable(String)]])
  end
  def self.read(path, options=T.unsafe(nil)); end
end

sig do
  params(
      io: T.any(::Sorbet::Private::Static::IOLike, String),
      options: T::Hash[Symbol, T.untyped],
  )
  .returns(CSV)
end

class CSV::Row < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: T.nilable(String))
end

def CSV(io=T.unsafe(nil), options=T.unsafe(nil)); end
