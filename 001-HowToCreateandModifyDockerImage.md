# [Ubuntu 14.04ltsのDockerイメージの作成]
# 1. debootstrapのインストール
`#aptitude install debootstrap`

# 2. schrootのインストール(不要?)
`#aptitude install schroot`

# 3. debootstarpの実行
`#debootstrap --variant=buildd --arch i386 trusty <Target Dir> http://archive.ubuntu.com/ubuntu/`
*1:trustyはhttp://archive.ubuntu.com/ubuntu/distsにあるディレクトリ名
*2:trusty = 14.04lts

# 4. 古いdockerのuninstall
`#aptitude remove docker`

# 5. Docker.ceのインストール
`wget -qO- https://get.docker.com/ | sh`
*2:https://get.docker.com/からスクリプトをDLしてshで実行。(aptの実行)

# 6. Dockerの設定
`#usermod -aG docker miyagawa`
dockerグループを作ってsudo権限を持つユーザに付与
`#/etc/init.d/docker restart`

# 7. Dockerイメージの生成(sha256が出力される)
`#tar -c . | docker import - ubuntu-14.04lts-i386
sha256:f84346eea18d468a859427e2c95f9bf58453114545f7eb5d5e56bc31dd9a5baf`
	
# 8. Dockerイメージを使ったコマンド実行
`$docker run ubuntu-14.04lts-i386 ls`

# 9. コンテナへのログイン
`$docker run -it ubuntu-14.04lts-i386 /bin/bash`
コンテナはイメージをインスタンス化したものと考える。
ここでrootfsを操作しただけではイメージに反映されない。
docker psでコンテナ一覧が参照できる。

# 10. chrootで色々やったあとイメージを作る
## 10.1.chroot
`#chroot <TargetDir> /bin/bash`
## 10.2.いろいろ設定
例：useradd miyagawa; apt-get install aptitude 等
## 10.3.imageの生成
`#tar -c . | docker import - ubuntu-14.04lts-i386`

# 11. docker commitで更新されたimageの複製
## 参照
<http://qiita.com/mats116/items/712575dc50513dfdf0a2>
## 11.1. コンテナの起動
`#docker run -i -t -d --name="default" <image名> /bin/bash
#docker attach`
## 11.2. コンテナの中でファイルシステムの更新
## 11.3. ctl+p ctl+qでコンテナから抜ける
## 11.4. コミット
`#docker commit default <Image名>:<Tag名>`
## 11.5. コンテナdefaultの終了
`#docker stop default`
## 11.6. 新しいTag名のついたimageをコンテナ化
`#docker run -i -t -d --name="newtag" <image名>:<Tag名> /bin/bash`
## 11.7. アタッチ
`#docker attach newtag`

## 12. isoをマウントしてのイメージ生成
    このやり方はダメだった。chrootのエラーになる。一旦諦め。
   12.1. インストールするisoイメージのマウント
 	#mount -t iso9660 -o ro,loop=/dev/loop0 \
 		../ubuntu-ja-14.04-desktop-i386.iso <マウント先パス>
 
   12.2. debootstrapの実行
	#debootstrap --arch i386 trusty <Target Dir> iso:<iso dir>
