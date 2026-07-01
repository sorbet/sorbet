# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-method: true

class LivenessRescue
  extend T::Sig

  sig {void}
  def side_effects_in_begin
    x = T.let(0, Integer)
    begin
      puts x; puts x + 1
#     ^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Method
    rescue
      puts "error"
    end
  end
end
