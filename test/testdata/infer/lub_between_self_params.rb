# @typed
class QueryProfile < Hash # error: MULTI
  def explain_for(which)
    self["explain_#{which}"] || self[:"explain_#{which}"] # error: MULTI
  end
end
