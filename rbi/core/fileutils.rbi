# typed: true
module FileUtils
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)

  sig do
    params(
        src: T.any(String, Pathname),
        dest: T.any(String, Pathname),
        preserve: T::Hash[Symbol, T.any(TrueClass, FalseClass)],
    )
    .returns(T::Array[String])
  end
  def self.cp_r(src, dest, preserve=T.unsafe(nil)); end

  sig do
    params(
        list: T.any(String, T::Array[String]),
        force: T.nilable(T.any(TrueClass, FalseClass)),
        noop: T.nilable(T.any(TrueClass, FalseClass)),
        verbose: T.nilable(T.any(TrueClass, FalseClass)),
        secure: T.nilable(T.any(TrueClass, FalseClass)),
    )
    .returns(T::Array[String])
  end
  def self.rm_r(list, force: nil, noop: nil, verbose: nil, secure: nil); end

  sig do
    params(
        list: T.any(String, Pathname),
        mode: T::Hash[Symbol, T.any(TrueClass, FalseClass)],
    )
    .returns(T::Array[String])
  end
  def self.mkdir_p(list, mode=T.unsafe(nil)); end
end

module FileUtils::DryRun
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end

class FileUtils::Entry_ < Object
  include FileUtils::StreamUtils_

  DIRECTORY_TERM = T.let(T.unsafe(nil), String)
  SYSCASE = T.let(T.unsafe(nil), String)
  S_IF_DOOR = T.let(T.unsafe(nil), Integer)
end

module FileUtils::NoWrite
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end

module FileUtils::Verbose
  LOW_METHODS = T.let(T.unsafe(nil), Array)
  METHODS = T.let(T.unsafe(nil), Array)
  OPT_TABLE = T.let(T.unsafe(nil), Hash)
end
