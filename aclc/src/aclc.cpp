#include "lexer.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    using namespace acl;

    String file = "test.accele";

    std::ifstream ifs(file);

    if (ifs) {
        StringBuffer buf;
        buf << ifs.rdbuf();
        ifs.close();

        Lexer lexer(file, buf);

        while (lexer.hasNext()) {
            auto t = lexer.nextToken();

            std::cout << "[" << (int) t->type << " @ " << t->meta.line << ":" << t->meta.col << "]: " << t->data << "\n";

            delete t;
        }
    }

    return 0;
}