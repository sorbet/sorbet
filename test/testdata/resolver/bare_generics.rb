# @typed
class Foo
  A = T.type
end

module Test
    sig(a: Foo) # error: Generic class without type arguments
    .returns(Foo) # error: Generic class without type arguments
    def run(a)
	spec = {
          api_method: api_method.short_name, # error: does not exist on Test
        }
	cspec = spec.clone
	cspec[:params] = 1
	a
    end
end
