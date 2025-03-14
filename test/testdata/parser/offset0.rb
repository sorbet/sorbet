# typed: true
module Model # parser-error: module definition in method body
  def a-b; end # parser-error: unexpected token
end
