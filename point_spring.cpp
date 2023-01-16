#include <iostream>

#include "external/grassland/external/glm/glm/glm.hpp"
#include <random>
#include <iostream>
#include <fstream>
#include <string>
using namespace glm;

const float INV_PI = 0.31830988618379067;
const float frac_coeff = 0.2;
const vec2 supporting_x = vec2{9.95f, 160.05f};
const vec2 supporting_z = vec2{349.95f, 580.05f};
const vec2 supporting_y = vec2{0.0f, 50.0f};
float hook = 30.0;
const float damp_factor = 0.99;

class spring_mass {
private:
    std::vector<glm::vec3> position_;
    std::vector<glm::vec3> force_;
    std::vector<glm::vec3> velocity_;
    std::vector<glm::uvec2> springs_;
    std::vector<float> o_length;
    std::vector<glm::uvec3> faces_;
    float mass;
    int size_x;
    int size_z;
public:
    spring_mass(vec3 origin, float x, float z, float grid) {
        int grid_x = floor(x / grid);
        size_x = grid_x + 1;
        int grid_z = floor(z / grid);
        size_z = grid_z + 1;
        mass = 1e4 / grid_x / grid_z;
        std::cout << mass << std::endl;

        for (int i = 0; i <= grid_x; i++) {
            for (int j = 0; j <= grid_z; j++) {
                int idx = position_.size();
                position_.push_back(origin + vec3{x * float(i) / grid_x, 0.0f, z * float(j) / grid_z});
                force_.push_back(vec3{0.0f, -9.8f, 0.0f});
                velocity_.push_back(vec3{0.0f,0.0f,0.0f});

                if (i > 0) {
                    springs_.push_back(uvec2(idx-grid_z-1, idx));
                    o_length.push_back(length(position_[idx] - position_[idx-grid_z-1]));
                }
                if (i > 1) {
                    springs_.push_back(uvec2(idx - 2*grid_z - 2, idx));

                    o_length.push_back(length(position_[idx] - position_[idx-2*grid_z-2]));
                }
                if (j > 0) {
                    springs_.push_back(uvec2(idx-1, idx));
                    o_length.push_back(length(position_[idx] - position_[idx-1]));
                }
                if (j > 1) {
                    springs_.push_back(uvec2(idx-2, idx));
                    o_length.push_back(length(position_[idx] - position_[idx-2]));
                }
                if (i > 0 && j > 0) {
                    faces_.push_back(
                        uvec3(idx, idx - grid_z - 2, idx - grid_z - 1));
                    faces_.push_back(
                        uvec3(idx, idx - 1, idx - grid_z - 2));
                    springs_.push_back(uvec2(idx - grid_z - 2, idx));
                    o_length.push_back(length(position_[idx] - position_[idx-grid_z-2]));
                    springs_.push_back(uvec2(idx - 1, idx - grid_z - 1));
                    o_length.push_back(length(position_[idx-1] - position_[idx-grid_z-1]));
                }
            }
        }

        std::cerr << "mesh generated!" << std::endl;


    }

    void evolve(float dt, float hook) {
        for (int i = 0; i < position_.size(); i++) {
            float supporting_force = 0;
            if (supporting_x[0] <= position_[i].x &&
                position_[i].x <= supporting_x[1] &&
                supporting_z[0] <= position_[i].z &&
                position_[i].z <= supporting_z[1] &&
                position_[i].y <= supporting_y[1] + 0.2f &&
                position_[i].y >= supporting_y[1] - 0.2f &&
                force_[i].y < 0) {

                supporting_force = -force_[i].y;
                force_[i].y = 0;
                velocity_[i].y = 0;
                if (length(velocity_[i]) == 0) {

                    if (length(force_[i]) <= frac_coeff * supporting_force) {
                        force_[i] = vec3{0.0f};
                    } else {
                        force_[i] -= frac_coeff * supporting_force * normalize(force_[i]);
                    }
                } else {
                    force_[i] -= frac_coeff * supporting_force * normalize(velocity_[i]);
                }
            }

            if (supporting_x[0] <= position_[i].x &&
                position_[i].x <= supporting_x[1] &&
                supporting_y[0] <= position_[i].y &&
                position_[i].y <= supporting_y[1] &&
                position_[i].z <= supporting_z[0] + 10.0f &&
                position_[i].z >= supporting_z[0] - 0.02f &&
                velocity_[i].z > 0)
                    velocity_[i].z = 0.0f;

            if (supporting_x[0] <= position_[i].x &&
                position_[i].x <= supporting_x[1] &&
                supporting_y[0] <= position_[i].y &&
                position_[i].y <= supporting_y[1] &&
                position_[i].z <= supporting_z[1] + 0.02f &&
                position_[i].z >= supporting_z[1] - 10.0f && 
                velocity_[i].z < 0)
                    velocity_[i].z = 0.0f;

            if (supporting_z[0] <= position_[i].z &&
                position_[i].z <= supporting_z[1] &&
                supporting_y[0] <= position_[i].y &&
                position_[i].y <= supporting_y[1] &&
                position_[i].x <= supporting_x[0] + 10.0f &&
                position_[i].x >= supporting_x[0] - 0.02f && 
                velocity_[i].x > 0)
                    velocity_[i].x = 0.0f;

            if (supporting_z[0] <= position_[i].z &&
                position_[i].z <= supporting_z[1] &&
                supporting_y[0] <= position_[i].y &&
                position_[i].y <= supporting_y[1] &&
                position_[i].x <= supporting_x[1] + 0.2f &&
                position_[i].x >= supporting_x[1] - 10.0f && 
                velocity_[i].x < 0)
                    velocity_[i].x = 0.0f;

            position_[i] += dt * velocity_[i];
            velocity_[i] *= damp_factor;
            velocity_[i] += dt * force_[i];
            force_[i] = vec3{0.0f, -9.8f, 0.0f};

        }

        for (int i = 0; i < springs_.size(); i++) {
            vec3 vec0to1 = -position_[springs_[i][0]] + position_[springs_[i][1]];
            float length = glm::length(vec0to1);
            vec3 force = hook * (length - o_length[i]) * glm::normalize(vec0to1);
            force_[springs_[i][0]] += force / mass;
            force_[springs_[i][1]] -= force / mass;
        }
    }

    void write_obj(std::string & path) {
        std::ofstream os;
        os.open(path);
        for (int i = 0; i < position_.size(); i++) {
            os << "v " << position_[i].x << ' ' << position_[i].y << ' ' << position_[i].z << std::endl;
        }

        for (int i = 0; i < position_.size(); i++) {
            os << "vt " << (i / size_z) / float(size_x)  << ' ' << (i % size_z) / float(size_z) << std::endl;
        }
        os << std::endl;

        for (int j = 0; j < faces_.size(); j++) {
            os << "f " << faces_[j][0] + 1 << '/' << faces_[j][0] + 1 << ' '
               << faces_[j][1] + 1 << '/' << faces_[j][1] + 1 << ' '
               << faces_[j][2] + 1 << '/' << faces_[j][2] + 1 << std::endl;
        }

        os.close();
    }
};

int main() {

    int evolves = 12000;
    float dt = 0.002;


    spring_mass spring_mass_ex = spring_mass(vec3{-10.0f, 50.1f, 330.0f}, 190.0f, 270.0f, 1.0f);
    for (int i = 0; i < evolves; i++) {
        spring_mass_ex.evolve(dt, hook);
    }
    std::cerr << "simulation done" << std::endl;
    std::string path = "cloth.obj";
    spring_mass_ex.write_obj(path);

    return 0;
}


