#include "sparks/renderer/path_tracer.h"

#include "sparks/util/util.h"

namespace sparks {
PathTracer::PathTracer(const RendererSettings *render_settings,
                       const Scene *scene) {
  render_settings_ = render_settings;
  scene_ = scene;
}

glm::vec3 PathTracer::SampleRay(glm::vec3 origin,
                                glm::vec3 direction,
                                int x,
                                int y,
                                int sample) const {
  glm::vec3 throughput{1.0f};
  glm::vec3 radiance{0.0f};
  HitRecord hit_record;
  const int max_bounce = render_settings_->num_bounces;
  const float rr_prob = render_settings_->rr_probability;
  std::mt19937 rd(sample ^ x ^ y);
    
    //   for (int i = 0; i < max_bounce; i++) {
    //     auto t = scene_->TraceRay(origin, direction, 1e-3f, 1e4f, &hit_record);
    //     if (t > 0.0f) {
    //       auto &material =
    //           scene_->GetEntity(hit_record.hit_entity_id).GetMaterial();
    //       if (material.material_type == MATERIAL_TYPE_EMISSION) {
    //         radiance += throughput * material.emission * material.emission_strength;
    //         break;
    //       } else {
    //         throughput *=
    //             material.albedo_color *
    //             glm::vec3{scene_->GetTextures()[material.albedo_texture_id].Sample(
    //                 hit_record.tex_coord)};
    //         origin = hit_record.position;
    //         direction = scene_->GetEnvmapLightDirection();
    //         radiance += throughput * scene_->GetEnvmapMinorColor();
    //         throughput *=
    //             std::max(glm::dot(direction, hit_record.normal), 0.0f) * 2.0f;
    //         if (scene_->TraceRay(origin, direction, 1e-3f, 1e4f, nullptr) < 0.0f) {
    //           radiance += throughput * scene_->GetEnvmapMajorColor();
    //         }
    //         break;
    //       }
    //     } else {
    //       radiance += throughput * glm::vec3{scene_->SampleEnvmap(direction)};
    //       break;
    //     }
    //   }
    //   return radiance;

  std::uniform_real_distribution<> u01_dis(0, 1);
  int bounces = 0;
  while (true) {
    auto t = scene_->TraceRay(origin, direction, 1e-3f, 1e4f, &hit_record);
    if (t <= 0) {
      radiance = throughput * glm::vec3{scene_->SampleEnvmap(direction)};
      break;
    }

    if (u01_dis(rd) > rr_prob) {
      radiance = glm::vec3{0.0f};
      // std::cout << "Terminated!\n";                       
      break;
    }

    const Material & material = scene_->GetEntity(hit_record.hit_entity_id).GetMaterial();
    const Texture & texture = scene_->GetTextures()[material.albedo_texture_id];

    if (material.material_type == MATERIAL_TYPE_EMISSION) {
      radiance += throughput * material.emission /* material.emission_strength*/;
      break;
    }

    if (glm::dot(direction, hit_record.normal) > 0)
      hit_record.normal = - hit_record.normal;

    // randomly sample an outward direction
    std::normal_distribution<> n_0_1_dis(0,1);
    //std::uniform_real_distribution<> u_0_2pi_dis(0,2*PI);
    //std::uniform_real_distribution<> u_0_pi2_dis(0,PI/2);
    // u01_dis(0, 1);
    glm::vec3 out_direction;
    do {
      out_direction.x = n_0_1_dis(rd);
      out_direction.y = n_0_1_dis(rd);
      out_direction.z = n_0_1_dis(rd);
    } while (out_direction.length() == 0);
    
    out_direction = glm::normalize(out_direction);
    if (glm::dot(out_direction, hit_record.normal) < 0) 
      out_direction = -out_direction;

    // std::cout << "Bounces: " << bounces++ << std::endl;

    glm::vec3 brdf = material.BRDF(texture, hit_record, direction, out_direction);
    float cosine = glm::dot(glm::normalize(out_direction), glm::normalize(hit_record.normal));
    if (cosine <= 0) {
      radiance = glm::vec3{1.0f};
      break;
    }
   //  std::cout << cosine << std::endl;

    throughput *= (brdf * cosine / rr_prob * (2*PI)  /*pdf(out_direction)*/  );
    // std::cout << throughput.x << ',' << throughput.y << ',' << throughput.z << std::endl;

    origin = hit_record.position;
    direction = out_direction;
  }

  return radiance;
}
}  // namespace sparks
