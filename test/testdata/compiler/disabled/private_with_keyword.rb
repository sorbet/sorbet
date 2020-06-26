# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  extend T::Sig

  sig {params(arg0: T.nilable(String)).returns(Integer)}
  private def private_with_keyword(arg0: nil)
    413
  end

  def forward_to_private(arg0)
    private_with_keyword(arg0: arg0)
  end
end

p Main.new.forward_to_private('moshi moshi')
p Main.new.forward_to_private(nil)
