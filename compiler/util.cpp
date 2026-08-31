#include <expected>
#include <filesystem>
#include <fstream>

#include "util.hpp"

namespace fla::compiler::util
{
    const std::expected<std::string, std::string> read_file(const std::filesystem::path path)
    {
        std::ifstream file(path);

        if (!file.is_open()) {
            return std::unexpected("unable to open file");
        }

        const std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

        return content;
    }
} // namespace fla::compiler::util
