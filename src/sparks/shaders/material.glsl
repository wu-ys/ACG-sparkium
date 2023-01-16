
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

float Fresnel_term(vec3 in_direction, vec3 half_direction, float eta) {
  float fresnel;
  float c = abs(dot(in_direction, half_direction));
  float gsquare = c*c - 1 + eta*eta;
  if (gsquare > 0) {
    float g = sqrt(gsquare);
    fresnel = 0.5 * (g-c) * (g-c) / (g+c) / (g+c) * (1 + (c*g + c*c -1) * (c*g + c*c - 1) / (c*g - c*c + 1) / (c*g - c*c + 1));
  } else {
    fresnel = 1;
  }

  return fresnel;
}

float Microfacet_pdf_term(vec3 half_direction, vec3 normal_direction, float alpha) {
  float microfacet_pdf;
  float cos_theta_mn = dot(half_direction, normal_direction);
  if (cos_theta_mn > 0)
    microfacet_pdf = alpha * alpha *INV_PI / pow(cos_theta_mn, 4) / pow((alpha*alpha - 1 + 1 / cos_theta_mn / cos_theta_mn),2); 
  else 
    microfacet_pdf = 0;
  return microfacet_pdf;
}

float Shadow_term(vec3 in_direction, vec3 out_direction, vec3 normal_direction, vec3 half_direction, float alpha) {
  float cos_theta_in = dot(-in_direction, normal_direction);
  float cos_theta_on = dot(out_direction, normal_direction);
  float cos_theta_im = dot(-in_direction, half_direction);
  float cos_theta_om = dot(out_direction, half_direction);
  float g1;
  if (cos_theta_im * cos_theta_in > 0)
    g1 = 2 / (1+sqrt(1 + alpha*alpha * (1/cos_theta_in/cos_theta_in - 1)));
  else 
    g1 = 0;

  float g2;
  if (cos_theta_om * cos_theta_on > 0)
    g2 = 2 / (1+sqrt(1 + alpha*alpha * (1/cos_theta_on/cos_theta_on - 1)));
  else 
    g2 = 0;
  float shadow = g1*g2;
  return shadow;
}

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
      float fr = 0;
      float ft = 0;
      float fresnel, microfacet_pdf, shadow;

      vec3 half_direction;

      float eta = material.transmissive_ratio;
      float alpha = material.alpha;

      // 1. determine the transmissive ratio
      if (dot(in_direction, normal_direction) > 0) {
        // from material into air
        // eta = eta_t / eta_i
        eta = 1 / eta;
      }

      // 2. determine transmission or reflection
      // calculate the half vector direction
      bool is_reflect;
      if (dot(-in_direction, normal_direction) * dot(out_direction, normal_direction) > 0) {
        is_reflect = true;
        half_direction = normalize(-in_direction + out_direction);
        half_direction = sign(dot(half_direction, normal_direction)) * half_direction;      
      } else {
        is_reflect = false;
        half_direction = -normalize(-in_direction + eta*out_direction);
        half_direction = sign(dot(half_direction, normal_direction)) * half_direction;        
      }

      // reflective
      // fresnel term
      fresnel = Fresnel_term(in_direction, half_direction, eta);

      // distribution term
      microfacet_pdf = Microfacet_pdf_term(half_direction, normal_direction, alpha);
      
      // shadow term
      shadow = Shadow_term(in_direction, out_direction, normal_direction, half_direction, alpha);

      if (is_reflect) {

      // reflective term
      fr = fresnel * shadow * microfacet_pdf / 4 / abs(dot(in_direction, normal_direction)) /*/ abs(dot(out_direction, normal_direction))*/;
      } else {
        // transmissive term
        ft = abs(dot(-in_direction, half_direction)) 
            * abs(dot(out_direction, half_direction)) 
            / abs(dot(-in_direction, normal_direction)) 
            /*/ abs(dot(out_direction, normal_direction)) */
            * eta * eta * (1-fresnel) * shadow * microfacet_pdf 
            / pow((dot(half_direction, -in_direction) 
            + eta * dot(half_direction, out_direction)), 2);
      }

      return vec3(fr+ft);
    }
    default: {
      return vec3(0.5);

    }

  }
}