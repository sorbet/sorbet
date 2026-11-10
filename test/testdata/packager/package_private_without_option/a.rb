# typed: strict

module Root
  class A
    package_private!

    X = 1
    package_private!(:X)
  end
end
