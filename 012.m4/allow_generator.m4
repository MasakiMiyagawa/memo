define(`vectoradd',`
	ifelse($#, 1, `
		define(`ac_vector', ac_vector `$1')
	',`
		ifdef(`ac_vector', `
			define(`ac_vector', ac_vector `$1')
			typeadd(shift($@))
		',`
			define(`ac_vector', `$1')
			typeadd(shift($@))
		')
	')
	
')

define(`typeadd',`
	ifdef(`object_type', `
		define(`object_type', object_type `$1')
		vectoradd(shift($@))
	',`
		define(`object_type', `$1')
		vectoradd(shift($@))
	')
')

define(`allow2', `
	define(`class', `$1')
	typeadd(shift($@))
	neverallow class:^{object_type} * 
')

allow2(file,hoge_t,write,hage_t,read)

