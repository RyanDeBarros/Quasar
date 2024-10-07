# Overview

Quasar asset files use the following format:

	1. First line is asset type, ex.: 'shader'
	2. Lines with the format '#:value'

## settings

	* 1: QuasarSettings::VERTEX_COUNT
	* 2: QuasarSettings::INDEX_COUNT
	* 3: QuasarSettings::TEXTURES_COUNT

## shader

	* 1: vertex filepath, relative to project root
	* 2: fragment filepath, relative to project root
	* 3: attribute lengths, for example '1|2|1|2|4|4'
