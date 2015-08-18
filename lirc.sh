#!/bin/sh
INPUT="linux-input.h"
cat <<EOF
begin remote
	name linux-input-layer
	bits 32
	begin codes
EOF
awk "
	/_MAX/			{ next };
	/KEY_RESERVED/		{ next };
	/#define (KEY|BTN)_/	{ gsub(/KEY_/,\"\",\$2);
				  printf(\"\t\t%-20s 0x%04x\n\",
					 \$2,0x10000+strtonum(\$3)) } 
" < $INPUT
cat <<EOF
	end codes
end remote
EOF
