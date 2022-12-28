#pragma once
#include "sparks/assets/aabb.h"
#include "sparks/assets/mesh.h"
#include <vector>
#include <set>

namespace sparks {

class TreeNode;
class InternalNode;
class LeafNode;

class TreeNode {
private:
  AxisAlignedBoundingBox aabb_{};
  bool is_leaf_;
  int depth_;

public:

  AxisAlignedBoundingBox GetAABB() const { return aabb_;}
  void SetAABB(AxisAlignedBoundingBox aabb) {
    aabb_ = aabb;
  }
  bool IsLeaf() const { return is_leaf_;}
  void SetLeaf(bool is_leaf) {
    is_leaf_ = is_leaf;
  }

  void SetDepth(int depth)  { depth_ = depth;}
  int GetDepth()  { return depth_;}

  virtual float FindIntersect(const glm::vec3 &origin,
                              const glm::vec3 &direction,
                              float t_min,
                              HitRecord *hit_record) const = 0;
  TreeNode() {}
  virtual ~TreeNode() {};
  virtual std::shared_ptr<InternalNode> SplitNode(int dim) = 0;
};

class InternalNode : public TreeNode {
 public:
  std::shared_ptr<TreeNode> childL_;
  std::shared_ptr<TreeNode> childR_;
 public:
  virtual float FindIntersect(const glm::vec3 &origin,
                              const glm::vec3 &direction,
                              float t_min,
                              HitRecord *hit_record) const;
                  
  InternalNode() = default;

  ~InternalNode() {
    childL_->~TreeNode();
    childR_->~TreeNode();
  }

  void Init() {
    if (childL_ && childR_)
    SetAABB(childL_->GetAABB() & childR_->GetAABB());
    SetLeaf(false);
  }

  virtual std::shared_ptr<InternalNode> SplitNode(int dim) {
    return nullptr;
  }
};

class LeafNode : public TreeNode {
 private:
  std::set<int32_t> faces_;
  const std::vector<Vertex> &vertices_;
  const std::vector<uint32_t> &indices_;

 public:
  virtual float FindIntersect(const glm::vec3 &origin,
                              const glm::vec3 &direction,
                              float t_min,
                              HitRecord *hit_record) const;

  LeafNode(const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices,
           std::set<int32_t> faces)
      : vertices_(vertices), indices_(indices), faces_(faces) {
    float x_min, x_max, y_min, y_max, z_min, z_max;
    x_min = y_min = z_min = FLT_MAX;
    x_max = y_max = z_max = FLT_MIN;
    for (auto face : faces) {
      for (int i = 0; i < 3; i++) {
        glm::vec3 pos = vertices_[indices_[3*face+i]].position;
        x_min = std::min(x_min, pos.x);
        x_max = std::max(x_max, pos.x);
        y_min = std::min(y_min, pos.y);
        y_max = std::max(y_max, pos.y);
        z_min = std::min(z_min, pos.z);
        z_max = std::max(z_max, pos.z);
      }
    }

    SetAABB(AxisAlignedBoundingBox(x_min, x_max, y_min, y_max, z_min, z_max));
    SetLeaf(true);
  }

  ~LeafNode() {}

  virtual std::shared_ptr<InternalNode> SplitNode(int dim);
};

class AcceleratedMesh : public Mesh {
 public:
  AcceleratedMesh() = default;
  explicit AcceleratedMesh(const Mesh &mesh, int level=10);
  AcceleratedMesh(const std::vector<Vertex> &vertices,
                  const std::vector<uint32_t> &indices,
                  int level=10);
  ~AcceleratedMesh() {
    root_->~TreeNode();
    //delete root_;
  }
  float TraceRay(const glm::vec3 &origin,
                 const glm::vec3 &direction,
                 float t_min,
                 HitRecord *hit_record) const override;
  void BuildAccelerationStructure(int level=10);

 private:
  /*
   * You can add your acceleration structure contents here.
   * */

  // max depth of BVH tree
  int32_t level_;
  std::shared_ptr<TreeNode> root_;
};
}  // namespace sparks
