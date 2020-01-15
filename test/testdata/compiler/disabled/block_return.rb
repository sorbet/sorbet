# typed: true
# compiled: true
def flag_overridable?(s)
    true
end
def flag_overrides_valid?(flag_overrides={})
  flag_overrides.each do |flag_name, flag_value|
    return false if !flag_overridable?(flag_name)
    return false if !flag_value.is_a?(Integer)
  end
  true
end

flag_overrides_valid?({name: "value"})
