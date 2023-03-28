
TODO:
TODO:

clang tidy
clang static analyzer   {
    "name": "Clang 16.0.0 x86_64-apple-darwin21.6.0",
    "compilers": {
      "C": "/usr/local/Cellar/llvm/16.0.0/bin/clang",
      "CXX": "/usr/local/Cellar/llvm/16.0.0/bin/clang++"
    },
    "cmakeSettings": [
      "-DCMAKE_CXX_FLAGS_ALL_WARNINGS:STRING=\"-Wall\""
    ]
  }