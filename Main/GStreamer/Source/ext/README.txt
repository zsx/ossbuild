On VS libnice compilation requieres to rename 2 file:
stun/debug.c => stun/debug-stun.c
stun/usages/turn.c => stun/usages/turn-stun.c
2 other file in the solution already exists with the names 'debug.c' and 'turn.c', 
and these ones need to be renamed so the generated object is not overwriteen

stun/rand.c needs to include <wincrypt.h> as WIN32_LEAN_AND_MEAN makes windows.h 
not to include this file