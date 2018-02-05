#ifndef NominalTypeDeclarationParser_h
#define NominalTypeDeclarationParser_h

#include "swift/Frontend/Frontend.h"
#include "swift/Obfuscation/DataStructures.h"
#include "swift/Obfuscation/DeclarationParsingUtils.h"

namespace swift {
namespace obfuscation {

llvm::Expected<ModuleNameAndParts>
nominalTypeIdentifierParts(const NominalTypeDecl *Declaration,
                           const std::string &ModuleName,
                           const std::string &SymbolName);
  
SingleSymbolOrError parse(const NominalTypeDecl* Declaration);

} //namespace obfuscation
} //namespace swift

#endif /* NominalTypeDeclarationParser_h */