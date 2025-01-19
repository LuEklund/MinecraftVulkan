#ifndef HASH_HPP
#define HASH_HPP
#include <glm/glm.hpp>
#include <functional>

namespace std {
    template<>
    struct hash<glm::vec3> {
        std::size_t operator()(const glm::vec3 &v) const {
            std::size_t seed = 0;
            // Combine the hashes of the vector's components
            seed ^= std::hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}
#endif //HASH_HPP