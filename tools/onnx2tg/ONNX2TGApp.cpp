//===- ONNX2TG.cpp --------------------------------------------------------===//
//
//                             The ONNC Project
//
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "ONNX2TGApp.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"
#include <cstdlib>
#include <memory>
#include <onnc/ADT/Color.h>
#include <onnc/ADT/ConstBuffer.h>
#include <onnc/Core/PassManager.h>
#include <onnc/IR/Module.h>
#include <onnc/IRReader/ONNXReader.h>
#include <onnc/Support/IOStream.h>
#include <onnc/Target/TargetBackend.h>
#include <onnc/Target/TargetOptions.h>
#include <onnc/Target/TargetRegistry.h>
#include <onnc/Target/TargetSelect.h>
#include <string>

using namespace onnc;
using namespace llvm;

class TGBackend;
//===----------------------------------------------------------------------===//
// ONNX2TG
//===----------------------------------------------------------------------===//
ONNX2TG::ONNX2TG(int pArgc, char *pArgv[])
    : onnc::CoreApplication(pArgc, pArgv), m_Config()
{
  InitializeAllPlatforms();
  InitializeAllBackends();
}

ONNX2TG::~ONNX2TG() {}

static ExitOnError ExitOnErr;

int ONNX2TG::compile()
{
  std::unique_ptr<Module> module;
  // TODO Figure out why Reader need to be defined here. (potential bug)
  onnc::onnx::Reader reader;
  {
    std::unique_ptr<MemoryBuffer> MB = ExitOnErr(
        errorOrToExpected(MemoryBuffer::getFileOrSTDIN(m_Config.input())));
    std::string data = MB.get()->getBuffer().str();
    onnc::ConstBuffer constBuffer(data);
    SystemError err;
    module = std::unique_ptr<Module>(reader.parse(constBuffer, err));
    if (!err.isGood()) {
      return EXIT_FAILURE;
    }
  }

  std::string quadruple;
  if (m_Config.march() == "bm1680") {
    quadruple = "sophonv1680-bitmain-linux-bmnet-all-0.1.0-none-tg";
  } else if (m_Config.march() == "bm1880") {
    quadruple = "sophonv1880-bitmain-linux-bmnet-all-0.1.0-none-tg";
  } else {
    ::onnc::errs() << Color::RED << "Error" << Color::RESET
                   << ": can not found march `" << m_Config.march() << "\n";
    return EXIT_FAILURE;
  }

  std::string error;
  const onnc::Target *target = TargetRegistry::Lookup(quadruple, error);
  if (nullptr == target) {
    ::onnc::errs() << Color::RED << "Error" << Color::RESET
                   << ": can not found target `" << quadruple << "`: " << error
                   << std::endl;
    return EXIT_FAILURE;
  }

  PassManager pm;
  TargetOptions options;
  if (m_Config.PrintModuleBeforeISel())
    options.PrintModuleBeforeSel = 1;
  TargetBackend *backend = target->createBackend(options);
  backend->addTensorSel(pm);
  backend->addMemAlloc(pm);
  backend->addCodeEmit(pm, m_Config.output());
  pm.run(*module);
  return EXIT_SUCCESS;
}
