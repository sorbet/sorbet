# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  extend T::Sig

  sig {params(arg0: T.nilable(String)).void}
  private def private_with_keyword(arg0: nil)
    p arg0
  end

  def forward_to_private_one(arg0)
    private_with_keyword(arg0: arg0)
  end

  def forward_to_private_zero()
    private_with_keyword()
  end
end

Main.new.forward_to_private_one('moshi moshi')
Main.new.forward_to_private_one(nil)
Main.new.forward_to_private_zero()
