# typed: strict

module T::Private::Methods::Modes
  sig {returns(String)}
  def self.standard; end

  sig {returns(String)}
  def self.abstract; end

  sig {returns(String)}
  def self.overridable; end

  sig {returns(String)}
  def self.override; end

  sig {returns(String)}
  def self.overridable_override; end

  sig {returns(String)}
  def self.untyped; end

  MODES = T.let([], T::Array[String])
  OVERRIDABLE_MODES = T.let([], T::Array[String])
  OVERRIDE_MODES = T.let([], T::Array[String])
  NON_OVERRIDE_MODES = T.let([], T::Array[String])
end
