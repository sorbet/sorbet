# typed: true

class A
  def b; end

  def b=(x)
#     ^ apply-rename: [A] newName: c= placeholderText: b=
  end
end

a = A.new
# `b` names both the getter and the setter here, so renaming either one renames it
a.b ||= 1
