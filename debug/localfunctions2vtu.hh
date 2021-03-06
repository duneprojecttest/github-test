// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <iostream>
#include <vector>
#include <string>
#include <cstring>

//for uint32_t etc
#include <stdint.h>

#include <dune/common/exceptions.hh> // We use exceptions
#include <dune/common/geometrytype.hh>
#include <dune/common/fvector.hh>

#include <dune/grid/common/virtualrefinement.hh>

typedef LOCAL_BASIS_TYPE LB;

////////////////////////////////////////////////////////////////////////
//
//  Hold vtkdata in memory
//

template<typename LocalBasisTraits>
struct VTKData {
  typedef typename LocalBasisTraits::DomainFieldType DomainFieldType;
  static const int dimDomain = LocalBasisTraits::dimDomain;
  typedef typename LocalBasisTraits::RangeFieldType RangeFieldType;
  static const int dimRange = LocalBasisTraits::dimRange;

  //! list of coordinates
  std::vector<Dune::FieldVector<DomainFieldType, dimDomain> > coords;
  //! list of points for each cell, index into the coords vector
  std::vector<std::vector<int32_t> > connectivity;
  //! vtk type of each cell
  std::vector<uint8_t> types;

  //! The data
  std::vector<std::vector<Dune::FieldVector<RangeFieldType, dimRange> > > data;
};


////////////////////////////////////////////////////////////////////////
//
//  Return the string identifying a type to vtk
//

template<typename T> const std::string vtkTypename();

template<> const std::string
vtkTypename< int8_t >() {
  return "Int8";
}
template<> const std::string
vtkTypename<uint8_t >() {
  return "UInt8";
}
template<> const std::string
vtkTypename< int16_t>() {
  return "Int16";
}
template<> const std::string
vtkTypename<uint16_t>() {
  return "UInt16";
}
template<> const std::string
vtkTypename< int32_t>() {
  return "Int32";
}
template<> const std::string
vtkTypename<uint32_t>() {
  return "UInt32";
}
template<> const std::string
vtkTypename< int64_t>() {
  return "Int64";
}
template<> const std::string
vtkTypename<uint64_t>() {
  return "UInt64";
}

template<> const std::string
vtkTypename<float   >() {
  return "Float32";
}
template<> const std::string
vtkTypename<double  >() {
  return "Float64";
}

////////////////////////////////////////////////////////////////////////
//
//  return vtk geometry type identifier
//

uint8_t vtkGeometryType(const Dune::GeometryType &g) {
  static const Dune::GeometryType::BasicType simplex = Dune::GeometryType::simplex;
  static const Dune::GeometryType::BasicType cube    = Dune::GeometryType::cube;
  static const Dune::GeometryType::BasicType pyramid = Dune::GeometryType::pyramid;
  static const Dune::GeometryType::BasicType prism   = Dune::GeometryType::prism;

  switch(g.dim()) {
  case 0 : return 1; // VTK_VERTEX
  case 1 : return 3; // VTK_LINE
  case 2 : switch(g.basicType()) {
    case simplex : return 5; // VTK_TRIANGLE
    case cube :    return 9; // VTK_QUAD
    default : DUNE_THROW(Dune::Exception, "Invalid GeometryType dim=2 basicType=" << g.basicType());
  }
  case 3 : switch(g.basicType()) {
    case simplex : return 10; // VTK_TETRA
    case cube :    return 12; // VTK_HEXAHEDRON
    case pyramid : return 14; // VTK_PYRAMID
    case prism :   return 13; // VTK_WEDGE
    default : DUNE_THROW(Dune::Exception, "Invalid GeometryType dim=3 basicType=" << g.basicType());
  }
  default : DUNE_THROW(Dune::Exception, "VTK can't handle GeometryType with dim>3");
  }
}

////////////////////////////////////////////////////////////////////////
//
//  Save and restore fmtflags and precision
//

class FMTFlagsSaver
{
  std::ios_base &stream;
  std::ios_base::fmtflags flags;
  std::streamsize precision;
  std::streamsize width;

public:
  FMTFlagsSaver(std::ios_base &s)
    : stream(s)
      , flags(s.flags())
      , precision(s.precision())
      , width(s.width())
  {}

  ~FMTFlagsSaver() {
    stream.flags(flags);
    stream.precision(precision);
    stream.width(width);
  }
};

////////////////////////////////////////////////////////////////////////
//
//  Wrapper class to write a value in full precision
//

template<typename T>
class FullPrecisionWriter {
  const T &value_;

  template<typename T1>
  friend std::ostream &operator<<(std::ostream &s, const FullPrecisionWriter<T1> &w);

public:
  FullPrecisionWriter(const T &value) : value_(value) {}
};

template<typename T>
std::ostream &operator<<(std::ostream &s, const FullPrecisionWriter<T> &w) {
  // automatically restore format flags when control leaves this function, be
  // it via return; or an exception
  FMTFlagsSaver saver(s);
  std::scientific(s);
  // numeric_limits<T>::digits10 gives the number of decimal digits that T can
  // store without loss of precision.  Since we want to store T in decimal
  // digits (which is just the other way round) I'm taking that value +1 as
  // the number of required digits.
  s.precision(std::numeric_limits<T>::digits10+1);
  s << w.value_;
  return s;
}

template<typename T>
FullPrecisionWriter<T> fullPrecision(const T& value)
{
  return FullPrecisionWriter<T>(value);
}

////////////////////////////////////////////////////////////////////////
//
//  sample a local basis for VTK
//
template<typename LocalBasis>
void sample(const LocalBasis &lb,
            const Dune::GeometryType::BasicType basicGeo,
            const int refinementLevel,
            VTKData<typename LocalBasis::Traits> &vtkData)
{
  typedef typename LocalBasis::Traits::DomainFieldType DomainFieldType;
  static const int dimDomain = LocalBasis::Traits::dimDomain;
  typedef typename LocalBasis::Traits::RangeType RangeType;
  typedef Dune::VirtualRefinement<dimDomain, DomainFieldType> VR;
  typedef typename VR::VertexIterator VertexIterator;
  typedef typename VR::ElementIterator ElementIterator;

  Dune::GeometryType geo(basicGeo, dimDomain);
  Dune::GeometryType simplex(Dune::GeometryType::simplex, dimDomain);
  VR &ref = Dune::buildRefinement<dimDomain, DomainFieldType>(geo, simplex);

  // init coords
  vtkData.coords.resize(0);
  vtkData.coords.reserve(ref.nVertices(refinementLevel));

  // init connectivity
  vtkData.connectivity.resize(0);
  vtkData.connectivity.reserve(ref.nElements(refinementLevel));

  // init and set types
  vtkData.types.resize(0);
  vtkData.types.resize(ref.nElements(refinementLevel), vtkGeometryType(geo));

  // init data
  vtkData.data.resize(0);
  vtkData.data.resize(lb.size());
  for(int i = 0, size = lb.size(); i < size; ++i)
    vtkData.data[i].reserve(ref.nVertices(refinementLevel));

  // sample coords and data
  for(VertexIterator it = ref.vBegin(refinementLevel),
      end = ref.vEnd(refinementLevel);
      it != end; ++it) {
    typename VR::CoordVector coords = it.coords();
    vtkData.coords.push_back(coords);

    std::vector<RangeType> values;
    lb.evaluateFunction(coords, values);
    for(int b = 0, size = lb.size(); b < size; ++b)
      vtkData.data[b].push_back(values[b]);
  }

  // dump connectivity
  for(ElementIterator it = ref.eBegin(refinementLevel),
      end = ref.eEnd(refinementLevel);
      it != end; ++it)
    vtkData.connectivity.push_back(it.vertexIndices());
}

////////////////////////////////////////////////////////////////////////
//
//  Write subsampling to stream
//
template<typename Traits, typename Stream>
Stream &operator<<(Stream &s, const VTKData<Traits> &vtkData) {
  typedef typename Traits::RangeFieldType RangeFieldType;
  static const int dimRange = Traits::dimRange;
  typedef typename Traits::DomainFieldType DomainFieldType;
  static const int dimDomain = Traits::dimDomain;
  static const std::string vtkRangeFieldType  = vtkTypename<RangeFieldType >();
  static const std::string vtkDomainFieldType = vtkTypename<DomainFieldType>();

  s << "<VTKFile type=\"UnstructuredGrid\" version=\"0.1\""
  <<         " byte_order=\"LittleEndian\">\n";
  s << "  <UnstructuredGrid>\n";
  s << "    <Piece NumberOfPoints=\"" << vtkData.coords.size() << "\""
  <<           " NumberOfCells=\"" << vtkData.connectivity.size() << "\">\n";
  s << "      <PointData>\n";
  for(unsigned bf = 0; bf < vtkData.data.size(); ++bf) {
    s << "        <DataArray type=\"" << vtkRangeFieldType << "\""
    <<                   " format=\"ascii\""
    <<                   " Name=\"LocalFunction" << bf << "\"";
    if(dimRange > 1) s <<  " NumberOfComponents=\"" << dimRange << "\"";
    s <<                   ">\n";

    for(unsigned index = 0; index < vtkData.data[bf].size(); ++index) {
      s << "         ";
      for(int component = 0; component < dimRange; ++component)
        s << " " << fullPrecision(vtkData.data[bf][index][component]);
      s << "\n";
    }
    s << "        </DataArray>\n";
  }
  s << "      </PointData>\n";
  s << "      <CellData>\n";
  s << "      </CellData>\n";
  s << "      <Points>\n";
  s << "        <DataArray type=\"" << vtkDomainFieldType << "\""
  <<                   " format=\"ascii\" NumberOfComponents=\"3\">\n";
  for(unsigned index = 0; index < vtkData.coords.size(); ++index) {
    s << "         ";
    for(int component = 0; component < dimDomain && component < 3; ++component)
      s << " " << fullPrecision(vtkData.coords[index][component]);
    for(int component = dimDomain; component < 3; ++component)
      s << " 0";
    s << "\n";
  }

  s << "        </DataArray>\n";
  s << "      </Points>\n";
  s << "      <Cells>\n";
  s << "        <DataArray type=\"Int32\" format=\"ascii\""
  <<                   " Name=\"connectivity\">\n";
  for(unsigned index = 0; index < vtkData.connectivity.size(); ++index) {
    s << "         ";
    for(unsigned corner = 0; corner < vtkData.connectivity[index].size(); ++corner)
      s << " " << vtkData.connectivity[index][corner];
    s << "\n";

  }
  s << "        </DataArray>\n";
  s << "        <DataArray type=\"Int32\" format=\"ascii\" Name=\"offsets\">\n";
  int32_t current_offset = 0;
  for(unsigned index = 0; index < vtkData.connectivity.size(); ++index) {
    current_offset += vtkData.connectivity[index].size();
    s << "          " << current_offset << "\n";
  }
  s << "        </DataArray>\n";
  s << "        <DataArray type=\"UInt8\" format=\"ascii\" Name=\"types\">\n";
  for(unsigned index = 0; index < vtkData.types.size(); ++index) {
    s << "          " << int(vtkData.types[index]) << "\n";
  }
  s << "        </DataArray>\n";
  s << "      </Cells>\n";
  s << "    </Piece>\n";
  s << "  </UnstructuredGrid>\n";
  s << "</VTKFile>\n";

  return s;
}

////////////////////////////////////////////////////////////////////////
//
//  just write some subsampling to a pre-opened stream
//

template<typename LB>
void write_to_stream(std::ostream &s, Dune::GeometryType::BasicType basicGeo, int refinementLevel) {
  VTKData<typename LB::Traits> vtkData;

  sample(LB(), basicGeo, refinementLevel, vtkData);
  s << vtkData << std::flush;
}

////////////////////////////////////////////////////////////////////////
//
//  Main program
//

int main(int argc, char** argv)
{
  try {
    int refinementLevel = 3;
    Dune::GeometryType::BasicType geo = Dune::GeometryType::simplex;

    for(++argv; *argv; ++argv) {
      if(std::strcmp(*argv, "--help") == 0 ||
         std::strcmp(*argv, "-h")     == 0 ||
         std::strcmp(*argv, "-?")     == 0) {
        std::cout <<
        PROG_NAME " -- debug output for " LOCAL_BASIS_TYPE_S "\n"
        "\n"
        PROG_NAME " --help|-h|-?\n"
        "        Show this help.\n"
        PROG_NAME " [--geo|-g TYPE] [LEVEL]\n"
        "        Dump " LOCAL_BASIS_TYPE_S " to stdout as a vtu.  Refine\n"
        "        LEVEL times when doing so (default is 3).  If --geo or\n"
        "        -g is given, use that basic geometry type instead of\n"
        "        simplex.  Possible values for TYPE are those from\n"
        "        Dune::GeometryType::BasicType, except that pyramid and\n"
        "        prism are not supported yet by VirtualRefinement so\n"
        "        won't work.\n";
        return 0;
      }
      else if(std::strcmp (*argv, "--geo")     == 0 ||
              std::strncmp(*argv, "--geo=", 6) == 0 ||
              std::strncmp(*argv, "-g", 2)     == 0 ) {
        char *arg;
        if(std::strncmp(*argv, "--geo=", 6) == 0)
          arg = *argv + 6;
        else if(std::strncmp(*argv, "-g", 2) == 0 && (*argv)[2] != '\0')
          arg = *argv + 2;
        else {
          ++argv;
          arg = *argv;
        }

        if     (std::strcmp(arg, "simplex") == 0)
          geo = Dune::GeometryType::simplex;
        else if(std::strcmp(arg, "cube")    == 0)
          geo = Dune::GeometryType::cube;
        else if(std::strcmp(arg, "pyramid") == 0)
          geo = Dune::GeometryType::pyramid;
        else if(std::strcmp(arg, "prism")   == 0)
          geo = Dune::GeometryType::prism;
        else {
          std::cerr << "Error: Unknown geometry type " << arg << "\n";
          std::cerr << "Error: Try --help\n";
          return 1;
        }
      }
      else {
        std::istringstream s(*argv);
        s >> refinementLevel;
        if(s.fail()) {
          std::cerr << "Error: Unknown argument " << *argv << "\n";
          std::cerr << "Error: Try --help\n";
          return 1;
        }
      }
    }

    write_to_stream<LB>(std::cout, geo, refinementLevel);
  }
  catch (Dune::Exception &e) {
    std::cerr << "Dune reported error: " << e << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Unknown exception thrown!" << std::endl;
    return 1;
  }
}
