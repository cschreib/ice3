#ifndef AXISALIGNEDBOX_HPP
#define AXISALIGNEDBOX_HPP

#include <lxgui/utils.hpp>
#include "vector3.hpp"
#include <iosfwd>

struct ray
{
    ray() {}
    ray(const vector3f& o, const vector3f& d) : mOrigin(o), mDirection(d) {}

    vector3f mOrigin, mDirection;
};


/// An axis aligned bounding box (AABB).
class axis_aligned_box
{
public :

    /// Enumerates all faces of an axis aligned box
    enum face
    {
        LEFT = 0,
        RIGHT,
        FRONT,
        BACK,
        TOP,
        BOTTOM
    };

    /// Default constructor.
    /** \note The default box is infinite.
    */
    axis_aligned_box();

    /// Constructor.
    /** \param mMin The first point of the box
    *   \param mMax The second point of the box
    *   \note mMin's coordinates must be inferior to mMax's.
    *         These two points are opposite corners of the box.
    */
    axis_aligned_box(const vector3f& mMin, const vector3f& mMax);

    /// Destructor.
    ~axis_aligned_box();

    /// Checks if this box contains another.
    /** \param mBox The other box
    *   \return 'true' if this box contains the other
    *   \note The box can be partially contained.
    */
    bool contains(const axis_aligned_box& mBox) const;

    /// Checks if this box contains a point.
    /** \param mPoint The point
    *   \return 'true' if this box contains the point
    */
    bool contains(const vector3f& mPoint) const;

    /// Enlarges this box to contain the provided point.
    /** \param mPoint The point
    */
    void include(const vector3f& mPoint);

    /// Checks if this box is infinite in all directions.
    /** \return 'true' if this box is infinite in all directions
    */
    bool is_infinite() const;

    /// Checks if a ray intersects this box.
    /** \param mRay               The ray
    *   \param[out] mIntersection The intersection point
    *   \return 'true' if the ray has intersected the box
    *   \note If the ray origin lies inside the bounding box, then this function
    *         automatically returns 'true', and mIntersection is set to mRay.mOrigin.
    */
    bool find_ray_intersection(const ray& mRay, vector3f& mIntersection) const;

    /// Checks if a ray intersects this box.
    /** \param mRay               The ray
    *   \param[out] mIntersection The intersection point
    *   \param[out] mFace         The face of the box on which relies the intersection
    *   \return 'true' if the ray has intersected the box
    *   \note If the ray origin lies inside the bounding box, then this function
    *         automatically returns 'true', and mIntersection is set to mRay.mOrigin.
    *         mFace is left unchanged.
    */
    bool find_ray_intersection(const ray& mRay, vector3f& mIntersection, face& mFace) const;

    /// Get the intersection of a ray and any of the faces of this bounding box.
    /** \param mRay               The ray
    *   \param[out] mIntersection The intersection point
    *   \param[out] mFace         The face of the box on which relies the intersection
    *   \note The ray origin is assumed to lie *inside* this bounding box.
    */
    void get_inside_ray_intersection(const ray& mRay, vector3f& mIntersection, face& mFace) const;

    /// Get the intersection of a ray and any of the faces of this bounding box, ignoring a face.
    /** \param mRay               The ray
    *   \param[out] mIntersection The intersection point
    *   \param[out] mFace         The face of the box on which relies the intersection
    *   \param mIgnore            The face to ignore
    *   \note The ray origin is assumed to lie *inside* this bounding box.
    */
    void get_inside_ray_intersection_ignore_face(const ray& mRay, vector3f& mIntersection, face& mFace, face mIgnore) const;

    vector3f operator[] (size_t uiIndex) const;

    axis_aligned_box operator + (const vector3f& mAdd) const;
    axis_aligned_box operator - (const vector3f& mAdd) const;

    void operator += (const vector3f& mAdd);
    void operator -= (const vector3f& mAdd);

    vector3f min, max;
};

std::ostream& operator << (std::ostream& o, const axis_aligned_box& mBox);

#endif
