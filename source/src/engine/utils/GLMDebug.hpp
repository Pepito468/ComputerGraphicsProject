#ifndef ENGINE_GLMDEBUG_H
#define ENGINE_GLMDEBUG_H

#include <format>
#include <glm/glm.hpp>
#include <string>

/// Specialization to make vec3 compatible with std::format
template <>
struct std::formatter<glm::vec3> : std::formatter<std::string> {
    auto format(glm::vec3 p, format_context& ctx) const {
        return formatter<string>::format(
          std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx);
    }
};

/// Specialization to make vec4 compatible with std::format
template <>
struct std::formatter<glm::vec4> : std::formatter<std::string> {
    auto format(glm::vec4 p, format_context& ctx) const {
        return formatter<string>::format(
          std::format("[{}, {}, {}, {}]", p.x, p.y, p.z, p.t), ctx);
    }
};

/// Specialization to make mat4 compatible with std::format
template <>
struct std::formatter<glm::mat4> : std::formatter<std::string> {
    auto format(glm::mat4 m, format_context& ctx) const {
        return formatter<string>::format(
          std::format("|{}||{}||{}||{}|", m[0], m[1], m[2], m[3]), ctx);
    }
};

#endif
