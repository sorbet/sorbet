# typed: true

extend T::Sig

sig {returns(NilClass).generated}
#    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed signature: `generated` is invalid in this context
#    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Non-private call to private method `generated` on `T::Private::Methods::DeclBuilder`
def generated
end
