// Library for vector relations
#ifndef COMPARISONS
#define COMPARISONS

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#define EPSILON 0.00001f

/// Returns true if for each component A == B (epsilon-equality)
template<glm::length_t L, typename T, glm::qualifier Q>
bool epsilonEqual(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b)
{
    return glm::all(glm::epsilonEqual(a, b, EPSILON));
}

inline bool epsilonEqual(const float a, const float b)
{
    return glm::epsilonEqual(a, b, EPSILON);
}

/// Returns true if for each component A >= B (epsilon-equality)
template<glm::length_t L, typename T, glm::qualifier Q>
bool epsilonGreaterThanEqual(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b)
{
    return glm::all(glm::greaterThan(a, b) || glm::epsilonEqual(a, b, EPSILON));
}

inline bool epsilonGreaterThanEqual(const float a, const float b)
{
    return a > b || epsilonEqual(a, b);
}

/// Returns true if for each component A <= B (epsilon-equality)
template<glm::length_t L, typename T, glm::qualifier Q>
bool epsilonLessThanEqual(const glm::vec<L, T, Q>& a, const glm::vec<L, T, Q>& b)
{
    return glm::all(glm::lessThan(a, b) || glm::epsilonEqual(a, b, EPSILON));
}

inline bool epsilonLessThanEqual(const float a, const float b)
{
    return a < b || epsilonEqual(a, b);
}

/// Returns true if for each component min <= x <= max (epsilon-equality)
template<glm::length_t L, typename T, glm::qualifier Q>
bool epsilonBetween(const glm::vec<L, T, Q>& min, const glm::vec<L, T, Q>& x, const glm::vec<L, T, Q>& max)
{
    return epsilonLessThanEqual(min, x) && epsilonLessThanEqual(x, max);
}

inline bool epsilonBetween(const float min, const float x, const float max)
{
    return epsilonLessThanEqual(min, x) && epsilonLessThanEqual(x, max);
}

/// Returns the maximum component of the vector
template<glm::length_t L, glm::qualifier Q>
float maxComponent(const glm::vec<L, float, Q>& v)
{
    float max = v[0];
    for(int i = 1; i < L; ++i)
    {
        if(v[i] > max) max = v[i];
    }
    return max;
}

/// Returns the minimum component of the vector
template<glm::length_t L, glm::qualifier Q>
float minComponent(const glm::vec<L, float, Q>& v)
{
    float max = v[0];
    for(int i = 1; i < L; ++i)
    {
        if(v[i] < max) max = v[i];
    }
    return max;
}
#endif