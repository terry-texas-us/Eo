#include "stdafx.h"

#include "Ge/GeKnotVector.h"
#include "EoGePolyline.h"

#include "EoGeNurbCurve3d.h"

EoGeNurbCurve3d::EoGeNurbCurve3d()
	: OdGeNurbCurve3d() {
}

int EoGeNurbCurve3d::GeneratePoints(const EoGeNurbCurve3d& spline) {
	const auto NumberOfControlPoints {spline.numControlPoints()};
	const auto Degree {EoMin(spline.degree(), NumberOfControlPoints - 1)};

	if (Degree == 1) {
		for (auto ArrayIndex = 0; ArrayIndex < NumberOfControlPoints; ArrayIndex++) {
			polyline::SetVertex(spline.controlPointAt(ArrayIndex));
		}
		return NumberOfControlPoints;
	}
	const auto Order {Degree + 1};

	// <tas="Large allocation for weight array is really not used. Allocation failure not tested for"</tas>
	auto Weight {new double[128 * 128]};
	for (auto i = 0; i < 128 * 128; i++) {
		Weight[i] = 0.0;
	}
	const auto KnotsLength {NumberOfControlPoints + Degree};
	auto iPts {8 * NumberOfControlPoints};
	
	// <tas="Test is no longer valid, since knots are in an sizing array. Weights may exceed limits..and crash"</tas>
	if (spline.knotAt(KnotsLength) != 0.0) {
		auto G {0.0};
		auto H {0.0};
		auto Z {0.0};
		double W1;
		double W2;
		const auto Step {spline.knotAt(KnotsLength) / (double(iPts) - 1.0)};
		auto iPts2 {0};
		for (auto i4 = Order - 1; i4 <= NumberOfControlPoints + 1; i4++) {
			for (auto i = 0; i <= KnotsLength - 1; i++) { // Calculate values for weighting value
				if (i != i4 || spline.knotAt(i) == spline.knotAt(i + 1))
					Weight[128 * i + 1] = 0.0;
				else
					Weight[128 * i + 1] = 1.0;
			}
			for (auto T = spline.knotAt(i4); T <= spline.knotAt(i4 + 1) - Step; T += Step) {
				iPts2++;
				for (auto i2 = 2; i2 <= Order; i2++) {
					for (auto i = 0; i <= NumberOfControlPoints - 1; i++) { // Determine first term of weighting function equation
						if (Weight[128 * i + i2 - 1] == 0.0)
							W1 = 0.0;
						else
							W1 = (T - spline.knotAt(i)) * Weight[128 * i + i2 - 1] / (spline.knotAt(i + i2 - 1) - spline.knotAt(i));

						if (Weight[128 * (i + 1) + i2 - 1] == 0.0) 	// Determine second term of weighting function equation
							W2 = 0.0;
						else
							W2 = (spline.knotAt(i + i2) - T) * Weight[128 * (i + 1) + i2 - 1] / (spline.knotAt(i + i2) - spline.knotAt(i + 1));

						Weight[128 * i + i2] = W1 + W2;
						G = spline.controlPointAt(i).x * Weight[128 * i + i2] + G;
						H = spline.controlPointAt(i).y * Weight[128 * i + i2] + H;
						Z = spline.controlPointAt(i).z * Weight[128 * i + i2] + Z;
					}
					if (i2 == Order)
						break;
					G = 0.0; 
					H = 0.0; 
					Z = 0.0;
				}
				polyline::SetVertex(OdGePoint3d(G, H, Z));
				G = 0.0; 
				H = 0.0; 
				Z = 0.0;
			}
		}
		iPts = iPts2 + 1;
	} else { // either Order greater than number of control points or all control points coincidental
		iPts = 2;
		polyline::SetVertex(spline.startPoint());
	}
	polyline::SetVertex(spline.endPoint());

	delete [] Weight;

	return iPts;
}
void EoGeNurbCurve3d::SetDefaultKnotVector(int degree, const OdGePoint3dArray& controlPoints, OdGeKnotVector& knots) {
	const auto Order {gsl::narrow_cast<unsigned>(degree + 1)};
	const auto NumberOfControlPoints {controlPoints.size()};

	knots.setLogicalLength(0);

	const auto KnotsLength {NumberOfControlPoints + Order};

	for (unsigned KnotIndex = 0; KnotIndex < KnotsLength; KnotIndex++) {
		if (KnotIndex <= Order - 1) { // Beginning of curve
			knots.append(0.0);
		}
		else if (KnotIndex >= NumberOfControlPoints + 1) { // End of curve
			knots.append(knots[static_cast<int>(KnotIndex) - 1]);
		} else {
			const auto i2 = KnotIndex - Order;
			if (controlPoints[i2] == controlPoints[i2 + 1]) { // Repeating vertices
				knots.append(knots[static_cast<int>(KnotIndex) - 1]);
			} else { // Successive internal vectors
				knots.append(knots[static_cast<int>(KnotIndex) - 1] + 1.0);
			}
		}
	}
}