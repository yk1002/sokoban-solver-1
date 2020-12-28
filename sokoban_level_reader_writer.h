#pragma once

#include <string>

#include "sokoban_level.h"

namespace sokoban {

// The following was taken from http://sokobano.de/wiki/index.php?title=Sok_format
//
// ::::::::::::::::::::::::::: Board ::::::::::::::::::::::::::
// :: Legend.................:      :.................Legend ::
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// :: Wall...................: #  # :...................Wall ::
// :: Pusher.................: p  @ :.................Pusher ::
// :: Pusher on goal square..: P  + :..Pusher on goal square ::
// :: Box....................: b  $ :....................Box ::
// :: Box on goal square.....: B  * :.....Box on goal square ::
// :: Goal square............: .  . :............Goal square ::
// :: Floor..................:      :..................Floor ::
// :: Floor..................: -  _ :..................Floor ::

// Create a Level from a string that represents a Sokoban level.
Level StringToLevel(std::string level_string);

// Converts a Sokoban level into its string notation.
std::string LevelToString(const Level& level);

}  // namespace sokoban
