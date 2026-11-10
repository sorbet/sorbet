# typed: strict
module Root::A
  class Secret
    package_private!
  end

  X = 1
  package_private!(:X)
end
