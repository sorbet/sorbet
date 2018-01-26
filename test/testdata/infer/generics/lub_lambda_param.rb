# @typed

class Box
  A = T.type

  declare_variables(
    :@value => A,
  )

   sig.returns(T.any(A, Integer))
   def read
     @value
   end
end
