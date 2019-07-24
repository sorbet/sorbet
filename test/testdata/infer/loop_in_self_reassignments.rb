# typed: true
class Repro
  extend T::Sig
  def api_sections
  end

  sig {void}
  def get_repro()
    api_sections.each do
      return api_sections.map {1}
    end
  end
end
