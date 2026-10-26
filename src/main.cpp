//
// Created by Kyrylo Pylinskyi on 26/10/2026.
//

#include <charconv>
#include <string_view>

namespace {

    struct Config {
        std::uint16_t port = 9878;
        std::string_view sender_comp_id = "GATEWAY";
    };

    void print_usage() {
        std::puts("usage: fixgate [--port N] [--sender-comp-id ID]");
    }

    bool parse_args(const int argc, char** argv, Config& cfg) {
        for (int i = 1; i < argc; ++i) {
            const std::string_view arg = argv[i];
            if (const bool has_value = i + 1 < argc; arg == "--port" && has_value) {
                const std::string_view v = argv[++i];
                if (const auto [ptr, ec]
                    = std::from_chars(v.data(), v.data() + v.size(), cfg.port);
                    ec != std::errc{} || ptr != v.data() + v.size() || cfg.port == 0)
                        return false;

            } else if (arg == "--sender-comp-id" && has_value) {
                cfg.sender_comp_id = argv[++i];
            } else {
                return false;
            }
        }
        return true;
    }

}  // namespace

int main(const int argc, char** argv) {
    Config cfg;
    if (!parse_args(argc, argv, cfg)) {
        print_usage();
        return 1;
    }
    std::printf("fixgate 0.1.0: port %u, SenderCompID %.*s\n", static_cast<unsigned>(cfg.port),
                static_cast<int>(cfg.sender_comp_id.size()), cfg.sender_comp_id.data());
    std::puts("Gateway not implemented yet: see README roadmap.");
    return 0;
}
