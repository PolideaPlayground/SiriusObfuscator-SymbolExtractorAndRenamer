#include "swift/Obfuscation/SourceFileWalker.h"
#include "swift/Obfuscation/DeclarationParser.h"
#include "swift/Obfuscation/DeclarationParsingUtils.h"
#include "swift/Obfuscation/ExpressionParser.h"
#include "swift/Obfuscation/ParameterDeclarationParser.h"
#include "swift/Obfuscation/Utils-Template.h"
#include "swift/Obfuscation/Utils.h"
#include "swift/Obfuscation/WhereClauseParser.h"

#include <string>

namespace swift {
namespace obfuscation {

struct SymbolsWalkerAndCollector: public SourceEntityWalker {

  static int SymbolIndex;
  std::set<IndexedSymbolWithRange,
           IndexedSymbolWithRange::SymbolWithRangeCompare> Symbols;

// Overriden methods called back as the AST is walked

  bool walkToExprPre(Expr *Expression) override {
    auto Symbols = extractSymbol(Expression);
    handleExtractionResult(Symbols);
    return true;
  }
  
  bool walkToDeclPre(Decl *Declaration, CharSourceRange Range) override {
    auto Symbols = extractSymbol(Declaration, Range);
    handleExtractionResult(Symbols);
    return true;
  }

  void handleWhereClausesIfNeeded(const std::vector<Decl*> &&Declarations) {
    WhereClauseParser WhereClauseParser;
    for (auto *Declaration : Declarations) {
      WhereClauseParser.collectSymbolsFromDeclaration(Declaration);
    }
    handleSymbols(WhereClauseParser.harvestSymbols());
  }

  // We need to extract symbols here even though we might have already seen
  // this node in the walkToDeclPre because here we have additional
  // contextual information like CtorTyRef
  bool visitDeclReference(ValueDecl *Declaration,
                          CharSourceRange Range,
                          TypeDecl *CtorTyRef,
                          ExtensionDecl *ExtTyRef,
                          Type T,
                          ReferenceMetaData Data) override {
    handleWhereClausesIfNeeded({ Declaration, CtorTyRef, ExtTyRef });
    auto Symbols = extractSymbol(CtorTyRef ? CtorTyRef : Declaration, Range);
    handleExtractionResult(Symbols);
    return true;
  }

// Methods for handling the extracted symbols

  void handleSymbol(const SymbolWithRange &Symbol) {
    auto InsertionResult =
      Symbols.insert(IndexedSymbolWithRange(SymbolIndex, Symbol));
    if (InsertionResult.second == true) {
      ++SymbolsWalkerAndCollector::SymbolIndex;
    }
  }

  void handleSymbols(std::vector<SymbolWithRange> &Symbols) {
    for (const auto &Symbol : Symbols) {
      handleSymbol(Symbol);
    }
  }

  void handleSymbols(std::set<SymbolWithRange> &&Symbols) {
    for (const auto &Symbol : Symbols) {
      handleSymbol(Symbol);
    }
  }

  void handleExtractionResult(SymbolsOrError &Symbols) {
    if (auto Error = Symbols.takeError()) {
      llvm::consumeError(std::move(Error));
    } else {
      handleSymbols(Symbols.get());
    }
  }

};

int SymbolsWalkerAndCollector::SymbolIndex;

std::set<IndexedSymbolWithRange, IndexedSymbolWithRange::SymbolWithRangeCompare>
walkAndCollectSymbols(SourceFile &SourceFile) {
  SymbolsWalkerAndCollector Walker;
  Walker.walk(SourceFile);
  return Walker.Symbols;
}

} //namespace obfuscation
} //namespace swift
