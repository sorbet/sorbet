# A much lighter stripped down version of core/ and stdlib/
#
# This is used in our test suit as well as for fast quick checks.
# Don't rely on this working anywhere of importance as we will remove and add
# things at our leisure.

class Chalk::ODM::Document
end
class SystemCallError < StandardError
end
class Data < Object
end
class Binding < Object
end
class LoadError < ScriptError
end
class RuntimeError < StandardError
end
class ScriptError < Exception
end
class RubyTyper::StubClass
end
class RubyTyper::ImplicitModuleSuperclass < BasicObject
end
module Kernel
  sig do
    params(
      predicate: BasicObject,
      msg: T.nilable(String),
      opts: T.untyped,
    )
    .returns(NilClass)
  end
  def hard_assert(predicate, msg=nil, **opts); end
end
class Chalk::ODM::Mutator::Private::HashMutator
  extend T::Generic
  K = type_member
  V = type_member
  Sorbet.sig {params(key: K, value: V).void}
  def []=(key, value)
  end
end
class Chalk::ODM::Mutator::Private::ArrayMutator
  extend T::Generic
  Elem = type_member
  Sorbet.sig {params(value: Elem).void}
  def <<(value)
  end
  Sorbet.sig {params(key: Integer, value: Elem).void}
  def []=(key, value)
  end
end
class Opus::DB::Model::Mixins::Encryptable::EncryptedValue < Chalk::ODM::Document
  Sorbet.sig {params(options: Hash).returns(Opus::DB::Model::Mixins::Encryptable::EncryptedValue)}
  def initialize(options)
  end
end
class Configatron
  class Store < BasicObject
    sig {params(name: T.any(Symbol, String)).returns(T::Boolean)}
    def key?(name)
    end
  end
  class RootStore < BasicObject
    sig {params(name: T.any(Symbol, String)).returns(T::Boolean)}
    def key?(name)
    end
  end
end
module Opus::DB::Model; end
::M = Opus::DB::Model
class Struct
  sig do
    params(
        arg0: T.any(Symbol, String),
        arg1: T.any(Symbol, String),
    )
    .returns(T.class_of(RubyTyper::DynamicStruct))
  end
  def self.new(arg0, *arg1); end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end
end
module RubyTyper
  sig do
    params(
        expr: T.untyped,
    )
    .void
  end
  def self.keep_for_ide(expr)
  end
  sig do
    params(
        expr: T.untyped,
    )
    .void
  end
  def self.keep_for_typechecking(expr)
  end
  sig do
    type_parameters(:U, :V).params(
      arg0: T::Enumerable[[T.type_parameter(:U), T.type_parameter(:V)]],
    )
    .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def self.enumerable_to_h(*arg0); end
end
class RubyTyper::DynamicStruct < Struct
  Elem = type_member(:out, fixed: T.untyped)
  sig do
    params(
        args: BasicObject,
    )
    .returns(RubyTyper::DynamicStruct)
  end
  def self.new(*args); end
end
class RubyTyper::Tuple < Array
  extend T::Generic
  Elem = type_member(:out)
  def [](*args); end
  def last(*args); end
  def first(*args); end
  def min(*args); end
  def to_a; end
  def concat(*arrays); end
end
class RubyTyper::Shape < Hash
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)
end
module RubyTyper::Void
end
class RubyTyper::ReturnTypeInference
    extend T::Sig
end
class Tempfile < File
  extend T::Sig
  extend T::Generic
  Elem = type_member(:out, fixed: String)
end
::RubyTyper::IOLike = T.type_alias(
  T.any(
    File,
    IO,
    StringIO,
    Tempfile
  )
)
class Sorbet
  sig {params(blk: T.proc.bind(Sorbet::Private::Builder).void).void}
  def self.sig(&blk)
  end
end
class Sorbet::Private::Builder
  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def type_parameters(*params); end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def generated; end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def abstract; end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def implementation; end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def override; end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def overridable; end
  Sorbet.sig {params(type: T.untyped).returns(Sorbet::Private::Builder)}
  def bind(type); end
  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def params(**params); end
  Sorbet.sig {params(type: T.untyped).returns(Sorbet::Private::Builder)}
  def returns(type); end
  Sorbet.sig {returns(Sorbet::Private::Builder)}
  def void; end
  Sorbet.sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def soft(**params); end
  Sorbet.sig {params(arg: T.untyped).returns(Sorbet::Private::Builder)}
  def checked(arg); end
end
module T::Sig
  sig {params(blk: T.proc.bind(Sorbet::Private::Builder).void).void}
  def sig(&blk); end
end
module T
  extend T::Sig
  sig {params(value: T.untyped).returns(T.untyped)}
  def self.unsafe(value); end
  sig do
    params(
      obj: T.untyped,
      type: Class
    ).returns(T.untyped)
  end
  def self.dynamic_cast(obj, type); end
  sig {params(value: T.untyped, type: T.untyped, checked: T.any(FalseClass, TrueClass)).returns(BasicObject)}
  def self.let(value, type, checked: true); end
  sig {params(value: T.untyped, type: T.untyped, checked: T.any(FalseClass, TrueClass)).returns(BasicObject)}
  def self.assert_type!(value, type, checked: true); end
  sig {params(value: T.untyped, type: T.untyped, checked: T.any(FalseClass, TrueClass)).returns(BasicObject)}
  def self.cast(value, type, checked: true); end
  sig {params(type: T.untyped).returns(BasicObject)}
  def self.nilable(type); end
  def self.proc; end
  def self.class_of(klass); end
  def self.noreturn; end
  def self.enum(values); end
  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.any(type_a, type_b, *types); end
  sig {params(type_a: T.untyped, type_b: T.untyped, types: T.untyped).returns(BasicObject)}
  def self.all(type_a, type_b, *types); end
  def self.reveal_type(value); end
  def self.type_parameter(name); end
  def self.self_type; end
  def self.type_alias(type); end
  sig {params(arg: T.untyped, msg: T.nilable(String)).returns(T.untyped)}
  def self.must(arg, msg=nil); end
  def self.coerce(type); end
end
module T::Generic
  include T::Helpers
  sig {params(params: T.untyped).returns(Sorbet::Private::Builder)}
  def type_parameters(*params); end
  def type_member(variance=:invariant, fixed: nil); end
  def type_template(variance=:invariant, fixed: nil); end
  def [](*types); end
end
module T::Helpers
end
module T::Array
  def self.[](type); end
end
module T::Hash
  def self.[](keys, values); end
end
module T::Set
  def self.[](type); end
end
module T::Range
  def self.[](type); end
end
module T::Enumerable
  def self.[](type); end
end
T::Boolean = T.type_alias(T.any(TrueClass, FalseClass))
class CSV::FieldInfo < Struct
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end
class CSV::Table < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
end
module Comparable
end
module DidYouMean::Correctable
end
module DidYouMean::Jaro
end
module DidYouMean::Levenshtein
end
module DidYouMean::NameErrorCheckers
end
class Enumerator::Generator < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
end
class Enumerator::Lazy < Enumerator
  extend T::Generic
  Elem = type_member(:out)
end
module Errno
end
class Errno::E2BIG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EACCES < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EADDRINUSE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EADDRNOTAVAIL < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EADV < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EAFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EAGAIN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EWOULDBLOCK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EALREADY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADF < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADFD < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADMSG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADRQC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBADSLT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBFONT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EBUSY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECANCELED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECHILD < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECHRNG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECOMM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECONNABORTED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECONNREFUSED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ECONNRESET < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EDEADLK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EDESTADDRREQ < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EDOM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EDOTDOT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EDQUOT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EEXIST < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EFAULT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EFBIG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EHOSTDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EHOSTUNREACH < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EHWPOISON < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EIDRM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EILSEQ < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EINPROGRESS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EINTR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EINVAL < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EIO < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EISCONN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EISDIR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EISNAM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EKEYEXPIRED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EKEYREJECTED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EKEYREVOKED < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EL2HLT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EL2NSYNC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EL3HLT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EL3RST < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELIBACC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELIBBAD < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELIBEXEC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELIBMAX < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELIBSCN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELNRNG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ELOOP < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EMEDIUMTYPE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EMFILE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EMLINK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EMSGSIZE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EMULTIHOP < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENAMETOOLONG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENAVAIL < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENETDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENETRESET < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENETUNREACH < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENFILE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOANO < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOBUFS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOCSI < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENODATA < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENODEV < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOENT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOEXEC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOKEY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOLCK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOLINK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOMEDIUM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOMEM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOMSG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENONET < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOPKG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOPROTOOPT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOSPC < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOSR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOSTR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOSYS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTBLK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTCONN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTDIR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTEMPTY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTNAM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTRECOVERABLE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTSOCK < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTTY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENOTUNIQ < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ENXIO < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EOPNOTSUPP < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EOVERFLOW < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EOWNERDEAD < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPERM < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPROTO < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPROTONOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EPROTOTYPE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ERANGE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EREMCHG < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EREMOTE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EREMOTEIO < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ERESTART < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ERFKILL < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EROFS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESHUTDOWN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESOCKTNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESRCH < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESRMNT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESTALE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ESTRPIPE < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ETIME < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ETIMEDOUT < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ETOOMANYREFS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::ETXTBSY < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EUCLEAN < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EUNATCH < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EUSERS < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EXDEV < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::EXFULL < SystemCallError
  Errno = T.let(nil, Integer)
end
class Errno::NOERROR < SystemCallError
  Errno = T.let(nil, Integer)
end
class Etc::Group < Struct
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end
class Etc::Passwd < Struct
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end
module FileTest
end
module FileUtils::LowMethods
end
module FileUtils::StreamUtils_
end
module Gem::Deprecate
end
class Gem::List < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
end
class IO::EAGAINWaitReadable < Errno::EAGAIN
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end
class IO::EAGAINWaitWritable < Errno::EAGAIN
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end
class IO::EINPROGRESSWaitReadable < Errno::EINPROGRESS
  include IO::WaitReadable
  Errno = T.let(nil, Integer)
end
class IO::EINPROGRESSWaitWritable < Errno::EINPROGRESS
  include IO::WaitWritable
  Errno = T.let(nil, Integer)
end
module IO::WaitReadable
end
module IO::WaitWritable
end
class Monitor < Object
  include MonitorMixin
end
module MonitorMixin
end
class Object < BasicObject
  include Kernel
end
class ObjectSpace::WeakMap < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
end
class Process::Tms < Struct
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end
module SingleForwardable
end
class SortedSet < Set
  extend T::Generic
  Elem = type_member(:out)
end
class StringIO < Data
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: String)
end
class Struct < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: T.untyped)
end
module URI::Escape
end
class URI::RFC2396_Parser < Object
  include URI::RFC2396_REGEXP
end
module URI::RFC2396_REGEXP
end
module URI::Util
end
module Warning
end
ARGF = T.let(T.unsafe(nil), Object)
ARGV = T.let(T.unsafe(nil), Array)
CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
FALSE = T.let(T.unsafe(nil), FalseClass)
NIL = T.let(T.unsafe(nil), NilClass)
RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
RUBY_ENGINE = T.let(T.unsafe(nil), String)
RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
RUBY_PLATFORM = T.let(T.unsafe(nil), String)
RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
RUBY_REVISION = T.let(T.unsafe(nil), Integer)
RUBY_VERSION = T.let(T.unsafe(nil), String)
STDERR = T.let(T.unsafe(nil), IO)
STDIN = T.let(T.unsafe(nil), IO)
STDOUT = T.let(T.unsafe(nil), IO)
TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
TRUE = T.let(T.unsafe(nil), TrueClass)
class Array < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def self.[](*arg0); end
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def &(arg0); end
  sig do
    params(
        arg0: T::Enumerable[Elem],
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def +(arg0); end
  sig do
    params(
        arg0: T::Array[T.untyped],
    )
    .returns(T::Array[Elem])
  end
  def -(arg0); end
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def <<(arg0); end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def [](arg0, arg1=T.unsafe(nil)); end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
        arg2: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: T::Range[Integer],
        arg1: Elem,
    )
    .returns(Elem)
  end
  def []=(arg0, arg1, arg2=T.unsafe(nil)); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  def at(arg0); end
  sig do
    type_parameters(:T).params(
        arrays: T::Array[T.type_parameter(:T)],
    )
    .returns(T::Array[T.any(Elem, T.type_parameter(:T))])
  end
  def concat(arrays); end
  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  def each(&blk); end
  sig {returns(T::Boolean)}
  def empty?(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .returns(Elem)
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Integer).returns(Elem),
    )
    .returns(Elem)
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(arg0=T.unsafe(nil)); end
  sig {params(depth: Integer).returns(T::Array[T.untyped])}
  def flatten(depth = -1); end
  sig do
    type_parameters(:U).params(
        arg0: T.type_parameter(:U),
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end
  sig {returns(Object)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Integer,
        arg1: Elem,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil), arg1=T.unsafe(nil)); end
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(arg0=T.unsafe(nil)); end
  sig {returns(Integer)}
  def length(); end
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def map(&blk); end
  sig do
    params(
        arg0: Elem,
    )
    .returns(T::Array[Elem])
  end
  def push(*arg0); end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def select(&blk); end
  sig {returns(T::Array[Elem])}
  def to_a(); end
  sig do
    params(
        arg0: T::Array[Elem],
    )
    .returns(T::Array[Elem])
  end
  def |(arg0); end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  sig do
    params(
        arg0: T.any(Integer, Float),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end
  Sorbet.sig {returns(T.any(Elem, Integer))}
  Sorbet.sig do
    type_parameters(:T).params(
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:T))
    ).returns(T.any(Integer, T.type_parameter(:T)))
  end
  Sorbet.sig do
    type_parameters(:T)
      .params(arg0: T.type_parameter(:T))
      .returns(T.any(Elem, T.type_parameter(:T)))
  end
  Sorbet.sig do
    type_parameters(:U).params(
      arg0: T.type_parameter(:U),
      blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U))
    ).returns(T.type_parameter(:U))
  end
  def sum(arg0=T.unsafe(0), &blk); end
  sig {returns(String)}
  def to_s(); end
end
class BasicObject
  sig {returns(T::Boolean)}
  def !(); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def !=(other); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end
end
class BigDecimal < Numeric
  BASE = T.let(T.unsafe(nil), Integer)
  EXCEPTION_ALL = T.let(T.unsafe(nil), Integer)
  EXCEPTION_INFINITY = T.let(T.unsafe(nil), Integer)
  EXCEPTION_OVERFLOW = T.let(T.unsafe(nil), Integer)
  EXCEPTION_UNDERFLOW = T.let(T.unsafe(nil), Integer)
  EXCEPTION_ZERODIVIDE = T.let(T.unsafe(nil), Integer)
  INFINITY = T.let(T.unsafe(nil), BigDecimal)
  NAN = T.let(T.unsafe(nil), BigDecimal)
  ROUND_CEILING = T.let(T.unsafe(nil), Integer)
  ROUND_DOWN = T.let(T.unsafe(nil), Integer)
  ROUND_FLOOR = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_DOWN = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_EVEN = T.let(T.unsafe(nil), Integer)
  ROUND_HALF_UP = T.let(T.unsafe(nil), Integer)
  ROUND_MODE = T.let(T.unsafe(nil), Integer)
  ROUND_UP = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_FINITE = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  SIGN_NEGATIVE_ZERO = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_FINITE = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_INFINITE = T.let(T.unsafe(nil), Integer)
  SIGN_POSITIVE_ZERO = T.let(T.unsafe(nil), Integer)
  sig do
    params(
      initial: T.any(Integer, Float, Rational, BigDecimal, String),
      digits: Integer,
    )
    .void
  end
  def initialize(initial, digits=0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end
  sig {returns(BigDecimal)}
  def -@(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([BigDecimal, BigDecimal])
  end
  def coerce(arg0); end
  sig {returns(T::Boolean)}
  def nan?(); end
  sig {returns(Integer)}
  sig do
    params(
        n: Integer,
        mode: T.any(Integer, Symbol),
    )
    .returns(BigDecimal)
  end
  def round(n=0, mode=T.unsafe(nil)); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {returns(String)}
  def to_s(); end
end
module BigMath
end
class Class < Module
  sig {returns(T.untyped)}
  def allocate(); end
  sig {params(args: T.untyped).returns(T.untyped)}
  def new(*args); end
  sig {returns(T.nilable(String))}
  def name(); end
  sig {returns(T.nilable(Class))}
  sig {returns(Class)}
  def superclass(); end
  sig {void}
  sig do
    params(
        superclass: Class,
    )
    .void
  end
  sig do
    params(
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  sig do
    params(
        superclass: Class,
        blk: T.proc.params(arg0: Class).returns(BasicObject),
    )
    .void
  end
  def initialize(superclass=_, &blk); end
end
class Complex < Numeric
  I = T.let(T.unsafe(nil), Complex)
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Complex)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end
  sig {returns(Complex)}
  def -@(); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Complex, Complex])
  end
  def coerce(arg0); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {returns(String)}
  def to_s(); end
end
module Enumerable
  extend T::Generic
  Elem = type_member(:out)
  abstract!
  sig do
    abstract.
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        n: Integer,
    )
    .returns(T.nilable(T::Array[Elem]))
  end
  def first(n=T.unsafe(nil)); end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end
  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T::Array[Elem])
  end
  def max_by(arg0=T.unsafe(nil), &blk); end
  sig {returns(T.nilable(Elem))}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(arg0=T.unsafe(nil), &blk); end
  sig {returns(Enumerator[Elem])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T.nilable(Elem))
  end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Enumerator[Elem])
  end
  sig do
    params(
        arg0: Integer,
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(T::Array[Elem])
  end
  def min_by(arg0=T.unsafe(nil), &blk); end
  sig {returns([T.nilable(Elem), T.nilable(Elem)])}
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(Comparable),
    )
    .returns(Enumerator[Elem])
  end
  def minmax_by(&blk); end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns([T::Array[Elem], T::Array[Elem]])
  end
  sig {returns(Enumerator[Elem])}
  def partition(&blk); end
  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  sig {returns(Enumerator[Elem])}
  def flat_map(&blk); end
  sig do
    type_parameters(:U).params(
        blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  sig {returns(Enumerator[Elem])}
  def map(&blk); end
  sig do
    type_parameters(:Any).params(
        initial: T.type_parameter(:Any),
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        arg0: Symbol,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        initial: Elem,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(Elem)
  end
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Elem),
    )
    .returns(T.nilable(Elem))
  end
  def reduce(initial=T.unsafe(nil), arg0=T.unsafe(nil), &blk); end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T::Array[Elem])
  end
  sig {returns(Enumerator[Elem])}
  def select(&blk); end
  sig {returns(T::Array[Elem])}
  def to_a(); end
end
class Enumerator < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.untyped)
  end
  sig {returns(T.self_type)}
  def each(&blk); end
  sig do
    type_parameters(:U).params(
        arg0: Integer,
        blk: T.proc.params(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
    )
    .returns(Object)
  end
  sig do
    type_parameters(:U).params(
        arg0: Proc,
        blk: T.proc.params(arg0: T::Array[T.type_parameter(:U)]).returns(BasicObject),
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil), &blk); end
end
class Exception < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: String,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil)); end
  sig {returns(String)}
  def to_s(); end
end
class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def |(obj)
  end
end
class File < IO
  ALT_SEPARATOR = T.let(T.unsafe(nil), NilClass)
  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  PATH_SEPARATOR = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  SEEK_END = T.let(T.unsafe(nil), Integer)
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SEPARATOR = T.let(T.unsafe(nil), String)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)
  extend T::Generic
  Elem = type_member(:out, fixed: String)
  sig do
    params(
        file: String,
        mode: String,
        perm: String,
        opt: Integer,
    )
    .void
  end
  def initialize(file, mode=T.unsafe(nil), perm=T.unsafe(nil), opt=T.unsafe(nil)); end
end
module File::Constants
  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)
end
class File::Stat < Object
  include Comparable
  sig do
    params(
        file: String,
    )
    .returns(Object)
  end
  def initialize(file); end
end
class Float < Numeric
  DIG = T.let(T.unsafe(nil), Integer)
  EPSILON = T.let(T.unsafe(nil), Float)
  INFINITY = T.let(T.unsafe(nil), Float)
  MANT_DIG = T.let(T.unsafe(nil), Integer)
  MAX = T.let(T.unsafe(nil), Float)
  MAX_10_EXP = T.let(T.unsafe(nil), Integer)
  MAX_EXP = T.let(T.unsafe(nil), Integer)
  MIN = T.let(T.unsafe(nil), Float)
  MIN_10_EXP = T.let(T.unsafe(nil), Integer)
  MIN_EXP = T.let(T.unsafe(nil), Integer)
  NAN = T.let(T.unsafe(nil), Float)
  RADIX = T.let(T.unsafe(nil), Integer)
  ROUNDS = T.let(T.unsafe(nil), Integer)
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end
  sig {returns(Float)}
  def -@(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        arg0: T.any(Integer, Float, Rational, BigDecimal),
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Float, Float])
  end
  def coerce(arg0); end
  sig {returns(T::Boolean)}
  def nan?(); end
  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T.any(Integer, Float))
  end
  def round(arg0=T.unsafe(nil)); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {params(base: Integer).returns(String)}
  def to_s(base=10); end
end
class Hash < Object
  include Enumerable
  extend T::Generic
  K = type_member(:out)
  V = type_member(:out)
  Elem = type_member(:out)
  sig do
    type_parameters(:U, :V).params(
      arg0: T::Array[[T.type_parameter(:U), T.type_parameter(:V)]],
    )
    .returns(T::Hash[T.type_parameter(:U), T.type_parameter(:V)])
  end
  def self.[](*arg0); end
  sig do
    params(
        arg0: K,
    )
    .returns(T.nilable(V))
  end
  def [](arg0); end
  sig do
    params(
        arg0: K,
        arg1: V,
    )
    .returns(V)
  end
  def []=(arg0, arg1); end
  sig do
    params(
        blk: T.proc.params(arg0: [K, V]).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  sig {returns(Enumerator[[K, V]])}
  def each(&blk); end
  sig {returns(T::Boolean)}
  def empty?(); end
  sig do
    params(
        arg0: K,
    )
    .returns(V)
  end
  sig do
   type_parameters(:X).params(
      arg0: K,
      arg1: T.type_parameter(:X),
    )
    .returns(T.any(V, T.type_parameter(:X)))
  end
  sig do
   type_parameters(:X).params(
        arg0: K,
        blk: T.proc.params(arg0: K).returns(T.type_parameter(:X)),
    )
    .returns(T.any(V, T.type_parameter(:X)))
  end
  def fetch(arg0, arg1=T.unsafe(nil), &blk); end
  sig {returns(Hash)}
  sig do
    params(
        default: Object,
    )
    .returns(Hash)
  end
  sig do
    params(
        blk: T.proc.params(hash: Hash, key: Object).returns(Object)
    )
    .void
  end
  def initialize(default=T.unsafe(nil), &blk); end
  sig do
    params(
        arg0: K,
    )
    .returns(T::Boolean)
  end
  def key?(arg0); end
  sig {returns(Integer)}
  def length(); end
  sig do
    params(
        blk: T.proc.params(arg0: K, arg1: V).returns(BasicObject),
    )
    .returns(T::Hash[K, V])
  end
  def select(&blk); end
  sig {returns(T::Array[[K, V]])}
  def to_a(); end
  sig {returns(String)}
  def to_s(); end
end
class Integer < Numeric
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def &(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end
  sig {returns(Integer)}
  def -@(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def <<(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(Integer)
  end
  def [](arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def ^(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([T.any(Integer, Float, Rational, BigDecimal), T.any(Integer, Float, Rational, BigDecimal)])
  end
  def coerce(arg0); end
  sig {returns(Integer)}
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def round(arg0=T.unsafe(nil)); end
  sig do
    params(
        blk: T.proc.params(arg0: Integer).returns(BasicObject),
    )
    .returns(Integer)
  end
  sig {returns(Enumerator[Integer])}
  def times(&blk); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {params(base: Integer).returns(String)}
  def to_s(base=10); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def |(arg0); end
end
class IO < Object
  include File::Constants
  include Enumerable
  extend T::Generic
  Elem = type_member(:out, fixed: String)
  APPEND = T.let(T.unsafe(nil), Integer)
  BINARY = T.let(T.unsafe(nil), Integer)
  CREAT = T.let(T.unsafe(nil), Integer)
  DIRECT = T.let(T.unsafe(nil), Integer)
  DSYNC = T.let(T.unsafe(nil), Integer)
  EXCL = T.let(T.unsafe(nil), Integer)
  FNM_CASEFOLD = T.let(T.unsafe(nil), Integer)
  FNM_DOTMATCH = T.let(T.unsafe(nil), Integer)
  FNM_EXTGLOB = T.let(T.unsafe(nil), Integer)
  FNM_NOESCAPE = T.let(T.unsafe(nil), Integer)
  FNM_PATHNAME = T.let(T.unsafe(nil), Integer)
  FNM_SHORTNAME = T.let(T.unsafe(nil), Integer)
  FNM_SYSCASE = T.let(T.unsafe(nil), Integer)
  LOCK_EX = T.let(T.unsafe(nil), Integer)
  LOCK_NB = T.let(T.unsafe(nil), Integer)
  LOCK_SH = T.let(T.unsafe(nil), Integer)
  LOCK_UN = T.let(T.unsafe(nil), Integer)
  NOATIME = T.let(T.unsafe(nil), Integer)
  NOCTTY = T.let(T.unsafe(nil), Integer)
  NOFOLLOW = T.let(T.unsafe(nil), Integer)
  NONBLOCK = T.let(T.unsafe(nil), Integer)
  NULL = T.let(T.unsafe(nil), String)
  RDONLY = T.let(T.unsafe(nil), Integer)
  RDWR = T.let(T.unsafe(nil), Integer)
  RSYNC = T.let(T.unsafe(nil), Integer)
  SEEK_CUR = T.let(T.unsafe(nil), Integer)
  SEEK_DATA = T.let(T.unsafe(nil), Integer)
  SEEK_END = T.let(T.unsafe(nil), Integer)
  SEEK_HOLE = T.let(T.unsafe(nil), Integer)
  SEEK_SET = T.let(T.unsafe(nil), Integer)
  SHARE_DELETE = T.let(T.unsafe(nil), Integer)
  SYNC = T.let(T.unsafe(nil), Integer)
  TMPFILE = T.let(T.unsafe(nil), Integer)
  TRUNC = T.let(T.unsafe(nil), Integer)
  WRONLY = T.let(T.unsafe(nil), Integer)
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.self_type)
  end
  def <<(arg0); end
  sig do
    params(
        sep: String,
        limit: Integer,
        blk: T.proc.params(arg0: String).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig do
    params(
        sep: String,
        limit: Integer,
    )
    .returns(Enumerator[String])
  end
  def each(sep=T.unsafe(nil), limit=T.unsafe(nil), &blk); end
  sig {returns(T.nilable(Integer))}
  def getbyte(); end
  sig do
    params(
        fd: Integer,
        mode: Integer,
        opt: Integer,
    )
    .void
  end
  def initialize(fd, mode=T.unsafe(nil), opt=T.unsafe(nil)); end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end
  sig do
    params(
        read_array: T::Array[IO],
        write_array: T::Array[IO],
        error_array: T::Array[IO],
        timeout: Integer,
    )
    .returns(T.nilable(T::Array[IO]))
  end
  def self.select(read_array, write_array=T.unsafe(nil), error_array=T.unsafe(nil), timeout=T.unsafe(nil)); end
  sig {returns(Integer)}
  def to_i(); end
end
module Kernel
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
  sig do
    params(
        start_or_range: Integer,
        length: Integer,
    )
    .returns(T.nilable(T::Array[String]))
  end
  sig do
    params(
        start_or_range: T::Range[Integer],
    )
    .returns(T.nilable(T::Array[String]))
  end
  sig {params.returns(T::Array[String])}
  def caller(start_or_range=T.unsafe(nil), length=T.unsafe(nil)); end
  sig do
    params(
        tag: Object,
        blk: T.proc.params(arg0: Object).returns(BasicObject),
    )
    .returns(BasicObject)
  end
  def catch(tag=Object.new, &blk); end
  sig do
    params(
        symbol: T.any(Symbol, String),
        method: T.any(Proc, Method, UnboundMethod)
    )
    .returns(Symbol)
  end
  sig do
    params(
        symbol: T.any(Symbol, String),
        blk: BasicObject
    )
    .returns(Symbol)
  end
  def define_singleton_method(symbol, method=T.unsafe(nil), &blk); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end
  sig {returns(T.self_type)}
  def clone(); end
  sig {returns(T.self_type)}
  def freeze(); end
  sig do
    params(
        arg0: T.any(Class, Module),
    )
    .returns(T::Boolean)
  end
  def is_a?(arg0); end
  sig {returns(T::Boolean)}
  def nil?(); end
  sig {returns(String)}
  def to_s(); end
  sig do
    params(
        initial: T.any(Integer, Float, Rational, BigDecimal, String),
        digits: Integer,
    )
    .returns(BigDecimal)
  end
  def BigDecimal(initial, digits=0); end
  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
    )
    .returns(Complex)
  end
  sig do
    params(
        x: String,
    )
    .returns(Complex)
  end
  def Complex(x, y=T.unsafe(nil)); end
  sig do
    params(
        arg: T.any(Numeric, String),
        base: Integer,
    )
    .returns(Integer)
  end
  def Integer(arg, base=T.unsafe(nil)); end
  sig do
    params(
        x: T.any(Numeric, String),
        y: T.any(Numeric, String),
    )
    .returns(Rational)
  end
  sig do
    params(
        x: Object,
    )
    .returns(Rational)
  end
  def Rational(x, y=T.unsafe(nil)); end
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def `(arg0); end
  sig {returns(T::Boolean)}
  def block_given?(); end
  sig {params(blk: T.proc.params.returns(T.untyped)).returns(T.untyped)}
  sig {returns(Enumerator[T.untyped])}
  def loop(&blk); end
  sig do
    params(
        name: String,
        rest: T.any(String, Integer),
        block: String,
    )
    .returns(T.nilable(IO))
  end
  def open(name, rest=T.unsafe(nil), block=T.unsafe(nil)); end
  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  def proc(&blk); end
  sig do
    params(
        blk: BasicObject,
    )
    .returns(Proc)
  end
  def lambda(&blk); end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(NilClass)
  end
  def puts(*arg0); end
  sig {returns(Float)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: T::Range[Integer],
    )
    .returns(Integer)
  end
  sig do
    params(
        arg0: T::Range[Float],
    )
    .returns(Float)
  end
  def rand(arg0=T.unsafe(nil)); end
  sig do
    params(
        feature: String,
    )
    .returns(T::Boolean)
  end
  def require_relative(feature); end
  sig do
    params(
        read: T::Array[IO],
        write: T::Array[IO],
        error: T::Array[IO],
        timeout: Integer,
    )
    .returns(T::Array[String])
  end
  def select(read, write=T.unsafe(nil), error=T.unsafe(nil), timeout=T.unsafe(nil)); end
  sig do
    params(
        tag: Object,
        obj: BasicObject,
    )
    .returns(T.noreturn)
  end
  def throw(tag, obj=nil); end
  sig {returns(T.noreturn)}
  sig do
    params(
        arg0: String,
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Class,
        arg1: String,
        arg2: T::Array[String],
    )
    .returns(T.noreturn)
  end
  sig do
    params(
        arg0: Exception,
        arg1: String,
        arg2: T::Array[String],
    )
    .returns(T.noreturn)
  end
  def raise(arg0=T.unsafe(nil), arg1=T.unsafe(nil), arg2=T.unsafe(nil)); end
end
class MatchData < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        i_or_start_or_range_or_name: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        i_or_start_or_range_or_name: Integer,
        length: Integer,
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T::Range[Integer],
    )
    .returns(T::Array[String])
  end
  sig do
    params(
        i_or_start_or_range_or_name: T.any(String, Symbol),
    )
    .returns(T.nilable(String))
  end
  def [](i_or_start_or_range_or_name, length=T.unsafe(nil)); end
  sig {returns(Integer)}
  def length(); end
  sig {returns(T::Array[String])}
  def to_a(); end
  sig {returns(String)}
  def to_s(); end
end
module Math
  E = T.let(T.unsafe(nil), Float)
  PI = T.let(T.unsafe(nil), Float)
end
class Method < Object
  sig {returns(Proc)}
  def to_proc; end
end
class Module < Object
  ARGF = T.let(T.unsafe(nil), Object)
  ARGV = T.let(T.unsafe(nil), Array)
  CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
  FALSE = T.let(T.unsafe(nil), FalseClass)
  NIL = T.let(T.unsafe(nil), NilClass)
  RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
  RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
  RUBY_ENGINE = T.let(T.unsafe(nil), String)
  RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
  RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
  RUBY_PLATFORM = T.let(T.unsafe(nil), String)
  RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
  RUBY_REVISION = T.let(T.unsafe(nil), Integer)
  RUBY_VERSION = T.let(T.unsafe(nil), String)
  STDERR = T.let(T.unsafe(nil), IO)
  STDIN = T.let(T.unsafe(nil), IO)
  STDOUT = T.let(T.unsafe(nil), IO)
  TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
  TRUE = T.let(T.unsafe(nil), TrueClass)
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def <(other); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end
  sig do
    params(
        other: Module,
    )
    .returns(T.nilable(T::Boolean))
  end
  def >(other); end
  sig do
    params(
        new_name: Symbol,
        old_name: Symbol,
    )
    .returns(T.self_type)
  end
  def alias_method(new_name, old_name); end
  sig {returns(T.self_type)}
  def freeze(); end
  sig do
    params(
        arg0: Module,
    )
    .returns(T.self_type)
  end
  def include(*arg0); end
  sig do
    params(
        arg0: Module,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end
  sig {returns(Object)}
  sig do
    params(
        blk: T.proc.params(arg0: Module).returns(BasicObject),
    )
    .void
  end
  def initialize(&blk); end
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def module_function(*arg0); end
  sig {returns(String)}
  def name(); end
  sig {returns(String)}
  def to_s(); end
end
class NilClass < Object
  sig do
    params(
        obj: BasicObject,
    )
    .returns(FalseClass)
  end
  def &(obj); end
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ^(obj); end
  sig {returns([])}
  def to_a(); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(T::Hash[T.untyped, T.untyped])}
  def to_h(); end
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def |(obj); end
end
class Numeric < Object
  include Comparable
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def -(arg0); end
  sig {returns(Numeric)}
  def -@(); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Numeric)
  end
  def round(arg0); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
end
class Proc < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T.untyped)
  end
  def call(*arg0); end
  sig {returns(T::Boolean)}
  def lambda(); end
  sig {returns(T.self_type)}
  def to_proc(); end
  sig {returns(String)}
  def to_s(); end
end
class Range < Object
  include Enumerable
  extend T::Generic
  Elem = type_member(:out)
  sig do
    type_parameters(:U).params(
      from: T.type_parameter(:U),
      to: T.type_parameter(:U),
      exclude_end: T::Boolean
    ).returns(T::Range[T.type_parameter(:U)])
  end
  def self.new(from, to, exclude_end=false); end
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(obj); end
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(obj); end
  sig do
    params(
        blk: T.proc.params(arg0: Elem).returns(BasicObject),
    )
    .returns(T.self_type)
  end
  sig {returns(Enumerator[Elem])}
  def each(&blk); end
  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def first(n=T.unsafe(nil)); end
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def include?(obj); end
  sig do
    params(
        _begin: Elem,
        _end: Elem,
        exclude_end: T::Boolean,
    )
    .void
  end
  def initialize(_begin, _end, exclude_end=T.unsafe(nil)); end
  sig {returns(Elem)}
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  def last(n=T.unsafe(nil)); end
  sig {returns(Elem)}
  sig do
    params(
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(Elem)
  end
  sig do
    params(
        n: Integer,
    )
    .returns(T::Array[Elem])
  end
  sig do
    params(
        n: Integer,
        blk: T.proc.params(arg0: Elem, arg1: Elem).returns(Integer),
    )
    .returns(T::Array[Elem])
  end
  def min(n=T.unsafe(nil), &blk); end
  sig {returns(String)}
  def to_s(); end
end
class Rational < Numeric
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(Rational)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(BigDecimal)
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns(Complex)
  end
  def -(arg0); end
  sig {returns(Rational)}
  def -@(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Float,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns(T::Boolean)
  end
  sig do
    params(
        arg0: BigDecimal,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Float,
    )
    .returns([Float, Float])
  end
  sig do
    params(
        arg0: Rational,
    )
    .returns([Rational, Rational])
  end
  sig do
    params(
        arg0: Complex,
    )
    .returns([Numeric, Numeric])
  end
  def coerce(arg0); end
  sig {returns(Integer)}
  sig do
    params(
        arg0: Integer,
    )
    .returns(Numeric)
  end
  def round(arg0=T.unsafe(nil)); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {returns(String)}
  def to_s(); end
end
class Regexp < Object
  EXTENDED = T.let(T.unsafe(nil), Integer)
  FIXEDENCODING = T.let(T.unsafe(nil), Integer)
  IGNORECASE = T.let(T.unsafe(nil), Integer)
  MULTILINE = T.let(T.unsafe(nil), Integer)
  NOENCODING = T.let(T.unsafe(nil), Integer)
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end
  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(other); end
  sig do
    params(
        arg0: String,
        options: BasicObject,
        kcode: String,
    )
    .returns(Object)
  end
  sig do
    params(
        arg0: Regexp,
    )
    .void
  end
  def initialize(arg0, options=T.unsafe(nil), kcode=T.unsafe(nil)); end
  sig {returns(String)}
  def to_s(); end
end
class String < Object
  include Comparable
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Object,
    )
    .returns(String)
  end
  def <<(arg0); end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ===(arg0); end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def [](arg0, arg1=T.unsafe(nil)); end
  sig do
    params(
        arg0: T.any(Integer, Object),
    )
    .returns(String)
  end
  def concat(arg0); end
  sig {returns(T::Boolean)}
  def empty?(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(T.nilable(Integer))
  end
  def getbyte(arg0); end
  sig do
    params(
        arg0: String,
    )
    .returns(T::Boolean)
  end
  def include?(arg0); end
  sig do
    params(
        str: String,
    )
    .void
  end
  def initialize(str=T.unsafe(nil)); end
  sig {returns(Symbol)}
  def intern(); end
  sig {returns(Integer)}
  def length(); end
  sig do
    params(
        arg0: T.any(Regexp, String),
    )
    .returns([String, String, String])
  end
  def partition(arg0); end
  sig do
    params(
        arg0: T.any(String, Regexp),
    )
    .returns([String, String, String])
  end
  def rpartition(arg0); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def sum(arg0=T.unsafe(nil)); end
  sig {returns(Float)}
  def to_f(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Integer)
  end
  def to_i(arg0=T.unsafe(nil)); end
  sig {returns(String)}
  def to_s(); end
  sig {returns(Symbol)}
  def to_sym(); end
  sig do
    params(
        arg0: Integer,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: T.any(T::Range[Integer], Regexp),
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: Integer,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: Regexp,
        arg1: String,
    )
    .returns(T.nilable(String))
  end
  sig do
    params(
        arg0: String,
    )
    .returns(T.nilable(String))
  end
  def slice(arg0, arg1=T.unsafe(nil)); end
end
class Symbol < Object
  include Comparable
  sig do
    params(
        obj: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(obj); end
  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def [](idx_or_range, n=T.unsafe(nil)); end
  sig {returns(T::Boolean)}
  def empty?(); end
  sig {returns(T.self_type)}
  def intern(); end
  sig {returns(Integer)}
  def length(); end
  sig {returns(Proc)}
  def to_proc(); end
  sig do
    params(
        idx_or_range: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: Integer,
        n: Integer,
    )
    .returns(String)
  end
  sig do
    params(
        idx_or_range: T::Range[Integer],
    )
    .returns(String)
  end
  def slice(idx_or_range, n=T.unsafe(nil)); end
  sig {returns(String)}
  def to_s(); end
  sig {returns(T.self_type)}
  def to_sym(); end
end
class Thread < Object
  sig {params(key: T.any(String, Symbol)).returns(T.untyped)}
  def [](key); end
  sig {params(key: T.any(String, Symbol), value: T.untyped).returns(T.untyped)}
  def []=(key, value); end
end
class Time < Object
  include Comparable
  RFC2822_DAY_NAME = T.let(T.unsafe(nil), Array)
  RFC2822_MONTH_NAME = T.let(T.unsafe(nil), Array)
  sig do
    params(
        seconds: T.any(Time, Numeric)
    )
    .returns(Time)
  end
  sig do
    params(
        seconds: Numeric,
        microseconds_with_frac: Numeric,
    )
    .returns(Time)
  end
  def self.at(seconds, microseconds_with_frac=T.unsafe(nil)); end
  sig {returns(Time)}
  def self.now(); end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def +(arg0); end
  sig do
    params(
        arg0: Time,
    )
    .returns(Float)
  end
  sig do
    params(
        arg0: Numeric,
    )
    .returns(Time)
  end
  def -(arg0); end
  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def <(arg0); end
  sig do
    params(
        arg0: Time,
    )
    .returns(T::Boolean)
  end
  def >(arg0); end
  sig do
    params(
        year: T.any(Integer, String),
        month: T.any(Integer, String),
        day: T.any(Integer, String),
        hour: T.any(Integer, String),
        min: T.any(Integer, String),
        sec: T.any(Numeric, String),
        usec_with_frac: T.any(Numeric, String),
    )
    .void
  end
  def initialize(year=T.unsafe(nil), month=T.unsafe(nil), day=T.unsafe(nil), hour=T.unsafe(nil), min=T.unsafe(nil), sec=T.unsafe(nil), usec_with_frac=T.unsafe(nil)); end
  sig {returns(Integer)}
  def min(); end
  sig do
    params(
        arg0: Integer,
    )
    .returns(Time)
  end
  def round(arg0); end
  sig {returns([Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, T::Boolean, String])}
  def to_a(); end
  sig {returns(Float)}
  def to_f(); end
  sig {returns(Integer)}
  def to_i(); end
  sig {returns(String)}
  def to_s(); end
end
class TrueClass
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj)
  end
end
class UnboundMethod
  sig {returns(Symbol)}
  def name; end
end
