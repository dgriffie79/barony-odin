// scrolls.odin -- Odin mirror of scrolls.hpp.
package main

NUMLABELS                :: 25
NUM_SCROLL_MAIL_OPTIONS  :: 23

// static char scroll_label[NUMLABELS][512] - the 25 label strings.
// In Odin the [512]u8 inner array gives the same 512-byte stride.
scroll_label :: [NUMLABELS][512]u8
