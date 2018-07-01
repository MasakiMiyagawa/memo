define(`allow_section_begin', `
	ifdef(`class', `
		errprint(ERROR:Nesting of allow_section is not allowed.)
	')
	define(`class',`$1')
	define(`domain',`$2')
')

define(`add_allow', `
	allow domain class:$1 $2;
	neverallow domain class:$1 ~{$2};
	ifdef(`object_type', `
		define(`object_type', object_type `$1')
	',`
		define(`object_type', `$1')
	')
')

define(`allow_section_end', `
	ifdef(`object_type', `
		neverallow domain class:~{object_type} *;
		undefine(`object_type') 
	',`
		neverallow domain class:* *;
	')
	
	ifelse($1, class, `
		undefine(`class')
	',`
		errprint(ERROR:allow_section parameter does not match.)
	')
	
	ifelse($2, domain, `
		undefine(`domain')
	',`
		errprint(ERROR:allow_section parameter does not match.)
	')
')
