# typed: core

class Object; end

class String < Object
  sig do
    params(
        arg0: String,
    )
    .returns(String)
  end
  def *(arg0); end
end
