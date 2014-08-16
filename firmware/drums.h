/*
 * drums.h
 *
 */

#ifndef DRUMS_H_
#define DRUMS_H_

#define TRUE 1
#define FALSE 0




/* Drum layout:
+---+---+---+
| 1 | 2 | 3 |
|-----------|
|   | o |   |
|-----------|
|   |   |   |
+-----------+
*/

// Here we define the LEFTMOST bounds for each drum
// in the horizontal plane.

#define DRUM_1 0
#define DRUM_2 0
#define DRUM_3 0
#define DRUMS_END 0

typedef unsigned char byte;

#define SWAP(x,y) swap = x; x = y; y = swap

byte swap;

void airdrums_stick();
void airdrums_receiver();

#endif /* DRUMS_H_ */
