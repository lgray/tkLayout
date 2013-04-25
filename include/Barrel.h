#ifndef BARREL_H
#define BARREL_H

#include <vector>
#include <string>
#include <memory>

#include <boost/ptr_container/ptr_vector.hpp>

#include "global_funcs.h"
#include "Property.h"
#include "Layer.h"

using std::string;
using std::vector;

class Barrel : public PropertyObject, public Buildable, public Identifiable<Barrel> {
public:
  typedef boost::ptr_vector<Layer> container;
private:
  Container layers_;

  Property<int, NoDefault> numLayers;
  Property<int, NoDefault> innerRadius;
  Property<int, NoDefault> outerRadius;
  Property<bool, Default> sameRods;

  PropertyNode<int> layerNode;
public:
  ReadonlyProperty<double, Computable> maxZ;

  Barrel() : 
      numLayers("numLayers", parsedAndChecked()),
      innerRadius("innerRadius", parsedAndChecked()),
      outerRadius("outerRadius", parsedAndChecked()),
      sameRods("sameRods", true),
      layerNode("Layer", parsedOnly()),
      maxZ(Computable<double>([&]() { double max = 0; for (auto& l : layers_) { max = MAX(max, l.maxZ()); } return max; })) 
  {}

  void build(); 

  const Container& layers() const { return layers_; }
};

#endif