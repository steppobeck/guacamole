/******************************************************************************
 * guacamole - delicious VR                                                   *
 *                                                                            *
 * Copyright: (c) 2011-2013 Bauhaus-Universität Weimar                        *
 * Contact:   felix.lauer@uni-weimar.de / simon.schneegans@uni-weimar.de      *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the Free *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   *
 * for more details.                                                          *
 *                                                                            *
 * You should have received a copy of the GNU General Public License along    *
 * with this program. If not, see <http://www.gnu.org/licenses/>.             *
 *                                                                            *
 ******************************************************************************/

#ifndef GUA_PLANE_SHAPE_HPP
#define GUA_PLANE_SHAPE_HPP

// guacamole headers
#include <gua/physics/CollisionShape.hpp>
#include <gua/math.hpp>

class btStaticPlaneShape;

namespace gua {
namespace physics {

/**
 * A class representing an infinite static collision plane.
 *
 * The plane shape can only be used for static rigid bodies.
 */
class PlaneShape : public CollisionShape {
 public:

  /**
   * Constructor.
   *
   * Creates a new plane shape with given normal vector components and
   * the plane constant (the distance of the plane's origin).
   *
   * \param x_normal The plane normal for X axis.
   * \param y_normal The plane normal for Y axis.
   * \param z_normal The plane normal for Z axis.
   * \param plane_constant The distance of the plane's origin.
   */
  PlaneShape(float x_normal,
             float y_normal,
             float z_normal,
             float plane_constant);

  /**
   * Destructor.
   *
   * Deletes the plane shape and frees all associated data.
   */
  virtual ~PlaneShape();

  math::vec3 const& get_normal() const;

  void set_normal(math::vec3 const& normal);

  float get_plane_constant() const;

  void set_plane_constant(float plane_constant);

 private:

  virtual void construct_dynamic(btCompoundShape* bullet_shape,
                                 const btTransform& base_transform);

  virtual btCollisionShape* construct_static();

  btStaticPlaneShape* shape_;
  math::vec3 normal_;
  float plane_constant_;

};

}
}

#endif  // GUA_PLANE_SHAPE_HPP
