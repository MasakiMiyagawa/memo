# meta-selinuxについて
## 参考
[SElinuxのレシピを書く](http://wankomagic.hatenablog.com/entry/2013/05/02/012203)
[SElinux FAQ Fedora](https://docs-old.fedoraproject.org/en-US/Fedora/13/html/SELinux_FAQ/#faq-entry-local.te)

## 1. メモ 

### TOOD


### Idea:アプリケーションがアクセスするオブジェクトを知る方法
- アプリケーションを何にもアクセスを許さないドメインに突っ込む
- Permissiveモードで動かす。
- audit2allowでteファイルを作る
- teファイルを読み解き、何にアクセスしたかったのかを知る。

## 2. core-image-selinux-minimal/core-image-selinuxのビルド

### 2.1 meta-selinuxをgit cloneしておく

### 2.2 bblaysers.confの変更

```
BBLAYERS ?= " \
  /home/miyagawa/poky/meta \
  /home/miyagawa/poky/meta-poky \
  /home/miyagawa/poky/meta-yocto-bsp \
  /home/miyagawa/poky/meta-openembedded/meta-oe \
  /home/miyagawa/poky/meta-openembedded/meta-python \
  /home/miyagawa/poky/meta-selinux \
  "
```

### 2.3 local.confの変更

PREFERRED_PROVIDER_virtual/refpolicy ?= "refpolicy-targeted"
PREFERRED_VERSION_refpolicy-targeted = "git"
DISTRO_FEATURES_append = " acl xattr pam selinux"

### 2.4 core-image-selinux-minimalでビルドした場合 auto relabelは動作しない

The 'core-image-selinux-minimal' does not automatically relabel the system.
So you must boot using the parameters "selinux=1 enforcing=0", and then
manually perform the setup.  Running 'fixfiles -f -F relabel' is available
in this configuration.

`runqemu qemuarm64 bootparams="selinux=1 enforcing=0"` 

## 3. audit2allowを使ってとりあえずすべてのアクセスが通るようにしてみる

### 3-1. core-image-selinuxのビルド

### 3-2. qemu起動

1回目の起動はrelabelが行われるので必ずpermissiveで起動する模様。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`

2回目の起動(単なる確認本来不要)

loginのドメインがexecできないので、
これをやるとloginできなくなる。qemuの場合プロセスをkillして一旦止める。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`

3回目の起動
permissiveで起動する。audit2allowを使ってとりあえず出まくっているdeniedを
なんとかする。

`runqemu qemuarm64 bootparams="selinux=1 enforcing=0" nographic`

### 3-3. audit2allowでどうポリシーを書けばとりあえず通るようになるか調べる

Permissiveモードで起動してrootでログインするととりあえずこれだけ監査ログがでる

```
root@qemuarm64:~# grep audit /var/log/kern.log 
Nov 23 03:27:51 qemuarm64 kernel: [    1.948259] audit: initializing netlink subsys (disabled)
Nov 23 03:27:51 qemuarm64 kernel: [    1.954168] audit: type=2000 audit(1.884:1): state=initialized audit_enabled=0 res=1
Nov 23 03:27:51 qemuarm64 kernel: [    3.586280] audit: type=1403 audit(1511407644.674:2): policy loaded auid=4294967295 ses=4294967295
Nov 23 03:27:51 qemuarm64 kernel: [   25.594336] audit: type=1400 audit(1511407666.466:3): avc:  denied  { dac_read_search } for  pid=300 comm="sm-notify" capability=2  scontext=system_u:system_r:rpcd_t:s0 tcontext=system_u:system_r:rpcd_t:s0 tclass=capability permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.802952] audit: type=1400 audit(1511407667.677:4): avc:  denied  { write } for  pid=313 comm="distccd" name="distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.805135] audit: type=1400 audit(1511407667.679:5): avc:  denied  { open } for  pid=313 comm="distccd" path="/run/distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   26.811319] audit: type=1400 audit(1511407667.686:6): avc:  denied  { getattr } for  pid=313 comm="distccd" path="/run/distcc.pid" dev="tmpfs" ino=8044 scontext=system_u:system_r:distccd_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   27.201058] audit: type=1400 audit(1511407668.279:7): avc:  denied  { search } for  pid=321 comm="modprobe" name="events" dev="tracefs" ino=93 scontext=system_u:system_r:kmod_t:s0 tcontext=system_u:object_r:tracefs_t:s0 tclass=dir permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.635010] audit: type=1400 audit(1511407671.711:8): avc:  denied  { read } for  pid=364 comm="avahi-daemon" name="resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.642064] audit: type=1400 audit(1511407671.716:9): avc:  denied  { open } for  pid=364 comm="avahi-daemon" path="/run/resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:51 qemuarm64 kernel: [   30.643457] audit: type=1400 audit(1511407671.719:10): avc:  denied  { getattr } for  pid=363 comm="avahi-daemon" path="/run/resolv.conf" dev="tmpfs" ino=7795 scontext=system_u:system_r:avahi_t:s0 tcontext=system_u:object_r:initrc_var_run_t:s0 tclass=file permissive=1
Nov 23 03:27:54 qemuarm64 kernel: [   33.368323] audit: type=1400 audit(1511407674.446:11): avc:  denied  { execute_no_trans } for  pid=410 comm="start_getty" path="/sbin/agetty" dev="vda" ino=10031 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:getty_exec_t:s0 tclass=file permissive=1
Nov 23 03:27:58 qemuarm64 kernel: [   37.560823] audit: type=1400 audit(1511407678.636:12): avc:  denied  { execute } for  pid=511 comm="login" name="bash.bash" dev="vda" ino=7708 scontext=system_u:system_r:local_login_t:s0-s0:c0.c1023 tcontext=system_u:object_r:bin_t:s0 tclass=file permissive=1
```

audit2allowでとりあえずのセキュリティコンテキストをどう設定すれば
このログが消えるか知ることができる。
ただし、これはdomain側の許可範囲(TE)を広げる設定のみ。
オブジェクトのFCを変更するのが正しい気もする。

```
root@qemuarm64:~# audit2allow -i /var/log/kern.log 

#============= avahi_t ==============
allow avahi_t initrc_var_run_t:file { getattr open read };

#============= distccd_t ==============
allow distccd_t initrc_var_run_t:file { getattr open write };

#============= getty_t ==============
allow getty_t getty_exec_t:file execute_no_trans;

#============= kmod_t ==============
allow kmod_t tracefs_t:dir search;

#============= local_login_t ==============

#!!!! This avc can be allowed using the boolean 'allow_polyinstantiation'
allow local_login_t bin_t:file execute;

#============= rpcd_t ==============
allow rpcd_t self:capability dac_read_search;
```

### 3-4. audit2allowでppファイルとteファイルを作る

audit2allowでppファイルとteファイルを作ることができる。

```
root@qemuarm64:~# cat /var/log/kern.log | audit2allow -M test
To make this policy package active, execute:  
semodule -i test.pp  
```

TEファイルは以下のような感じ。

```
root@qemuarm64:~# cat test.te 

module test 1.0;

require {
	type kmod_t;
	type distccd_t;
	type tracefs_t;
	type local_login_t;
	type avahi_t;
	type getty_t;
	type initrc_var_run_t;
	type bin_t;
	type rpcd_t;
	type getty_exec_t;
	class capability dac_read_search;
	class file { execute execute_no_trans getattr open read write };
	class dir search;
}

#============= avahi_t ==============
allow avahi_t initrc_var_run_t:file { getattr open read };

#============= distccd_t ==============
allow distccd_t initrc_var_run_t:file { getattr open write };

#============= getty_t ==============
allow getty_t getty_exec_t:file execute_no_trans;

#============= kmod_t ==============
allow kmod_t tracefs_t:dir search;

#============= local_login_t ==============

#!!!! This avc can be allowed using the boolean 'allow_polyinstantiation'
allow local_login_t bin_t:file execute;

#============= rpcd_t ==============
allow rpcd_t self:capability dac_read_search;
```

### 3-5. ppファイルをロード(恒久的)

ポリシーをロードする

```
#semodule -i test.pp
#dmesg
[ 1603.578150] SELinux: 32768 avtab hash slots, 111836 rules.
[ 1603.872804] SELinux: 32768 avtab hash slots, 111836 rules.
[ 1604.034342] SELinux:  6 users, 14 roles, 4587 types, 250 bools, 1 sens, 1024 cats
[ 1604.034933] SELinux:  127 classes, 111836 rules
[ 1614.437105] audit: type=1403 audit(1511409255.514:13): policy loaded auid=4294967295 ses=4294967295
```

### 3-6. reboot

今度はenforceでもログインできる

`runqemu qemuarm64 bootparams="selinux=1 enforcing=1" nographic`

### 3-7. ppファイルをアンロード(恒久的)

拡張子なしになるので注意
`#semodule -r test`

## 4. core-image-minimalをselinux化する

以下meta-selinux/SELinux-FAQより
```
2.2 - How can I add SELinux to my custom images?

If you only want to add SELinux to your custom image, then you should perform
the following steps:

   1. Add meta-selinux path to BUILDDIR/conf/bblayers.conf file

   2. Add DISTRO_FEATURES_append=" pam selinux" in BUILDDIR/conf/local.conf
      file.

   3. Add packagegroup-core-selinux to your custom image.
      For example, if core-image-custom.bb is your building image file, then
      you should add packagegroup-core-selinux to IMAGE_INSTALL in
      core-image-custom.bb.

   4. Build your custom image in build directory

       $ bitbake core-image-custom
```

### 4.1 bblayers.confの設定

```
BBLAYERS ?= " \
  /home/miyagawa/poky/meta \
  /home/miyagawa/poky/meta-poky \
  /home/miyagawa/poky/meta-yocto-bsp \
  /home/miyagawa/poky/meta-openembedded/meta-oe \
  /home/miyagawa/poky/meta-openembedded/meta-python \
  /home/miyagawa/poky/meta-selinux \
  "
```

### 4.2 local.confの追加設定

acl, xattrはなぜいらない？
書き忘れ？

```
STRO_FEATURES_append = " pam selinux"
PREFERRED_PROVIDER_virtual/refpolicy ?= "refpolicy-targeted"
PREFERRED_VERSION_refpolicy-targeted = "git"
```

### 4.3 core-image-minimal.bbの修正

```
$ cat meta/recipes-core/images/core-image-minimal.bb 
SUMMARY = "A small image just capable of allowing a device to boot."

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_INSTALL += "packagegroup-core-selinux"
IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"
IMAGE_ROOTFS_EXTRA_SPACE_append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "" ,d)}"
```

### 4.4 bitbake

`$bitbake core-image-minimal`

## 5.teファイルから.ppを作る
### 5.1 core-image-selinuxにログイン

package-groupを見るとcore-image-selinuxで作成されたイメージはポリシーを記述
するためのツールが用意されているっぽい。

### 5.2 以下のコマンドでppファイルができる

二段階コンパイルになっている
hoge.teからhoge.modを生成し、hoge.modからhoge.ppが生成される。

```
$checkmodule -M -m -o test.mod test.te 
$semodule_package -o test.pp -m test.mod
semodule -i test.pp 
```

[参考](https://docs-old.fedoraproject.org/en-US/Fedora/13/html/SELinux_FAQ/#faq-entry-local.te)

## 6.サンプルアプリケーションのポリシを作る

### 6.1 折角なのでYoctoで作る

1. レイヤの追加

`scripts/yocto-layer create my-selinux-test`

2. ここにファイルの読み書きをするプログラムを書く

```
#include <stdio.h>
#include <unistd.h>
#define TESTFILE "TESTFILE"
int main(int argc, char **argv)
{
	FILE *fp;
	char filepath[256] = {0};
	int err = 0;
	char buf[256] = {0};
	
	if (argc < 2) {
		printf("You need to specify test dir path.");
		return -1;
	}

	sprintf(filepath, "%s/%s", argv[1], TESTFILE);

	printf("Test 1 : Open test file write mode\n");
	fp = fopen((const char *)filepath, "w");
	if (!fp) {
		perror("fopen(w):");
		return -1;
	}
	
	printf("Test 2 : write test\n");
	err = fprintf(fp, "Hello! this statement is written by %s", __FILE__);
	if (err < 0) {
		perror("fprintf(w):");
		return -1;
	}
	
	fclose(fp);
	fp = NULL;
	
	printf("Test 3 : Open test file read mode\n");
	fp = fopen((const char *)filepath, "r");
	if (!fp) {
		perror("fopen(r):");
		return -1;
	}

	printf("Test 4 : read test\n");
	err = fread(buf, 1, sizeof(buf), fp);
	if (err < 0) {
		perror("fread:");
		return -1;
	}
	printf("Test 5 : checking file \"%s\"\n", buf);
	
	fclose(fp);
	fp = NULL;
	
	printf("Test 6 : unlink test\n");
	err = unlink((const char *)filepath);
	if (err < 0) {
		perror("unlink:");
		return -1;
	}

	return 0;
}
```

3.IMAGE_INSTALL_appendにレシピを加える @local.conf

`IMAGE_INSTALL_append = " selinux-test-app"`

4.SELinux化されたcore-image-minimalで実行する

helloworld(Yocto-layerのデフォルトから変更していない名前)はgetty_tドメイン
で動作している。getty_tドメインにはuser_home_dir_tタイプのディレクトリ
に対するwrite, addname, remove_nameの許可がなく、user_home_dir_tタイプの
ファイルへのcreate,getattr,read,unlinkの許可もない。
```
# dmesg | grep helloworld
[   82.837532] audit: type=1400 audit(1512189754.985:27): avc:  denied  { write } for  pid=300 comm="helloworld" name="root" dev="vda" ino=3764 scontext=system_u:system_r:getty_t:s0 tcontext=unconfined_u:object_r:user_home_dir_t:s0 tclass=dir permissive=1
[   82.846498] audit: type=1400 audit(1512189754.993:28): avc:  denied  { add_name } for  pid=300 comm="helloworld" name="TESTFILE" scontext=system_u:system_r:getty_t:s0 tcontext=unconfined_u:object_r:user_home_dir_t:s0 tclass=dir permissive=1
[   82.852553] audit: type=1400 audit(1512189754.995:29): avc:  denied  { create } for  pid=300 comm="helloworld" name="TESTFILE" scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:user_home_dir_t:s0 tclass=file permissive=1
[   82.857479] audit: type=1400 audit(1512189755.005:30): avc:  denied  { write open } for  pid=300 comm="helloworld" path="/home/root/TESTFILE" dev="vda" ino=5978 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:user_home_dir_t:s0 tclass=file permissive=1
[   82.863437] audit: type=1400 audit(1512189755.010:31): avc:  denied  { getattr } for  pid=300 comm="helloworld" path="/home/root/TESTFILE" dev="vda" ino=5978 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:user_home_dir_t:s0 tclass=file permissive=1
[   82.869519] audit: type=1400 audit(1512189755.012:32): avc:  denied  { read } for  pid=300 comm="helloworld" name="TESTFILE" dev="vda" ino=5978 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:user_home_dir_t:s0 tclass=file permissive=1
[   82.876532] audit: type=1400 audit(1512189755.017:33): avc:  denied  { remove_name } for  pid=300 comm="helloworld" name="TESTFILE" dev="vda" ino=5978 scontext=system_u:system_r:getty_t:s0 tcontext=unconfined_u:object_r:user_home_dir_t:s0 tclass=dir permissive=1
[   82.879523] audit: type=1400 audit(1512189755.018:34): avc:  denied  { unlink } for  pid=300 comm="helloworld" name="TESTFILE" dev="vda" ino=5978 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:user_home_dir_t:s0 tclass=file permissive=1
```

### 6.2 とりあえず、getty_tのセキュリティポリシを変更しないで新たなドメインを定義

1.teファイル作成

```
module helloworld 1.0;

require {
        attribute domain;
        attribute file_type;
        attribute exec_type;
}

type helloworld_t, domain;
type helloworld_exec_t, file_type, exec_type;
type helloworld_data_t, file_type;
```

module行はcheckmodule -m オプションで必要らしい。
書かないと以下のエラーがでる。

```
~ # checkmodule -M -m -o helloworld.mod ./helloworld.te 
checkmodule:  loading policy configuration from ./helloworld.te
./helloworld.te:1:ERROR 'Building a policy module, but no module specification found.
' at token 'type' on line 1:


checkmodule:  error(s) encountered while parsing configuration
```

ちなみに-mオプションは以下の意味があるらしい。
ポリシーモジュールとは/etc/selinux下にあるポリシーファイルの差分の
ようなものか。

```
#checkmodule -h
  -m         build a policy module instead of a base module
```

require行も必要になる。書かないと以下のエラーになる。
```
~ # checkmodule -M -m -o helloworld.mod ./helloworld.te 
checkmodule:  loading policy configuration from ./helloworld.te
./helloworld.te:8:ERROR 'attribute file_type is not declared' at token ';' on line 8:
type helloworld_exec_t, file_type, exec_type;
type helloworld_t, domain;
checkmodule:  error(s) encountered while parsing configuration
```

.modができたら.ppファイルを作る

`# semodule_package -o helloworld.pp -m helloworld.mod`

.ppファイルをインストールする。policy.30のサイズが変化する

```
# ls -l /etc/selinux/targeted/policy/policy.30 
-rw-r--r-- 1 root root 2427381 Dec  2 05:59 /etc/selinux/targeted/policy/policy.30
# semodule -i helloworld.pp 
# ls -l /etc/selinux/targeted/policy/policy.30 
-rw-r--r-- 1 root root 2427367 Dec  2 06:01 /etc/selinux/targeted/policy/policy.30
```

この状態でPermissiveモードで動作させるとどうなるか -> authログに変化なし。
現状のセキュリティポリシの問題は
1. helloworldのバイナリファイルのfcがbin_tのまま
2. domain遷移の定義が書かれていない
ため、helloworldはhelloworld_tで動作しない

ならどうするか。

まずは2から対応する。
新しいteファイルは以下。
domain_auto_trans()マクロを使いたいがcheck_moduleコマンドでコンパイルするとき
はm4マクロが働かないのでとりあえずdomain_auto_trans()の定義をrefpolicyから参照
し、そのまま書いた。domain_auto_trans()の中身はclass processを使用しているので
requireに追加しなければならない。(アクセスベクタまで書かなきゃならんのが面倒)

```
module helloworld 1.0;
require {
	attribute domain;
	attribute file_type;
	attribute exec_type;
	role system_r;
	type getty_t;
	type unlabeled_t;
	class process
	{
        	fork
        	transition
        	sigchld # commonly granted from child to parent
        	sigkill # cannot be caught or ignored
        	sigstop # cannot be caught or ignored
        	signull # for kill(pid, 0)
        	signal  # all other signals
        	ptrace
        	getsched
        	setsched
        	getsession
        	getpgid
        	setpgid
        	getcap
        	setcap
        	share
        	getattr
        	setexec
        	setfscreate
        	noatsecure
        	siginh
        	setrlimit
        	rlimitinh
        	dyntransition
        	setcurrent
        	execmem
        	execstack
        	execheap
        	setkeycreate
        	setsockcreate
       		getrlimit
	};
}


type helloworld_t, domain;
type helloworld_exec_t, file_type, exec_type;
type helloworld_data_t, file_type;

role system_r types helloworld_t;

#domain_auto_trans(getty_t, helloworld_exec_t, helloworld_t)
type_transition getty_t helloworld_exec_t:process helloworld_t;

```
これでドメイン遷移するためのteはできた。
このteファイルはhelloworld_exec_tのタイプを持つ実行ファイルをドメイン遷移
にするということを以下の文で表している。

`type_transition getty_t helloworld_exec_t:process helloworld_t;`

なおtype_transitionでは遷移元ドメインをgetty_tにしている。
これはauthログでbashで実行したhelloworldがgetty_tで動作していたから。
さらに、以下はrole system_rにhelloworld_tのドメインに入る権利を与えている。

`role system_r types helloworld_t;`

次に考えるべきなのはhelloworldにhelloworld_exec_tタイプを与えることだが
以下で出来た。これは一時的なものらしい。

```
~ # chcon -t helloworld_exec_t /usr/bin/helloworld 
~ # ls -Z /usr/bin/helloworld 
system_u:object_r:helloworld_exec_t:s0 /usr/bin/helloworld
```

この状態でPermissiveモードで動かした時のログは以下。
authログは増えているけどhelloworldのドメインが変わっている。

```
[   52.942856] audit: type=1400 audit(1512201288.197:18): avc:  denied  { getattr } for  pid=298 comm="chcon" path="/usr/bin/helloworld" dev="vda" ino=534 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:helloworld_exec_t:s0 tclass=file permissive=1
[  100.883631] audit: type=1400 audit(1512201336.135:20): avc:  denied  { getattr } for  pid=295 comm="sh" path="/home/root/helloworld.pp" dev="vda" ino=6001 scontext=system_u:system_r:getty_t:s0 tcontext=unconfined_u:object_r:user_home_t:s0 tclass=file permissive=1
[  100.890497] audit: type=1400 audit(1512201336.142:21): avc:  denied  { getattr } for  pid=295 comm="sh" path="/home/root/helloworldsandbox" dev="vda" ino=5996 scontext=system_u:system_r:getty_t:s0 tcontext=unconfined_u:object_r:user_home_t:s0 tclass=dir permissive=1
[  111.201166] audit: type=1400 audit(1512201346.453:22): avc:  denied  { execute } for  pid=302 comm="sh" name="helloworld" dev="vda" ino=534 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:helloworld_exec_t:s0 tclass=file permissive=1
[  111.209325] audit: type=1400 audit(1512201346.456:23): avc:  denied  { read open } for  pid=302 comm="sh" path="/usr/bin/helloworld" dev="vda" ino=534 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:object_r:helloworld_exec_t:s0 tclass=file permissive=1
[  111.210814] audit: type=1400 audit(1512201346.457:24): avc:  denied  { transition } for  pid=302 comm="sh" path="/usr/bin/helloworld" dev="vda" ino=534 scontext=system_u:system_r:getty_t:s0 tcontext=system_u:system_r:helloworld_t:s0 tclass=process permissive=1
[  111.219230] audit: type=1400 audit(1512201346.465:25): avc:  denied  { entrypoint } for  pid=302 comm="sh" path="/usr/bin/helloworld" dev="vda" ino=534 scontext=system_u:system_r:helloworld_t:s0 tcontext=system_u:object_r:helloworld_exec_t:s0 tclass=file permissive=1
[  111.225112] audit: type=1400 audit(1512201346.476:26): avc:  denied  { use } for  pid=302 comm="helloworld" path="/dev/ttyS0" dev="devtmpfs" ino=8994 scontext=system_u:system_r:helloworld_t:s0 tcontext=system_u:system_r:getty_t:s0 tclass=fd permissive=1
[  111.231976] audit: type=1400 audit(1512201346.476:27): avc:  denied  { read write } for  pid=302 comm="helloworld" path="/dev/ttyS0" dev="devtmpfs" ino=8994 scontext=system_u:system_r:helloworld_t:s0 tcontext=system_u:object_r:tty_device_t:s0 tclass=chr_file permissive=1
[  111.238066] audit: type=1400 audit(1512201346.477:28): avc:  denied  { rlimitinh } for  pid=302 comm="helloworld" scontext=system_u:system_r:getty_t:s0 tcontext=system_u:system_r:helloworld_t:s0 tclass=process permissive=1
[  111.249823] audit: type=1400 audit(1512201346.477:29): avc:  denied  { siginh } for  pid=302 comm="helloworld" scontext=system_u:system_r:getty_t:s0 tcontext=system_u:system_r:helloworld_t:s0 tclass=process permissive=1
[  111.254870] audit: type=1400 audit(1512201346.482:30): avc:  denied  { read execute } for  pid=302 comm="helloworld" path="/usr/bin/helloworld" dev="vda" ino=534 scontext=system_u:system_r:helloworld_t:s0 tcontext=system_u:object_r:helloworld_exec_t:s0 tclass=file permissive=1
[  111.259862] audit: type=1400 audit(1512201346.483:31): avc:  denied  { noatsecure } for  pid=302 comm="helloworld" scontext=system_u:system_r:getty_t:s0 tcontext=system_u:system_r:helloworld_t:s0 tclass=process permissive=1
```


#.Yoctoでppファイルを作る

teファイル、ifファイル、icファイルをSRC_URIに追加して
do_compileでpolicy/modules/appsにコピーされるようにする。
do_fetch->do_unpackで${WORKDIR}に移動しているので。

```
--- a/recipes-security/refpolicy/refpolicy_common.inc
+++ b/recipes-security/refpolicy/refpolicy_common.inc
@@ -12,6 +12,9 @@ RPROVIDES_${PN} += "refpolicy"
 SRC_URI += "file://customizable_types \
             file://setrans-mls.conf \
             file://setrans-mcs.conf \
+            file://selinuxtest.te \
+            file://selinuxtest.if \
+            file://selinuxtest.fc \
           "
 
 S = "${WORKDIR}/refpolicy"
@@ -83,6 +86,7 @@ python __anonymous () {
 }
 
 do_compile() {
+       cp ${WORKDIR}/selinuxtest.* ${S}/policy/modules/apps
        oe_runmake conf
        oe_runmake policy
 }
```

今度はteファイルではマクロが使える。以下のような感じ。

```
policy_module(selinuxtest, 1.2.0)

require {
        attribute domain;
        attribute file_type;
        attribute exec_type;
        role system_r;
        type getty_t;
}

type selinux-test_t, domain;
type selinux-test_exec_t, file_type, exec_type;

role system_r types selinux-test_t;
domain_auto_trans(getty_t, selinux-test_exec_t, selinux-test_t)
```

policy_moduleに書かれた名前はteファイルの拡張子をなくした名前と一致していない
とだめ。

ifファイルはこのモジュールが提供するm4マクロを定義するが、<xml>を書いておかない
とこれまたコンパイルエラーになるみたい。

```
# <summary>SElinux Test application</summary>
```

fcファイルは、/usr/bin/selinux-testがselinux-test_exec_tタイプになるようにして
いる。このタイプはteファイルでドメイン切り替えのエントリポイントとなっている

```
usr/bin/selinux-test   --      gen_context(system_u:object_r:selinux-test_exec_t,s0)
```

以上でcore-image-minimalをビルドすると、selinux-test_tドメインで動作する
auditログが得られる。あとは、このログがなくなるようにいろいろするだけ。

#.備忘録
SELinux無効中に作られたファイルにはセキュリティコンテキストが付与されない

```
~ # ls -Z .
                                   ? helloworld.fc
                                   ? helloworld.te
system_u:object_r:user_home_dir_t:s0 helloworld.txt
                                   ? helloworldsandbox
```
