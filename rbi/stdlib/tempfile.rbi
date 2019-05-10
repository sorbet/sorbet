# typed: core

class Tempfile < File
  extend T::Sig

  extend T::Generic
  Elem = type_member(:out, fixed: String)

  sig do
    type_parameters(:U).params(
      basename: String,
      tmpdir: T.nilable(String),
      mode: String,
      options: T::Hash[Symbol, T.untyped],
      blk: T.proc.params(arg0: File).returns(T.type_parameter(:U)),
    )
    .returns(T.any(File, T.type_parameter(:U)))
  end
  sig do
    type_parameters(:U).params(
      basename: [String, String],
      tmpdir: T.nilable(String),
      mode: String,
      options: T::Hash[Symbol, T.untyped],
      blk: T.proc.params(arg0: File).returns(T.type_parameter(:U)),
    )
    .void
  end
  def self.create(basename="", tmpdir=nil, mode='o', options={}, &blk); end

  sig do
    type_parameters(:U).params(
      basename: String,
      tmpdir: T.nilable(String),
      mode: String,
      options: T::Hash[Symbol, T.untyped],
      blk: T.proc.params(arg0: Tempfile).returns(T.type_parameter(:U)),
    )
    .returns(T.any(Tempfile, T.type_parameter(:U)))
  end
  def self.open(basename='', tmpdir=nil, mode='o', options={}, &blk); end

  sig do
    type_parameters(:U).params(
      basename: String,
      tmpdir: T.nilable(String),
      mode: String,
      options: T::Hash[Symbol, T.untyped],
    )
    .void
  end
  sig do
    type_parameters(:U).params(
      basename: [String, String],
      tmpdir: T.nilable(String),
      mode: String,
      options: T::Hash[Symbol, T.untyped],
    )
    .void
  end
  def initialize(basename='', tmpdir=nil, mode='o', options={}); end

  sig {params(unlink_now: T::Boolean).void}
  def close(unlink_now=false); end

  sig {void}
  def close!; end

  sig {returns(T::Boolean)}
  def delete; end

  sig {returns(Integer)}
  def length; end

  sig {returns(Tempfile)}
  def open; end

  # path returns nil if the Tempfile has been unlinked.
  sig {returns(T.nilable(String))}
  def path; end

  sig {returns(Integer)}
  def size; end

  sig {returns(T::Boolean)}
  def unlink; end
end
