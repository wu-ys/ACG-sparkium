
#include "constants.glsl"

struct Material {
  vec3 albedo_color;
  int albedo_texture_id;
  vec3 emission;
  float emission_strength;
  float alpha;
  float transmissive_ratio;
  uint material_type;
};

#define MATERIAL_TYPE_EMISSION 0
#define MATERIAL_TYPE_LAMBERTIAN 1
#define MATERIAL_TYPE_SPECULAR 2
#define MATERIAL_TYPE_PRINCIPLED 3
#define MATERIAL_TYPE_TRANSMISSIVE 4
#define MATERIAL_TYPE_BTDFTEST 5

vec3 bsdf(Material material,
          vec3 in_direction, 
          vec3 out_direction, 
          vec3 normal_direction) {
  switch (material.material_type) {
    case MATERIAL_TYPE_LAMBERTIAN: {
      if (dot(-in_direction, normal_direction) * dot(out_direction, normal_direction) >= 0)
        return material.albedo_color * INV_PI;
      else 
        return vec3(0.0);
    }
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
    case MATERIAL_TYPE_SPECULAR: {
      if ((length(in_direction) == length(out_direction)) && (normalize(in_direction + out_direction) == normal_direction))
        return vec3(1.0);
      else 
        return vec3(0.0);
    }
    case MATERIAL_TYPE_TRANSMISSIVE: {
      return vec3(0.0);
    }
    case MATERIAL_TYPE_BTDFTEST: {
      float fr, ft;
      float fresnel;
      float shadow;

      vec3 half_direction;

      float c = dot(in_direction, normal_direction);
      float eta = material.transmissive_ratio;

      if (c > 0) {
        // from material into air
        // eta = eta_t / eta_i
        eta = 1 / material.transmissive_ratio;
      }

      if (dot(-in_direction, normal_direction) * dot(out_direction, normal_direction) > 0) {
        // reflective
        half_direction = normalize(-in_direction + out_direction);
      }
      else {
        // transmissive
        half_direction = -normalize(-in_direction + eta*out_direction);
      }

      float cos_theta_m = dot(half_direction, normal_direction);
      float cos_theta_i = dot(-in_direction, normal_direction);
      float cos_theta_o = dot(out_direction, normal_direction);

      float alpha = material.alpha;
      
      // fresnel term
      c = abs(dot(-in_direction, half_direction));
      float g = sqrt(c*c - 1 + eta*eta);
      fresnel = 1/2 * (g-c) * (g-c) / (g+c) / (g+c) * (1 + (c*g + c*c -1) * (c*g + c*c - 1) / (c*g - c*c - 1) / (c*g - c*c - 1));

      // distribution term
      float microfacet_pdf;
      if (cos_theta_m > 0)
        microfacet_pdf = alpha * alpha *INV_PI / pow(cos_theta_m, 4) / pow((alpha*alpha - 1 + 1 / cos_theta_m / cos_theta_m),2); 
      else 
        microfacet_pdf = 0;
      
      // shadow term
      float g1;
      if (dot(-in_direction, half_direction) * dot(-in_direction, normal_direction) > 0)
        g1 = 2 / (1+sqrt(1 + alpha*alpha * (1/cos_theta_i/cos_theta_i - 1)));
      else 
        g1 = 0;

      float g2;
      if (dot(out_direction, half_direction) * dot(out_direction, normal_direction) > 0)
        g2 = 2 / (1+sqrt(1 + alpha*alpha * (1/cos_theta_o/cos_theta_o - 1)));
      else 
        g2 = 0;
      shadow = g1*g2;

      if (dot(-in_direction, normal_direction) * dot(out_direction, normal_direction) > 0) {
        // reflective
        fr = fresnel * shadow * microfacet_pdf / 4 / abs(dot(in_direction, normal_direction)) / abs(dot(out_direction, normal_direction));
        return vec3(fr);
      }
      else {
        // transmissive
        ft = abs(dot(-in_direction, half_direction)) * abs(dot(out_direction, half_direction)) / abs(cos_theta_i) / abs(cos_theta_o) * eta * eta * (1-fresnel) * shadow * microfacet_pdf / pow((dot(half_direction, -in_direction) + eta * dot(half_direction, out_direction)), 2);
        return vec3(ft);
      }
    }
    default: {
      return vec3(0.5);

    }

  }
}