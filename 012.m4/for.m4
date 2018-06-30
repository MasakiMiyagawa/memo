define(`for',`pushdef(`$1', `$2')_for(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_for',`$4`'ifelse($1, `$3', ,`define(`$1', incr($1))_for(`$1', `$2', `$3', `$4')')')
