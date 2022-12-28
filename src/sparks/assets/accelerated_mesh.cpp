#include "sparks/assets/accelerated_mesh.h"

#include "algorithm"
#include <climits>
#include <cfloat>
#include <queue>

namespace sparks {

  float InternalNode::FindIntersect(const glm::vec3 &origin,
                                    const glm::vec3 &direction,
                                    float t_min,
                                    HitRecord *hit_record) const {
    float t_max = FLT_MAX;
    float res,t1,t2;
    HitRecord hit_record1, hit_record2;
    if (childL_->GetAABB().IsIntersect(origin, direction, t_min, t_max)) {
      t1 = childL_->FindIntersect(origin, direction, t_min, &hit_record1);
      if (t1 >= 0)
        t_max = t1;
    } else {
      t1 = -1;
    }
    if (childR_->GetAABB().IsIntersect(origin, direction, t_min, t_max)) {
      t2 = childR_->FindIntersect(origin, direction, t_min, &hit_record2);
    } else {
      t2 = -1;
    }

    if (t2 >= 0 && t1 < 0) {
      if (hit_record)
        *hit_record = hit_record2;
      return t2;
    } 
    else if (t2 < 0 && t1 >= 0) {
      if (hit_record)
        *hit_record = hit_record1;
      return t1;
    } 
    else if (t2 >= 0 && t1 >= 0 && t2 <= t1) {
      if (hit_record)
        *hit_record = hit_record2;
      return t2;
    }
    else if (t2 >= 0 && t1 >= 0 && t2 > t1) {
      if (hit_record)
        *hit_record = hit_record1;
      return t1;
    }
    else {  // t1 = t2 = -1
      return -1;
    }

  }

  float LeafNode::FindIntersect(const glm::vec3 &origin,
                                const glm::vec3 &direction,
                                float t_min,
                                HitRecord *hit_record) const {
    float result = -1.0f;

    for (auto face : faces_) {
      const auto &v0 = vertices_[indices_[3*face]];
      const auto &v1 = vertices_[indices_[3*face + 1]];
      const auto &v2 = vertices_[indices_[3*face + 2]];

      glm::mat3 A = glm::mat3(v1.position - v0.position,
                              v2.position - v0.position, -direction);
      if (std::abs(glm::determinant(A)) < 1e-9f) {
        continue;
      }
      A = glm::inverse(A);
      auto uvt = A * (origin - v0.position);
      auto &t = uvt.z;
      if (t < t_min || (result > 0.0f && t > result)) {
        continue;
      }
      auto &u = uvt.x;
      auto &v = uvt.y;
      auto w = 1.0f - u - v;
      auto position = origin + t * direction;
      if (u >= 0.0f && v >= 0.0f && u + v <= 1.0f) {
        result = t;
        if (hit_record) {
          auto geometry_normal = glm::normalize(
              glm::cross(v2.position - v0.position, v1.position - v0.position));
          if (glm::dot(geometry_normal, direction) < 0.0f) {
            hit_record->position = position;
            hit_record->geometry_normal = geometry_normal;
            hit_record->normal = v0.normal * w + v1.normal * u + v2.normal * v;
            hit_record->tangent =
                v0.tangent * w + v1.tangent * u + v2.tangent * v;
            hit_record->tex_coord =
                v0.tex_coord * w + v1.tex_coord * u + v2.tex_coord * v;
            hit_record->front_face = true;
          } else {
            hit_record->position = position;
            hit_record->geometry_normal = -geometry_normal;
            hit_record->normal = -(v0.normal * w + v1.normal * u + v2.normal * v);
            hit_record->tangent =
                -(v0.tangent * w + v1.tangent * u + v2.tangent * v);
            hit_record->tex_coord =
                v0.tex_coord * w + v1.tex_coord * u + v2.tex_coord * v;
            hit_record->front_face = false;
          }
        }
      }
    }
    return result;
  }

  AcceleratedMesh::AcceleratedMesh(const Mesh &mesh, int level) : Mesh(mesh), level_(level) {
    int num_faces = indices_.size() / 3;
    std::set<int32_t> faces;
    faces.clear();
    for (int i = 0; i < num_faces; i++) {
      faces.insert(i);
    }
    root_ = std::make_shared<LeafNode>(vertices_, indices_, faces);
    root_->SetDepth(0);
    BuildAccelerationStructure(level);
  }

  AcceleratedMesh::AcceleratedMesh(const std::vector<Vertex> &vertices,
                                   const std::vector<uint32_t> &indices,
                                   int level)
      : Mesh(vertices, indices), level_(level) {
    int num_faces = indices_.size() / 3;
    std::set<int32_t> faces;
    faces.clear();
    for (int i = 0; i < num_faces; i++) {
      faces.insert(i);
    }
    root_ = std::make_shared<LeafNode>(vertices_, indices_, faces);

    root_->SetDepth(0);
    BuildAccelerationStructure(level);
  }

  float AcceleratedMesh::TraceRay(const glm::vec3 &origin,
                                  const glm::vec3 &direction, float t_min,
                                  HitRecord *hit_record) const {
    /* supposed to be the accelerated tracing algorithm */
    return root_->FindIntersect(origin, direction, t_min, hit_record);
  }

  void AcceleratedMesh::BuildAccelerationStructure(int level) {
    std::shared_ptr<InternalNode> cur = root_->SplitNode(0);

    std::queue<std::shared_ptr<InternalNode>> node_queue;
  
    if (cur) {
      root_ = cur;      
      node_queue.push(cur);
    }

    while (!node_queue.empty()) {
      cur = node_queue.front();
      node_queue.pop();

      if (!cur) {
        continue;
      }

      int depth = cur->GetDepth();

      if (depth >= level_)
        continue;

      auto newleft = cur->childL_->SplitNode((depth+1) % 3);
      auto newright = cur->childR_->SplitNode((depth+1) % 3);

      if (newleft) {
        cur->childL_ = newleft;
        node_queue.push(newleft);
      }

      if (newright) {
        cur->childR_ = newright;
        node_queue.push(newright);
      }
    }



  }

  std::shared_ptr<InternalNode> LeafNode::SplitNode(int dim) {
    if (faces_.size() <= 10)
      return nullptr;

    float min_x = FLT_MAX;
    float max_x = FLT_MIN;

    // find (max+min)/2 in this section

    for (auto face : faces_) {
      for (int i = 0; i < 3; i++) {
        min_x = std::min(min_x, vertices_[indices_[face*3 + i]].position[dim]);
        max_x = std::max(max_x, vertices_[indices_[face*3 + i]].position[dim]);
      }
    }
    float mid_x = (min_x + max_x) / 2;

    // divide vertices into two groups, and the face belongs to
    // the group with more nodes

    std::set<int32_t> group1,group2;
    group1.clear();
    group2.clear();

    for (auto face : faces_) {
      int cnt = 0;
      for (int i = 0; i < 3; i++) {
        if (vertices_[indices_[face*3 + i]].position[dim] < mid_x)
          cnt += 1;
        else 
          cnt += 2;
      }
      if (cnt <= 4)
        group1.insert(face);
      else 
        group2.insert(face);
    }

    // construct new vertices 

    std::shared_ptr<LeafNode> lnode = std::make_shared<LeafNode>(vertices_, indices_, group1);
    std::shared_ptr<LeafNode> rnode = std::make_shared<LeafNode>(vertices_, indices_, group2);
    lnode->SetDepth(this->GetDepth() + 1);
    rnode->SetDepth(this->GetDepth() + 1);

    std::shared_ptr<InternalNode> inode = std::make_shared<InternalNode>();
    inode->childL_ = lnode;
    inode->childR_ = rnode;
    inode->SetDepth(this->GetDepth() + 1);

    return inode;
  }

}  // namespace sparks
