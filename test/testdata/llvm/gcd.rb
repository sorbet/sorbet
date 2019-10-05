#typed: true
module GCD
 extend T::Helpers

 sig { params(a: Integer, b:Integer).returns(Integer) }
 def self.gcd_typed(a, b)
   while (a != b)
     if a > b
       a -= b
     else
       b -= a
     end
   end
   a
   end
 
 def self.gcd_untyped(a, b)
   while (a != b)
     if a > b
       a -= b
     else
       b -= a
     end
   end
   a
 end
end
