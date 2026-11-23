#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "prehistoric/core/util/math/Math.h"
#include "prehistoric/core/util/math/MathT.h"

#include <utility>

using namespace Prehistoric;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {
    void near(float actual, float expected) {
        CHECK_THAT(actual, WithinAbs(expected, 1e-5) || WithinRel(expected, 1e-5f));
    }

    template<typename V>
    void components(const V &actual, const V &expected, int count) {
        for (int i = 0; i < count; ++i) {
            INFO("component " << i);
            near(actual[i], expected[i]);
        }
    }

    void matrix(const Matrix4f &actual, const Matrix4f &expected) {
        for (int i = 0; i < 16; ++i) {
            INFO("matrix element " << i);
            near(actual.m[i], expected.m[i]);
        }
    }

    template<typename V>
    void vectorArithmetic(int count) {
        const V a(6.0f), b(2.0f);
        components(V(), V(0.0f), count);
        components(a + b, V(8.0f), count);
        components(a - b, V(4.0f), count);
        components(a * b, V(12.0f), count);
        components(a / b, V(3.0f), count);
        components(a + 2.0f, V(8.0f), count);
        components(a - 2.0f, V(4.0f), count);
        components(a * 2.0f, V(12.0f), count);
        components(a / 2.0f, V(3.0f), count);
        V c(a);
        c += b;
        c -= b;
        c *= b;
        c /= b;
        components(c, a, count);
        c += 2.0f;
        c -= 2.0f;
        c *= 2.0f;
        c /= 2.0f;
        components(c, a, count);
        CHECK(a == V(6.0f));
        CHECK(a != b);
        components(a.negate(), V(-6.0f), count);
        components(a, V(6.0f), count);
        c.negated();
        components(c, V(-6.0f), count);
    }
}

TEST_CASE (
"Angles convert between degrees and radians"
,
"[math]"
)
{
    near(ToRadians(180.0f), static_cast<float>(PI));
    near(ToDegrees(static_cast<float>(PI / 2)), 90.0f);
    for (float degrees : {-360.0f, -90.0f, 0.0f, 45.0f, 720.0f})
    {
        near(ToDegrees(ToRadians(degrees)), degrees);
    }
}

TEST_CASE (
"Vector2 arithmetic and normalization"
,
"[math][vector]"
)
{
    vectorArithmetic<Vector2f>(2);
    const Vector2f v(3, 4);
    near(v.length(), 5);
    near(v.lengthSquared(), 25);
    near(v.dot(Vector2f(2, -1)), 2);
    components(v.normalise(), Vector2f(0.6f, 0.8f), 2);
    components(v.swap(), Vector2f(4, 3), 2);
    components(Vector2f(-3, 4).abs(), v, 2);
}

TEST_CASE (
"Vector3 arithmetic and normalization"
,
"[math][vector]"
)
{
    vectorArithmetic<Vector3f>(3);
    const Vector3f v(2, -3, 6);
    near(v.lengthSquared(), 49);
    near(v.length(), 7);
    near(v.normalise().length(), 1);
    near(v.dot(Vector3f(4, 5, -2)), -19);
    components(v.abs(), Vector3f(2, 3, 6), 3);
    components(Vector3f(Vector2f(1, 2), 3), Vector3f(1, 2, 3), 3);
}

TEST_CASE (
"Vector4 arithmetic and normalization"
,
"[math][vector]"
)
{
    vectorArithmetic<Vector4f>(4);
    const Vector4f v(1, 2, 2, 4);
    near(v.lengthSquared(), 25);
    near(v.length(), 5);
    near(v.normalise().length(), 1);
    components(Vector4f(Vector3f(1, 2, 3), 4), Vector4f(1, 2, 3, 4), 4);
    components(Vector4f(Vector2f(1, 2), Vector2f(3, 4)), Vector4f(1, 2, 3, 4), 4);
    components(Vector4f(-1, -2, 3, 4).abs(), Vector4f(1, 2, 3, 4), 4);
}

TEST_CASE (
"Cross product follows the right hand rule"
,
"[math][vector]"
)
{
    const Vector3f x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
    components(x.cross(y), z, 3);
    components(y.cross(x), z.negate(), 3);
    const Vector3f a(2, -3, 4), b(-5, 2, 7);
    near(a.cross(b).dot(a), 0);
    near(a.cross(b).dot(b), 0);
    components(a.cross(a), Vector3f(0), 3);
}

TEST_CASE (
"Reflection preserves tangential components"
,
"[math][vector]"
)
{
    components(Vector3f(2, -3, 4).reflect(Vector3f(0, 1, 0)), Vector3f(2, 3, 4), 3);
    components(Vector3f(0, -1, 0).refract(Vector3f(0, 1, 0), 1.0f), Vector3f(0, -1, 0), 3);
}

TEST_CASE (
"Vector2 interpolation supports endpoints midpoint and inversion"
,
"[math][vector]"
)
{
    const Vector2f a(2, 4), b(6, 12);
    components(a.lerp(b, 0.0f, false), a, 2);
    components(a.lerp(b, 1.0f, false), b, 2);
    components(a.lerp(b, 0.5f, true), Vector2f(8, 4), 2);
    components(a.lerp(b, Vector2f(0.25f, 0.75f), false), Vector2f(3, 10), 2);
}

TEST_CASE (
"Component interpolation affects every vector lane"
,
"[math][vector]"
)
{
    components(Vector3f(2, 4, 6).lerp(Vector3f(6, 12, 18), Vector3f(0.5f)), Vector3f(4, 8, 12), 3);
    components(Vector4f(2, 4, 6, 8).lerp(Vector4f(6, 12, 18, 24), Vector4f(0.5f)), Vector4f(4, 8, 12, 16), 4);
}

TEST_CASE (
"Axis rotation uses degrees and preserves length"
,
"[math][rotation]"
)
{
    components(Vector3f(1, 0, 0).rotate(Vector3f(0, 0, 1), 90.0f), Vector3f(0, 1, 0), 3);
    const Vector3f v(2, -3, 5), axis = Vector3f(1, 2, -1).normalise();
    for (float angle : {-180.0f, -37.0f, 0.0f, 73.0f, 360.0f})
    {
        INFO("angle " << angle);
        const auto rotated = v.rotate(axis, angle);
        near(rotated.length(), v.length());
        components(rotated.rotate(axis, -angle), v, 3);
    }
}

TEST_CASE (
"Quaternion identity conjugate and Hamilton product"
,
"[math][quaternion]"
)
{
    const Quaternionf identity(0, 0, 0, 1), q(1, 2, 3, 4);
    components(q * identity, q, 4);
    components(identity * q, q, 4);
    components(q.Conjugate(), Quaternionf(-1, -2, -3, 4), 4);
    components(q * q.Conjugate(), Quaternionf(0, 0, 0, 30), 4);
    components(Quaternionf(1, 0, 0, 0) * Quaternionf(0, 1, 0, 0), Quaternionf(0, 0, 1, 0), 4);
    near(q.normalise().length(), 1);
    components(q.normalise() * q.normalise().Conjugate(), identity, 4);
}

TEST_CASE (
"Quaternion sign does not change represented rotation"
,
"[math][quaternion]"
)
{
    const Quaternionf q = Quaternionf(1, 2, 3, 4).normalise();
    const Vector3f v(2, -3, 5);
    const auto negative = q.negate();
    components(q * v * q.Conjugate(), negative * v * negative.Conjugate(), 4);
}

TEST_CASE (
"Matrix identity zero and arithmetic"
,
"[math][matrix]"
)
{
    const auto identity = Matrix4f::Identity();
    const auto zero = Matrix4f::Zero();
    const auto transform = Matrix4f::Translation(Vector3f(2, 3, 4));
    matrix(identity * transform, transform);
    matrix(transform * identity, transform);
    matrix(zero * transform, zero);
    matrix((transform + identity) - identity, transform);
    matrix((transform + 3.0f) - 3.0f, transform);
    components(identity * Vector4f(1, 2, 3, 4), Vector4f(1, 2, 3, 4), 4);
}

TEST_CASE (
"Matrix storage is column major and rows are addressable"
,
"[math][matrix]"
)
{
    Matrix4f m;
    m.Set(2, 1, 7);
    near(m.Get(2, 1), 7);
    near(m.m[9], 7);
    near(m.getRow(1)[2], 7);
}

TEST_CASE (
"Translation affects points but not directions"
,
"[math][matrix]"
)
{
    const auto t = Matrix4f::Translation(Vector3f(5, -2, 3));
    components(t * Vector4f(1, 2, 3, 1), Vector4f(6, 0, 6, 1), 4);
    components(t * Vector4f(1, 2, 3, 0), Vector4f(1, 2, 3, 0), 4);
}

TEST_CASE (
"Transform composition applies scale then rotation then translation"
,
"[math][matrix]"
)
{
    const auto t = Matrix4f::Transformation(Vector3f(5, -2, 3), Vector3f(0, 0, 90), Vector3f(2, 3, 4));
    components(t * Vector4f(1, 0, 0, 1), Vector4f(5, 0, 3, 1), 4);
    components(Matrix4f::Scaling(Vector3f(2, 3, 4)) * Vector4f(1, -2, 3, 1), Vector4f(2, -6, 12, 1), 4);
}

TEST_CASE (
"Matrix rotations use degrees and compose Y X Z"
,
"[math][matrix]"
)
{
    components(Matrix4f::Rotation(Vector3f(0, 90, 0)) * Vector4f(1, 0, 0, 0), Vector4f(0, 0, -1, 0), 4);
    const auto combined = Matrix4f::Rotation(Vector3f(30, 40, 50));
    matrix(combined, Matrix4f::Rotation(Vector3f(0, 40, 0)) * Matrix4f::Rotation(Vector3f(30, 0, 0)) * Matrix4f::Rotation(Vector3f(0, 0, 50)));
}

TEST_CASE (
"Invertible transforms round trip on both sides"
,
"[math][matrix]"
)
{
    for (int i = 1; i <= 20; ++i)
    {
        INFO("fixture " << i);
        const auto m = Matrix4f::Transformation(Vector3f(i, -2, 3), Vector3f(i * 7, -23, 41), Vector3f(2, 3, -4));
        const auto inverse = m.Invert();
        matrix(m * inverse, Matrix4f::Identity());
        matrix(inverse * m, Matrix4f::Identity());
        components(inverse * (m * Vector4f(2, -1, 3, 1)), Vector4f(2, -1, 3, 1), 4);
    }
}

TEST_CASE (
"Matrix copies own independent storage and moves preserve values"
,
"[math][matrix]"
)
{
    const auto original = Matrix4f::Translation(Vector3f(2, 3, 4));
    auto copy(original);
    copy.m[12] = 99;
    near(original.m[12], 2);
    Matrix4f assigned;
    assigned = original;
    assigned.m[13] = 99;
    near(original.m[13], 3);
    auto moved(std::move(copy));
    near(moved.m[12], 99);
}

TEST_CASE (
"Perspective maps negative Z near and far to minus one and plus one"
,
"[math][projection]"
)
{
    const auto p = Matrix4f::PerspectiveProjection(90, 2, 1, 11);
    const auto nearPoint = p * Vector4f(0, 0, -1, 1);
    const auto farPoint = p * Vector4f(0, 0, -11, 1);
    near(nearPoint.z / nearPoint.w, -1);
    near(farPoint.z / farPoint.w, 1);
    const auto edge = p * Vector4f(2, 1, -1, 1);
    near(edge.x / edge.w, 1);
    near(edge.y / edge.w, 1);
}

TEST_CASE (
"View matrix maps its supplied basis to view axes"
,
"[math][matrix]"
)
{
    const Vector3f forward(0, 0, 1), up(0, 1, 0);
    const auto view = Matrix4f::View(forward, up);
    components(view * Vector4f(forward.cross(up), 0), Vector4f(1, 0, 0, 0), 4);
    components(view * Vector4f(up, 0), Vector4f(0, 1, 0, 0), 4);
    components(view * Vector4f(forward, 0), Vector4f(0, 0, 1, 0), 4);
}

TEST_CASE (
"Vector4 split constructor preserves every component"
,
"[math][vector]"
)
{
    components(Vector4f(Vector2f(1, 2), 3, 4), Vector4f(1, 2, 3, 4), 4);
    components(Vector4f(Vector2f(-2, 5), -7, 0), Vector4f(-2, 5, -7, 0), 4);
}

TEST_CASE (
"Quaternion split constructor preserves every component"
,
"[math][quaternion]"
)
{
    components(Quaternionf(Vector2f(1, 2), 3, 4), Quaternionf(1, 2, 3, 4), 4);
    components(Quaternionf(Vector2f(-2, 5), -7, 0), Quaternionf(-2, 5, -7, 0), 4);
}

TEST_CASE (
"Asymmetric orthographic projection maps corners and center"
,
"[math][projection]"
)
{
    // As with perspective, near/far distances correspond to negative view-space Z.
    const auto p = Matrix4f::OrthographicProjection(2, 6, 3, 9, 1, 5);
    components(p * Vector4f(2, 3, -1, 1), Vector4f(-1, -1, -1, 1), 4);
    components(p * Vector4f(6, 9, -5, 1), Vector4f(1, 1, 1, 1), 4);
    components(p * Vector4f(4, 6, -3, 1), Vector4f(0, 0, 0, 1), 4);
    const auto symmetric = Matrix4f::OrthographicProjection(-2, 2, -3, 3, -4, 4);
    components(symmetric * Vector4f(2, 3, -4, 1), Vector4f(1, 1, 1, 1), 4);
}

TEST_CASE (
"Vector2 ordering compares matching components"
,
"[math][vector]"
)
{
    const Vector2f low(1, 5), high(2, 6), mixed(2, 4);
    CHECK(low < high);
    CHECK(high > low);
    CHECK(low <= high);
    CHECK(high >= low);
    CHECK(low <= low);
    CHECK(low >= low);
    CHECK_FALSE(low < low);
    CHECK_FALSE(low > low);
    CHECK_FALSE(low < mixed);
    CHECK_FALSE(low <= mixed);
    CHECK_FALSE(mixed > low);
    CHECK_FALSE(mixed >= low);
    CHECK(Vector2f(1, 5) <= Vector2f(2, 5));
    CHECK(Vector2f(2, 5) >= Vector2f(1, 5));
}

#if defined(PR_FAST_MATH)
TEST_CASE ("KNOWN SIMD scalar interpolation affects y", "[math][known-defect][!shouldfail]")
{
    CHECK(Vector3f(2, 4, 6).lerp(Vector3f(6, 12, 18), 0.5f).y == 8);
}
#else
TEST_CASE (
"Scalar interpolation affects every component"
,
"[math][vector]"
)
{
    components(Vector3f(2, 4, 6).lerp(Vector3f(6, 12, 18), 0.5f), Vector3f(4, 8, 12), 3);
    components(Vector4f(2, 4, 6, 8).lerp(Vector4f(6, 12, 18, 24), 0.5f), Vector4f(4, 8, 12, 16), 4);
}

#endif

TEST_CASE (
"Vector component arithmetic keeps distinct lanes independent"
,
"[math][vector]"
)
{
    const Vector3f a(2, -3, 8), b(4, 2, -2);
    components(a + b, Vector3f(6, -1, 6), 3);
    components(a - b, Vector3f(-2, -5, 10), 3);
    components(a * b, Vector3f(8, -6, -16), 3);
    components(a / b, Vector3f(0.5f, -1.5f, -4), 3);
    components(Vector4f(1, 2, 3, 4) * Vector4f(4, -3, 2, -1), Vector4f(4, -6, 6, -4), 4);
    components(Vector2f(2, -3) + Vector2f(7, 5), Vector2f(9, 2), 2);
}

#if !defined(PR_FAST_MATH)
TEST_CASE (
"Integer and double vector templates retain their component types"
,
"[math][vector]"
)
{
    const Vector3i a(2, -3, 8), b(4, 2, -2);
    CHECK(a + b == Vector3i(6, -1, 6));
    CHECK(a.cross(b) == Vector3i(-10, 36, 16));
    const Vector3d precise(1.000000001, 2.000000002, 3.000000003);
    CHECK_THAT((precise * 2.0).x, WithinAbs(2.000000002, 1e-12));
    CHECK_THAT(precise.normalise().length(), WithinAbs(1.0, 1e-12));
}
#endif