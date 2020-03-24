#include "Driver.hh"
#include "Lexer.hh"
#include "PrintRBI.hh"
#include <getopt.h>

using namespace rbs_parser;

int main(int argc, char *argv[]) {
    int c;
    std::string typed = "true";

    while (1) {
        int i = 0;
        static struct option longOptions[] = {{"typed", required_argument, 0, 't'}, {0, 0, 0, 0}};
        c = getopt_long(argc, argv, "t:", longOptions, &i);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 't':
                typed = optarg;
                break;
            case '?':
                break;
            default:
                std::cerr << "Error: only accepted option is `--typed`" << std::endl;
                exit(1);
        }
    }

    if (argc - optind != 1) {
        std::cerr << "Usage: rbs_parser [--typed typedLevel] file.rbs" << std::endl;
        exit(1);
    }

    File file(argv[optind]);
    Driver driver(&file);
    try {
        Lexer lexer(file.source());
        Parser parser(driver, lexer);
        parser.parse();

        PrintRBI visitor(std::cout, typed);
        file.acceptVisitor(&visitor);
    } catch (ParseError e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (FileNotFoundException e) {
        std::cerr << "Error: file `" << file.filename << "` not found." << std::endl;
        return 1;
    }

    return 0;
}
