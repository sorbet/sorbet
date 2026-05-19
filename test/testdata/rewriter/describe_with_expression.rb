# typed: true

class B
end

class Test
  describe("".dne) do
            # ^^^ error: Method `dne` does not exist on `String` component of `String("")`
  end

  describe A do
         # ^ error: Unable to resolve constant `A`
  end

  describe B do
  end

  describe "a" do
  end
end
