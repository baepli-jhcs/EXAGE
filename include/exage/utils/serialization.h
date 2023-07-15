#pragma once

#include <cereal/types/string.hpp>
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

// // // std::filesystem::path serialization with cereal
// // template<class Archive>
// // void serialize(Archive& archive, std::filesystem::path& path)
// // {
// //     std::string str;
// //     if constexpr (Archive::is_loading::value)
// //     {
// //         archive(str);
// //         path = str;
// //     }
// //     else
// //     {
// //         str = path.string();
// //         archive(str);
// //     }
// // }

// namespace cereal
// {
//     //! Saving for boost::filesystem::path
//     template<class Archive>
//     inline void save(Archive& ar, ::std::filesystem::path const& p)
//     {
//         ar(CEREAL_NVP_("path", p.native()));
//     }

//     //! Loading for boost::filesystem::path
//     template<class Archive>
//     inline void load(Archive& ar, ::std::filesystem::path& p)
//     {
//         ::std::filesystem::path::string_type s;
//         ar(CEREAL_NVP_("path", s));
//         p = s;
//     }

//     //! Saving for boost::filesystem::filesystem_error
//     template<class Archive>
//     inline void save(Archive& ar, ::std::filesystem::filesystem_error const& err)
//     {
//         ar(CEREAL_NVP_("ec", err.code()));
//         std::string what(err.what());
//         ar(CEREAL_NVP_("what", what));
//         ar(CEREAL_NVP_("path1", err.path1()));
//         ar(CEREAL_NVP_("path2", err.path2()));
//     }

//     //! Loading for boost::filesystem::filesystem_error
//     template<class Archive>
//     inline void load(Archive& ar, ::std::filesystem::filesystem_error& err)
//     {
//         ::std::error_code ec;
//         std::string what;
//         ::std::filesystem::path p1, p2;
//         ar(CEREAL_NVP_("ec", ec));
//         ar(CEREAL_NVP_("what", what));
//         ar(CEREAL_NVP_("path1", p1));
//         ar(CEREAL_NVP_("path2", p2));
//         err = ::std::filesystem::filesystem_error(what, p1, p2, ec);
//     }

//     //! Saving for boost::filesystem::file_status
//     template<class Archive>
//     inline void save(Archive& ar, ::std::filesystem::file_status const& fs)
//     {
//         ar(CEREAL_NVP_("type", fs.type()));
//         ar(CEREAL_NVP_("permissions", fs.permissions()));
//     }

//     //! Loading for boost::filesystem::file_status
//     template<class Archive>
//     inline void load(Archive& ar, ::std::filesystem::file_status& fs)
//     {
//         ::std::filesystem::file_type ft;
//         ::std::filesystem::perms permissions;
//         ar(CEREAL_NVP_("type", ft));
//         ar(CEREAL_NVP_("permissions", permissions));

//         fs = ::std::filesystem::file_status(ft, permissions);
//     }

//     //             //! Saving for boost::filesystem::directory_entry
//     //    template <class Archive> inline
//     //       void save(Archive & ar, ::std::filesystem::directory_entry  const & de)
//     //    {
//     // 		ar( CEREAL_NVP_("path",  de.path()));
//     // 		ar( CEREAL_NVP_("status",  de.status()));
//     // 		ar( CEREAL_NVP_("symlink_status",  de.symlink_status()));
//     //    }

//     //    //! Loading for boost::filesystem::directory_entry
//     //    template <class Archive> inline
//     //       void load(Archive & ar, ::std::filesystem::directory_entry  & de)
//     //    {
//     // 		::std::filesystem::path 	p;
//     // 		::std::filesystem::file_status		status;
//     // 		::std::filesystem::file_status		symlink_status;
//     // 		ar( CEREAL_NVP_("path",  p));
//     // 		ar( CEREAL_NVP_("status",  status));
//     // 		ar( CEREAL_NVP_("symlink_status",  symlink_status));

//     // 		de = ::std::filesystem::directory_entry(p,status,symlink_status);
//     //    }

//     // No attempt made to serialize iterators - it would require serializing the path referenced
//     as
//     // well.

// }  // namespace cereal