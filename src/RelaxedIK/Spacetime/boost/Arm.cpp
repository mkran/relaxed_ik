#include <iostream>
#include <vector>
#include <string>
#include <Eigen/Dense>
#include <boost/python.hpp>
#include <math.h>
#include <chrono>
// #include <python/numpy.hpp>


using namespace std;
using namespace boost::python;
using namespace Eigen;
using namespace std::chrono;


#define PI 3.14159265

class Arm {
public:
  int numDOF;
  list displacements;
  list axes;
  list rotOffsets;
  boost::python::tuple dispOffset;
  list velocity_limits;
  list joint_limits;
  Matrix3d rotX;
  Matrix3d rotY;
  Matrix3d rotZ;


  Arm(list axes, list displacements, list rotOffsets, boost::python::tuple dispOffset) {
    this->axes = axes;
    this->displacements = displacements;
    this->rotOffsets = rotOffsets;
    this->dispOffset = dispOffset;
    this->numDOF = len(displacements);

    this->rotX = MatrixXd::Zero(3,3);
    this->rotY = MatrixXd::Zero(3,3);
    this->rotZ = MatrixXd::Zero(3,3);

    this->rotX(0,0) = 1.0;
    this->rotY(1,1) = 1.0;
    this->rotZ(2,2) = 1.0;
  }

  Matrix3d& rot3(char axis, float s, float c) {
    if (axis == 'z' || axis == 'Z') {
      this->rotZ(0,0) = c;
      this->rotZ(0,1) = -s;
      this->rotZ(1,0) = s;
      this->rotZ(1,1) = c;
      return this->rotZ;
    }
    else if(axis == 'y' || axis == 'Y') {
      this->rotY(0,0) = c;
      this->rotY(0,2) = s;
      this->rotY(2,0) = -s;
      this->rotY(2,2) = c;
      return this->rotY;
    }
    else if(axis == 'x' || axis == 'X') {
      this->rotX(1,1) = c;
      this->rotX(1,2) = -s;
      this->rotX(2,1) = s;
      this->rotX(2,2) = c;
      return this->rotX;
    }
    else {
      cout << "ERROR: not a valid axis label";
    }
  }

  list getFrames(list state) {
    list ret;
    list pts;
    Vector3d pt = this->array(this->dispOffset);
    pts.append(this->tolist(pt));
    list frames;
    Matrix3d rot = MatrixXd::Zero(3,3);
    rot(0,0) = 1.0;
    rot(1,1) = 1.0;
    rot(2,2) = 1.0;
    frames.append(this->tolist(rot));

    for(int i = 0; i < this->numDOF; i++) {
      float s = sin(extract<float>(state[i]));
      float c = cos(extract<float>(state[i]));

      //TODO: do rotation offsets

      char curr_axis = extract<char>(this->axes[i]);

      Matrix3d rmat = this->rot3(curr_axis,s,c);
      rot = rot*rmat;
      Vector3d disp;
      disp(0) = extract<float>((this->displacements[i])[0]);
      disp(1) = extract<float>((this->displacements[i])[1]);
      disp(2) = extract<float>((this->displacements[i])[2]);
      // cout << rot(1,1);
      // cout << "\n";
      // high_resolution_clock::time_point t1 = high_resolution_clock::now();
      pt = (rot*disp) + pt;
      // high_resolution_clock::time_point t2 = high_resolution_clock::now();
      // duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      // std::cout << "It took me " << time_span.count() << " seconds.\n";

      pts.append(this->tolist(pt));
      frames.append(this->tolist(rot));
    }

    ret.append(pts);
    ret.append(frames);
    return ret;
  }

private:

  Vector3d array(list input) {
    Vector3d v;
    v(0) = extract<float>(input[0]);
    v(1) = extract<float>(input[1]);
    v(2) = extract<float>(input[2]);
    return v;
  }

  Vector3d array(boost::python::tuple input) {
    Vector3d v;
    v(0) = extract<float>(input[0]);
    v(1) = extract<float>(input[1]);
    v(2) = extract<float>(input[2]);
    return v;
  }

  list tolist(Vector3d v) {
    list l;
    l.append(v(0));
    l.append(v(1));
    l.append(v(2));
    return l;
  }

  list tolist(Matrix3d m) {
    list ret;
    list row1;
    list row2;
    list row3;

    row1.append(m(0,0));
    row1.append(m(0,1));
    row1.append(m(0,2));

    row2.append(m(1,0));
    row2.append(m(1,1));
    row2.append(m(1,2));

    row3.append(m(2,0));
    row3.append(m(2,1));
    row3.append(m(2,2));

    ret.append(row1);
    ret.append(row2);
    ret.append(row3);

    return ret;
  }
};

BOOST_PYTHON_MODULE(Arm_ext) {
    class_<Arm>("Arm", init<list, list, list, boost::python::tuple>())
        .def("getFrames", &Arm::getFrames)
        .def_readwrite("numDOF", &Arm::numDOF)
        .def_readwrite("velocity_limits", &Arm::velocity_limits)
        .def_readwrite("rotOffsets", &Arm::rotOffsets)
        .def_readwrite("dispOffset", &Arm::dispOffset)
        .def_readwrite("joint_limits", &Arm::joint_limits)
        .def_readwrite("displacements", &Arm::displacements)
        .def_readwrite("axes", &Arm::axes)

    ;
}