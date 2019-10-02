#include "block_gen.h"

void BlockGen::Generate() {
  GenHeaderFile();
  GenCppFile();
}

void BlockGen::GenHeaderFile() {
  mHeaderFile.WriteOneLine("#ifndef __BLOCK_GEN_H__", 23);
  mHeaderFile.WriteOneLine("#define __BLOCK_GEN_H__", 23);

  //mHeaderFile.WriteFormattedBuffer(&(FuncIsSeparator.mDecl));

  mHeaderFile.WriteOneLine("#endif", 6);
}

void BlockGen::GenCppFile() {
  //mCppFile.WriteFormattedBuffer(&(FuncIsSeparator.mHeader));
  //mCppFile.WriteFormattedBuffer(&(FuncIsSeparator.mBody));
}

