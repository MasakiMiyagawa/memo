# YoctoProject学習: レシピの作り方入門
## 参考
https://www.slideshare.net/iwamatsu/ss-31662659  
## layerを作る方法
'$git clone git://git.yoctoproject.org/poky'  
'$git checkout -b pyro origin/pyro'  
'cd poky'  
'scripts/yocto-layer create my-recipes'
## ビルド対象にレイヤを加える方法
build/conf/local.confのBB_LAYERSに追加レイヤを書く  
bitbakeすると依存レイヤのチェックが行われる。都度探す。   
## recipeに含まれるtaskはなにかを調べる方法
'bitbake -c listtasks <recipe name>'  
例：busyboxの場合  
'bitbake -c listtasks busybox'  
## ビルド環境に含まれるレイヤーはなにか知る
'bitbake-layers show-layers'  
## レイヤに含まれるレシピをチェック
'bitbake-layers show-recipes | grep -A 1 ^example'
## レシピが含まれるレイヤーをチェック
'bitbake-layers show-recipes example'
## レシピの状態(bbappendで変更されているもの)を確認する方法
'bitbake-layers show-appends'
## downloadディレクトリの指定方法
build/conf/local.confのDL_DIRを指定
## 特定のレシピのworkを消去+タスクの初期化
'bitbake <recipe-name> -c cleansstate'
## レシピに出てくる変数の意味
* PR: レシピレビジョン 
* PN: レシピ名 .bbappendはPN,PVが一致していると追加される
* PV: レシピバージョン .bbappendはPN,PVが一致していると追加される
* DEPENDS: レシピのビルドに必要なレシピ
* RDEPENDS: レシピ成果物実行に必要なレシピ
* EXTRA_OECONF: configure実行オプション
* EXTRA_OEMAKE: make実行オプション
## レシピのタスクを上書きするとき
### bbファイルまたはbbappendファイルにてdo_configureを上書きする例
	do_configure() {
		echo "Override do_configure"
	}
### bbファイルまたはbbappendファイルにてdo_configureの前に処理を追加する例
	do_configure_pretend() {
		echo "Pretend do_configure"
	}
### bbファイルまたはbbappendファイルにてdo_configureの前に処理を追加する例
	do_configure_append() {
		echo "Append do_configure"
	}
## 自作タスクを定義する
### do_configureの前に実行されるdo_mytask_a()
	do_mytask_a() {
		echo "new task"
	}
	addtask mytask_a before do_configure
### do_configureの後に実行されるdo_mytask_b()
	do_mytask_b() {
		echo "new task"
	}
	addtask mytask_b before do_configure
# YoctoProject学習: arm64 qemuで動作させるBSPを追加
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

