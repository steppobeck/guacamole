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

@include "shaders/common/header.glsl"

// input
layout(location=0) in vec3 gua_in_position;
layout(location=1) in vec2 gua_in_texcoord;
layout(location=2) in vec3 gua_in_normal;

// uniforms
@include "shaders/uber_shaders/common/gua_camera_uniforms.glsl"

uniform mat4 gua_light_shadow_map_projection_view_matrix;

// output
out vec3  gua_lightinfo1;
out vec3  gua_lightinfo2;
out float gua_lightinfo3;
out mat4  gua_lightinfo4;

// BASE LIGHTING CALCULATIONS --------------------------------------------------

// base lighting calculations for point lights
subroutine void CalculateLightType();
subroutine uniform CalculateLightType compute_light;

subroutine( CalculateLightType )
void gua_calculate_point_light() {

  vec3  light_position = (gua_model_matrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  float light_radius = length(light_position - (gua_model_matrix * vec4(0.0, 0.0, 1.0, 1.0)).xyz);

  gua_lightinfo1 = light_position;
  gua_lightinfo2 = vec3(0.0, 0.0, 0.0);
  gua_lightinfo3 = light_radius;
}

// base lighting calculations for spot lights
subroutine( CalculateLightType )
void gua_calculate_spot_light() {

  const mat4 bias = mat4(0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 0.5, 0.0,
  0.5, 0.5, 0.5, 1.0);

  vec3 light_position   = (gua_model_matrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 beam_direction   = (gua_model_matrix * vec4(0.0, 0.0, -1.0, 1.0)).xyz - light_position;
  float half_beam_angle = dot(normalize((gua_model_matrix * vec4(0.0, 0.5, -1.0, 0.0)).xyz), normalize(beam_direction));
  mat4 shadow_map_coords_mat = bias * gua_light_shadow_map_projection_view_matrix;

  gua_lightinfo1 = light_position;
  gua_lightinfo2 = beam_direction;
  gua_lightinfo3 = half_beam_angle;
  gua_lightinfo4 = shadow_map_coords_mat;
}

// main ------------------------------------------------------------------------
void main() {
  compute_light();
  vec3 position = (gua_model_matrix * vec4(gua_in_position, 1.0)).xyz;
  gl_Position = gua_projection_matrix * gua_view_matrix * vec4(position, 1.0);
}
