# typed: true

module Root
  class ModifierParent
    package_private private def package_private_private; end
    private package_private def private_package_private; end

    package_private protected def package_private_protected; end
    protected package_private def protected_package_private; end

    package_private public def package_private_public; end
    public package_private def public_package_private; end
  end
end
