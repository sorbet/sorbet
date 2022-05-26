# frozen_string_literal: true
# typed: strict

# Runtime support
class ::Module < Object
  extend T::Sig

  sig {params(args: T.any(Symbol, String)).returns(T.self_type)}
  def package_private(*args); self; end

  sig {params(args: T.any(Symbol, String)).returns(T.self_type)}
  def package_private_class_method(*args); self; end
end
