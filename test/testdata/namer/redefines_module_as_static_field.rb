# typed: true
# disable-stress-incremental: true
module T

end

module T::Private
end

module T::Private::Methods
end

module T::Private::Methods::SignatureValidation
end

module T::Private::Methods::Modes
end

module T::Private::Methods::CallValidation
end

module T::AbstractUtils
end

  # The lines below tickle name mangling corner cases in namer.

  T::AbstractUtils::Methods = T::Private::Methods
  # `T::AbstractUtils::Methods` is a field, and we're trying to define `::SignatureValidation::Methods` on it
  # (where `::SignatureValidation` does not exist -- two levels of names)
  T::AbstractUtils::Methods::SignatureValidation::Methods = T::Private::Methods # error: Can't nest `SignatureValidation` under `T::AbstractUtils::Methods`
  # Now we are trying to define a field directly on `T::AbstractUtils::Methods`. This should use the same mangled
  # class as the previous line of code.
  T::AbstractUtils::Methods::Modes = T::Private::Methods::Modes # error: Can't nest `Modes` under `T::AbstractUtils::Methods`
  T::AbstractUtils::Methods::CallValidation::Modes = T::Private::Methods::Modes # error: Can't nest `CallValidation` under `T::AbstractUtils::Methods`
  T::AbstractUtils::Methods::CallValidation = T::Private::Methods::CallValidation 
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot initialize the class or module `CallValidation` by constant assignment
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Can't nest `CallValidation` under `T::AbstractUtils::Methods`
  # Defines a property on what was just previously a field.
  T::AbstractUtils::Methods::CallValidation::CallValidation = T::Private::Methods::CallValidation
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Can't nest `CallValidation` under `T::AbstractUtils::Methods` because `T::AbstractUtils::Methods` 
