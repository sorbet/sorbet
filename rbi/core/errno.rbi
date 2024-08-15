# typed: __STDLIB_INTERNAL

# Ruby exception objects are subclasses of
# [`Exception`](https://docs.ruby-lang.org/en/2.7.0/Exception.html). However,
# operating systems typically report errors using plain integers.
# [`Module`](https://docs.ruby-lang.org/en/2.7.0/Module.html)
# [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html) is created
# dynamically to map these operating system errors to Ruby classes, with each
# error number generating its own subclass of
# [`SystemCallError`](https://docs.ruby-lang.org/en/2.7.0/SystemCallError.html).
# As the subclass is created in module
# [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html), its name will start
# `Errno::`.
#
# The names of the `Errno::` classes depend on the environment in which Ruby
# runs. On a typical Unix or Windows platform, there are
# [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html) classes such as
# Errno::EACCES, Errno::EAGAIN, Errno::EINTR, and so on.
#
# The integer operating system error number corresponding to a particular error
# is available as the class constant `Errno::`*error*`::Errno`.
#
# ```ruby
# Errno::EACCES::Errno   #=> 13
# Errno::EAGAIN::Errno   #=> 11
# Errno::EINTR::Errno    #=> 4
# ```
#
# The full list of operating system errors on your particular platform are
# available as the constants of
# [`Errno`](https://docs.ruby-lang.org/en/2.7.0/Errno.html).
#
# ```ruby
# Errno.constants   #=> :E2BIG, :EACCES, :EADDRINUSE, :EADDRNOTAVAIL, ...
# ```
#
# System call error module used by webrick for cross platform compatibility.
#
# EPROTO
# :   protocol error
# ECONNRESET
# :   remote host reset the connection request
# ECONNABORTED
# :   Client sent TCP reset (RST) before server has accepted the connection
#     requested by client.
module Errno
end

class Errno::E2BIG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EACCES < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EADDRINUSE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EADDRNOTAVAIL < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EADV < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EAFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EAGAIN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EWOULDBLOCK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EALREADY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADF < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADFD < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADMSG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADRQC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBADSLT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBFONT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EBUSY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ECANCELED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ECHILD < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ECHRNG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ECOMM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

# Client sent TCP reset (RST) before server has accepted the connection
# requested by client.
class Errno::ECONNABORTED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ECONNREFUSED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

# Remote host reset the connection request.
class Errno::ECONNRESET < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EDEADLK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EDESTADDRREQ < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EDOM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EDOTDOT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EDQUOT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EEXIST < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EFAULT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EFBIG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EHOSTDOWN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EHOSTUNREACH < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EHWPOISON < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EIDRM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EILSEQ < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EINPROGRESS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EINTR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EINVAL < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EIO < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EISCONN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EISDIR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EISNAM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EKEYEXPIRED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EKEYREJECTED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EKEYREVOKED < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EL2HLT < SystemCallError
  Errno = T.let(nil, Integer)

 sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EL2NSYNC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EL3HLT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EL3RST < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELIBACC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELIBBAD < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELIBEXEC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELIBMAX < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELIBSCN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELNRNG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ELOOP < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EMEDIUMTYPE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EMFILE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EMLINK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EMSGSIZE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EMULTIHOP < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENAMETOOLONG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENAVAIL < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENETDOWN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENETRESET < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENETUNREACH < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENFILE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOANO < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOBUFS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOCSI < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENODATA < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENODEV < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOENT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOEXEC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOKEY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOLCK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOLINK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOMEDIUM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOMEM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOMSG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENONET < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOPKG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOPROTOOPT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOSPC < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOSR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOSTR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOSYS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTBLK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTCONN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTDIR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTEMPTY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTNAM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTRECOVERABLE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTSOCK < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTTY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENOTUNIQ < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ENXIO < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EOPNOTSUPP < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EOVERFLOW < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EOWNERDEAD < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EPERM < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EPFNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EPIPE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

# Protocol error.
class Errno::EPROTO < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EPROTONOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EPROTOTYPE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ERANGE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EREMCHG < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EREMOTE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EREMOTEIO < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ERESTART < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ERFKILL < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EROFS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESHUTDOWN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESOCKTNOSUPPORT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESPIPE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESRCH < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESRMNT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESTALE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ESTRPIPE < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ETIME < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ETIMEDOUT < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ETOOMANYREFS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::ETXTBSY < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EUCLEAN < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EUNATCH < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EUSERS < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EXDEV < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::EXFULL < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end

class Errno::NOERROR < SystemCallError
  Errno = T.let(nil, Integer)

  sig { params(message: T.nilable(String), func: T.nilable(Object)).returns(T.attached_class) }
  def self.new(message = nil, func = nil); end

  sig { returns(Integer) }
  def errno; end
end
