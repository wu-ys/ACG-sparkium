
#include "constants.glsl"

struct Material {
  vec3 albedo_color;
  int albedo_texture_id;
  vec3 emission;
  float emission_strength;
  float alpha;
  uint material_type;
};

#define MATERIAL_TYPE_LAMBERTIAN 0
#define MATERIAL_TYPE_SPECULAR 1
#define MATERIAL_TYPE_TRANSMISSIVE 2
#define MATERIAL_TYPE_PRINCIPLED 3
#define MATERIAL_TYPE_EMISSION 4

vec3 bsdf(Material material,
          vec3 in_direction, 
          vec3 out_direction, 
          vec3 normal_direction) {
  switch (material.material_type) {
    case MATERIAL_TYPE_LAMBERTIAN:
      return material.albedo_color * INV_PI;
    case MATERIAL_TYPE_PRINCIPLED: {
      // Phong model
      float kd = 0.2;
      float ks = 1-kd;
      int n = 8;

      vec3 median = -dot(in_direction, normal_direction) * normal_direction;
      vec3 spe_out_direction = 2*median + in_direction;
      float cosine = max(0,dot(out_direction, spe_out_direction));

      float t1 = kd * INV_PI;
      float t2 = ks * (n+2) * INV_PI / 2 * pow(cosine,n);

      return vec3(t1+t2) * material.albedo_color;
    }

  }
}