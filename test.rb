# typed: true

require 'sorbet-runtime'

class User
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { abstract.returns(T::Boolean).narrows_to(Admin) }
  def admin?; end
end

class Admin < User
  sig { override.returns(TrueClass).narrows_to(Admin) }
  def admin?
    pp(true)
  end
end

class Staff < User
  sig { override.returns(FalseClass).narrows_to(Admin) }
  def admin?
    pp(false)
  end
end

foo = T.let(Admin.new, User)
if foo.admin?
  T.reveal_type(foo)
end

