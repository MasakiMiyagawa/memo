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
'Which qemu architecture would you like to use? -> 3) ARM'  
'Would you like to use the default (4.10) kernel? -> y'  
'Do you need a new machine branch for this BSP? -> y'  
'Please choose a machine branch to base your new BSP branch on -> standard/qemuarm64'  
'Would you like SMP support -> y'  
'Does your BSP has touchscreen? -> n'  
'Does your BSP has keyboard? -> y' 
meta-<BSP名>というレイヤーが生成される。 
### 6.レイヤーを追加
build/conf/bblayers.confを修正  
	BBLAYERS ?= " \
		/home/miyagawa/work/yocto/poky/meta \
		/home/miyagawa/work/yocto/poky/meta-poky \
		/home/miyagawa/work/yocto/poky/meta-yocto-bsp \
		/home/miyagawa/work/yocto/poky/build/meta-test \
		"
### 7.ターゲットとなるmachineをarm64に変更
build/conf/local.conf
	MACHINE ??= "qemuarm64"

### 8.runqemuで実行(qemuもyoctoでビルドされたものが使用される)
'runqemu tmp/deploy/images/qemuarm64/core-image-minimal-qemuarm64-20170906122920.qemuboot.conf nographic'

