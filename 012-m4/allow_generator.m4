define(`_both_directive',`
	allow domain class:$1 $2;
	neverallow domain class:$1 ~{$2};
	
	ifelse($#, 2, `
		define(`object_type', object_type `$1')
	',`
		ifdef(`object_type', `
			define(`object_type', object_type `$1')
			_both_directive(shift(shift($@)))
		',`
			define(`object_type', `$1')
			_both_directive(shift(shift($@)))
		')
	')
')

define(`both_directive', `
	define(`domain', `$1')
	define(`class', `$2')
	_both_directive(shift(shift($@)))
	neverallow domain class:~{object_type} *;
	undefine(`object_type') 
')
