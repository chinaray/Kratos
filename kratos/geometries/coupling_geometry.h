//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Tobias Teschemacher
//  contributors:    Pooyan Dadvand
//                   Philipp Bucher
//

#if !defined(KRATOS_COUPLING_GEOMETRY_H_INCLUDED )
#define  KRATOS_COUPLING_GEOMETRY_H_INCLUDED

// System includes

// External includes

// Project includes
#include "geometry.h"


namespace Kratos
{
///@name Kratos Classes
///@{

/**
 * @class CouplingGeometry
 * @ingroup KratosCore
 * @brief The CouplingGeometry can connect different geometries, those
 can be of different entities but have to coincide with the dimension.
 */
template<class TPointType> class CouplingGeometry
    : public Geometry<TPointType>
{
public:
    ///@}
    ///@name Type Definitions
    ///@{

    /**
     * Geometry as base class.
     */
    typedef Geometry<TPointType> BaseType;
    typedef Geometry<TPointType> GeometryType;

    typedef typename GeometryType::Pointer GeometryPointer;

    /**
     * Pointer definition of CouplingGeometry
     */
    KRATOS_CLASS_POINTER_DEFINITION( CouplingGeometry );

    typedef TPointType PointType;

    typedef typename BaseType::IndexType IndexType;
    typedef typename BaseType::SizeType SizeType;

    typedef typename BaseType::PointsArrayType PointsArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor for coupling one master to one slave geometry.
    CouplingGeometry(
        GeometryPointer pMasterGeometry,
        GeometryPointer pSlaveGeometry)
        : BaseType(PointsArrayType(), &(pMasterGeometry->GetGeometryData()))
    {
        KRATOS_DEBUG_ERROR_IF(pMasterGeometry->Dimension() != pSlaveGeometry->Dimension())
            << "Geometries of different dimensional size!" << std::endl;

        mpGeometries.resize(2);

        mpGeometries[0] = pMasterGeometry;
        mpGeometries[1] = pSlaveGeometry;
    }

    explicit CouplingGeometry(const PointsArrayType& ThisPoints)
        : BaseType()
    {
    }

    /**
     * Copy constructor.
     * Construct this geometry as a copy of given geometry.
     *
     * @note This copy constructor does not copy the points and new
     * geometry shares points with given source geometry. It is
     * obvious that any change to this new geometry's point affect
     * source geometry's points too.
     */
    CouplingGeometry( CouplingGeometry const& rOther )
        : BaseType( rOther )
    {
    }

    /**
     * Copy constructor from a geometry with other point type.
     * Construct this geometry as a copy of given geometry which
     * has different type of points. The given goemetry's
     * TOtherPointType* must be implicity convertible to this
     * geometry PointType.
     *
     * @note This copy constructor does not copy the points and new
     * geometry shares points with given source geometry. It is
     * obvious that any change to this new geometry's point affect
     * source geometry's points too.
     */
    template<class TOtherPointType> explicit CouplingGeometry(
        CouplingGeometry<TOtherPointType> const& rOther )
        : BaseType( rOther )
    {
    }

    /**
     * Destructor. Does nothing!!!
     */
    ~CouplingGeometry() override {}


    GeometryData::KratosGeometryFamily GetGeometryFamily() const override
    {
        return GeometryData::Kratos_generic_family;
    }

    GeometryData::KratosGeometryType GetGeometryType() const override
    {
        return GeometryData::Kratos_generic_type;
    }

    ///@}
    ///@name Operators
    ///@{

    /**
     * Assignment operator.
     *
     * @note This operator don't copy the points and this
     * geometry shares points with given source geometry. It's
     * obvious that any change to this geometry's point affect
     * source geometry's points too.
     *
     * @see Clone
     * @see ClonePoints
     */
    CouplingGeometry& operator=( const CouplingGeometry& rOther )
    {
        BaseType::operator=( rOther );
        return *this;
    }

    /**
     * Assignment operator for geometries with different point type.
     *
     * @note This operator don't copy the points and this
     * geometry shares points with given source geometry. It's
     * obvious that any change to this geometry's point affect
     * source geometry's points too.
     *
     * @see Clone
     * @see ClonePoints
     */
    template<class TOtherPointType>
    CouplingGeometry& operator=(
        CouplingGeometry<TOtherPointType> const & rOther )
    {
        BaseType::operator=( rOther );
        return *this;
    }

    ///@}
    ///@name Operations
    ///@{

    typename BaseType::Pointer Create(
        PointsArrayType const& ThisPoints ) const override
    {
        return typename BaseType::Pointer(
            new CouplingGeometry( ThisPoints) );
    }

    ///@}
    ///@name Input and output
    ///@{

    /**
    * This function returns the geometry part which is accessable with a 
    * certain index. Here, 0 is the master, all above 0 is for the slaves
    * of the geometry. Checks if index is available only in debug mode.
    * @param Index of the geometry part. 0->Master; 1->Slave
    * @return geometry, which is connected through the Index
     */
    typename GeometryType& GetGeometryPart(IndexType Index) const override
    {
        KRATOS_DEBUG_ERROR_IF(mpGeometries.size() < Index) << "Index out of range: "
            << Index << " composite contains only of: "
            << mpGeometries.size() << " geometries." << std::endl;

        return *mpGeometries[Index];
    }

    /**
    * ONLY for coupling_geometry. Not necessary in base class.
    * Allows to exchange certain geometries.
    * @param Index of the geometry part. 0->Master; 1->Slave
     */
    void SetGeometryPart(IndexType Index, GeometryPointer pGeometry)
    {
        KRATOS_DEBUG_ERROR_IF(mpGeometries.size() < Index) << "Index out of range: "
            << Index << " composite contains only of: "
            << mpGeometries.size() << " geometries." << std::endl;

        mpGeometries[Index] = pGeometry;
    }

    /**
    * ONLY for coupling_geometry. Not necessary in base class.
    * Allows to enhance the coupling geometry, with another geometry.
    * @param Index of the geometry part. 0->Master; 1->Slave
     */
    IndexType AddGeometryPart(GeometryPointer pGeometry)
    {
        KRATOS_DEBUG_ERROR_IF(mpGeometries[0]->Dimension() != pGeometry->Dimension())
            << "Geometries of different dimensional size!" << std::endl;

        IndexType new_index = mpGeometries.size();

        mpGeometries.resize(new_index + 1);
        mpGeometries[new_index] = pGeometry;

        return new_index;
    }

    /**
    * ONLY for coupling_geometry. Not necessary in base class.
    * Allows to enhance the coupling geometry, with another geometry.
    * @param Index of the geometry part. 0->Master; 1->Slave
    */
    SizeType NumberOfGeometryParts() const
    {
        return mpGeometries.size();
    }

    ///@}

    /** Returns the domain size of the master geometry.
    */
    double DomainSize() const override
    {
        return mpGeometries[0]->DomainSize();
    }

    /** Returns the center of the master geometry.
    */
    Point Center() const override
    {
        return mpGeometries[0]->Center();
    }

    ///@}
    ///@name Input and output
    ///@{

    std::string Info() const override
    {
        return "Coupling geometry that holds a master and a set of slave geometries.";
    }

    void PrintInfo( std::ostream& rOStream ) const override
    {
        rOStream << "Coupling geometry that holds a master and a set of slave geometries.";
    }

    void PrintData( std::ostream& rOStream ) const override
    {
        BaseType::PrintData( rOStream );
        std::cout << std::endl;
        rOStream << "    CouplingGeometry with " << mpGeometries.size() << " geometries.";
    }

    ///@}
protected:

private:
    ///@name Private Member Variables
    ///@{

    std::vector<GeometryPointer> mpGeometries;

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save( Serializer& rSerializer ) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, BaseType );
        rSerializer.save("Geometries", mpGeometries);
    }

    void load( Serializer& rSerializer ) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, BaseType );
        rSerializer.load("Geometries", mpGeometries);
    }

    CouplingGeometry() : BaseType() {}

    ///@}
    ///@name Private Friends
    ///@{

    template<class TOtherPointType> friend class CouplingGeometry;

    ///@}
}; // Class Geometry

///@}
///@name Input and output
///@{
/**
 * input stream functions
 */
template<class TPointType> inline std::istream& operator >> (
    std::istream& rIStream,
    CouplingGeometry<TPointType>& rThis );
/**
 * output stream functions
 */
template<class TPointType> inline std::ostream& operator << (
    std::ostream& rOStream,
    const CouplingGeometry<TPointType>& rThis )
{
    rThis.PrintInfo( rOStream );
    rOStream << std::endl;
    rThis.PrintData( rOStream );
    return rOStream;
}

///@}
}// namespace Kratos.

#endif // KRATOS_COUPLING_GEOMETRY_H_INCLUDED  defined
