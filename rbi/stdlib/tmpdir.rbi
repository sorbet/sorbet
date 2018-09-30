# typed: strict
class Dir
  sig do
    params(
      prefix_suffix: T.any(String, T::Array[String]),
      rest: String,
    ).returns(String)
  end
  def self.mktmpdir(prefix_suffix = nil, *rest); end
end
