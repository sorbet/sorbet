# typed: true

module T::Private::RuntimeLevels
  @check_tests = T.let(false, T::Boolean)
  @wrapped_tests_with_validation = T.let(false, T::Boolean)
  @has_read_default_checked_level = T.let(false, T::Boolean)
  @default_checked_level = T.let(:always, Symbol)

  sig {returns(T::Boolean)}
  def self.check_tests?; end

  sig { void }
  def self.enable_checking_in_tests; end

  sig {returns(Symbol)}
  def self.default_checked_level; end

  sig {params(level: Symbol).returns(Symbol)}
  def self.default_checked_level=(level); end

  sig { params(checked: T::Boolean).void }
  def self._toggle_checking_tests(checked); end
end
