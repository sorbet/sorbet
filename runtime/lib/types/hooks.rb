# frozen_string_literal: true
# typed: true

module T::Hooks
  Private = T::Private

  # This is rarely required since `declare_sig` will do it for you, but in
  # certain cases, we need to get the hooks installed without `sig`ing
  # anything. E.g. `class << self`.
  #
  # Example:
  #
  #   class Foo
  #     T::Hooks.install(self)
  #
  #     class << self
  #       sig {void}
  #       def foo; end
  #     end
  #   end
  def self.install(pass_self_here)
    Private::Methods.install_hooks(pass_self_here)
  end
end
