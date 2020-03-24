#ifndef RBS_PARSER_FILE_HH
#define RBS_PARSER_FILE_HH

#include "ast.hh"
#include <iostream>

namespace rbs_parser {
class FileNotFoundException {};

class File {
public:
    std::string filename;
    std::vector<Decl *> decls;

    File(std::string filename) : filename(filename){};

    void acceptVisitor(Visitor *v) { v->visit(this); };

    std::string source() {
        FILE *fp = std::fopen((std::string(filename)).c_str(), "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            int sz = ftell(fp);
            std::string contents(sz, '\0');
            rewind(fp);
            int readBytes = fread(&contents[0], 1, sz, fp);
            fclose(fp);
            if (readBytes != contents.size()) {
                throw FileNotFoundException();
            }
            return contents;
        }
        throw FileNotFoundException();
    }
};
} // namespace rbs_parser

#endif
