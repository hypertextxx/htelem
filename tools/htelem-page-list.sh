#!/bin/sh
objdump -hw "$1" | sed -nE 's/[0-9]+ (ht:[[:graph:]]+)[[:print:]]+/\1/p' | cut -c2-

