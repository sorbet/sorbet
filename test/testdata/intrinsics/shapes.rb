# @typed
class TestShapes
   def takesHash(a:)
   end

   def test
    h = { :a => 2}
    takesHash(h.freeze)
   end
end
