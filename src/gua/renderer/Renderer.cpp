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

// class header
#include <gua/renderer/Renderer.hpp>

// guacamole headers
#include <memory>
#include <gua/platform.hpp>
#include <gua/scenegraph.hpp>
#include <gua/renderer/RenderClient.hpp>
#include <gua/renderer/Pipeline.hpp>
#include <gua/utils.hpp>
#include <gua/memory.hpp>

namespace gua {

std::shared_ptr<Renderer::const_render_vec_t> garbage_collected_copy(
    std::vector<SceneGraph const*> const& scene_graphs) {
  auto sgs = std::make_shared<Renderer::render_vec_t>();
  for (auto graph : scene_graphs) {
    sgs->push_back(gua::make_unique<SceneGraph>(*graph));
  }
  return sgs;
}

Renderer::Renderer(std::vector<Pipeline*> const& pipelines)
    : render_clients_(),
      application_fps_(20) {
  application_fps_.start();
  for (auto& pipeline : pipelines) {
    auto fun = [pipeline, this](
        std::shared_ptr<const_render_vec_t> const & sg, float render_fps) {
      pipeline->process(*sg, this->application_fps_.fps, render_fps);
    };

    render_clients_.push_back(gua::make_unique<renderclient_t>(fun));
  }
}

void Renderer::queue_draw(std::vector<SceneGraph const*> const& scene_graphs) {
  for (auto graph : scene_graphs) {
    graph->update_cache();
  }
  auto sgs = garbage_collected_copy(scene_graphs);
  for (auto& rclient : render_clients_) {
    rclient->queue_draw(sgs);
  }
  application_fps_.step();
}

}
