# typed: strict

module RootPackage::Bar
  extend T::Sig

  sig {params(a: Integer).returns(String)}
  def main(a)
    if a > 10
      RootPackage::Foo::Constant
      #                 ^^^^^^^^ hover: String
    elsif a < 4
      RootPackage::Foo::Bar::Constant
      #                      ^^^^^^^^ hover: String
    else
      RootPackage::Foo::Bar::Baz::Constant
      #                           ^^^^^^^^ hover: String
    end
  end
end
