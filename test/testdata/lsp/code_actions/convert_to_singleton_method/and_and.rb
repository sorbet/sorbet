# typed: true
# selective-apply-code-action: refactor.rewrite
extend T::Sig

class A
  extend T::Sig

  sig {returns(T.nilable(String))}
  def returns_nilable
    # | apply-code-action: [A] Convert to singleton class method (best effort)
  end
end

# Our <Magic> methods are hard.
if A.new.returns_nilable && A.new.returns_nilable.start_with?("prefix") # error: assumes result type doesn't change
  puts "yup"
else
  puts "nope"
end
