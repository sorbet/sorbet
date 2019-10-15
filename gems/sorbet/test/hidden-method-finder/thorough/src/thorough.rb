# typed: false

C1 = Class.new do
  define_method(:m_three) do; end
end

C2 = Class.new do
  define_singleton_method(:m_four) do; end
end

class C3; end
C3.const_set('X', 5)


# the following are included to avoid including these in
# hidden-definitions, as they differ between OS X and Linux

class BasicSocket < IO
  def read_nonblock(*_); end
end

Errno::EAUTH = Errno::NOERROR
Errno::EBADEXEC = Errno::NOERROR
Errno::EBADARCH = Errno::NOERROR
Errno::EBADMACHO = Errno::NOERROR
Errno::EBADRPC = Errno::NOERROR
Errno::EDEADLOCK = Errno::NOERROR
Errno::EDEVERR = Errno::NOERROR
Errno::EFTYPE = Errno::NOERROR
Errno::ELAST = Errno::NOERROR
Errno::ENEEDAUTH = Errno::NOERROR
Errno::ENOATTR = Errno::NOERROR
Errno::ENOPOLICY = Errno::NOERROR
Errno::EPROCLIM = Errno::NOERROR
Errno::EPROCUNAVAIL = Errno::NOERROR
Errno::EPROGMISMATCH = Errno::NOERROR
Errno::EPROGUNAVAIL = Errno::NOERROR
Errno::EPWROFF = Errno::NOERROR
Errno::EQFULL = Errno::NOERROR
Errno::ERPCMISMATCH = Errno::NOERROR
Errno::EPWROFF = Errno::NOERROR
Errno::EPROGUNAVAIL = Errno::NOERROR
Errno::EQFULL = Errno::ELAST
Errno::ESHLIBVERS = Errno::NOERROR

class IPSocket < BasicSocket
  def self.getaddress_orig(*_); end
end

module Process
  CLOCK_MONOTONIC_RAW_APPROX = ::T.let(nil, ::T.untyped)
  CLOCK_UPTIME_RAW = ::T.let(nil, ::T.untyped)
  CLOCK_UPTIME_RAW_APPROX = ::T.let(nil, ::T.untyped)
end
