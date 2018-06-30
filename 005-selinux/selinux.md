# SELinuxに関するメモ書き
## TODO
- neverallowがModulePolicyで使用できないのか確認
- permisiveがModulePolicyで使用できるか確認

## AV (Access Vectorルールについて)
### 参考
- http://hetarena.com/archives/1535
### neverallow ルール
このルールは他の３つとは別格で、ルールのコンパイル時に許さないルールを規定する。
allow/auditallow ルールで許可されていても neverallow に引っかかったらコンパイル時にエラーとなる。
### 現在のAVを確認する
現在設定されている AV ルールを表示するには sesearch を使用

```
root@qemux86:~# sesearch --allow 
```

### neverallowはbase ポリシーじゃないと動作しない？
https://selinuxproject.org/page/AVCRules#auditallow
にしめされている
portconについてもmoduleポリシーでは動作しないことが示されている。※portconはcorenetworkでのみ定義されていてcorenetworkはmodules.conf上BasePolicyである。
https://selinuxproject.org/page/NetworkStatements#portcon
system/authlogin.teにはneverallowステートメントが使用されている。authlogin.teはBase Policyでなければならない。
一方でmodules.confは以下のようになっている。

```
# Layer: system
# Module: authlogin
#
# Common policy for authentication and user login.
# 
authlogin = module
```
これはリファレンスポリシーにおいてModulePolicyでもneverallowが使用していることを示している。
公式ドキュメントの誤記？
試しにneverallow+allowの矛盾が生じる以下のポリシーを作成してみる。
```
neverallow unconfined_t user_home_t:file read;
allow unconfined_t user_home_t:file read;
```
モジュールとしてのコンパイルは普通に通る
baseポリシとしてコンパイル時はエラーになるのか未確認
```
$ make HEADERDIR=~/refpolicy-minimum/2.20170204-r0/sysroot-destdir/usr/share/selinux/minimum/include 
Compiling ubuntu selinuxtest module
selinuxtest.te:30: Warning: domain_auto_trans() has been deprecated, please use domain_auto_transition_pattern() instead.
selinuxtest.te:31: Warning: domain_auto_trans() has been deprecated, please use domain_auto_transition_pattern() instead.
selinuxtest.te:32: Warning: domain_auto_trans() has been deprecated, please use domain_auto_transition_pattern() instead.
selinuxtest.te:33: Warning: domain_auto_trans() has been deprecated, please use domain_auto_transition_pattern() instead.
/usr/bin/checkmodule:  loading policy configuration from tmp/selinuxtest.tmp
/usr/bin/checkmodule:  policy configuration loaded
/usr/bin/checkmodule:  writing binary representation (version 17) to tmp/selinuxtest.mod
Creating ubuntu selinuxtest.pp policy package
rm tmp/selinuxtest.mod.fc tmp/selinuxtest.mod

```
semodule -i 時にチェックはできる。
```
root@qemux86:~# semodule -r selinuxtest
libsemanage.semanage_direct_remove_key: Unable to remove module selinuxtest at priority 400. (No such file or directory).
semodule:  Failed!
root@qemux86:~# semodule -i selinuxtest.pp
neverallow check failed at /var/lib/selinux/minimum/tmp/modules/400/selinuxtest/cil:30
  (neverallow unconfined_t user_home_t (file (read)))
    <root>
    allow at /var/lib/selinux/minimum/tmp/modules/100/base/cil:4108
      (allow files_unconfined_type file_type (file (ioctl read write create getattr setattr lock relabelfrom relabelto append unlink link rename execute swapon quotaon mounton execute_no_trans open audit_access)))
    <root>
    allow at /var/lib/selinux/minimum/tmp/modules/400/selinuxtest/cil:31
      (allow unconfined_t user_home_t (file (read)))
    <root>
    allow at /var/lib/selinux/minimum/tmp/modules/100/unconfined/cil:246
      (allow unconfined_t user_home_t (file (ioctl read write create getattr setattr lock append unlink link rename open)))

Failed to generate binary
semodule:  Failed!
root@qemux86:~# tail /etc/selinux/semanage.conf 
# version is necessary.
policy-version = 30

# By default, semanage will generate policies for the SELinux target.
# To build policies for Xen, uncomment the following line.
#target-platform = xen

# Don't check the entire policy hierarchy when inserting / expanding a policy
# module.  This results in a significant speed-up in policy loading.
expand-check=1 <== ★これがチェックされていないとインストールできる
```
### neverallow-allowの矛盾はbaseとしてビルドするときは検出される
modules.confを見るとdomain.teはbaseモジュールとしてビルドされるように見える。
domain.teにはneverallow文があるので以下のように矛盾するようにし、bitbake
```
# Transitions only allowed from domains to other domains
neverallow domain ~domain:process { transition dyntransition };
allow domain ~domain:process { transition dyntransition };
```
do compileでエラーになることを確認
ログ配下
```
DEBUG: Executing shell function do_compile
NOTE: make NAME=minimum TYPE=mcs DISTRO=redhat UBAC=n UNK_PERMS=allow DIRECT_INITRC=n SYSTEMD=y MONOLITHIC=n CUSTOM_BUILDOPT= QUIET=y MLS_SENS=0 MLS_CATS=1024 MCS_CATS=1024 tc_usrbindir=/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin OUTPUT_POLICY=31 CC=gcc  CFLAGS=-isystem/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/include -O2 -pipe PYTHON=/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin/python-native/python conf
Creating policy.xml
Updating policy/booleans.conf and policy/modules.conf
NOTE: make NAME=minimum TYPE=mcs DISTRO=redhat UBAC=n UNK_PERMS=allow DIRECT_INITRC=n SYSTEMD=y MONOLITHIC=n CUSTOM_BUILDOPT= QUIET=y MLS_SENS=0 MLS_CATS=1024 MCS_CATS=1024 tc_usrbindir=/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin OUTPUT_POLICY=31 CC=gcc  CFLAGS=-isystem/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/include -O2 -pipe PYTHON=/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin/python-native/python policy
Creating minimum base module base.conf
Compiling minimum base module
policy/modules/kernel/domain.te:21:ERROR '~ not allowed in this type of rule' at token ';' on line 9103:
allow domain ~domain:process { transition dyntransition };
neverallow domain ~domain:process { transition dyntransition };
/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin/checkmodule:  error(s) encountered while parsing configuration
/home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/recipe-sysroot-native/usr/bin/checkmodule:  loading policy configuration from base.conf
Rules.modular:98: recipe for target 'tmp/base.mod' failed
make: *** [tmp/base.mod] Error 1
ERROR: oe_runmake failed
WARNING: exit code 1 from a shell command.
ERROR: Function failed: do_compile (log file is located at /home/miyagawa/poky/selinux-qemu/tmp/work/qemux86-poky-linux/refpolicy-minimum/2.20170204-r0/temp/log.do_compile.25474)
```

### base policyとmodule policyの切り分け方は？
module.confに書かれた内容でbase moduleとしてコンパイルされるのか、policy moduleとしてコンパイルされるのかわかる。

#### BasePolicy
```
# Layer: kernel
# Module: corenetwork
# Required in base
#
# Policy controlling access to network objects
# 
corenetwork = base
```

#### ModulePolicy
```
# Layer: system
# Module: userdomain
#
# Policy for user domains
# 
userdomain = module
```

## permissive ドメインについて
ドメインをpermissiveにする方法は以下の2つがある
### permissiveステートメントを使う
Module Policyでも使用可能
ifステートメントの中では使えない。
permissiveを付与するppを作れば、enforceモードから部分的にpermissiveに切り替えることもできそう。
```
# This is the simple statement that would allow permissive mode
# to be set on the httpd_t domain, however this statement is
# generally built into a loadable policy module so that the
# permissive mode can be easily removed by removing the module.
# 
permissive httpd_t
```


### semanageを使う
permissive付与

```
root@qemux86:~# semanage permissive -a unconfined_t
```

permissive剥奪

```
root@qemux86:~# semanage permissive -d unconfined_t

```

## Android公式 SELinux
- https://source.android.com/security/selinux/
- https://source.android.com/security/selinux/concepts
- https://source.android.com/security/selinux/implement
- https://source.android.com/security/selinux/customize
- https://source.android.com/security/selinux/validate
- https://source.android.com/security/selinux/device-policy




