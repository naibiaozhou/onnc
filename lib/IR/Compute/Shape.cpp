//===- Shape.cpp ------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <onnc/IR/Compute/Shape.h>

using namespace onnc;

char Shape::ID = 0;

//===----------------------------------------------------------------------===//
// Shape
//===----------------------------------------------------------------------===//
Shape::Shape()
  : ComputeOperator("Shape", ID) {
}



void Shape::print(std::ostream& pOS) const
{
}

bool Shape::classof(const ComputeOperator* pOp)
{
  if (nullptr == pOp)
    return false;
  return (pOp->getID() == &ID);
}
