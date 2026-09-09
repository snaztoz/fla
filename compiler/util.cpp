#include <expected>
#include <filesystem>
#include <format>
#include <fstream>

#include "error.hpp"
#include "util.hpp"

namespace fla::compiler::util
{
    const std::expected<std::string, error::Error> read_file(const std::filesystem::path &path)
    {
        std::ifstream file(path);

        if (!file.is_open()) {
            return std::unexpected(error::Error {
                0,
                0,
                0,
                0,
                std::format("unable to open {}", path.string()),
            });
        }

        const std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());

        return content;
    }
} // namespace fla::compiler::util
