# typed: strict
class A
  def initialize
    spec_list.map do # error: Method `spec_list` does not exist on `A`
      begin
        1
      rescue StandardError => se
        2
     end
    end
  end
end
