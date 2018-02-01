# @typed
module Test
  def normalize_params(v)
    if v.is_a?(Hash)
      normalize_params(v.to_a).sort
    elsif v.is_a?(Array)
      v.map {|e| normalize_params(e)}
    else
      v
    end
  end
end
