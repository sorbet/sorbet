def __t_props_generated_serialize(strict)
  h = {}
  if @primitive.nil?
    required_prop_missing_from_serialize(:primitive) if strict
  else
    h["primitive"] = @primitive
  end


  if @nilable.nil?

  else
    h["nilable"] = @nilable
  end


  if @nilable_on_read.nil?
    required_prop_missing_from_serialize(:nilable_on_read) if strict
  else
    h["nilable_on_read"] = @nilable_on_read
  end


  if @primitive_default.nil?
    required_prop_missing_from_serialize(:primitive_default) if strict
  else
    h["primitive_default"] = @primitive_default
  end


  if @primitive_nilable_default.nil?

  else
    h["primitive_nilable_default"] = @primitive_nilable_default
  end


  if @factory.nil?
    required_prop_missing_from_serialize(:factory) if strict
  else
    h["factory"] = @factory
  end


  if @primitive_array.nil?
    required_prop_missing_from_serialize(:primitive_array) if strict
  else
    h["primitive_array"] = @primitive_array.dup
  end


  if @array_default.nil?
    required_prop_missing_from_serialize(:array_default) if strict
  else
    h["array_default"] = @array_default.dup
  end


  if @primitive_hash.nil?
    required_prop_missing_from_serialize(:primitive_hash) if strict
  else
    h["primitive_hash"] = @primitive_hash.dup
  end


  if @array_of_nilable.nil?
    required_prop_missing_from_serialize(:array_of_nilable) if strict
  else
    h["array_of_nilable"] = @array_of_nilable.dup
  end


  if @nilable_array.nil?

  else
    h["nilable_array"] = @nilable_array.dup
  end


  if @substruct.nil?
    required_prop_missing_from_serialize(:substruct) if strict
  else
    h["substruct"] = @substruct.serialize(strict)
  end


  if @nilable_substract.nil?

  else
    h["nilable_substract"] = @nilable_substract.serialize(strict)
  end


  if @default_substruct.nil?
    required_prop_missing_from_serialize(:default_substruct) if strict
  else
    h["default_substruct"] = @default_substruct.serialize(strict)
  end


  if @array_of_substruct.nil?
    required_prop_missing_from_serialize(:array_of_substruct) if strict
  else
    h["array_of_substruct"] = @array_of_substruct.map {|v| v.serialize(strict)}
  end


  if @hash_of_substruct.nil?
    required_prop_missing_from_serialize(:hash_of_substruct) if strict
  else
    h["hash_of_substruct"] = @hash_of_substruct.transform_values {|v| v.serialize(strict)}
  end


  if @custom_type.nil?
    required_prop_missing_from_serialize(:custom_type) if strict
  else
    h["custom_type"] = T::Props::CustomType.checked_serialize(Opus::Types::Test::Props::SerializableTest::CustomType, @custom_type)
  end


  if @nilable_custom_type.nil?

  else
    h["nilable_custom_type"] = T::Props::CustomType.checked_serialize(Opus::Types::Test::Props::SerializableTest::CustomType, @nilable_custom_type)
  end


  if @default_custom_type.nil?
    required_prop_missing_from_serialize(:default_custom_type) if strict
  else
    h["default_custom_type"] = T::Props::CustomType.checked_serialize(Opus::Types::Test::Props::SerializableTest::CustomType, @default_custom_type)
  end


  if @array_of_custom_type.nil?
    required_prop_missing_from_serialize(:array_of_custom_type) if strict
  else
    h["array_of_custom_type"] = @array_of_custom_type.map {|v| T::Props::CustomType.checked_serialize(Opus::Types::Test::Props::SerializableTest::CustomType, v)}
  end


  if @hash_of_custom_type_to_substruct.nil?
    required_prop_missing_from_serialize(:hash_of_custom_type_to_substruct) if strict
  else
    h["hash_of_custom_type_to_substruct"] = @hash_of_custom_type_to_substruct.each_with_object({}) {|(k,v),h| h[T::Props::CustomType.checked_serialize(Opus::Types::Test::Props::SerializableTest::CustomType, k)] = v.serialize(strict)}
  end


  if @unidentified_type.nil?
    required_prop_missing_from_serialize(:unidentified_type) if strict
  else
    h["unidentified_type"] = T::Props::Utils.deep_clone_object(@unidentified_type)
  end


  if @nilable_unidentified_type.nil?

  else
    h["nilable_unidentified_type"] = T::Props::Utils.deep_clone_object(@nilable_unidentified_type)
  end


  if @array_of_unidentified_type.nil?
    required_prop_missing_from_serialize(:array_of_unidentified_type) if strict
  else
    h["array_of_unidentified_type"] = @array_of_unidentified_type.map {|v| T::Props::Utils.deep_clone_object(v)}
  end


  if @defaulted_unidentified_type.nil?
    required_prop_missing_from_serialize(:defaulted_unidentified_type) if strict
  else
    h["defaulted_unidentified_type"] = T::Props::Utils.deep_clone_object(@defaulted_unidentified_type)
  end


  if @hash_with_unidentified_types.nil?
    required_prop_missing_from_serialize(:hash_with_unidentified_types) if strict
  else
    h["hash_with_unidentified_types"] = @hash_with_unidentified_types.each_with_object({}) {|(k,v),h| h[T::Props::Utils.deep_clone_object(k)] = T::Props::Utils.deep_clone_object(v)}
  end

  h
end



def __t_props_generated_deserialize(hash)
  found = 26
  val = hash["primitive"]
  @primitive = if val.nil?
                 found -= 1 unless hash.key?("primitive")
                 self.class.decorator.raise_nil_deserialize_error("primitive")
               else
                 val
               end


  val = hash["nilable"]
  @nilable = if val.nil?
               found -= 1 unless hash.key?("nilable")
               nil
             else
               val
             end


  val = hash["nilable_on_read"]
  @nilable_on_read = if val.nil?
                       found -= 1 unless hash.key?("nilable_on_read")
                       required_prop_missing_from_deserialize(:nilable_on_read)
                     else
                       val
                     end


  val = hash["primitive_default"]
  @primitive_default = if val.nil?
                         found -= 1 unless hash.key?("primitive_default")
                         0
                       else
                         val
                       end


  val = hash["primitive_nilable_default"]
  @primitive_nilable_default = if val.nil?
                                 found -= 1 unless hash.key?("primitive_nilable_default")
                                 nil
                               else
                                 val
                               end


  val = hash["factory"]
  @factory = if val.nil?
               found -= 1 unless hash.key?("factory")
               self.class.decorator.props_with_defaults.fetch(:factory).default
             else
               val
             end


  val = hash["primitive_array"]
  @primitive_array = if val.nil?
                       found -= 1 unless hash.key?("primitive_array")
                       self.class.decorator.raise_nil_deserialize_error("primitive_array")
                     else
                       val.dup
                     end


  val = hash["array_default"]
  @array_default = if val.nil?
                     found -= 1 unless hash.key?("array_default")
                     []
                   else
                     val.dup
                   end


  val = hash["primitive_hash"]
  @primitive_hash = if val.nil?
                      found -= 1 unless hash.key?("primitive_hash")
                      self.class.decorator.raise_nil_deserialize_error("primitive_hash")
                    else
                      val.dup
                    end


  val = hash["array_of_nilable"]
  @array_of_nilable = if val.nil?
                        found -= 1 unless hash.key?("array_of_nilable")
                        self.class.decorator.raise_nil_deserialize_error("array_of_nilable")
                      else
                        val.dup
                      end


  val = hash["nilable_array"]
  @nilable_array = if val.nil?
                     found -= 1 unless hash.key?("nilable_array")
                     nil
                   else
                     val.dup
                   end


  val = hash["substruct"]
  @substruct = if val.nil?
                 found -= 1 unless hash.key?("substruct")
                 self.class.decorator.raise_nil_deserialize_error("substruct")
               else
                 Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(val)
               end


  val = hash["nilable_substract"]
  @nilable_substract = if val.nil?
                         found -= 1 unless hash.key?("nilable_substract")
                         nil
                       else
                         Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(val)
                       end


  val = hash["default_substruct"]
  @default_substruct = if val.nil?
                         found -= 1 unless hash.key?("default_substruct")
                         self.class.decorator.props_with_defaults.fetch(:default_substruct).default
                       else
                         Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(val)
                       end


  val = hash["array_of_substruct"]
  @array_of_substruct = if val.nil?
                          found -= 1 unless hash.key?("array_of_substruct")
                          self.class.decorator.raise_nil_deserialize_error("array_of_substruct")
                        else
                          val.map {|v| Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(v)}
                        end


  val = hash["hash_of_substruct"]
  @hash_of_substruct = if val.nil?
                         found -= 1 unless hash.key?("hash_of_substruct")
                         self.class.decorator.raise_nil_deserialize_error("hash_of_substruct")
                       else
                         val.transform_values {|v| Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(v)}
                       end


  val = hash["custom_type"]
  @custom_type = if val.nil?
                   found -= 1 unless hash.key?("custom_type")
                   self.class.decorator.raise_nil_deserialize_error("custom_type")
                 else
                   Opus::Types::Test::Props::SerializableTest::CustomType.deserialize(val)
                 end


  val = hash["nilable_custom_type"]
  @nilable_custom_type = if val.nil?
                           found -= 1 unless hash.key?("nilable_custom_type")
                           nil
                         else
                           Opus::Types::Test::Props::SerializableTest::CustomType.deserialize(val)
                         end


  val = hash["default_custom_type"]
  @default_custom_type = if val.nil?
                           found -= 1 unless hash.key?("default_custom_type")
                           ""
                         else
                           Opus::Types::Test::Props::SerializableTest::CustomType.deserialize(val)
                         end


  val = hash["array_of_custom_type"]
  @array_of_custom_type = if val.nil?
                            found -= 1 unless hash.key?("array_of_custom_type")
                            self.class.decorator.raise_nil_deserialize_error("array_of_custom_type")
                          else
                            val.map {|v| Opus::Types::Test::Props::SerializableTest::CustomType.deserialize(v)}
                          end


  val = hash["hash_of_custom_type_to_substruct"]
  @hash_of_custom_type_to_substruct = if val.nil?
                                        found -= 1 unless hash.key?("hash_of_custom_type_to_substruct")
                                        self.class.decorator.raise_nil_deserialize_error("hash_of_custom_type_to_substruct")
                                      else
                                        val.each_with_object({}) {|(k,v),h| h[Opus::Types::Test::Props::SerializableTest::CustomType.deserialize(k)] = Opus::Types::Test::Props::SerializableTest::MySerializable.from_hash(v)}
                                      end


  val = hash["unidentified_type"]
  @unidentified_type = if val.nil?
                         found -= 1 unless hash.key?("unidentified_type")
                         self.class.decorator.raise_nil_deserialize_error("unidentified_type")
                       else
                         T::Props::Utils.deep_clone_object(val)
                       end


  val = hash["nilable_unidentified_type"]
  @nilable_unidentified_type = if val.nil?
                                 found -= 1 unless hash.key?("nilable_unidentified_type")
                                 nil
                               else
                                 T::Props::Utils.deep_clone_object(val)
                               end


  val = hash["array_of_unidentified_type"]
  @array_of_unidentified_type = if val.nil?
                                  found -= 1 unless hash.key?("array_of_unidentified_type")
                                  self.class.decorator.raise_nil_deserialize_error("array_of_unidentified_type")
                                else
                                  val.map {|v| T::Props::Utils.deep_clone_object(v)}
                                end


  val = hash["defaulted_unidentified_type"]
  @defaulted_unidentified_type = if val.nil?
                                   found -= 1 unless hash.key?("defaulted_unidentified_type")
                                   self.class.decorator.props_with_defaults.fetch(:defaulted_unidentified_type).default
                                 else
                                   T::Props::Utils.deep_clone_object(val)
                                 end


  val = hash["hash_with_unidentified_types"]
  @hash_with_unidentified_types = if val.nil?
                                    found -= 1 unless hash.key?("hash_with_unidentified_types")
                                    self.class.decorator.raise_nil_deserialize_error("hash_with_unidentified_types")
                                  else
                                    val.each_with_object({}) {|(k,v),h| h[T::Props::Utils.deep_clone_object(k)] = T::Props::Utils.deep_clone_object(v)}
                                  end

  found
end



