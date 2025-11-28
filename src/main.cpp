#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string history_file = ""; // file containing all previous moves
    std::string output_file = ""; // file where we will write out next move

    // loop through all arguments passed to program sand store file that comes after "-H" and "-m"
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-H" && i + 1 < argc) {
            history_file = argv[++i];
        }
        else if (arg == "-m" && i + 1 < argc) {
            output_file = argv[++i];
        }
    }

    // for now, just print the paths to check we received them correctly (apo will add the opening/reading/writing of the files)
    std::cout << "History file: " << history_file << std::endl;
    std::cout << "Output file: " << output_file << std::endl;

    return 0;
}