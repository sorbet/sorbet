# typed: true
#
  class AttemptType < T::Enum
      enums do
          A001 = new
          A002 = new
          A003 = new
          A004 = new
          A005 = new
          A006 = new
          A007 = new
          A008 = new
          A009 = new
          A010 = new
          A011 = new
          A012 = new
          A013 = new
          A014 = new
          A015 = new
          A016 = new
          A017 = new
          A018 = new
          A019 = new
          A020 = new
          A021 = new
          A022 = new
          A023 = new
          A024 = new
          A025 = new
          A026 = new
          A027 = new
          A028 = new
          A029 = new
          A030 = new
          A031 = new
          A032 = new
          A033 = new
          A034 = new
          A035 = new
          A036 = new
          A037 = new
          A038 = new
          A039 = new
          A040 = new
          A041 = new
          A042 = new
          A043 = new
          A044 = new
          A045 = new
          A046 = new
          A047 = new
          A048 = new
          A049 = new
          A050 = new
          A051 = new
          A052 = new
          A053 = new
          A054 = new
          A055 = new
          A056 = new
          A057 = new
          A058 = new
          A059 = new
          A060 = new
          A061 = new
          A062 = new
          A063 = new
          A064 = new
          A065 = new
          A066 = new
          A067 = new
          A068 = new
          A069 = new
          A070 = new
          A071 = new
          A072 = new
          A073 = new
          A074 = new
          A075 = new
          A076 = new
          A077 = new
          A078 = new
          A079 = new
          A080 = new
          A081 = new
          A082 = new
          A083 = new
          A084 = new
          A085 = new
          A086 = new
          A087 = new
          A088 = new
          A089 = new
          A090 = new
          A091 = new
          A092 = new
          A093 = new
          A094 = new
          A095 = new
          A096 = new
          A097 = new
          A098 = new
          A099 = new
          A100 = new
          A101 = new
          A102 = new
          A103 = new
          A104 = new
          A105 = new
          A106 = new
          A107 = new
          A108 = new
          A109 = new
          A110 = new
          A111 = new
          A112 = new
          A113 = new
          A114 = new
          A115 = new
          A116 = new
          A117 = new
          A118 = new
          A119 = new
          A120 = new
          A121 = new
          A122 = new
          A123 = new
          A124 = new
          A125 = new
          A126 = new
          A127 = new
          A128 = new
          A129 = new
          A130 = new
          A131 = new
          A132 = new
          A133 = new
          A134 = new
          A135 = new
          A136 = new
          A137 = new
          A138 = new
          A139 = new
          A140 = new
          A141 = new
          A142 = new
          A143 = new
          A144 = new
          A145 = new
          A146 = new
          A147 = new
          A148 = new
          A149 = new
          A150 = new
          A151 = new
          A152 = new
          A153 = new
          A154 = new
          A155 = new
          A156 = new
          A157 = new
          A158 = new
          A159 = new
          A160 = new
          A161 = new
          A162 = new
          A163 = new
          A164 = new
          A165 = new
          A166 = new
          A167 = new
          A168 = new
          A169 = new
          A170 = new
          A171 = new
          A172 = new
          A173 = new
          A174 = new
          A175 = new
          A176 = new
          A177 = new
          A178 = new
          A179 = new
          A180 = new
          A181 = new
          A182 = new
          A183 = new
          A184 = new
          A185 = new
          A186 = new
          A187 = new
          A188 = new
          A189 = new
          A190 = new
          A191 = new
          A192 = new
          A193 = new
          A194 = new
          A195 = new
          A196 = new
          A197 = new
          A198 = new
          A199 = new
          A200 = new
      end
end

extend T::Sig

    sig do
      params(
        type: AttemptType,
      )
      .returns(Integer)
    end
    def self.partner(type)
      case type
      when AttemptType::A001
        1
      when AttemptType::A010
              2
      when AttemptType::A020
              3
      when AttemptType::A050
              4
      when AttemptType::A080
              5
      when AttemptType::A090
              5
      else
              raise "nope"
      end
    end
