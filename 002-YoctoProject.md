# YoctoProject学習1 arm64 qemuで動作させる
## 参考
http://wiki.yoctoproject.org/wiki/Transcript:_Using_the_Yocto_BSP_tools_to_create_a_qemu_BSP
## qemu-system-aarch64(ARM64)で動作するrootfsイメージをbuildする
### 1. git clone と branchへのスイッチ
'$git clone git://git.yoctoproject.org/poky'  
'$git checkout -b pyro origin/pyro'  
### 2. oe-init-build-env実行
'$source oe-init-build-env'  
### 3. BSPとしてサポートされているkernelアーキテクチャの確認
'$yocto bsp list karch'  
### 4. QEMU ARMのBSPレイヤの生成
'$yocto bsp create <BSP名> '
例：
'$yocto bsp create armqemu qemu'
### 5. インタラクティブな質問に答えながらコンフィグレーション
