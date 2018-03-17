valgrind + LD_PRELOAD実験メモ

$ valgrind --version
valgrind-3.11.0

1.preload.c
rand()関数をpreloadする。preloadされたrand()関数はreturnせずに無限ループに入る。
コンパイル方法
$gcc -shared -fPIC -o preload.so preload.c

2.test.c
rand()関数呼び出し側。malloc()で100Byte確保し開放しない。
コンパイル方法
$gcc test.c

3.run.sh
test.cをビルドしたa.outをシンプルに実行する。valgrindの--trace-children機能の
確認用。

4.run_preload.sh
preload.soをLD_PRELOADした状態でa.outを実行するスクリプト

5.実験1:LD_PRELOADを設定してvalgrindを実行する

$ LD_PRELOAD=./preload.so valgrind --tool=memcheck ./a.out 
==5340== Memcheck, a memory error detector
==5340== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
==5340== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
==5340== Command: ./a.out
==5340== 
Busy loop is started.

上記を見ると、LD_PRELOADは効いているように見える。(実装上向かないので却下)
SIGTERMを送るとvalgrindは適切にログされる。

6.実験2:valgrindが実行するプロセスでLD_PRELOADする。
この場合、子プロセスに対する実行結果は見えない。

$ valgrind --tool=memcheck ./run_preload.sh 
==5588== Memcheck, a memory error detector
==5588== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
==5588== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
==5588== Command: ./run_preload.sh
==5588== 
Busy loop is started.

7.実験3:シェルスクリプトでラップして更に--trace-childrenを使う。
子プロセスまでログが取得できる。

$ valgrind --tool=memcheck --leak-check=full --trace-children=yes ./run_preload.sh 
==5746== Memcheck, a memory error detector
==5746== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
==5746== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
==5746== Command: ./run_preload.sh
==5746== 
==5747== Memcheck, a memory error detector
==5747== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
==5747== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
==5747== Command: ./a.out
==5747== 
Busy loop is started.

psで見るとプロセスは以下のようになる(5617がbash、5618がa.out)

$ pgrep memcheck
5746
5747

両方にSIG_TERMを送る
$ pgrep memcheck | xargs kill

Busy loop is started.
==5747== 
==5747== Process terminating with default action of signal 15 (SIGTERM)
==5746== 
==5746== Process terminating with default action of signal 15 (SIGTERM)
==5747==    at 0x51082F0: __nanosleep_nocancel (syscall-template.S:84)
==5746==    at 0x4F0612A: wait4 (syscall-template.S:84)
==5747==    by 0x5139D93: usleep (usleep.c:32)
==5746==    by 0x11BDB1: ??? (in /bin/dash)
==5747==    by 0x4E3A6F9: rand (in /home/miyagawa/work/github/memo/008.valgrind/preload.so)
==5746==    by 0x11BB11: ??? (in /bin/dash)
==5746==    by 0x117570: ??? (in /bin/dash)
==5747==    by 0x4005D3: main (in /home/miyagawa/work/github/memo/008.valgrind/a.out)
==5746==    by 0x116BB4: ??? (in /bin/dash)
==5746==    by 0x1221C3: ??? (in /bin/dash)
==5746==    by 0x116A30: ??? (in /bin/dash)
==5746==    by 0x4E5A82F: (below main) (libc-start.c:291)
==5747== 
==5747== HEAP SUMMARY:
==5747==     in use at exit: 100 bytes in 1 blocks
==5747==   total heap usage: 2 allocs, 1 frees, 1,124 bytes allocated
==5747== 
==5746== 
==5746== HEAP SUMMARY:
==5746==     in use at exit: 2,799 bytes in 84 blocks
==5746==   total heap usage: 84 allocs, 0 frees, 2,799 bytes allocated
==5746== 
==5747== 100 bytes in 1 blocks are definitely lost in loss record 1 of 1
==5747==    at 0x4C2DB8F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==5747==    by 0x4005CE: main (in /home/miyagawa/work/github/memo/008.valgrind/a.out)
==5747== 
==5747== LEAK SUMMARY:
==5747==    definitely lost: 100 bytes in 1 blocks
==5747==    indirectly lost: 0 bytes in 0 blocks
==5747==      possibly lost: 0 bytes in 0 blocks
==5747==    still reachable: 0 bytes in 0 blocks
==5747==         suppressed: 0 bytes in 0 blocks
==5747== 
==5747== For counts of detected and suppressed errors, rerun with: -v
==5747== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
==5746== LEAK SUMMARY:
==5746==    definitely lost: 0 bytes in 0 blocks
==5746==    indirectly lost: 0 bytes in 0 blocks
==5746==      possibly lost: 0 bytes in 0 blocks
==5746==    still reachable: 2,799 bytes in 84 blocks
==5746==         suppressed: 0 bytes in 0 blocks
==5746== Reachable blocks (those to which a pointer was found) are not shown.
==5746== To see them, rerun with: --leak-check=full --show-leak-kinds=all
==5746== 
==5746== For counts of detected and suppressed errors, rerun with: -v
==5746== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
Terminated

100バイトのメモリリーク(5747)が検出される。

親->子の順でSIGTERMしてもログは出る
子->親の順だと、親が子を待っている状態なので子が終わった瞬間親も終了して
ログを出す。

最終的には以下のコマンドでメモリリークはxmlファイルに吐き出せそう。
ただし末端の子プロセスが対象になるので注意が必要かも。
$ valgrind --tool=memcheck --leak-check=full --xml=yes --xml-file=./log.xml --trace-children=yes ./run_preload.sh

--log-file=./log.txtでログファイル出力できるけどxmlと共存出来ないっぽい。

5. ValgrindMe()
gcc valgrind_me_main.c valgrind_me.c
時プロセスをvalgrindの管理下で再起動するサンプル
./a.outでプロセスはvalgrindで動作する。
LD_PRELOAD=./preload.so ./a.outでLD_PRELOADも有効
7の問題のため、valgrindをrootで実行するようにしている。
rootで実行してもCan't execute setuid/setgid/setcapが出る

6. force_ld_preload.c
LD_PRELOADを強制してexecするプログラムも試してみたがこれはダメ。
valgrindで実行した時は常にLD_PRELOADがセットされているから。
単にLD_PRELOADを追加したいだけなら5の解決方法のほうが良いか。
LD_PRELOAD is already set as /usr/lib/valgrind/vgpreload_core-amd64-linux.so:/usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so

7. valgrindのcapability
permission.cは/root/testのファイルをオープンするだけのプログラム。
#setcap cap_dac_override+ep a.out
をするとpermission deniedにならない。
この状態で
valgrind ./a.outだとpermission deniedとなってしまう。
Can't execute setuid/setgid/setcap executable: ./a.out
valgrindはsetcapしたバイナリを正しく実行できない模様。
