# @typed
# In vanilla Ruby, this would be an error.
#
# In Stripe Ruby, we statically analyze and determine that `A < B`,
# and emit that as an autoloader stub. We then allow this definition,
# and end up with `A.superclass == B`.

class B
end

class A
end

class A < B
end
