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

```
qemu-system-aarch64 \
  -machine virt,virtualization=on,gic-version=2 \
  -cpu cortex-a72 \
  -smp 4 \
  -m 1G \
  -nographic \
  -net none \
  -chardev stdio,id=con,mux=on \
  -serial chardev:con \
  -mon chardev=con,mode=readline \
  -kernel ./kernel.elf
```  

上記コマンドにより、テストメッセージがコンソール上に表示されれば起動成功です。