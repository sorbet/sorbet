# typed: true
class Foo
  extend T::Generic
  A = type_member
end

module Test
    extend T::Helpers

    sig do
      params(a: Foo) # error: Generic class without type arguments
      .returns(Foo) # error: Generic class without type arguments
    end
    def run(a)
	spec = {
          api_method: api_method.short_name, # error: does not exist on `Test`
        }
	cspec = spec.clone
	cspec[:params] = 1
	a
    end
end
