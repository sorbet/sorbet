# typed: true
class QueryProfile < Hash
  def explain_for(which)
    self["explain_#{which}"] || self[:"explain_#{which}"]
  # ^^^^^^^^^^^^^^^^^^^^^^^^ error: `String` doesn't match `QueryProfile::K` for argument `arg0`
                              # ^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Symbol` doesn't match `QueryProfile::K` for argument `arg0`
  end
end
