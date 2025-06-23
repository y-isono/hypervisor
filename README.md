# hypervisor
ハイパーバイザーを作ってみる。

## ビルド、テスト方法

### x86 環境
```
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/cross-toolchain-clang-aarch64.cmake
cmake --build .
```

上記コマンドにより kernel.elf が作成されていればビルド成功です。