# typed: true

class A
  def b; end

  def b=(x)
#     ^ apply-rename: [A] newName: c= placeholderText: b=
  end
end

a = A.new
a.b = 1
