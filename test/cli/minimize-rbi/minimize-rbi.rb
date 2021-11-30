# typed: true

class OnlyInFirst; end

class CommonToBoth
  def method_only_in_first; end

  def method_common_to_both; end
end

module ModuleOnlyInFirst; end
module ModuleCommonToBoth; end
