#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "nlohmann/json.hpp"

namespace glm
{
    template<class Archive>
    void serialize(Archive& archive, glm::vec2& v)
    {
        archive(v.x, v.y);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::vec3& v)
    {
        archive(v.x, v.y, v.z);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::vec4& v)
    {
        archive(v.x, v.y, v.z, v.w);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::ivec2& v)
    {
        archive(v.x, v.y);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::ivec3& v)
    {
        archive(v.x, v.y, v.z);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::ivec4& v)
    {
        archive(v.x, v.y, v.z, v.w);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::uvec2& v)
    {
        archive(v.x, v.y);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::uvec3& v)
    {
        archive(v.x, v.y, v.z);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::uvec4& v)
    {
        archive(v.x, v.y, v.z, v.w);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dvec2& v)
    {
        archive(v.x, v.y);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dvec3& v)
    {
        archive(v.x, v.y, v.z);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dvec4& v)
    {
        archive(v.x, v.y, v.z, v.w);
    }

    // glm matrices serialization
    template<class Archive>
    void serialize(Archive& archive, glm::mat2& m)
    {
        archive(m[0], m[1]);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dmat2& m)
    {
        archive(m[0], m[1]);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::mat3& m)
    {
        archive(m[0], m[1], m[2]);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::mat4& m)
    {
        archive(m[0], m[1], m[2], m[3]);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dmat4& m)
    {
        archive(m[0], m[1], m[2], m[3]);
    }

    template<class Archive>
    void serialize(Archive& archive, glm::quat& q)
    {
        archive(q.x, q.y, q.z, q.w);
    }
    template<class Archive>
    void serialize(Archive& archive, glm::dquat& q)
    {
        archive(q.x, q.y, q.z, q.w);
    }

    inline void to_json(nlohmann::json& j, const glm::vec2& v)
    {
        j = nlohmann::json::array({v.x, v.y});
    }

    inline void from_json(const nlohmann::json& j, glm::vec2& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
    }

    inline void to_json(nlohmann::json& j, const glm::vec3& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z});
    }

    inline void from_json(const nlohmann::json& j, glm::vec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const glm::vec4& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z, v.w});
    }

    inline void from_json(const nlohmann::json& j, glm::vec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    inline void to_json(nlohmann::json& j, const glm::ivec2& v)
    {
        j = nlohmann::json::array({v.x, v.y});
    }

    inline void from_json(const nlohmann::json& j, glm::ivec2& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
    }

    inline void to_json(nlohmann::json& j, const glm::ivec3& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z});
    }

    inline void from_json(const nlohmann::json& j, glm::ivec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const glm::ivec4& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z, v.w});
    }

    inline void from_json(const nlohmann::json& j, glm::ivec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    inline void to_json(nlohmann::json& j, const glm::uvec2& v)
    {
        j = nlohmann::json::array({v.x, v.y});
    }

    inline void from_json(const nlohmann::json& j, glm::uvec2& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
    }

    inline void to_json(nlohmann::json& j, const glm::uvec3& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z});
    }

    inline void from_json(const nlohmann::json& j, glm::uvec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const glm::uvec4& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z, v.w});
    }

    inline void from_json(const nlohmann::json& j, glm::uvec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    inline void to_json(nlohmann::json& j, const glm::dvec2& v)
    {
        j = nlohmann::json::array({v.x, v.y});
    }

    inline void from_json(const nlohmann::json& j, glm::dvec2& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
    }

    inline void to_json(nlohmann::json& j, const glm::dvec3& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z});
    }

    inline void from_json(const nlohmann::json& j, glm::dvec3& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const glm::dvec4& v)
    {
        j = nlohmann::json::array({v.x, v.y, v.z, v.w});
    }

    inline void from_json(const nlohmann::json& j, glm::dvec4& v)
    {
        j.at(0).get_to(v.x);
        j.at(1).get_to(v.y);
        j.at(2).get_to(v.z);
        j.at(3).get_to(v.w);
    }

    // glm matrices serialization

    inline void to_json(nlohmann::json& j, const glm::mat2& m)
    {
        j = nlohmann::json::array({m[0], m[1]});
    }

    inline void from_json(const nlohmann::json& j, glm::mat2& m)
    {
        j.at(0).get_to(m[0]);
        j.at(1).get_to(m[1]);
    }

    inline void to_json(nlohmann::json& j, const glm::dmat2& m)
    {
        j = nlohmann::json::array({m[0], m[1]});
    }

    inline void from_json(const nlohmann::json& j, glm::dmat2& m)
    {
        j.at(0).get_to(m[0]);
        j.at(1).get_to(m[1]);
    }

    inline void to_json(nlohmann::json& j, const glm::mat3& m)
    {
        j = nlohmann::json::array({m[0], m[1], m[2]});
    }

    inline void from_json(const nlohmann::json& j, glm::mat3& m)
    {
        j.at(0).get_to(m[0]);
        j.at(1).get_to(m[1]);
        j.at(2).get_to(m[2]);
    }

    inline void to_json(nlohmann::json& j, const glm::mat4& m)
    {
        j = nlohmann::json::array({m[0], m[1], m[2], m[3]});
    }

    inline void from_json(const nlohmann::json& j, glm::mat4& m)
    {
        j.at(0).get_to(m[0]);
        j.at(1).get_to(m[1]);
        j.at(2).get_to(m[2]);
        j.at(3).get_to(m[3]);
    }

    inline void to_json(nlohmann::json& j, const glm::dmat4& m)
    {
        j = nlohmann::json::array({m[0], m[1], m[2], m[3]});
    }

    inline void from_json(const nlohmann::json& j, glm::dmat4& m)
    {
        j.at(0).get_to(m[0]);
        j.at(1).get_to(m[1]);
        j.at(2).get_to(m[2]);
        j.at(3).get_to(m[3]);
    }

    inline void to_json(nlohmann::json& j, const glm::quat& q)
    {
        j = nlohmann::json::array({q.x, q.y, q.z, q.w});
    }

    inline void from_json(const nlohmann::json& j, glm::quat& q)
    {
        j.at(0).get_to(q.x);
        j.at(1).get_to(q.y);
        j.at(2).get_to(q.z);
        j.at(3).get_to(q.w);
    }

    inline void to_json(nlohmann::json& j, const glm::dquat& q)
    {
        j = nlohmann::json::array({q.x, q.y, q.z, q.w});
    }

    inline void from_json(const nlohmann::json& j, glm::dquat& q)
    {
        j.at(0).get_to(q.x);
        j.at(1).get_to(q.y);
        j.at(2).get_to(q.z);
        j.at(3).get_to(q.w);
    }

}  // namespace glm
