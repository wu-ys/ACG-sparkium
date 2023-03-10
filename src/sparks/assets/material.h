#pragma once
#include "cstdint"
#include "glm/glm.hpp"
#include "sparks/assets/util.h"
#include "sparks/assets/texture.h"
#include "sparks/assets/hit_record.h"

namespace sparks {

enum MaterialType : int {
    MATERIAL_TYPE_EMISSION = 0,
    MATERIAL_TYPE_LAMBERTIAN = 1,
    MATERIAL_TYPE_SPECULAR = 2,
    MATERIAL_TYPE_PRINCIPLED = 3,
    MATERIAL_TYPE_TRANSMISSIVE = 4,
    MATERIAL_TYPE_BTDF_TEST = 5
};

class Scene;

struct Material {
  glm::vec3 albedo_color{0.8f};
  int albedo_texture_id{0};
  glm::vec3 emission{0.0f};
  float emission_strength{1.0f};
  float alpha{1.0f};
  float tranmissive_ratio{1.5f};
  MaterialType material_type{MATERIAL_TYPE_LAMBERTIAN};
  float reserve[1]{};
  Material() = default;
  explicit Material(const glm::vec3 &albedo);
  Material(Scene *scene, const tinyxml2::XMLElement *material_element);

  glm::vec3 BRDF(const HitRecord& hit_record,
                 const glm::vec3 in_direction,
                 const glm::vec3 out_direction) const;
};
}  // namespace sparks
