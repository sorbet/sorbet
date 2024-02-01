# typed: true
# selective-apply-code-action: refactor.rewrite

class A
  def example(x:)
    # | apply-code-action: [A] Convert to singleton class method (best effort)
  end
end

A.new.example(
  x: (0)
)

A.new.example(
  x: begin
       0
  end
)
