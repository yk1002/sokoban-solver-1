#pragma once

#include <string>

#include "sokoban_level.h"

namespace sokoban {

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

Level StringToLevel(std::string level_string);
std::string LevelToString(const Level& level);

}  // namespace sokoban
