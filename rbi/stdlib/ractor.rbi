# typed: __STDLIB_INTERNAL

class Ractor
  sig { returns(Ractor) }
  def self.main; end

  sig { returns(Ractor) }
  def self.current; end

  sig { params(obj: T.untyped, move: T::Boolean).void }
  def self.yield(obj, move: false); end

  sig { returns(T.untyped) }
  def self.receive; end

  sig { returns(T.untyped) }
  def self.recv; end

  sig { params(blk: T.proc.params(message: T.untyped).returns(T.nilable(T::Boolean))).returns(T.untyped) }
  def self.receive_if(&blk); end

  sig { params(rest: T.untyped, blk: T.proc.params(arg: T.untyped).returns(T.untyped)).void }
  def self.initialize(**rest, &blk); end

  sig { returns(Integer) }
  def self.count; end

  sig { params(ractors: Ractor, yield_value: T.untyped, move: T::Boolean).returns([T.any(Ractor, Symbol), T.untyped]) }
  def self.select(*ractors, yield_value: nil, move: false); end

  sig { params(obj: T.untyped, copy: T::Boolean).returns(T.untyped) }
  def self.make_shareable(obj, copy: false); end

  sig { params(obj: T.untyped).returns(T::Boolean) }
  def self.shareable?(obj); end

  sig { returns(T.nilable(String)) }
  def name; end

  sig { params(key: T.any(Symbol, String)).returns(T.untyped) }
  def [](key); end

  sig { params(key: T.any(Symbol, String), value: T.untyped).returns(T.untyped) }
  def []=(key, value); end

  sig { returns(T.untyped) }
  def take; end

  sig { params(obj: T.untyped, move: T::Boolean).returns(Ractor) }
  def send(obj, move: false); end

  sig { params(obj: T.untyped, move: T::Boolean).returns(Ractor) }
  def <<(obj, move: false); end

  sig { void }
  def close_incoming; end

  sig { void }
  def close_outgoing; end
end
