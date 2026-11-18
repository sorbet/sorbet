# typed: true

class A
  def b; end

  def b=(x)
#     ^ apply-rename: [A] newName: c= placeholderText: b= invalid: true expectedErrorMessage: Failed to rename `b=` method call at test/testdata/lsp/rename/method_setter_op_asgn.rb:12
  end
end

a = A.new
a.b ||= 1
