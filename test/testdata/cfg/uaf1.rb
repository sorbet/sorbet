class A
  def initialize
    spec_list.map do
      begin
        1
      rescue StandardError => se
        2
     end
    end
  end
end
