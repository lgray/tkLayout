#ifndef SENSOR_H
#define SENSOR_H

#include <string>
#include <exception>

#include "global_funcs.h"
#include "Polygon3d.h"
#include "Property.h"
#include "CoordinateOperations.h"
#include "Visitor.h"


enum ModuleSubdetector { BARREL = 1, ENDCAP = 2 };
enum SensorPosition { NO, LOWER, UPPER };
enum class SensorType { Pixel, Largepix, Strip, None };

class DetectorModule;

class Sensor : public PropertyObject, public Buildable, public Identifiable<int>, public DetIdentifiable {
  const DetectorModule* parent_;
  ModuleSubdetector subdet_;
  SensorPosition innerOuter_ = SensorPosition::NO;
  mutable const Polygon3d<4>* hitPoly_ = 0;
  mutable const Polygon3d<4>* hitMidPoly_ = 0;
  mutable const Polygon3d<8>* envelopePoly_ = 0;
  mutable const Polygon3d<8>* envelopeMidPoly_ = 0;
  Polygon3d<4>* buildHitPoly(const double polyOffset) const;
  Polygon3d<8>* buildEnvelopePoly(const Polygon3d<4>& basePoly) const;
public:
  ReadonlyProperty<int, NoDefault> numStripsAcross;
  ReadonlyProperty<double, NoDefault> pitchEstimate;
  ReadonlyProperty<int, NoDefault> numSegments;
  ReadonlyProperty<double, NoDefault> stripLengthEstimate;
  ReadonlyProperty<int, NoDefault> numROCX, numROCY;
  ReadonlyProperty<double, NoDefault> sensorThickness;
  ReadonlyProperty<SensorType, Default> type;
  ReadonlyProperty<double, Computable> minR, maxR;
  ReadonlyProperty<double, Computable> minZ, maxZ;

 Sensor() :
  numStripsAcross("numStripsAcross", parsedOnly()),
    pitchEstimate("pitchEstimate", parsedOnly()),
    numSegments("numSegments", parsedOnly()),
    stripLengthEstimate("stripLengthEstimate", parsedOnly()),
    numROCX("numROCX", parsedOnly()),
    numROCY("numROCY", parsedOnly()),
    sensorThickness("sensorThickness", parsedAndChecked()),
    type("sensorType", parsedOnly(), SensorType::None)
      {}

  void parent(const DetectorModule* m) { parent_ = m; }

  ModuleSubdetector subdet(ModuleSubdetector s) { subdet_ = s; }
  ModuleSubdetector subdet() const { return subdet_; }
  SensorPosition innerOuter(SensorPosition pos) { innerOuter_ = pos; }
  SensorPosition innerOuter() const { return innerOuter_; }

  int numStripsAcrossEstimate() const;
  int numSegmentsEstimate() const;
  int numChannels() const { return numStripsAcrossEstimate() * numSegmentsEstimate(); }
  double minPitch() const;
  double maxPitch() const;
  double pitch() const;
  double stripLength() const;

  int numROCRows() const { return numStripsAcrossEstimate() / numROCX(); } 
  int numROCCols() const { return numSegmentsEstimate() / numROCY(); }

  int totalROCs() const { return numROCX() * numROCY(); }

  double sensorNormalOffset() const;  // offset of the sensor center, with respect to the module center
  const XYZVector& center() const { return hitPoly().getCenter(); }  // center of the sensor
  const Polygon3d<4>& hitPoly() const;  // sensor plane containing the sensor center
  const Polygon3d<4>& hitMidPoly() const;  // losange formed by the mid-points of hitPoly
  const Polygon3d<8>& envelopePoly() const; // parallelepiped rectangle formed by the sensor (6 faces)
  const Polygon3d<8>& envelopeMidPoly() const;  // parallelepiped formed by the mid-points of envelopePoly
  void clearPolys();

  std::pair<XYZVector, int> checkHitSegment(const XYZVector& trackOrig, const XYZVector& trackDir) const;

  void check() override;

  void build() { 
    try { check(); } 
    catch (PathfulException& pe) { pe.pushPath(*this, myid()); throw; }
    cleanup(); 
  }

  void setup() {
    clearPolys();
    minR.setup([&]() { return CoordinateOperations::computeMinR(envelopePoly()); });
    maxR.setup([&]() { return CoordinateOperations::computeMaxR(envelopePoly()); });
    minZ.setup([&]() { return CoordinateOperations::computeMinZ(envelopePoly()); });
    maxZ.setup([&]() { return CoordinateOperations::computeMaxZ(envelopePoly()); });
  }

  void accept(SensorGeometryVisitor& v) { 
    v.visit(*this);
  }
};

#endif
