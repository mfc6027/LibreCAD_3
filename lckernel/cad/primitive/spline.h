#pragma once

#include "cad/const.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geospline.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"
#include "cad/interface/draggable.h"
#include <cad/builders/spline.h>

namespace lc {
namespace entity {
/**
     * @brief The Spline class
     *
     *
     * @todo SPLINE IMPLEMENTATION.
     * THIS CLASS NEEDS TO BE VERIFIED.
     * SPLINE IS DEPENDENT ON BEZIER CURVE.
     *
     *
     */

class Spline
    : public std::enable_shared_from_this<Spline>,
      public CADEntity,
      public geo::Spline,
      public Snapable,
      virtual public Visitable,
      public Draggable {
public:

    /**
     * @brief Spline, Constructor with MetaTypes
     * @param vector<Coordinate> control_points
     * @param int degree
     * @param bool closed
     * @param Layer_CSPtr layer
     * @param MetaTypes metaTypes
     */
    Spline(const std::vector<geo::Coordinate> &controlPoints,
           const std::vector<double> &knotPoints,
           const std::vector<geo::Coordinate> &fitPoints,
           int degree, bool closed,
           double fitTolerance,
           double stanx, double stany, double stanz,
           double etanx, double etany, double etanz,
           double nx, double ny, double nz, splineflag flags,
           meta::Layer_CSPtr layer,
           meta::MetaInfo_CSPtr metaInfo = nullptr,
           meta::Block_CSPtr block = nullptr);
    Spline(const Spline_CSPtr& other, bool sameID = false);
    Spline(const lc::builder::SplineBuilder& builder);

    std::vector<EntityCoordinate> snapPoints(const geo::Coordinate &coord,
            const SimpleSnapConstrain & constrain,
            double minDistanceToSnap,
            int maxNumberOfSnapPoints) const override;
    geo::Coordinate nearestPointOnPath(const geo::Coordinate &coord) const override;
    geo::Coordinate nearestPointOnEntity(const geo::Coordinate &coord) const override;
    CADEntity_CSPtr move(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr copy(const geo::Coordinate &offset) const override;
    CADEntity_CSPtr rotate(const geo::Coordinate &rotation_center, double rotation_angle) const override;
    CADEntity_CSPtr scale(const geo::Coordinate &scale_center,
                          const geo::Coordinate &scale_factor) const override;
    CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;
    const geo::Area boundingBox() const override;
    CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;
    void accept(GeoEntityVisitor &v) const override {
        v.visit(*this);
    }
    void dispatch(EntityDispatch &ed) const override {
        ed.visit(shared_from_this());
    }
    std::map<unsigned int, lc::geo::Coordinate> dragPoints() const override;
    CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;
    PropertiesMap availableProperties() const override;
    CADEntity_CSPtr setProperties(const PropertiesMap& propertiesMap) const override;

private:
    void calculateBoundingBox();
    geo::Area _boundingBox;
};

DECLARE_SHORT_SHARED_PTR(Spline)
}
}

