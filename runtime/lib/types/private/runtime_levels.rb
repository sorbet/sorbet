# frozen_string_literal: true
# typed: true

# Used in `sig.checked(level)` to determine when runtime type checking
# is enabled on a method.
module T::Private::RuntimeLevels
  LEVELS = [
    # Validate every call in every environment
    :always,
    # Validate in tests, but not in production
    :tests,
    # Don't even validate in tests, b/c too expensive,
    # or b/c we fully trust the static typing
    :never,
  ].freeze

  @check_tests = false
  @wrapped_tests_with_validation = false

  def self.check_tests?
    # Assume that this code path means that some `sig.checked(:tests)`
    # has been wrapped (or not wrapped) already, which is a trapdoor
    # for toggling `@check_tests`.
    @wrapped_tests_with_validation = true

    @check_tests
  end

  def self.enable_checking_in_tests
    if !@check_tests && @wrapped_tests_with_validation
      raise "Toggle `:tests`-level runtime type checking earlier. " \
        "There are already some methods wrapped with `sig.checked(:tests)`." \
    end

    _toggle_checking_tests(true)
  end

  def self._toggle_checking_tests(checked)
    @check_tests = checked
  end
end
