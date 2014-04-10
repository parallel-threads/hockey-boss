<?php
/**
  @file
  @brief This file defines the API for creating and
    manipulating the Stat content types used by the
    Hockey Boss site.
*/

// ----- I. Content Types -----

define ('hockey_boss_stat_PPS_TYPE', 'hockey_boss_stat_pps');
define ('hockey_boss_stat_PGS_TYPE', 'hockey_boss_stat_pgs');

function hockey_boss_stat_pgs_type () {
  module_load_include ('inc', 'hockey_boss');
  return array (
    'name'              => 'Player Game Stat',
    'description'       => 'Represents player game statistics.',
    'node_type'         => hockey_boss_stat_PGS_TYPE,
    'subject_node_type' => hockey_boss_PLAYER_PROFILE_TYPE,
  );
}

function hockey_boss_stat_pps_type () {
  module_load_include ('inc', 'hockey_boss');
  return array (
    'name'              => 'Player Performance Stat',
    'description'       => 'Represents player performance statistics.',
    'node_type'         => hockey_boss_stat_PPS_TYPE,
    'subject_node_type' => hockey_boss_PLAYER_PROFILE_TYPE
  );
}

/**
  @brief Returns an array listing the stat types
    defined by this module.
  @return (array) the stat types.
*/
function hockey_boss_stat_get_stat_types () {
  module_load_include ('inc', 'hockey_boss');

  return array (
    hockey_boss_stat_pgs_type (),
    hockey_boss_stat_pps_type ()
  );
}

/**
  @brief Implements hook_stat_create_<node type>_stat_type_node_type ().
  @details
    Adds fields to the Player Game Stats node type.
*/
function hockey_boss_stat_stat_create_hockey_boss_stat_pgs_stat_type_node_type ($stat_type_node_type) {
  module_load_include ('inc', 'hockey_boss');
  simple_field_create_entity_reference ($stat_type_node_type, $stat_type_node_type->type . '_game', t ('Game'), array (hockey_boss_GAME_TYPE), 'node', 1);
}

/**
  @brief Accepts a Player Game Stat node and returns its game node id.
  @param $node (object) the stat node.
  @return (int) the game node id.
*/
function hockey_boss_stat_pgs_get_game ($node) {
  return $node->{$node->type . '_game'}['und'][0]['target_id'];
}

/**
  @brief Accepts a Player Game Stat node and a game
    node id and sets the stat node to point to the
    given game node.
  @param $node (object) the stat node.
  @param $game_nid (int) the game node id.
*/
function hockey_boss_stat_pgs_set_game ($node, $game_nid) {
  $node->{$node->type . '_game'}['und'][0]['target_id'] = $game_nid;
}

/// Creates the stat types defined by this module.
function hockey_boss_stat_create_stat_types () {
  module_load_include ('inc', 'stat');
  stat_create_stat_types (hockey_boss_stat_get_stat_types ());
}

/// Deletes the stat types defined by this module.
function hockey_boss_stat_delete_stat_types () {
  module_load_include ('inc', 'stat');
  stat_delete_stat_types (hockey_boss_stat_get_stat_types ());
}
