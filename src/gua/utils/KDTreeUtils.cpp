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

#include <gua/utils/KDTree.hpp>

#include <iostream>
#include <algorithm>

 // external headers
#if ASSIMP_VERSION == 3
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#else
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#endif

namespace gua {

// Ray -------------------------------------------------------------------------

const float Ray::END(std::numeric_limits<float>::max());

Ray::Ray() : origin_(), direction_(), t_max_(-1.f) {}

Ray::Ray(math::vec3 const& origin, math::vec3 const& direction, float t_max)
    : origin_(origin), direction_(direction), t_max_(t_max) {}

std::pair<float, float> Ray::intersect(
    math::BoundingBox<math::vec3> const& box) const {

  math::vec3 t1((box.min - origin_) / direction_);
  math::vec3 t2((box.max - origin_) / direction_);

  math::vec3 tmin1(
      std::min(t1[0], t2[0]), std::min(t1[1], t2[1]), std::min(t1[2], t2[2]));
  math::vec3 tmax1(
      std::max(t1[0], t2[0]), std::max(t1[1], t2[1]), std::max(t1[2], t2[2]));

  float tmin = std::max(std::max(tmin1[0], tmin1[1]), tmin1[2]);
  float tmax = std::min(std::min(tmax1[0], tmax1[1]), tmax1[2]);

  if (tmax >= tmin) {
    // there are two intersections
    if (tmin > 0.0 && tmax < t_max_)
      return std::make_pair(tmin, tmax);

    // there is only one intersection, the ray ends inside the box
    else if (tmin > 0.0)
      return std::make_pair(tmin, END);

    // there is only one intersection, the ray starts inside the box
    else
      return std::make_pair(END, tmax);
  }

  // there is no intersection

  return std::make_pair(END, END);
}

Ray const Ray::intersection(math::BoundingBox<math::vec3> const& box) const {
  auto hits(intersect(box));

  // there are to hits -> clamp ray on both sides
  if (hits.first != END && hits.first != END)
    return Ray(origin_ + direction_ * hits.first,
               direction_,
               hits.second - hits.first);

  // the ray ends inside the box -> clamp the origin
  if (hits.first != END)
    return Ray(
        origin_ + direction_ * hits.first, direction_, t_max_ - hits.first);

  // the ray starts inside the box -> clamp the end
  if (hits.second != END)
    return Ray(origin_, direction_, hits.second);

  // there is no intersection
  return Ray();
}

// Triangle --------------------------------------------------------------------

Triangle::Triangle() : face_id_(0) {}

Triangle::Triangle(unsigned face_id)
  : face_id_(face_id) {}

float Triangle::intersect(aiMesh* mesh, Ray const& ray) const {
  std::vector<math::vec3> points(3);
  // math::vec3 normal(0, 0, 0);
  for (unsigned i = 0; i < 3; ++i) {
    points[i] = get_vertex(mesh, i);
  }

  // Find Triangle Normal
  math::vec3 normal =
      scm::math::cross(points[1] - points[0], points[2] - points[0]);
  scm::math::normalize(normal);

  // Find distance from LP1 and LP2 to the plane defined by the triangle
  float dist1 = scm::math::dot(ray.origin_ - points[0], normal);
  float dist2 =
      scm::math::dot(ray.origin_ + ray.direction_ - points[0], normal);

  if ((dist1 * dist2) >= 0.0f) {
    return Ray::END;
  }  // line doesn't cross the triangle.

  if (dist1 == dist2) {
    return Ray::END;
  }  // line and plane are parallel

  // Find point on the line that intersects with the plane
  float t = -dist1 / (dist2 - dist1);

  if (t > ray.t_max_) {
    return Ray::END;
  }  // intersection is too far away

  math::vec3 intersection = ray.origin_ + (ray.direction_) * t;

  // Find if the interesection point lies inside the triangle by testing it
  // against all edges
  math::vec3 test = scm::math::cross(normal, points[1] - points[0]);
  if (scm::math::dot(test, intersection - points[0]) < 0.0f) {
    return Ray::END;
  }

  test = scm::math::cross(normal, points[2] - points[1]);
  if (scm::math::dot(test, intersection - points[1]) < 0.0f) {
    return Ray::END;
  }

  test = scm::math::cross(normal, points[0] - points[2]);
  if (scm::math::dot(test, intersection - points[0]) < 0.0f) {
    return Ray::END;
  }

  return t;
}

math::vec3 Triangle::get_vertex(aiMesh* mesh, unsigned vertex_id) const {
  math::vec3 vertex;
  if (vertex_id < 3) {
    vertex = math::vec3(
                        mesh->mVertices[mesh->mFaces[face_id_].mIndices[vertex_id]].x,
                        mesh->mVertices[mesh->mFaces[face_id_].mIndices[vertex_id]].y,
                        mesh->mVertices[mesh->mFaces[face_id_].mIndices[vertex_id]].z);
  }

  return vertex;
}

math::vec3 Triangle::get_normal(aiMesh* mesh) const {
  math::vec3 normal;
  if (mesh->HasNormals()) {
    for (unsigned i = 0; i < 3; ++i) {
    normal += math::vec3(
                        mesh->mNormals[mesh->mFaces[face_id_].mIndices[i]].x,
                        mesh->mNormals[mesh->mFaces[face_id_].mIndices[i]].y,
                        mesh->mNormals[mesh->mFaces[face_id_].mIndices[i]].z);
    }
  }

  return normal/3;
}

math::vec3 Triangle::get_normal_interpolated(aiMesh* mesh, math::vec3 const& position) const {
  /// TODO: actually interpolate
  return get_normal(mesh);
}

math::vec2 Triangle::get_texture_coords_interpolated(aiMesh* mesh, math::vec3 const& position) const {
  /// TODO: actually interpolate
  math::vec2 tex_coords;
  if (mesh->HasTextureCoords(0)) {
    for (unsigned i = 0; i < 3; ++i) {
    tex_coords += math::vec2(
                        mesh->mTextureCoords[0][mesh->mFaces[face_id_].mIndices[i]].x,
                        mesh->mTextureCoords[0][mesh->mFaces[face_id_].mIndices[i]].y);
    }
  }

  return tex_coords/3;
}


}
