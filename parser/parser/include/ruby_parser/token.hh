#ifndef RUBY_PARSER_TOKEN_HH
#define RUBY_PARSER_TOKEN_HH

#include <cstddef>
#include <memory>
#include <string>

// these token values are mirrored in src/grammars/*.y
// any changes *must* be applied to the grammars as well.
#define RUBY_PARSER_TOKEN_TYPES(XX) \
    XX(eof, 0)                      \
    XX(error, -1)                   \
    XX(kCLASS, 1001)                \
    XX(kMODULE, 1002)               \
    XX(kDEF, 1003)                  \
    XX(kUNDEF, 1004)                \
    XX(kBEGIN, 1005)                \
    XX(kRESCUE, 1006)               \
    XX(kENSURE, 1007)               \
    XX(kEND, 1008)                  \
    XX(kIF, 1009)                   \
    XX(kUNLESS, 1010)               \
    XX(kTHEN, 1011)                 \
    XX(kELSIF, 1012)                \
    XX(kELSE, 1013)                 \
    XX(kCASE, 1014)                 \
    XX(kWHEN, 1015)                 \
    XX(kWHILE, 1016)                \
    XX(kUNTIL, 1017)                \
    XX(kFOR, 1018)                  \
    XX(kBREAK, 1019)                \
    XX(kNEXT, 1020)                 \
    XX(kREDO, 1021)                 \
    XX(kRETRY, 1022)                \
    XX(kIN, 1023)                   \
    XX(kDO, 1024)                   \
    XX(kDO_COND, 1025)              \
    XX(kDO_BLOCK, 1026)             \
    XX(kDO_LAMBDA, 1027)            \
    XX(kRETURN, 1028)               \
    XX(kYIELD, 1029)                \
    XX(kSUPER, 1030)                \
    XX(kSELF, 1031)                 \
    XX(kNIL, 1032)                  \
    XX(kTRUE, 1033)                 \
    XX(kFALSE, 1034)                \
    XX(kAND, 1035)                  \
    XX(kOR, 1036)                   \
    XX(kNOT, 1037)                  \
    XX(kIF_MOD, 1038)               \
    XX(kUNLESS_MOD, 1039)           \
    XX(kWHILE_MOD, 1040)            \
    XX(kUNTIL_MOD, 1041)            \
    XX(kRESCUE_MOD, 1042)           \
    XX(kALIAS, 1043)                \
    XX(kDEFINED, 1044)              \
    XX(klBEGIN, 1045)               \
    XX(klEND, 1046)                 \
    XX(k__LINE__, 1047)             \
    XX(k__FILE__, 1048)             \
    XX(k__ENCODING__, 1049)         \
    XX(tIDENTIFIER, 1050)           \
    XX(tFID, 1051)                  \
    XX(tGVAR, 1052)                 \
    XX(tIVAR, 1053)                 \
    XX(tCONSTANT, 1054)             \
    XX(tLABEL, 1055)                \
    XX(tCVAR, 1056)                 \
    XX(tNTH_REF, 1057)              \
    XX(tBACK_REF, 1058)             \
    XX(tSTRING_CONTENT, 1059)       \
    XX(tINTEGER, 1060)              \
    XX(tFLOAT, 1061)                \
    XX(tUPLUS, 1062)                \
    XX(tUMINUS, 1063)               \
    XX(tUNARY_NUM, 1064)            \
    XX(tPOW, 1065)                  \
    XX(tCMP, 1066)                  \
    XX(tEQ, 1067)                   \
    XX(tEQQ, 1068)                  \
    XX(tNEQ, 1069)                  \
    XX(tEQL, 1070)                  \
    XX(tGEQ, 1071)                  \
    XX(tLEQ, 1072)                  \
    XX(tANDOP, 1073)                \
    XX(tOROP, 1074)                 \
    XX(tMATCH, 1075)                \
    XX(tNMATCH, 1076)               \
    XX(tDOT, 1077)                  \
    XX(tDOT2, 1078)                 \
    XX(tDOT3, 1079)                 \
    XX(tAREF, 1080)                 \
    XX(tASET, 1081)                 \
    XX(tLSHFT, 1082)                \
    XX(tRSHFT, 1083)                \
    XX(tCOLON2, 1084)               \
    XX(tCOLON3, 1085)               \
    XX(tOP_ASGN, 1086)              \
    XX(tASSOC, 1087)                \
    XX(tLPAREN, 1088)               \
    XX(tLPAREN2, 1089)              \
    XX(tRPAREN, 1090)               \
    XX(tLPAREN_ARG, 1091)           \
    XX(tLBRACK, 1092)               \
    XX(tLBRACK2, 1093)              \
    XX(tRBRACK, 1094)               \
    XX(tLBRACE, 1095)               \
    XX(tLBRACE_ARG, 1096)           \
    XX(tSTAR, 1097)                 \
    XX(tSTAR2, 1098)                \
    XX(tAMPER, 1099)                \
    XX(tAMPER2, 1100)               \
    XX(tTILDE, 1101)                \
    XX(tPERCENT, 1102)              \
    XX(tDIVIDE, 1103)               \
    XX(tDSTAR, 1104)                \
    XX(tPLUS, 1105)                 \
    XX(tMINUS, 1106)                \
    XX(tLT, 1107)                   \
    XX(tGT, 1108)                   \
    XX(tPIPE, 1109)                 \
    XX(tBANG, 1110)                 \
    XX(tCARET, 1111)                \
    XX(tLCURLY, 1112)               \
    XX(tRCURLY, 1113)               \
    XX(tBACK_REF2, 1114)            \
    XX(tSYMBEG, 1115)               \
    XX(tSTRING_BEG, 1116)           \
    XX(tXSTRING_BEG, 1117)          \
    XX(tREGEXP_BEG, 1118)           \
    XX(tREGEXP_OPT, 1119)           \
    XX(tWORDS_BEG, 1120)            \
    XX(tQWORDS_BEG, 1121)           \
    XX(tSYMBOLS_BEG, 1122)          \
    XX(tQSYMBOLS_BEG, 1123)         \
    XX(tSTRING_DBEG, 1124)          \
    XX(tSTRING_DVAR, 1125)          \
    XX(tSTRING_END, 1126)           \
    XX(tSTRING_DEND, 1127)          \
    XX(tSTRING, 1128)               \
    XX(tSYMBOL, 1129)               \
    XX(tNL, 1130)                   \
    XX(tEH, 1131)                   \
    XX(tCOLON, 1132)                \
    XX(tCOMMA, 1133)                \
    XX(tSPACE, 1134)                \
    XX(tSEMI, 1135)                 \
    XX(tLAMBDA, 1136)               \
    XX(tLAMBEG, 1137)               \
    XX(tCHARACTER, 1138)            \
    XX(tRATIONAL, 1139)             \
    XX(tIMAGINARY, 1140)            \
    XX(tLABEL_END, 1141)            \
    XX(tANDDOT, 1142)               \
    XX(tRATIONAL_IMAGINARY, 1143)   \
    XX(tFLOAT_IMAGINARY, 1144)      \
    XX(tBDOT2, 1145)                \
    XX(tBDOT3, 1146)

namespace ruby_parser {
enum class token_type : int {
#ifndef YYBISON
#define XX(name, value) name = value,
    RUBY_PARSER_TOKEN_TYPES(XX)
#undef XX
#endif
};

class token {
    token_type _type;
    size_t _start;
    size_t _end;
    std::string_view _string;
    size_t _lineStart;

public:
    token(token_type type, size_t start, size_t end, std::string_view str, size_t line);
    // Don't allow people to rely on implicit conversion to std::string_view:
    // they should make sure their string views live in storage that will outlive
    // the lexer.
    token(token_type type, size_t start, size_t end, const std::string &str, size_t lineStart) = delete;

    token_type type() const;
    size_t start() const;
    size_t end() const;
    void setEnd(size_t end);
    size_t lineStart() const;
    std::string_view view() const;
    std::string asString() const;

    static std::string_view tokenTypeName(token_type type) {
#ifndef YYBISON
        switch (type) {
#define XX(name, value)    \
    case token_type::name: \
        return std::string_view(#name);
            RUBY_PARSER_TOKEN_TYPES(XX)
#undef XX
        }
#endif
    }
};

using token_t = token *;
} // namespace ruby_parser

std::ostream &operator<<(std::ostream &o, const ruby_parser::token &token);

#endif
