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

def test_two_way_union
  user = T.let(Admin.new, T.any(Admin, Staff))
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  else
    T.reveal_type(user) # error: Revealed type: `Staff`
  end
end

def test_three_way_union
  user = T.let(Guest.new, T.any(Admin, Staff, Guest))
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  else
    T.reveal_type(user) # error: Revealed type: `T.any(Staff, Guest)`
  end
end

def test_nested_narrowing
  user = T.let(Staff.new, T.any(Admin, Staff, Guest))
  
  if user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  elsif user.staff?
    T.reveal_type(user) # error: Revealed type: `Staff`
  else
    T.reveal_type(user) # error: Revealed type: `Guest`
  end
end

def test_case_statement
  user = T.let(Guest.new, T.any(Admin, Staff, Guest))
  
  case
  when user.admin?
    T.reveal_type(user) # error: Revealed type: `Admin`
  when user.staff?
    T.reveal_type(user) # error: Revealed type: `Staff`
  else
    T.reveal_type(user) # error: Revealed type: `Guest`
  end
end

def test_unless_statement
  user = T.let(Staff.new, T.any(Admin, Staff))
  
  unless user.admin?
    T.reveal_type(user) # error: Revealed type: `Staff`
  else
    T.reveal_type(user) # error: Revealed type: `Admin`
  end
end