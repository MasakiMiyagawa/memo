# 参考URL
http://www.yoctoproject.org/docs/2.3.1/bitbake-user-manual/bitbake-user-manual.html
# Chapter 1. Overview
## 1.1.Introduction
## 1.2.History and Goals
## 1.3.Concepts
  BitBakeはPythonで書かれたプログラム。GNU Makeがmakefileを入力としてタスクを実行するのと同様レシピを入力としてタスクを実行する。
## 1.3.1.Recipes
  .bb,.bbappendファイル。
## 1.3.2.Configuration Files
　.confファイル。ビルド対象マシンアーキテクチャの指定、コンパイラオプション、配布オプションなど。
## 1.3.3.Classes
  .base .bbclassファイル。メタデータ間でのデータ共有に用いる。フェッチ、アンパック、コンパイルなどの標準タスクの動作が定義されている。プロジェクトで開発されたクラスによってオーバーライドされることがある。
## 1.3.4.Layers
  confディレクトリを持つ親ディレクトリ。
## 1.3.5.Appendfiles
  .bbappendのワイルドカード記法  
`busybox_1.21.%.bbappend`  
は以下にマッチする。
- busybox_1.21.1.bb  
- busybox_1.21.2.bb  
- busybox_1.21.3.bb  
## 1.4.Obtaining Bitbake
## 1.5.The BitBake Command
オプションの表示  
`$ bitbake -h`  
依存関係グラフの表示  
`$ bitbake -g`  
## 1.5.1.Usage and syntax
## 1.5.2.Example
単一のレシピに対してタスクを実行(依存関係無視)  
`$ bitbake -b foo_1.0.bb`  
一連のレシピファイルに対するdo_cleanの実行  
`$ bitbake -c clean foo`
依存関係グラフの生成。fooが依存するレシピのグラフを表示。virutal/kernelとeglibcは省略。
`$bitbake -g -Ivirtual/kernel -I egilbc foo`  
生成される4つのファイル  
- package-depends.dot : Graphizで表示できるランタイムターゲットの依存関係  
- pn-depends.dot : レシピ間の依存関係を表示  
- task-depends.dot : タスク間の依存関係  
- pn-buildlist : ビルドされるターゲットのシンプルなリストを表  
# Chapter 2. Execution
　BitBakeを走らせる主要な目的はimageやkernelやSDKのアウトプットを得ることである。もちろんBitBakeコマンドにオプションを加えて実行することで、単一のレシピのコンパイル、クリアデータのキャプチャ、実行環境の情報取得等も可能である。このチャプタではImageをつくるまでの最初から最後までのBitBakeの実行プロセスを記述する。プロセスは以下のコマンドで起動する。  
`$ bitbake <target>`
## 2.1.Parsing the Base Configuration Metadata
　まず最初にBitBakeが行うことはmetadataのコンフィグレーションのパースである。metadataの基本コンフィグレーションはbblayers.confファイルにあり、BitBakeが認識すべきlayerを定義する。さらにすべてのlayerに配置されたlayer.confファイル及びbitbake.confファイルが基本コンフィグレーションである。   
- Recipes: 特定のソフトウェアの詳細
- Class Data:共通のビルド情報のアブストラクト(e.g. どのようにLinux Kernelをビルドするかなど)
- Configuration Data: MACHINE依存の設定、ポリシー定義など  

layerに配置されているlayer.confはBBPATHやBBFILESなど値を設定するために使用する。BBPATHはconfやclassディレクトリの下にあるconfigurationファイルやclassファイルを探索するために用いられる。BBFILESはrecipe(.bb and .bbappend)を探索するために用いられる。bblayers.confが存在しない場合、bitbakeはBBPATHやBBFILESは環境変数に定義されているとみなす。  
　次にbitbake.confはBBPATHいよって探索される。bitbake.confはincludeやrequireディレクティブを用いて他のコンフィグレーションを設定に含める。  
コンフィグレーションをパースするためにBitBakeは以下に含まれる定数をみる。  

- BB_ENV_WHITELIST
- BB_PRESERVE_ENV
- BB_ENV_EXTRAWHITE
- BITBAKE_UI  

　環境変数がどのようにBitBakeの実行環境に渡されるかは"Passing Information Into the Build Task Environment"を参照。  
　基本コンフィグレーションmetadataはグローバルであるため、すべてのレシピのタスク実行に影響を与える。  
　BitBakeはconf/bblayers.confから探索を開始し、このファイルのBBLAYERSにはスペースで区切られたlayerディレクトリのリストが記述されている。  
  BBLAYERSに示されたパスは、layerごとのconf/layer.confを探索するためのパスであり、LAYERDIRにはlayerが見つかったディレクトリパスが示されている。この手法はBBPATHや他の変数がlayerごとに自動的に正しくセットされることを目的としている。  
　BitBakeがBBPATHによりconf/bitbake.confを見つけた時(レイヤがmeta,meta-poky,meta-poky-bspのみ場合、bitbake.confはmeta/conf/bitbake.confが見つかる)特定のアーキテクチャ、MACHINE、ローカル環境に依存するコンフィグレーションファイルがインクルードされる。	  
　以下はMACHINEをqemuarm64に定義して、bitbake -e core-image-minmalを実行した時のログbitbake.confがqemuarm64.confをインクルードするため、MACHINEに依存してarch-armv8.incなどがincludeされていく様子がわかる。  
```
  # conf/bitbake.conf includes:
  #   /home/miyagawa/poky/meta/conf/abi_version.conf
  #   conf/site.conf
  #   conf/auto.conf
  #   /home/miyagawa/poky/build/conf/local.conf
  #   /home/miyagawa/poky/meta/conf/multiconfig/default.conf
  #   /home/miyagawa/poky/meta/conf/machine/qemuarm64.conf includes:
  #     /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv8.inc includes:
  #       /home/miyagawa/poky/meta/conf/machine/include/arm/arch-arm64.inc includes:
  #         /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv7ve.inc includes:
  #           /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv7a.inc includes:
  #             /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv6.inc includes:
  #               /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv5-dsp.inc includes:
  #                 /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv5.inc includes:
  #                   /home/miyagawa/poky/meta/conf/machine/include/arm/arch-armv4.inc includes:
  #                     /home/miyagawa/poky/meta/conf/machine/include/arm/arch-arm.inc
  #                     /home/miyagawa/poky/meta/conf/machine/include/arm/feature-arm-thumb.inc
  #                   /home/miyagawa/poky/meta/conf/machine/include/arm/feature-arm-vfp.inc
  #             /home/miyagawa/poky/meta/conf/machine/include/arm/feature-arm-neon.inc
  #     /home/miyagawa/poky/meta/conf/machine/include/qemu.inc
  #   /home/miyagawa/poky/meta/conf/machine-sdk/x86_64.conf
```  
　変数の定義とincludeディレクティブのみが.confファイルで許されている。いくつかの変数はBitBakeそのものの振る舞いに直接影響する。これらの変数は.confで定義されたり、環境変数によって定義されたりする。"Variable Glossary"にてすべての変数のリストを示している。   
　コンフィグレーションファイルをパースした後、BitBakeは初歩的な継承メカニズム(inheritディレクティブによる)を用いる。  
  base.bbclassはbitbake.confと同様に常にインクルードされる。inheritディレクティブに示された他のクラスも同様である。BitBakeはclassesサブディレクトリを探索してbbclassを見つけ出す。  
## 2.2.Locating and Parsing Recipes
  コンフィグレーションをパースする間、BitBakeはBBFILEをセットする。BitBakeは現在、パースするレシピのリストをBBFILESに加える(.bbappendも同様に加えられる)。BBFILESはワイルドカードをサポートするスペース区切りのリストである。以下に例を示す。  
`BBFILES = "/path/to/bbfiles/*bb /path/toappends/*bbappends"`  
  それぞれのファイルにおいて一行パースするごとにフレッシュコピーとなるコンフィグレーションが生成される。inheritステートメントが示されるとき、BitBakeはBBPATHに従い、.bbclassを探索する。  
　ひとつの共通のしきたりはメタデータの定義にファイル名を用いることである。例えば、bitbake.confは各レシピのPN,PVを以下のように定義している。   
`PN = "${@bb.parse.BBHandler.vars_from_file(d.getVar('FILE', False),d)[0] or 'defaultpkgname'}"`  
`PV = "${@bb.parse.BBHandler.vars_from_file(d.getVar('FILE', False),d)[1] or '1.0'}"`  
この定義により、例えば"something_1.2.3.bb"というレシピのPNはsomethingとなり、PVは1.2.3となる。  
　レシピのパースの完了により、BitBakeはタスクのリストを生成し、キーと値からなるデータセット、レシピの定義、様々な変数もまた、タスクの依存関係に影響するものとなる。  
　BitBakeはこれらの情報を必要としない。BitBakeはレシピを決定するとても小さなデータセットのみ必要とするのである。Bitbakeは関連する情報をキャッシュするが、残りの情報はストアしないのである。経験的にディスクの値を読み出すより、レシピを再度パースするほうが早いことが知られているためである。  
　BitBakeは出来る限りレシピ情報のキャッシュを用いる。このキャッシュの有効性は、最初に計算されるチェックサム(BB_HASHCONFIG_WHITELIST)で評価される。  
　Recipe file collectionはユーザがあるパッケージのために複数のbbファイルのリポジトリを持つことを許容する(実装の異なる同じパッケージをsysrootにインストールする？)ために存在する。例えば、  
```
	BBFILES = "/stuff/openembedded/*/*.bb /stuff/openembeded.modified/*/*.bb"
	BBFILE_COLLECTIONS = "upstream local"
	BBFILE_PATTERN_upstream = "^/stuff/openembeded/"
	BBFILE_PATTERN_local = "^/stuff/openembeded.modified/"
	BBIFLE_PRIORITY_upstream = "5"
	BBFILE_PRIORITY_local = "10"
```
## 2.3 Preferences and Providers
  BitBakeが実行されて、すべてのレシピがパースされたとして、BitBakeはどのようにTargetをビルドするのか見極める。まず、BitbakeはレシピのPROVIDES変数をみる。デフォルトのPROVIDESはレシピのPNであるが、レシピは他の名前を提供することもできる。  
  extra providerを追加する例として、foo_1.0.bbが以下のステートメントを含んでいたとする。   
`PROVIDES += "virtual/bar_1.0`
  このレシピは"foo_1.0"という名前と"virtual/bar_1.0"という名前のプロバイダとなる。"virtual/"という名前空間は同じパッケージを提供する複数のプロバイダがいる場合にしばしば利用される。  
  あるターゲットに複数のプロバイダが予想されるときの例としてKernelを上げる。"virtual/kernel"というプロバイダ名はそれぞれのkernelのレシピにて定義されている。MACHINEを定義するとMACHINEのコンフィグレーションファイルは最も好ましいkernelのプロバイダを以下のように定義している。これはつまり、あるレシピがkernelに依存しているとき、"virtual/kernel"という名前を持つレシピがプロバイダになるが、その候補は複数ある。この内どれを選択するかを示すのがPREFERRED_PROVIDERである。MACHINEにqemuarm64を設定した場合は、qemu.incにPREFERRED_PROVIDERが以下のように定義されているので、kernelのレシピを提供するプロバイダは"linux-yocto"である。  
`PREFERRED_PROVIDER_virtual/kernel = "linux-yocto"`  
  デフォルトのPREFFERRED_PROVIDERはターゲットの名称と同じである。  
  どのようにプロバイダが選択されるかを理解することは複数のバージョンのレシピが存在しうることが困難にしている。BitBakeはDebianと同じ方式でレシピおバージョンを評価し、ユーザはPREFERRED_VERSIONでバージョンの指定ができる。また、バージョン評価のオーダもDEFAULT_PREFERENCEで変更可能である。デフォルトでは本値は0であり、各レシピでDEFAULT_PREFERENCEを-1にすると、レシピの優先度が下がり、DEFALT_PREFERENCEを1にするとレシピが選択されやすくなる。しかし、PREFERRED_VERSIONはいかなるDEFAULT_PREFERRENCEより優先される。  
  レシピのバージョンはPVである。レシピ内で上書きされない限りa_1.1.bbというレシピのPNはaになり、PVは1.1となる。ここでa_1.2.bbというレシピが存在する場合、Bitbakeは1.2をデフォルトで選択するが、PREFERRED_VERSION_a = "1.1"が.confファイルで定義されていたら、a_1.1.bbが選択されることになる。
## 2.4.Dependencies
  BitBakeのタスクはfetch,unpack,patch,configure,compileのような複数のタスクからなる。マルチコアシステムのパフォーマンスを最大限に発揮させるため、BitBakeはそれぞれのタスクの依存関係を考慮する。  
  依存関係はいくつかの変数によって定義される(Variables Glossary参照)。基本的なレベルではDEPENDSとRDEPENDSを使用していると理解すること。より詳細な情報はDependenciesセクションに記述している。  
## 2.5.The Task List
  生成されたプロバイダのリストや依存関係の情報によって、Bitbakeは必要なタスクとそれをどの順番で実行すべきかを正しく計算する。Executing TasksセクションはBitBakeがどのような順でタスクを実行するかを詳細に記述している。
  タスクはBB_NUMBER_THREADSで定義されたスレッド数で並列実行する。適切なBB_NUMBER_THREADS設定によりより良いパフォーマンスが得られる。
  それぞれのタスクが完了した時、timestampがSTAMP変数で定義されたディレクトリに書き込まれる。サブシーケンスの起動において、BitBakeはビルドディレクトリのtmp/stampを監視し、すでに完了したタスクは再実行しないようにしている。現在timestampはレシピ毎にせっていされる。例えばconfigure stampがcompile timestmapより大きかったらcompile taskは再実行される。しかし、このレシピに依存するプロバイダには影響しない。   
## 2.6.Executing Tasks
  タスクは、shellのタスクまたはpythonのタスクになりうる。すべてのshellタスクについてはBitbakはshellスクリプトを${T}/run.do_taskname.pidファイル書き、スクリプトを実行する。生成されたスクリプトはexportされた変数とshell関数を含む。${T}/log.do_taskname.pidにshellスクリプトの出力が記述される。  
  BitBakeのタスクはそのタスクスケジューラによって制御される。これは、スケジューラを変更できる可能性を残している。より詳細な情報はBB_SCHEDULERやBB_SCHEDULERSに記載される。
　タスクの前後に動作するタスクを追加することも可能である。これはprefuncsやpostfuncsフラグを使うことで可能である。   
  以下はmeta/classes/autotools.bbclassの記述
```
do_configure[prefuncs] += "autotools_preconfigure autotools_aclocals ${EXTRACONFFUNCS}"
do_configure[postfuncs] += "autotools_postconfigure"
```   
## 2.7.Checksums (Signatures)
  Checksumはシグネチャーというタスク入力の一つである。シグネチャーはタスクが再実行されるひつようがあるかを判定するために用いられる。つまり、入力されるシグネチャーが変更されている場合、BitBakeはそのタスクの入力値の何かが変更されたと判断し、タスクを再実行する。
  複雑な問題のため、いくつかの要素はChecksumに含まれない。まず、ワーキングディレクトリのパスである。ワーキングディレクトリの変更はタスクの再実行が起きないようにするべきである。Cheksumに含まれない情報はBB_HASHBASE_WHITELISTで定義される。
　他の問題はスクリプトが呼び出されないシェル関数を持っている可能性である。インクリメンタルなビルドはシェル関数の依存関係を発見するコードを持つ。このコードは動作するスクリプトを最小のセットに切り出す役割を持ち、呼び出されない可能性がある関数の問題を軽くしたうえ、スクリプトの可読性を向上している。  
  シェルスクリプトについては一定の解決方法を導き出しているが、Pythonのタスクについてはどうか。同じアプローチを適用してもPhthonの場合、より困難である。あるPythonのタスクが他のどのPython関数を呼び出すのか判断しなくてはならない。重複するが、インクリメンタルなビルドは関数・変数の依存関係を解決し、タスクのインプットとなるChecksumを生成する。
  ワーキングディレクトリのように何らかの依存関係が無視されてほしいケースは祖納する。このようなケースではビルドプロセスに対し以下のように無視する依存関係を指定できる。   
`PACKAGE_ARCHS[vardepsexclude]="MACHINE"`  

  この例ではPACKAGE_ARCHSがMACHINEに依存しないように設定されている。PACKAGE_ARCHSとMACHINEの依存関係が切られている。MACHINEが変更されてもPACKAGE_ARCHの入力Checksumは変更されないため、MACHINEの変更によりPACKAGE_ARCHSに依存するものがPACKAGE_ARCHSが変更されたと見なさなくなる。
  同様にBitBakeが発見できない依存関係を追加するには以下である。例えばinline-Pythonの場合。
`PACKAGE_ARCHS[vardeps]="MACHINE"`  

  meta/classes/distutils3.bbclassではdistutils3_do_compileがMACHINEに依存していないことを明記している。

`distutils3_do_compile[vardepsexclude] = "MACHINE"`  

  このセクションではタスクへの直接入力について言及してきたが、基本はbasehashを用いるのである。しかしながら、直接入力ではない依存関係(すでにビルドディレクトリでビルドされたものなど)が存在する。特定のタスクのためのChecksumやSignatureはそのタスクが依存するすべてのタスクのハッシュが追加されなければならない。どの依存関係が用いられるかはポリシーで決定することだが、その効果はbasehashとタスクの依存関係のhashを混ぜあわせるマスターChecksumを生成することである。
  コードレベルでは、basehashとタスクの依存関係のhash両方に影響する方法は複数ある。BitBakeコンフィグレーションファイルでは、BitBakeにbasehashを定義するためのいくつかの追加情報を渡すことができる。以下のステートメントは依存関係から除外されるグローバル変数を定義するもである(meta/conf/bitbake.confより)。この例ではワーキングディレクトリTMPDIRが除外されていることがわかる。
```
BB_HASHBASE_WHITELIST ?= "TMPDIR FILE PATH PWD BB_TASKHASH BBPATH BBSERVER DL_DIR \
    SSTATE_DIR THISDIR FILESEXTRAPATHS FILE_DIRNAME HOME LOGNAME SHELL TERM \
    USER FILESPATH STAGING_DIR_HOST STAGING_DIR_TARGET COREBASE PRSERV_HOST \
    PRSERV_DUMPDIR PRSERV_DUMPFILE PRSERV_LOCKDOWN PARALLEL_MAKE \
    CCACHE_DIR EXTERNAL_TOOLCHAIN CCACHE CCACHE_NOHASHDIR LICENSE_PATH SDKPKGSUFFIX \
    WARN_QA ERROR_QA WORKDIR STAMPCLEAN PKGDATA_DIR BUILD_ARCH SSTATE_PKGARCH \
    BB_WORKERCONTEXT BB_LIMITEDDEPS extend_recipe_sysroot DEPLOY_DIR"
```  

　依存タスクのハッシュが依存関係のチェインに含まれるかの決定ルールは、Pythonによりより複雑で一般的に解決される。meta/lib/oe/sstatesig.pyは2つの例を示しており、さらにユーザがどのようにユーザ自身のポリシーをビルドシステムに組み込むかを描いている。このファイルは2つの基本的なシグネチャージェネレータをていぎしている　(OEBasig,OEBasicHash)。デフォルトではダミーのnoopシグネチャハンドラがBitBakeにより有効化される。これは振る舞いが以前のバージョンから変更されていないことを意味している。OE-COreは"OEBasicHash"シグネチャハンドラを以下のbitbake.confファイルのせっていにより使用する。  
`BB_SIGNATURE_HANDLER ?= "OEBasicHash"`  


  OEBasicHashはOEBasicと同様であるがstampファイルハッシュする機能を保持している。結果として、タスクのhashを変更するmetadataの変更が、タスクの再起動を自動的に発生させるのである。  
  これらのシグネチャジェネレータの結果が依存関係を済々し、ハッシュ情報がビルドのために歌謡であることは注目すべきである。これらの情報は以下を含む。
- BB_BASEHASH_task-<taskname> : タスクごとのbase hashに含まれるもの。
- BB_BASEHASH_<filename:taskname> : あるレシピのタスクのbase hashに含まれるもの
- BB_BASEDEPS_<filename:taskname>: あるレシピのタスクが依存するタスク
- BB_TASKHASH : 現在動作しているタスクのハッシュ  

  BitBakeのオプション-SはBitBakeのシグネチャを調べるのに有効であることに注意すべきである。-Sは異なる複数のデバッギングモードでBitBakeを起動するが最もシンプルなパラメタ-S "none"を渡すことでこれはシグネチャ情報のセットがビルドターゲットに関連するSTAMP_DIRにかかれる。他にも-S "printdff"があり、BitBakeは最も近いシグネチャを生成することを試みる。

## 2.8. Setscene
  setsceneプロセスはBitbakeはビルド済みの成果物を利用できる。これらの成果物を再利用することで、Bitbakeは何度も同じことをすうる必要がないという利点が得られる。BitBakeは成果物が互換性があるかを示す信頼性の高いデータを必要とし、2.7で説明したシグネチャは互換性の検証に理想的な方法を提供する。シグネチャが同じなら成果物の再利用が可能となる。  
  もしオブジェクトが再利用可能であれば、問題はどのように与えられたタスクまたはタスクのセットを事前にビルドされた成果物に置き換えるかということになる。setsceneはこの問題を解決する。  
  BitBakeがビルドするターゲットから問い合わせられた時、何かをビルドする前に、キャッシュされた情報があるか、またはタスク途中のターゲットがあるかを確認する。BitBakeはこれらの情報をメインタスクの起動の代わりに使用する。  
  BitBakeが最初に呼び出す関数はBB_HASHCHECK_FUNCTIONに定義されており、たくさんのタスクと関連するハッシュが示されている。この機能は成果物を得るために必要なタスクのリストを素早く返す。  
  次に、返される可能性があるタスクについて、BitBakeは可能な成果物がカバーするタスクの設定されたバージョンを実行する。タスクのセッションバージョンには、タスク名に"_setscene"という文字列が付加されている。たとえば、xxxという名前のタスクにはxxx_setsceneという名前のタスクがある。タスクの設定されたバージョンが実行され、成功または失敗のいずれかを戻すのに必要な成果物が提供される。  
  すでに述べたように、成果物はタスク以上をカバーできる。例えば、もし、あなたがすでにコンパイルされたバイナリを持っていたら、コンパイラを得ることは無意味である。この問題を扱うため、BitBakeはすべての成功したsetsceneタスクがタスクの依存関係を知る必要があるか否かを知るために、BB_SETSENE_DEPVALIDを呼び出す。  
  最後にすべてのsetsceneタスクが実行された後、BitBakeはBB_SETSCENE_VERIFY_FUNCTIONのリストをBitBkaeが"coverd"と判断したタスクとともに呼びだす。metadataはリストが正しい事を確認し、設定結果に関係なく特定のタスクを実行することをBitBakeに通知できる。この詳細はTask Checksums and Setsceneセクションに記載されている。

# 3.Syntax and Operators
## 3.1. Basic Syntax
## 3.1.1. Basic Variable Setting
変数に値を代入する  
`VARIABLE = "value"`  
スペースは保持される  
`VARIABLE = " value"`  
空値にしたければ空の文字列を代入する。  
`VARIABLE = ""`
## 3.1.2. Variable Expansion
Bashとほぼ同じルールで""の中で変数値を展開する。
`A = "aval"`  
`B = "pre${A}post"`  
この時、Bはpreavalpostである。BはAに依存した変数であるので、この後Aの値が変更されたらそのタイミングでBも変更される。  
## 3.1.3. Setting a default value (?=)
ステートメントがパースされたタイミングでAがセットされていなければA=avalになる。  
`A ?= "aval"`  
## 3.1.4. Setting a weak default value (??=)
すべてのパースが完了したタイミングでAがセットされていなければA=avalになる。
`A ??= "aval"`  
## 3.1.5. Immediate variable expansion (:=)
`:=`は変数の拡張を即時に行う。
`T = "123"`  
`A := "${B} ${A} test ${T}"  
`T = "456"`  
`B = "${T} bval"`  
`C = "cval"`  
`C := "${C}append"  
Cはcvalappendとなる。Aはtest 123となる。   
## 3.1.6. Appending (+=) and prepending (=+) With Spaces
`B = "bval"`  
`B += "additionaldata"`  
`C = "cval"`  
`C =+ "test"`   
Bはbval additionaldataとなり、Cはtest cvalとなる。 
## 3.1.7. Appending (.=) and Prepending (=.) Without Spaces
`B = "bval"`  
`B += "additionaldata"`  
`C = "cval"`   
`C =+ "test"`   
Bはbvaladditionaldataとなり、Cはtestcvalとなる。 
## 3.1.8. Appending and Prepending (Override Style Syntax)
`.=`や`.=`と同じ
`B = "bval"`  
`B_append = " additional data"`  
`C = "cval"`  
`C_prepend = "additional data "`  
`D = "dval"`  
`D_append = "addtional data"`  
この時Bはbval additional data、Cはadditional data cval、Dはdvaladdtional dataとなる。Spaceは入らないので明示する必要あり。
## 3.1.9. Removal (Override Style Syntax)
`FOO = "123 456 789 123456 123 456 123 456`  
`FOO_remove = "123"`  
`FOO_remove = "456"`  
`FOO2 = "abc def ghi abcdef abc def abc def"`  
`FOO2_remove = "abc def"`  
この時FOOは789 123456となり、FOO2はghi abcdefとなる。
## 3.1.10. Variable Flag Syntax
`FOO[a] = "abc"`  
`FOO[b] = "123"`  
`FOO[a] += "456"`  
配列に近い考え方。ただし、要素インデックスは任意。定義、append、pretendなどのシンタックスはオーバーライドシンタックスを除き動作する。 
この時FOO[a]はabc 456 FOO[B]は123。
## 3.1.11. Inline Python Variable Expansion
`DATE = "${@time.strftime('%Y%m%d',time.gmtime())}"
インラインPythonの結果を変数定義できる。
## 3.1.12. Providing Pathnames
パス名を指定する場合"~"は使用できない。
## 3.2. Condtional Syntax (Overrides)
BitBakeはBitBakeによるパースの後どの変数がオーバーライドされるかを制御するOVERRIDESという考え方を使う。
このセクションではどのようにOVERRIDESを使用するか、OVERRIDESに関連するキーの拡張をいくつかの例とともに記述数r。
### 3.2.1. Conditional Metadata
特定のバージョンの変数を書き換えたりappend,prependを適用したりできる。
OVERRIDES変数はコロンでセパレートされたリストである。  
- Selecting a Variable:  
`OVERRIDES = "architecture:os:machine"`  
`TEST = "default"`  
`TEST_os = "osspecific"`  
`TEST_nooverride = "othiercondvalue"`  
この例の場合、OVERRIDEはarchitecture,os,machineの3つの条件からなる。TEST自身の値はdefaultで初期化されているが、このレシピ実行時にosが選択されていたら、TESTの値はosspecificと解釈される。
- Appending and Prepending
`DEPENDS = "glibc ncurses"`  
`OVERRIDES = "machine:local"`  
`DEPENDS_append_machine = "libmod"`  
machineが選択された時、DEPENDSはglibc ncurses libmodになる。　　
### 3.2.2. Key Expansion
`A${B} = "X"`  
`B = "2"`  
`A2 = "Y"`  
この例ではすべてのパースが完了した後でオーバライドが扱われる前、BitBakeは${B}を2に置き換え、AをA2に拡張する。
AはXとなりA2はYとなる。
### 3.2.3. Examples
`OVERRIDES = ""`  
`A = "Z"`  
`A_foo = "X"`  
この例は無条件にかつ直ちに追加される変数AをZに設定し、A_fooはXが設定される。なぜならOVERRIDEはまだ適用されていないからである。  
`OVERRIDES = "foo"`  
`A = "Z"`  
`A_foo = "X"`  
OVERRIDEを適用すると、違いが生じる。fooがOVERRIDEのリストに存在する場合、'A_foo="X"'はOVERRIDE条件fooが示された場合、Aに"X"を代入と解釈され、AはXとなる。

## 3.3. Sharing Functionality
  BitBakeはmetadataがincludeファイル(.inc)やクラスファイル(.bbclass)を用いてのデータ共有を許容している。複数のレシピで共有したいタスクや共通機能を定義したい場合、.bbclassファイルを定義し、inheritディレクティブでレシピがclassを継承するようにレシピを書く。このセクションではBitBakeがレシピ間で機能を共有する仕組み、特にinclude,inherit,INHERIT,requireディレクティブについて説明する。
### 3.3.1. Locating Include and Class Files
  BitBakeはincludeやclassファイルの探索のためにBBPATHを使用する。BitBakeにincludeファイルやclassファイルを見つけさせるため、それらはclassesサブディレクトリに存在する必要がある。
`meta/classes`や`meta-poky/classes`等
### 3.3.2. inherit Directive
  レシピやclassを書いたら、inheritディレクティブでクラスの機能を継承できる。inheritはレシピ(bb)またはクラスファイル(.bbclass)にしか適用できない。inheritはなんのクラスの機能がレシピによって要求されるかを示している。例えば以下の例はautotoolsの機能をレシピから使用することが可能になることを示している。
`inherit autotools`  
このケースではBitBakeはclasses/autotools.bbclassを探しだす。
### 3.3.3. include Directive
  BitBakeはincludeディレクティブを理解する。このディレクティブによりBitBakeが指定されたどのようなファイルでもパースする。includeディレクティブは相対パスを指定されない限り、Makeと同等である。相対パスを指定した場合、BitBakeはBBPATH直下を探索し最初に見つかったファイルをincludeする。includeディレクティブはエラーを出力しないのでrequireを使用したほうが良い！！！
### 3.3.4. require Directive
  includeディレクティブのパースエラー検出版。
### 3.3.5. INHERIT Configuration Directive
  .confファイルからbbclassを継承するためには大文字のINHERITを使用する。
## 3.4. Functions
  BitBakeは以下のタイプのFunctionをサポートする。
- Shell Function  
- BitBake Style Python Function
- Python Function
- Anonymous Python Function.
### 3.4.1. Shell Functions
  シェルスクリプトで書かれた関数である。関数・タスク・関数タスク両方として動作する。shellのプログラミングルールに従うひつようがある。スクリプトは/bin/shによって実行される。
```
	some_function () {
		echo "Hello World"
	}
```
### 3.4.2. BitBake Style Python Functions
  Pythonで記述され、BitBakeにより実行またはbb.build.exec_func()にて他のPython functionとして実行される。
```
	python some_python_function () {
		d.setVar("TEXT", "Hello World")
		print d.getVar("TEXT", True)
	}
```
### 3.4.3. Python Functions
他のPython codeに実行されるPythonで書かれたコード
```
	def get_depends(d):
		if d.getVar('SOMECONDITION', True):
			return "dependencywithcond"
		else:
			return "dependency"
	SOMECONDITION = "1"
	DEPENDS = "${@get_depends(d)}"
```  
Python Functionは  
- 引数をモテる
- BitBakeのデータストアが使えない。パラメタとして渡す必要がある。
### 3.4.4. Anonymous Python Functions
```
	python __anonymous() {
		if d.getVar('SCMEVAR', True) == 'value':
			d.setVar('ANOTHERVAR', 'value2')
	}
```  
anonymousはオプショナル↓   
```
	python () {
		if d.getVar('SCMEVAR', True) == 'value':
			d.setVar('ANOTHERVAR', 'value2')
	}
```
  このPython関数は他のPython関数とは異なり解析時に実行される。
d.getVarやd.setVarで変数を解析時に読みだしたり切り替えることができる。  
bitbakeの文法に依存しない。

### 3.4.5. Flexible Inheritance for Class Functions
  未読です

## 3.5. Tasks
### 3.5.1. Promoting a Function to a Task
### 3.5.2. Deleting a Task
### 3.5.3. Passing Information Into the Build Task Environment
## 3.6. Variable Flags
## 3.7. Events
## 3.8. Variants - Class Extension Mechanism
## 3.9. Dependencies
### 3.9.1. Dependencies Internal to the .bb File
### 3.9.2. Build Dependencies
### 3.9.3. Runtime Dependencies
### 3.9.4. Recursive Dependencies
### 3.9.5. Inter-Task Dependencies
## 3.10. Accessing Datastore Variables Using Python
## 3.11. Task Checksums and Setscene
