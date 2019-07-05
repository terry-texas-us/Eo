#pragma once
class EoGePoint4d;
class EoGeMatrix3d;

/// <summary>Determines if a point is contained by a window.</summary>
/// <returns>true if point is in window, false otherwise</returns>
bool ContainmentOf(const OdGePoint3d& point, const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) noexcept;

/// <summary>Projects a point toward or beyond another point.</summary>
/// <param name="ptQ">point defining direction vector</param>
/// <param name="fromPoint"></param>
/// <param name="toPoint"></param>
/// <param name="distance">magnitude of projection</param>
/// <returns> projected point or itself if points coincide</returns>
OdGePoint3d ProjectToward(const OdGePoint3d& fromPoint, const OdGePoint3d& toPoint, double distance);
/// <summary>Determines relationship of a point to a window.</summary>
// Returns:
//		0 - point is contained in window
//		bit 1 set - point above window
//		bit 2 set - point below window
//		bit 4 set - point to the right of window
//		bit 8 set - point to the left of window
unsigned RelationshipToRectangleOf(const OdGePoint3d& point, const OdGePoint3d& lowerLeftPoint, const OdGePoint3d& upperRightPoint) noexcept;
