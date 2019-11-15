// This file is MACHINE GENERATED! Do not edit.

#ifndef TENSORFLOW_CC_OPS_IMAGE_OPS_INTERNAL_H_
#define TENSORFLOW_CC_OPS_IMAGE_OPS_INTERNAL_H_

// This file is MACHINE GENERATED! Do not edit.

#include "tensorflow/cc/framework/ops.h"
#include "tensorflow/cc/framework/scope.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/framework/tensor_shape.h"
#include "tensorflow/core/framework/types.h"
#include "tensorflow/core/lib/gtl/array_slice.h"

namespace tensorflow {
namespace ops {
namespace internal {
// NOTE: This namespace has internal TensorFlow details that
// are not part of TensorFlow's public API.

/// @defgroup image_ops_internal Image Ops Internal
/// @{

/// Computes the gradient of bicubic interpolation.
///
/// Arguments:
/// * scope: A Scope object
/// * grads: 4-D with shape `[batch, height, width, channels]`.
/// * original_image: 4-D with shape `[batch, orig_height, orig_width, channels]`,
/// The image tensor that was resized.
///
/// Optional attributes (see `Attrs`):
/// * align_corners: If true, the centers of the 4 corner pixels of the input and grad tensors are
/// aligned. Defaults to false.
///
/// Returns:
/// * `Output`: 4-D with shape `[batch, orig_height, orig_width, channels]`.
/// Gradients with respect to the input image. Input image must have been
/// float or double.
class ResizeBicubicGrad {
 public:
  /// Optional attribute setters for ResizeBicubicGrad
  struct Attrs {
    /// If true, the centers of the 4 corner pixels of the input and grad tensors are
    /// aligned. Defaults to false.
    ///
    /// Defaults to false
    TF_MUST_USE_RESULT Attrs AlignCorners(bool x) {
      Attrs ret = *this;
      ret.align_corners_ = x;
      return ret;
    }

    /// Defaults to false
    TF_MUST_USE_RESULT Attrs HalfPixelCenters(bool x) {
      Attrs ret = *this;
      ret.half_pixel_centers_ = x;
      return ret;
    }

    bool align_corners_ = false;
    bool half_pixel_centers_ = false;
  };
  ResizeBicubicGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input grads,
                  ::tensorflow::Input original_image);
  ResizeBicubicGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input grads,
                  ::tensorflow::Input original_image, const
                  ResizeBicubicGrad::Attrs& attrs);
  operator ::tensorflow::Output() const { return output; }
  operator ::tensorflow::Input() const { return output; }
  ::tensorflow::Node* node() const { return output.node(); }

  static Attrs AlignCorners(bool x) {
    return Attrs().AlignCorners(x);
  }
  static Attrs HalfPixelCenters(bool x) {
    return Attrs().HalfPixelCenters(x);
  }

  Operation operation;
  ::tensorflow::Output output;
};

/// Computes the gradient of bilinear interpolation.
///
/// Arguments:
/// * scope: A Scope object
/// * grads: 4-D with shape `[batch, height, width, channels]`.
/// * original_image: 4-D with shape `[batch, orig_height, orig_width, channels]`,
/// The image tensor that was resized.
///
/// Optional attributes (see `Attrs`):
/// * align_corners: If true, the centers of the 4 corner pixels of the input and grad tensors are
/// aligned. Defaults to false.
///
/// Returns:
/// * `Output`: 4-D with shape `[batch, orig_height, orig_width, channels]`.
/// Gradients with respect to the input image. Input image must have been
/// float or double.
class ResizeBilinearGrad {
 public:
  /// Optional attribute setters for ResizeBilinearGrad
  struct Attrs {
    /// If true, the centers of the 4 corner pixels of the input and grad tensors are
    /// aligned. Defaults to false.
    ///
    /// Defaults to false
    TF_MUST_USE_RESULT Attrs AlignCorners(bool x) {
      Attrs ret = *this;
      ret.align_corners_ = x;
      return ret;
    }

    /// Defaults to false
    TF_MUST_USE_RESULT Attrs HalfPixelCenters(bool x) {
      Attrs ret = *this;
      ret.half_pixel_centers_ = x;
      return ret;
    }

    bool align_corners_ = false;
    bool half_pixel_centers_ = false;
  };
  ResizeBilinearGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input grads,
                   ::tensorflow::Input original_image);
  ResizeBilinearGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input grads,
                   ::tensorflow::Input original_image, const
                   ResizeBilinearGrad::Attrs& attrs);
  operator ::tensorflow::Output() const { return output; }
  operator ::tensorflow::Input() const { return output; }
  ::tensorflow::Node* node() const { return output.node(); }

  static Attrs AlignCorners(bool x) {
    return Attrs().AlignCorners(x);
  }
  static Attrs HalfPixelCenters(bool x) {
    return Attrs().HalfPixelCenters(x);
  }

  Operation operation;
  ::tensorflow::Output output;
};

/// Computes the gradient of nearest neighbor interpolation.
///
/// Arguments:
/// * scope: A Scope object
/// * grads: 4-D with shape `[batch, height, width, channels]`.
/// * size: = A 1-D int32 Tensor of 2 elements: `orig_height, orig_width`. The
/// original input size.
///
/// Optional attributes (see `Attrs`):
/// * align_corners: If true, the centers of the 4 corner pixels of the input and grad tensors are
/// aligned. Defaults to false.
///
/// Returns:
/// * `Output`: 4-D with shape `[batch, orig_height, orig_width, channels]`. Gradients
/// with respect to the input image.
class ResizeNearestNeighborGrad {
 public:
  /// Optional attribute setters for ResizeNearestNeighborGrad
  struct Attrs {
    /// If true, the centers of the 4 corner pixels of the input and grad tensors are
    /// aligned. Defaults to false.
    ///
    /// Defaults to false
    TF_MUST_USE_RESULT Attrs AlignCorners(bool x) {
      Attrs ret = *this;
      ret.align_corners_ = x;
      return ret;
    }

    /// Defaults to false
    TF_MUST_USE_RESULT Attrs HalfPixelCenters(bool x) {
      Attrs ret = *this;
      ret.half_pixel_centers_ = x;
      return ret;
    }

    bool align_corners_ = false;
    bool half_pixel_centers_ = false;
  };
  ResizeNearestNeighborGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input
                          grads, ::tensorflow::Input size);
  ResizeNearestNeighborGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input
                          grads, ::tensorflow::Input size, const
                          ResizeNearestNeighborGrad::Attrs& attrs);
  operator ::tensorflow::Output() const { return output; }
  operator ::tensorflow::Input() const { return output; }
  ::tensorflow::Node* node() const { return output.node(); }

  static Attrs AlignCorners(bool x) {
    return Attrs().AlignCorners(x);
  }
  static Attrs HalfPixelCenters(bool x) {
    return Attrs().HalfPixelCenters(x);
  }

  Operation operation;
  ::tensorflow::Output output;
};

/// TODO: add doc.
///
/// Arguments:
/// * scope: A Scope object
///
/// Returns:
/// * `Output`: The output tensor.
class ScaleAndTranslateGrad {
 public:
  /// Optional attribute setters for ScaleAndTranslateGrad
  struct Attrs {
    /// Defaults to "lanczos3"
    TF_MUST_USE_RESULT Attrs KernelType(StringPiece x) {
      Attrs ret = *this;
      ret.kernel_type_ = x;
      return ret;
    }

    /// Defaults to true
    TF_MUST_USE_RESULT Attrs Antialias(bool x) {
      Attrs ret = *this;
      ret.antialias_ = x;
      return ret;
    }

    StringPiece kernel_type_ = "lanczos3";
    bool antialias_ = true;
  };
  ScaleAndTranslateGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input
                      grads, ::tensorflow::Input original_image,
                      ::tensorflow::Input scale, ::tensorflow::Input
                      translation);
  ScaleAndTranslateGrad(const ::tensorflow::Scope& scope, ::tensorflow::Input
                      grads, ::tensorflow::Input original_image,
                      ::tensorflow::Input scale, ::tensorflow::Input
                      translation, const ScaleAndTranslateGrad::Attrs& attrs);
  operator ::tensorflow::Output() const { return output; }
  operator ::tensorflow::Input() const { return output; }
  ::tensorflow::Node* node() const { return output.node(); }

  static Attrs KernelType(StringPiece x) {
    return Attrs().KernelType(x);
  }
  static Attrs Antialias(bool x) {
    return Attrs().Antialias(x);
  }

  Operation operation;
  ::tensorflow::Output output;
};

}  // namespace internal
}  // namespace ops
}  // namespace tensorflow

#endif  // TENSORFLOW_CC_OPS_IMAGE_OPS_INTERNAL_H_
