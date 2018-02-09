# @typed

class Box
  A = type_member

  declare_variables(
    :@value => A,
  )

   sig.returns(T.any(A, Integer))
   def read
     @value
   end
end
