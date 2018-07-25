# typed: strict

class Tempfile < File
  extend T::Helpers

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  type_parameters(:U).sig(
    basename: String,
    tmpdir: T.nilable(String),
    mode: String,
    options: T::Hash[Symbol, T.untyped],
    blk: T.proc(arg0: File).returns(T.type_parameter(:U)),
  )
  .returns(T.any(File, T.type_parameter(:U)))
  def self.create(basename="", tmpdir=nil, mode='o', options={}, &blk); end

  type_parameters(:U).sig(
    basename: String,
    tmpdir: T.nilable(String),
    mode: String,
    options: T::Hash[Symbol, T.untyped],
    blk: T.proc(arg0: Tempfile).returns(T.type_parameter(:U)),
  )
  .returns(T.any(Tempfile, T.type_parameter(:U)))
  def self.open(basename='', tmpdir=nil, mode='o', options={}, &blk); end

  type_parameter(:U).sig(
    basename: String,
    tmpdir: T.nilable(String),
    mode: String,
    options: T::Hash[Symbol, T.untyped],
  )
  .void
  def initialize(basename='', tmpdir=nil, mode='o', options={}); end

  sig(unlink_now: T.any(TrueClass, FalseClass)).void
  def close(unlink_now=false); end

  sig.void
  def close!; end

  sig.returns(T.any(TrueClass, FalseClass))
  def delete; end

  sig.returns(Integer)
  def length; end

  sig.returns(Tempfile)
  def open; end

  # path returns nil of the Tempfile has been unlinked.
  sig.returns(T.nilable(String))
  def path; end

  sig.returns(Integer)
  def size; end

  sig.returns(T.any(TrueClass, FalseClass))
  def unlink; end
end
