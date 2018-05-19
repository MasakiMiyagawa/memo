# openssl
## httpsクライアントを作る

'''
$sudo aptitude install libssl-dev
'''

## 乱数生成処理の差し替え
https-client.c , alternate-rand-method.c を参照。

'''
gcc -g -Werror https-client.c alternate-rand-method.c -o https-client -lssl -lcrypto -lstdc++
'''

