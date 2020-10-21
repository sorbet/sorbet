# This file was originally plucked from the autogenerated subprocess RBI in
# Stripe's codebase.

# typed: true

module ::Subprocess
  PIPE = ::T.let(nil, ::T.untyped)
  STDOUT = ::T.let(nil, ::T.untyped)
  VERSION = ::T.let(nil, ::T.untyped)

  def self.call(cmd, opts=T.unsafe(nil), &blk); end
  def self.check_call(cmd, opts=T.unsafe(nil), &blk); end
  def self.check_output(cmd, opts=T.unsafe(nil), &blk); end
  def self.popen(cmd, opts=T.unsafe(nil), &blk); end
  def self.status_to_s(status, convert_high_exit=T.unsafe(nil)); end
end

class ::Subprocess::CommunicateTimeout < StandardError
  def initialize(cmd, stdout, stderr); end
  def stderr(); end
  def stdout(); end
end

class ::Subprocess::NonZeroExit < StandardError
  def command(); end
  def initialize(cmd, status); end
  def status(); end
end

class ::Subprocess::Process
  def command(); end
  def communicate(input=T.unsafe(nil), timeout_s=T.unsafe(nil)); end
  def drain_fd(fd, buf=T.unsafe(nil)); end
  def initialize(cmd, opts=T.unsafe(nil), &blk); end
  def pid(); end
  def poll(); end
  def send_signal(signal); end
  def status(); end
  def stderr(); end
  def stdin(); end
  def stdout(); end
  def terminate(); end
  def wait(); end
  def self.catching_sigchld(pid); end
  def self.handle_sigchld(); end
  def self.register_pid(pid, fd); end
  def self.unregister_pid(pid); end
  def self.wakeup_sigchld(); end
end
