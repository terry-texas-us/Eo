#pragma once
#include "EoGeMatrix3d.h"

class EoGsModelTransform {
	unsigned m_Depth {0};
	EoGeMatrix3d m_CurrentModelTransform;
	EoGeMatrix3dList m_TransformMatrixList;
public:
	EoGsModelTransform();
	~EoGsModelTransform() = default;
	EoGeMatrix3d ModelMatrix() const noexcept;
	unsigned Depth() const noexcept;
	/// <summary> Removes the top transformation off the current transformation stack.</summary>
	void PopModelTransform();
	/// <summary> The specified transformation is concatenated to the current model transformation (which is initially the identity transform).</summary>
	void PushModelTransform(const EoGeMatrix3d& transformation);
};
