# typed: true

class User
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.returns(T::Boolean).narrows_to(Admin) }
  def admin?; end

  sig { abstract.returns(T::Boolean).narrows_to(Staff) }
  def staff?; end
end

class Admin < User
  sig { override.returns(TrueClass).narrows_to(Admin) }
  def admin?; true; end

  sig { override.returns(FalseClass) }
  def staff?; false; end
end

class Staff < User
  sig { override.returns(FalseClass) }
  def admin?; false; end

  sig { override.returns(TrueClass).narrows_to(Staff) }
  def staff?; true; end
end

class Guest < User
  sig { override.returns(FalseClass) }
  def admin?; false; end

  sig { override.returns(FalseClass) }
  def staff?; false; end
end

def test_basic_type_guard
  user = T.let(Admin.new, User)
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  else
    T.reveal_type(user) # error: Revealed type: `User`
  end
end

def test_multiple_type_guards
  user = T.let(Staff.new, User)
  
  if user.staff?
    T.reveal_type(user) # error: Revealed type: `Staff`
  end
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  else
    T.reveal_type(user) # error: Revealed type: `User`
  end
end

def test_unless_type_guard
  user = T.let(Guest.new, User)
  
  unless user.admin?
    T.reveal_type(user) # error: Revealed type: `User`
  else
    T.reveal_type(user) # error: Revealed type: `Admin`
  end
end

def test_case_with_type_guard
  user = T.let(Admin.new, User)
  
  case
  when user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  when user.staff?
    T.reveal_type(user) # error: Revealed type: `Staff`
  else
    T.reveal_type(user) # error: Revealed type: `User`
  end
end

def test_nested_conditionals
  user = T.let(Admin.new, User)
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
    if user.staff?
      T.reveal_type(user) # error: This code is unreachable
    end
  end
end

def test_ternary_with_type_guard
  user = T.let(Admin.new, User)
  
  result = user.admin? ? user : nil
  T.reveal_type(result) # error: Revealed type: `T.nilable(Admin)`
end