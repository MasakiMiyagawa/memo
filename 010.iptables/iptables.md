# TODO
+ レシピ作成
+ systemd enable Yoctoでのやり方を調査
+ ipv6をどうするか検討

# Yoctoでiptables入のimageを作る
## パッケージをイメージに追加する

```
IMAGE_INSTALL_append = " iptables"
```

をlocal.confに追加。

## カーネルモジュール(xt_tcpudp, xt_state)がインストールされるようにレシピ変更
### 変更内容

RRECOMMENDSに"kernel-module-xt-state"と"kernel-module-xt-tcpudp"を追加する。
RRECOMMENDSはRDEPENDSよりゆるい設定。
パッケージが存在しなければ、インストールされない。

```
miyagawa@miyagawa-dynabook-Satellite-B551-E:~/poky/meta$ git diff
diff --git a/meta/recipes-extended/iptables/iptables_1.6.1.bb b/meta/recipes-extended/iptables/iptables_1.6.1.bb
index b37c55a..4dea684 100644
--- a/meta/recipes-extended/iptables/iptables_1.6.1.bb
+++ b/meta/recipes-extended/iptables/iptables_1.6.1.bb
@@ -8,6 +8,8 @@ LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263\
                     file://iptables/iptables.c;beginline=13;endline=25;md5=c5cffd09974558cf27d0f763df2a12dc"
 
 RRECOMMENDS_${PN} = "kernel-module-x-tables \
+                     kernel-module-xt-state \
+                     kernel-module-xt-tcpudp \
                      kernel-module-ip-tables \
                      kernel-module-iptable-filter \
                      kernel-module-iptable-nat \
```

### xt_tcpudp
"-m tcp"を使うために必要。このモジュールが/lib/modulesにインストールされ、
depmodされていないと、SSHDポートを開ける設定(-m tcp -p tcp --dport 22)にて
エラー"iptables: No chain/target/match by that name."が出力される。
depmodされていれば、iptables実行時にiptablesからxt_tcpudpがinsmod(modprobe)
される。

### xt_state
"-m state"を使うために必要。このモジュールが/lib/modulesにインストールされ、
depmodされていないと、stateがESTABLISHED,RELATEDであるパケット(こちらから
要求を出した応答パケット)を通す設定ができない。
エラー"iptables: No chain/target/match by that name."が出力される。
depmodされていれば、iptables実行時にiptablesからxt_stateがinsmod(modprobe)
される。

# 設定反映の実験

## ホワイトリストの設定
※ ホワイトリストとは：明示的に許可されない通信はDROP。
以下のようにsample.rulesを書く

```
*filter
:INPUT DROP [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
COMMIT
```

この状態だとpingに対する応答もない。

## ping応答を返す設定を追加する。

```
*filter
:INPUT DROP [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A INPUT -p icmp -j ACCEPT
COMMIT
```

pingが通るようになる

```
miyagawa@miyagawa-dynabook-Satellite-B551-E:~$ ping 192.168.7.2
PING 192.168.7.2 (192.168.7.2) 56(84) bytes of data.
64 bytes from 192.168.7.2: icmp_seq=104 ttl=64 time=0.894 ms
64 bytes from 192.168.7.2: icmp_seq=105 ttl=64 time=0.733 ms
```

## stateがESTABLISHED,RELATEDのパケット(こちらからの要求への応答)を許可する

```
*filter
:INPUT DROP [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
# ACCEPT ping
-A INPUT -p icmp -j ACCEPT
# ACCEPT ESTABLISHED connections
-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
```

## SSHログインできるように設定追加

```
*filter
:INPUT DROP [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
# ACCEPT ping
-A INPUT -p icmp -j ACCEPT
# ACCEPT ESTABLISHED connections
-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
# ACCEPT ssh
-A INPUT -m state --state NEW -m tcp -p tcp --dport 22 -j ACCEPT
COMMIT
```

SSHログインできるようになる。

```
master/etc/iptables$ ssh root@192.168.7.2
The authenticity of host '192.168.7.2 (192.168.7.2)' can't be established.
RSA key fingerprint is SHA256:/v6Zu3pPf1mIq+PsXfV2XRi+Vm7mcjleT9Ox6iaY00Q.
Are you sure you want to continue connecting (yes/no)? 
```

## その他ループバックデバイスからの入力を許可

```
*filter
:INPUT DROP [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
             
# For loopback interface.        
-A INPUT -i lo -j ACCEPT        
-A INPUT -d 127.0.0.1 -j ACCEPT                        
                                                       
# ACCEPT ping                                                   
-A INPUT -p icmp -j ACCEPT                                      
                                                                
# ACCEPT ESTABLISHED connections                                
-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
                                                                
# ACCEPT ssh                                                    
-A INPUT -m state --state NEW -m tcp -p tcp --dport 22 -j ACCEPT
      
COMMIT
```

## 設定を反映する

```
iptables-restore < sample.rules
```

 


