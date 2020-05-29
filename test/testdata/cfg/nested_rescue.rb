# frozen_string_literal: true
# typed: true

class C
  extend T::Sig

  sig {params(x: T::Boolean).returns(T::Boolean)}
  def test(x)
    begin
      true
    rescue
      begin
        false
      rescue
        raise if x
        true
      end
    end
  end
end
