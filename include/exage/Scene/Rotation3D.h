#pragma once

#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "exage/utils/math.h"

namespace exage
{
    enum class RotationType : uint8_t
    {
        ePitchYawRoll,
        eYawPitchRoll,
        eQuaternion
    };

    class Rotation3D
    {
      public:
        Rotation3D() noexcept = default;
        inline explicit Rotation3D(glm::vec3 euler,
                                   RotationType type = RotationType::ePitchYawRoll) noexcept;
        inline Rotation3D(glm::quat rotation) noexcept;  // NOLINT

        [[nodiscard]] auto getRotationType() const noexcept -> RotationType { return _type; }
        [[nodiscard]] auto getQuaternion() const noexcept -> glm::quat { return _rotation; }

        [[nodiscard]] inline auto getEuler() const noexcept -> std::optional<glm::vec3>;

        [[nodiscard]] inline auto getAngleAxis() const noexcept -> std::pair<glm::vec3, float>;

        [[nodiscard]] inline auto getForwardVector() const noexcept -> glm::vec3;
        [[nodiscard]] inline auto getRightVector() const noexcept -> glm::vec3;
        [[nodiscard]] inline auto getUpVector() const noexcept -> glm::vec3;

        [[nodiscard]] inline auto getViewMatrix(glm::vec3 position) const noexcept -> glm::mat4;
        [[nodiscard]] inline auto getRotationMatrix() const noexcept -> glm::mat4;

        inline void setRotationType(RotationType type) noexcept;
        inline void setQuatRotation(glm::quat quat);
        inline void setEulerRotation(glm::vec3 euler,
                                     RotationType type = RotationType::ePitchYawRoll) noexcept;

        [[nodiscard]] inline auto operator==(const Rotation3D& other) const noexcept -> bool;
        [[nodiscard]] inline auto operator!=(const Rotation3D& other) const noexcept -> bool;

        [[nodiscard]] inline auto operator*(const Rotation3D& other) const noexcept -> Rotation3D;
        [[nodiscard]] inline auto operator*(float scalar) const noexcept -> Rotation3D;
        [[nodiscard]] inline auto operator/(float scalar) const noexcept -> Rotation3D;
        [[nodiscard]] inline auto operator+(const Rotation3D& other) const noexcept -> Rotation3D;
        [[nodiscard]] inline auto operator-(const Rotation3D& other) const noexcept -> Rotation3D;

        operator glm::quat() const noexcept { return getQuaternion(); }  // NOLINT

      private:
        glm::quat _rotation {glm::identity<glm::quat>()};
        glm::vec3 _euler {0.F};
        RotationType _type {RotationType::eQuaternion};
    };

    inline Rotation3D::Rotation3D(glm::vec3 euler, RotationType type) noexcept
        : _euler(euler)
        , _type(type)
    {
        switch (type)
        {
            case RotationType::ePitchYawRoll:
            {
                _rotation = glm::quat(_euler);
                break;
            }
            case RotationType::eYawPitchRoll:
            {
                _rotation = glm::quat(glm::vec3(_euler.y, _euler.x, _euler.z));
                break;
            }
            case RotationType::eQuaternion:
            default:
                break;
        }
    }

    inline Rotation3D::Rotation3D(glm::quat rotation) noexcept
        : _rotation(rotation)
    {
    }

    inline auto Rotation3D::getEuler() const noexcept -> std::optional<glm::vec3>
    {
        if (_type == RotationType::eQuaternion)
        {
            return std::nullopt;
        }

        return _euler;
    }

    inline auto Rotation3D::getAngleAxis() const noexcept -> std::pair<glm::vec3, float>
    {
        return {glm::axis(_rotation), glm::angle(_rotation)};
    }

    inline auto Rotation3D::getForwardVector() const noexcept -> glm::vec3
    {
        return _rotation * Z_AXIS;
    }

    inline auto Rotation3D::getRightVector() const noexcept -> glm::vec3
    {
        return _rotation * X_AXIS;
    }

    inline auto Rotation3D::getUpVector() const noexcept -> glm::vec3
    {
        return _rotation * Y_AXIS;
    }

    inline auto Rotation3D::getViewMatrix(glm::vec3 position) const noexcept -> glm::mat4
    {
        glm::vec3 forward = getForwardVector();
        return glm::lookAt(position, position + forward, getUpVector());
    }

    inline auto Rotation3D::getRotationMatrix() const noexcept -> glm::mat4
    {
        return glm::toMat4(_rotation);
    }

    inline void Rotation3D::setRotationType(RotationType type) noexcept
    {
        if (_type == type)
        {
            return;
        }

        switch (type)
        {
            case RotationType::ePitchYawRoll:
            {
                _euler = glm::eulerAngles(_rotation);
                break;
            }
            case RotationType::eYawPitchRoll:
            {
                glm::vec3 euler = glm::eulerAngles(_rotation);
                _euler = {euler.y, euler.x, euler.z};
                break;
            }
            case RotationType::eQuaternion:
            default:
                break;
        }

        _type = type;
    }

    inline void Rotation3D::setQuatRotation(glm::quat quat)
    {
        _rotation = quat;
        _type = RotationType::eQuaternion;
    }

    inline void Rotation3D::setEulerRotation(glm::vec3 euler, RotationType type) noexcept
    {
        _euler = euler;
        _type = type;

        switch (type)
        {
            case RotationType::ePitchYawRoll:
            {
                _rotation = glm::quat(_euler);
                break;
            }
            case RotationType::eYawPitchRoll:
            {
                _rotation = glm::quat(glm::vec3(_euler.y, _euler.x, _euler.z));
                break;
            }
            case RotationType::eQuaternion:
            default:
                break;
        }
    }

    inline auto Rotation3D::operator==(const Rotation3D& other) const noexcept -> bool
    {
        return _rotation == other._rotation;
    }

    inline auto Rotation3D::operator!=(const Rotation3D& other) const noexcept -> bool
    {
        return _rotation != other._rotation;
    }

    inline auto Rotation3D::operator*(const Rotation3D& other) const noexcept -> Rotation3D
    {
        return {_rotation * other._rotation};
    }

    inline auto Rotation3D::operator*(float scalar) const noexcept -> Rotation3D
    {
        return {_rotation * scalar};
    }

    inline auto Rotation3D::operator/(float scalar) const noexcept -> Rotation3D
    {
        return {_rotation / scalar};
    }

    inline auto Rotation3D::operator+(const Rotation3D& other) const noexcept -> Rotation3D
    {
        return {_rotation + other._rotation};
    }

    inline auto Rotation3D::operator-(const Rotation3D& other) const noexcept -> Rotation3D
    {
        return {_rotation - other._rotation};
    }
}  // namespace exage