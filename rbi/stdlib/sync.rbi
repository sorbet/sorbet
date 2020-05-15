# typed: __STDLIB_INTERNAL

# A class that provides two-phase lock with a counter. See
# [`Sync_m`](https://docs.ruby-lang.org/en/2.6.0/Sync_m.html) for details.
class Sync
  include(::Sync_m)

  def exclusive?; end

  def lock(m = _); end

  def locked?; end

  def shared?; end

  def synchronize(mode = _); end

  def try_lock(mode = _); end

  def unlock(m = _); end
end

# A module that provides a two-phase lock with a counter.
module Sync_m
  EX = T.let(T.unsafe(nil), Symbol)

  SH = T.let(T.unsafe(nil), Symbol)

  # lock mode
  UN = T.let(T.unsafe(nil), Symbol)

  def self.new(*args); end

  def sync_ex_count; end

  def sync_ex_count=(_); end

  def sync_ex_locker; end

  def sync_ex_locker=(_); end

  def sync_exclusive?; end

  def sync_extend; end

  def sync_inspect; end

  def sync_lock(m = _); end

  # accessing
  def sync_locked?; end

  def sync_mode; end

  def sync_mode=(_); end

  def sync_sh_locker; end

  def sync_sh_locker=(_); end

  def sync_shared?; end

  def sync_synchronize(mode = _); end

  # locking methods.
  def sync_try_lock(mode = _); end

  def sync_unlock(m = _); end

  def sync_upgrade_waiting; end

  def sync_upgrade_waiting=(_); end

  def sync_waiting; end

  def sync_waiting=(_); end

  def self.append_features(cl); end

  def self.define_aliases(cl); end

  def self.extend_object(obj); end
end

# exceptions
class Sync_m::Err < ::StandardError
  def self.Fail(*opt); end
end

class Sync_m::Err::LockModeFailer < ::Sync_m::Err
  Message = T.let(T.unsafe(nil), String)

  def self.Fail(mode); end
end

class Sync_m::Err::UnknownLocker < ::Sync_m::Err
  Message = T.let(T.unsafe(nil), String)

  def self.Fail(th); end
end

# A class that provides two-phase lock with a counter. See
# [`Sync_m`](https://docs.ruby-lang.org/en/2.6.0/Sync_m.html) for details.
Synchronizer = Sync

# A module that provides a two-phase lock with a counter.
Synchronizer_m = Sync_m
