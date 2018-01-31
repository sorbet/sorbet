# @typed
class QueryProfile < Hash
  def explain_for(which)
    self["explain_#{which}"] || self[:"explain_#{which}"] # error: MULTI
  end
end
