#ifndef ENGINE_CONECOLLIDER_HPP
#define ENGINE_CONECOLLIDER_HPP

#include "Collider.hpp"

/// A collider shaped like a spherical cone
class ConeCollider : public Collider
{
    private:
        PointSet getOBBCorners() const
        {
            PointSet res = PointSet();

            const float endRad = radius * std::sin(aperture);
            res.insert(toGlobalSpace({endRad, endRad, 0}));
            res.insert(toGlobalSpace({endRad, -endRad, 0}));
            res.insert(toGlobalSpace({-endRad, endRad, 0}));
            res.insert(toGlobalSpace({-endRad, -endRad, 0}));
            res.insert(toGlobalSpace({endRad, endRad, (float)radius}));
            res.insert(toGlobalSpace({endRad, -endRad, (float)radius}));
            res.insert(toGlobalSpace({-endRad, endRad, (float)radius}));
            res.insert(toGlobalSpace({-endRad, -endRad, (float)radius}));
            return res;
        }

    public:
        /// The radius of the cone along the Z-axis
        BoundFloat radius = PositiveFloat(1.0f);

        /// The angle between the Z-axis of the cone and its extremes, in radians [0, PI / 2]
        BoundFloat aperture = BoundFloat(0.0f, M_PI/2, M_PI/6);

        ConeCollider() : Collider() {}
        ConeCollider(const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(position, rotation, scale) {}
        ConeCollider(const bool isTrigger, const glm::vec3 position, const glm::vec3 rotation, const glm::vec3 scale) : Collider(isTrigger, position, rotation, scale) {}
        ConeCollider(const float radius, const float aperture) : Collider()
        {
            this->radius = radius;
            this->aperture = aperture;
        }

        bool inBounds(const glm::vec3 point) const override
        {
            const glm::vec3 p = toLocalSpace(point);
            if (!epsilonLessThanEqual(glm::length(p), radius)) return false;

            const float angle = glm::acos(glm::dot(VEC3_Z, glm::normalize(p)));
            return epsilonLessThanEqual(angle, aperture);
        }

        PointSet getPointSet() const override
        {
            PointSet res = PointSet();

            //Add origin and cap peak
            res.insert(getGlobalPosition());
            res.insert(toGlobalSpace({0, 0, (float)radius}));

            glm::vec3 sidePointer = glm::rotate(MAT4_I, (float)aperture, VEC3_X) * glm::vec4(VEC3_Z, 1);
            const float delta = radius / POINT_PARAMETER;
            constexpr float loopAngle = glm::radians(360.0f / POINT_PARAMETER);
            glm::vec3 capPointers[] = {
                glm::rotate(MAT4_I, aperture / 4, VEC3_X) * glm::vec4(VEC3_Z, 1),
                glm::rotate(MAT4_I, aperture / 2, VEC3_X) * glm::vec4(VEC3_Z, 1),
                glm::rotate(MAT4_I, aperture * 3 / 4, VEC3_X) * glm::vec4(VEC3_Z, 1)
            };

            for (int i = 0; i < POINT_PARAMETER; i++)
            {
                //Add points along sides
                for (int j = 1; j < POINT_PARAMETER; j++)
                {
                    res.insert(toGlobalSpace(sidePointer * (delta * j)));
                }
                sidePointer = glm::rotate(MAT4_I, loopAngle, VEC3_Z) * glm::vec4(sidePointer, 1);

                //Add points on cap
                for (int j = 0; j < std::size(capPointers); j++)
                {
                    res.insert(toGlobalSpace(capPointers[j]));
                    capPointers[j] = glm::rotate(MAT4_I, loopAngle, VEC3_Z) * glm::vec4(capPointers[j], 1);
                }
            }

            return res;
        }

        SphereBounds getSphereBounds() const override
        {
            const glm::vec3 center = toGlobalSpace({0, 0, radius / 2.0f});
            //Point on the circumference of the cone
            const glm::vec3 edgePoint = toGlobalSpace({radius * glm::sin(aperture), 0, radius * glm::cos(aperture)});

            return SphereBounds(center, glm::distance(center, edgePoint));
        }

        AABBExtents getAABBExtents() const override
        {
            return AABBExtents(getOBBCorners());
        }

        static ConeCollider* fromJSON(const nlohmann::json& json, ConeCollider* node = nullptr) {
            ConeCollider *newNode = node ? node: new ConeCollider();

            Node3D::fromJSON(json, newNode);

            if (json.contains("isTrigger")) newNode->isTrigger = json.at("isTrigger").get<bool>();
            if (json.contains("movement")) newNode->movementStatus = json.at("movement").get<MovementStatus>();
            if (json.contains("radius")) newNode->radius = json.at("radius").get<float>();
            if (json.contains("aperture")) newNode->aperture = glm::radians(json.at("aperture").get<float>());

            return newNode;
        }
};

template <>
struct std::formatter<ConeCollider> : std::formatter<Node> {};
template <>
struct std::formatter<ConeCollider*> : std::formatter<Node*> {};
#endif