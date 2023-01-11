#pragma once
#include "sparks/assets/aabb.h"
#include "sparks/assets/mesh.h"
#include <vector>
#include <set>

namespace sparks {

class TreeNode {
private:
  AxisAlignedBoundingBox aabb_{};
  bool is_leaf_;
  int depth_;
  std::set<int32_t> faces_;
  const std::vector<Vertex> &vertices_;
  const std::vector<uint32_t> &indices_;

protected:
  std::shared_ptr<TreeNode> childL_;
  std::shared_ptr<TreeNode> childR_;

public:

  AxisAlignedBoundingBox GetAABB() const { return aabb_;}
  bool IsLeaf() const { return is_leaf_;}
  int GetDepth()  { return depth_;}

  std::shared_ptr<TreeNode> GetLeft() const { return childL_;}
  std::shared_ptr<TreeNode> GetRight() const { return childR_;}


  float FindIntersect(const glm::vec3 &origin,
                      const glm::vec3 &direction,
                      float t_min,
                      HitRecord *hit_record) const;

  float FindIntersect_Leaf(const glm::vec3 &origin,
                           const glm::vec3 &direction,
                           float t_min,
                           HitRecord *hit_record) const;

  float FindIntersect_Inner(const glm::vec3 &origin,
                            const glm::vec3 &direction,
                            float t_min,
                            HitRecord *hit_record) const;
  // initializing with a set of faces: Leaf
  TreeNode(const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices,
           std::set<int32_t> faces, int depth)
      : vertices_(vertices), indices_(indices), faces_(faces), childL_(nullptr), childR_(nullptr), is_leaf_(true), depth_(depth) {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    x_min = y_min = z_min = FLT_MAX;
    x_max = y_max = z_max = FLT_MIN;
    for (auto face : faces) {
      for (int i = 0; i < 3; i++) {
        glm::vec3 pos = vertices_[indices_[3*face + i]].position;
        x_min = std::min(x_min, pos.x);
        x_max = std::max(x_max, pos.x);
        y_min = std::min(y_min, pos.y);
        y_max = std::max(y_max, pos.y);
        z_min = std::min(z_min, pos.z);
        z_max = std::max(z_max, pos.z);
      }
    }
    aabb_ = AxisAlignedBoundingBox(x_min, x_max, y_min, y_max, z_min, z_max);
  }

  // initializing with left and right child ptr: Inner
  TreeNode(const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices,
           std::shared_ptr<TreeNode> l,
           std::shared_ptr<TreeNode> r, int depth)
      : vertices_(vertices), indices_(indices), childL_(l), childR_(r), is_leaf_(false), depth_(depth) {
    if (childL_ && childR_)
      aabb_ = (childL_->GetAABB() & childR_->GetAABB());
    else 
      std::cerr << "Uninitialized pointers to childs!";
  }

  bool SplitNode(int dim);
};


class AcceleratedMesh : public Mesh {
 public:
  AcceleratedMesh() = default;
  explicit AcceleratedMesh(const Mesh &mesh, int level=10);
  AcceleratedMesh(const std::vector<Vertex> &vertices,
                  const std::vector<uint32_t> &indices,
                  int level = 10);

  float TraceRay(const glm::vec3 &origin,
                 const glm::vec3 &direction,
                 float t_min,
                 HitRecord *hit_record) const override;
  void BuildAccelerationStructure(int level = 10);

 private:
  /*
   * You can add your acceleration structure contents here.
   * */

  // max depth of BVH tree
  int32_t level_;
  std::shared_ptr<TreeNode> root_;
};
}  // namespace sparks
