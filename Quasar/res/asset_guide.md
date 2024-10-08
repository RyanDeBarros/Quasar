# Overview

Quasar asset files use the following format:

	1. First line is asset type, ex.: 'shader'
	2. Lines with the format 'name:value'
	3. Possible values are:
		* string
		* integral
		* floating point
		* array (separate by '|')

## settings

	* 1: VERTEX_COUNT
	* 2: INDEX_COUNT
	* 3: TEXTURES_COUNT

## shader

	* 1: VERT - vertex filepath, relative to project root
	* 2: FRAG - fragment filepath, relative to project root
	* 3: ATTRIBS - attribute lengths, for example '1|2|1|2|4|4'
