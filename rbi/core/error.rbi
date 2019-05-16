# typed: core
class Exception < Object
  sig do
    params(
        arg0: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(arg0); end

  sig {returns(T::Array[String])}
  def backtrace(); end

  sig {returns(T::Array[Thread::Backtrace::Location])}
  def backtrace_locations(); end

  sig {returns(T.nilable(Exception))}
  def cause(); end

  sig do
    params(
        arg0: String,
    )
    .returns(Exception)
  end
  def exception(arg0=T.unsafe(nil)); end

  sig do
    params(
        arg0: String,
    )
    .void
  end
  def initialize(arg0=T.unsafe(nil)); end

  sig {returns(String)}
  def inspect(); end

  sig {returns(String)}
  def message(); end

  sig do
    params(
        arg0: T.any(String, T::Array[String]),
    )
    .returns(T::Array[String])
  end
  def set_backtrace(arg0); end

  sig {returns(String)}
  def to_s(); end
end

class ArgumentError < StandardError
end

class EncodingError < StandardError
end

class IndexError < StandardError
end

class Interrupt < SignalException
end

class KeyError < IndexError
end

class LoadError < ScriptError
end

class NameError < StandardError
end

class NoMemoryError < Exception
end

class NoMethodError < NameError
end

class NotImplementedError < ScriptError
end

class RangeError < StandardError
end

class RuntimeError < StandardError
end

class ScriptError < Exception
end

class SecurityError < Exception
end

class SignalException < Exception
end

class StandardError < Exception
end

class SyntaxError < ScriptError
end

class SystemCallError < StandardError
end

class SystemExit < Exception
end

class TypeError < StandardError
end

module Warning
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

