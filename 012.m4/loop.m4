#一番最後のパラメタだけ標準出力
define(`loop', `
	ifelse($#, 1, 
		$1, 
		`loop(shift($@))
	')
')
loop(a,b,c,d,e,f,g,h,i,j,k)
